#ifndef _GRID_H_
#define _GRID_H_

#include "BroadPhase.h"

#include "Polygon.h"
#include "GlobalVariables.h"
#include "World.h"
#include <math.h>
#include <unordered_set>
#include "Renderer.h"

namespace std {

    template <>
    struct hash<Vec2Int>
    {
        std::size_t operator()(const Vec2Int& k) const
        {
            using std::size_t;
            using std::hash;
            using std::string;

            // Compute individual hash values for first,
            // second and third and combine them using XOR
            // and bit shifting:

            return (std::size_t) ((3463 * k.x) ^ (3467 * k.y));
        }
    };

}

static Vec2Int Vec2ToGridCellPos(const Vec2& pos, float gridCellSize)
{
    return
    {
        int(floor(pos.x / gridCellSize)),
        int(floor(pos.y / gridCellSize))
    };
}

struct AABBInt
{
    Vec2Int pMin;
    Vec2Int pMax;

    AABBInt(AABB aabb, float gridCellSize) : 
        pMin(Vec2ToGridCellPos(aabb.pMin, gridCellSize)),
        pMax(Vec2ToGridCellPos(aabb.pMax, gridCellSize))

    {

    }
};

#include <Polygon.h>
#include "Conversion.h"

class CGrid : public IBroadPhase
{
public:
    float gridCellSize = 2;
    std::unordered_map<SPolygonPair, int> pairsToCheck; // pairs to check / the number of times it is in the same cell

    using TGrid = std::unordered_map<Vec2Int, std::vector<CPolygonPtr>>;
    using TCell = TGrid::iterator;
    TGrid grid;

    struct PolygonAdditionalGeometryData
    {
        AABB baseAABB;
        AABBInt lastAABBInt;
    };
    std::unordered_map<CPolygonPtr, PolygonAdditionalGeometryData> polygonsAdditionalGeometryData;

    CGrid(float gridCellSize) : gridCellSize(gridCellSize)
    {

    }

    virtual void GetCollidingPairsToCheck(std::vector<SPolygonPair>& pairsToCheck) override
    {
        if (gVars->bDebug)
            DisplayDebug();

        pairsToCheck.reserve(pairsToCheck.size());
        for (const auto& it : this->pairsToCheck)
        {
            pairsToCheck.emplace_back(it.first);
        }
    }

    void clear()
    {
        grid.clear();
        pairsToCheck.clear();
    }

    virtual void OnObjectAdded(const CPolygonPtr& newPolygon) override
    {
        AABB baseAABB = PolyToBaseAABB(newPolygon);
        AABB transformedAABB = PolyToTransformedAABB(newPolygon, baseAABB);
        AABBInt aabbInt = AABBInt(transformedAABB, gridCellSize);
        for (int gridX = aabbInt.pMin.x; gridX <= aabbInt.pMax.x; gridX++)
        {
            for (int gridY = aabbInt.pMin.y; gridY <= aabbInt.pMax.y; gridY++)
            {
                AddElementToCell(newPolygon, Vec2Int{ gridX, gridY });
            }
        }

        PolygonAdditionalGeometryData geometryData{ baseAABB, aabbInt };

        polygonsAdditionalGeometryData.emplace(newPolygon, geometryData);

        newPolygon->onTransformUpdatedCallback = ([this, newPolygon](const CPolygon& poly)
        {
	        OnObjectUpdated(newPolygon);
        });
    }

    virtual void OnObjectRemoved(const CPolygonPtr& removedPolygon) override
    {
        removedPolygon->onTransformUpdatedCallback = nullptr;
        auto geometryDataIt = polygonsAdditionalGeometryData.find(removedPolygon);
        const AABBInt& aabbInt = geometryDataIt->second.lastAABBInt;
        for (int gridX = aabbInt.pMin.x; gridX <= aabbInt.pMax.x; gridX++)
        {
            for (int gridY = aabbInt.pMin.y; gridY <= aabbInt.pMax.y; gridY++)
            {
                RemoveElementFromCell(removedPolygon, Vec2Int{ gridX, gridY });
            }
        }

        polygonsAdditionalGeometryData.erase(geometryDataIt);
    }

    std::pair<TCell, bool> GetOrInsertCell(const Vec2Int& gridLocation)
    {
        return grid.emplace(gridLocation, std::vector<CPolygonPtr>());;
    }

    inline static std::vector<CPolygonPtr>& GetCellPolygons(const TCell& cell)
    {
        return cell->second;
    }

    inline static Vec2Int GetCellPosition(const TCell& cell)
    {
        return cell->first;
    }

    void AddElementToCell(const CPolygonPtr& addedPolygon, const Vec2Int& gridLocation)
    {
        std::pair<TCell, bool> it = GetOrInsertCell(gridLocation);
        for (CPolygonPtr& otherPolygon : GetCellPolygons(it.first))
        {
            // Add pairs to check
            auto result = pairsToCheck.emplace(SPolygonPair(addedPolygon, otherPolygon), 1);
            if (!result.second)
            {
                result.first->second++;
            }
        }

        // Add polygon in cell
        GetCellPolygons(it.first).emplace_back(addedPolygon);
    }

