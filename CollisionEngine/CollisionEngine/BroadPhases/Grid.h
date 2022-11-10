#ifndef _GRID_H_
#define _GRID_H_

#include "BroadPhase.h"

#include "Polygon.h"
#include "GlobalVariables.h"
#include "World.h"
#include <math.h>
#include <unordered_set>

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

namespace std {

    template <>
    struct hash<SPolygonPair>
    {
        std::size_t operator()(const SPolygonPair& pair) const
        {
            using std::size_t;
            using std::hash;
            using std::string;

            // Compute individual hash values for first,
            // second and third and combine them using XOR
            // and bit shifting:

            size_t p1 = (size_t)pair.GetpolyA().get(); 
            size_t p2 = (size_t)pair.GetpolyB().get(); 

            return (3463 * p1) ^ (3467 * p2);
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

class CGrid : public IBroadPhase
{
public:
    float gridCellSize = 2;
    std::unordered_map<SPolygonPair, int> pairsToCheck; // pairs to check / the number of times it is in the same cell

    using TGrid = std::unordered_map<Vec2Int, std::vector<CPolygonPtr>>;
    using TCell = TGrid::iterator;
    TGrid grid;

    std::unordered_map<CPolygonPtr, AABBInt> previousAABB;

    CGrid(float gridCellSize) : gridCellSize(gridCellSize)
    {

    }

    virtual void GetCollidingPairsToCheck(std::vector<SPolygonPair>& pairsToCheck) override
    {
        pairsToCheck.reserve(pairsToCheck.size());
        for (auto it : this->pairsToCheck)
        {
            pairsToCheck.emplace_back(it.first);
        }
    }

    void clear()
    {
        grid.clear();
        pairsToCheck.clear();
    }

    virtual void OnObjectAdded(CPolygonPtr newPolygon) override
    {
        AABB aabb = newPolygon->transformedAABB;
        AABBInt aabbInt = AABBInt(aabb, gridCellSize);
        for (int gridX = aabbInt.pMin.x; gridX <= aabbInt.pMax.x; gridX++)
        {
            for (int gridY = aabbInt.pMin.y; gridY <= aabbInt.pMax.y; gridY++)
            {
                AddElementToCell(newPolygon, Vec2Int{ gridX, gridY });
            }
        }
        previousAABB.emplace(newPolygon, aabbInt);
    }

    virtual void OnObjectRemoved(CPolygonPtr removedPolygon) override
    {
        AABB aabb = removedPolygon->transformedAABB;
        AABBInt aabbInt = AABBInt(aabb, gridCellSize);
        for (int gridX = aabbInt.pMin.x; gridX <= aabbInt.pMax.x; gridX++)
        {
            for (int gridY = aabbInt.pMin.y; gridY <= aabbInt.pMax.y; gridY++)
            {
                RemoveElementFromCell(removedPolygon, Vec2Int{ gridX, gridY });
            }
        }

        previousAABB.erase(removedPolygon);
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

    // Can be optimized
    virtual void OnObjectUpdated(CPolygonPtr polygon) override
    {
        const AABB& newAABB = polygon->transformedAABB;
        AABBInt newAABBInt = AABBInt(newAABB, gridCellSize);

        auto previousAABBIt = previousAABB.find(polygon);
        const AABBInt& oldAABBInt = previousAABBIt->second;

        // TODO : better opti

        std::unordered_set<Vec2Int> toAdd;
        std::unordered_set<Vec2Int> toRemove;

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

        previousAABBIt->second = newAABBInt;
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
            if (it.first.GetpolyA() == it.first.GetpolyB())
            {
                std::cout << "ERROR" << std::endl;
            }
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
#pragma endregion
};

#endif