    void RemoveElementFromCell(const CPolygonPtr& removedPolygon, const Vec2Int& gridLocation)
    {
        TCell cell = grid.find(gridLocation);
        if (cell != grid.end())
        {
            std::vector<CPolygonPtr>& cellPolygons = GetCellPolygons(cell);

            // Remove polygon from cell
            auto removedPolygonIt = std::find(cellPolygons.begin(), cellPolygons.end(), removedPolygon);
            if (removedPolygonIt != cellPolygons.end())
                cellPolygons.erase(removedPolygonIt);

            // Remove pairs to check
            for (CPolygonPtr& otherPolygons : cellPolygons)
            {
                std::unordered_map<SPolygonPair, int>::iterator it2 = pairsToCheck.find(SPolygonPair(removedPolygon, otherPolygons));
                if (it2 != pairsToCheck.end())
                {
                    it2->second--;
                    if (it2->second == 0)
                    {
                        pairsToCheck.erase(it2);
                    }
                }
            }

            // Remove cell if empty
            if (cellPolygons.empty())
            {
                grid.erase(cell);
            }
        }
    }

    std::unordered_set<Vec2Int> toAdd;
    std::unordered_set<Vec2Int> toRemove;

    // Can be optimized
    void OnObjectUpdated(const CPolygonPtr& polygon) 
    {
        auto geometryDataIt = polygonsAdditionalGeometryData.find(polygon);

        AABB transformedAABB = PolyToTransformedAABB(polygon, geometryDataIt->second.baseAABB);
        AABBInt newAABBInt = AABBInt(transformedAABB, gridCellSize);

        const AABBInt& oldAABBInt = geometryDataIt->second.lastAABBInt;

        // TODO : better opti

        toAdd.clear();
        toRemove.clear();
        int reserved = (oldAABBInt.pMax.y - oldAABBInt.pMin.y) * (oldAABBInt.pMax.x - oldAABBInt.pMin.x);
        toAdd.reserve(reserved);
        toRemove.reserve(reserved);

        for (int gridX = oldAABBInt.pMin.x; gridX <= oldAABBInt.pMax.x; gridX++)
        {
            for (int gridY = oldAABBInt.pMin.y; gridY <= oldAABBInt.pMax.y; gridY++)
            {
                Vec2Int coords = Vec2Int{ gridX, gridY };
                toRemove.emplace(coords);
            }
        }

        for (int gridX = newAABBInt.pMin.x; gridX <= newAABBInt.pMax.x; gridX++)
        {
            for (int gridY = newAABBInt.pMin.y; gridY <= newAABBInt.pMax.y; gridY++)
            {
                Vec2Int coords = Vec2Int{ gridX, gridY };
                auto it = toRemove.find(coords);
                if (it != toRemove.end())
                {
                    toRemove.erase(it);
                }
                else
                {
                    toAdd.emplace(coords);
                }
                //toAdd.emplace(coords);
            }
        }


        for (Vec2Int cell : toRemove)
        {
            RemoveElementFromCell(polygon, cell);
        }

        for (Vec2Int cell : toAdd)
        {
            AddElementToCell(polygon, cell);
        }

        geometryDataIt->second.lastAABBInt = newAABBInt;
    }

    void UpdatePairsToCheck()
    {
        pairsToCheck.clear();

        std::vector<SPolygonPair> pairs;
        for (auto cell : grid)
        {
            CBroadPhaseBrut::AddCollidingPairsToCheck(cell.second, pairs);
        }

        for (const SPolygonPair& p : pairs)
        {
            pairsToCheck.emplace(p, 1);
        }
    }

#pragma region DEBUG
    void CheckPairs()
    {
        // check for same poly
        for (const std::pair<SPolygonPair, int>& it : pairsToCheck)
        {
            assert(it.first.GetpolyA() != it.first.GetpolyB());
        }

        while (FixDuplicates());
    }

    bool FixDuplicates()
    {
        for (const std::pair<const SPolygonPair, int>& it : pairsToCheck)
        {
            //for (const std::pair<SPolygonPair, int>& it2 : pairsToCheck)
            for (auto it3 = pairsToCheck.begin(); it3 != pairsToCheck.end(); it3++)
            {
                auto it2 = *it3;
                if (it == it2)
                    continue;

                if (it.first == it2.first)
                {
                    //std::cout << "ERROR 2" << std::endl;
                    pairsToCheck.erase(it3);
                    return true;
                }
            }
        }
        return false;
    }

    void DisplayDebug()
    {
        gVars->pRenderer->DisplayText("Grid");

        for (auto& pair : grid)
        {
            pair.first;
            float left = pair.first.x * gridCellSize;
            float right = pair.first.x * gridCellSize + gridCellSize;
            float bottom = pair.first.y * gridCellSize;
            float up = pair.first.y * gridCellSize + gridCellSize;
            gVars->pRenderer->DrawLine(Vec2(left, up), Vec2(right, up), 1,0,0);
            gVars->pRenderer->DrawLine(Vec2(left, bottom), Vec2(right, bottom), 1, 0, 0);
            gVars->pRenderer->DrawLine(Vec2(left, up), Vec2(left, bottom), 1, 0, 0);
            gVars->pRenderer->DrawLine(Vec2(right, up), Vec2(right, bottom), 1, 0, 0);
        }
    }
#pragma endregion
};

#endif