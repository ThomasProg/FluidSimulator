#pragma once

#ifndef _QUAD_TREE_
#define _QUAD_TREE_

#include "BroadPhase.h"

#include "Polygon.h"
#include "GlobalVariables.h"
#include "World.h"
#include "Conversion.h"
#include <cassert>
#include "Renderer.h"

struct PolygonWithQuadTreeData
{
	CPolygonPtr poly;
	MoveableAABB moveableAABB;
	class QuadTreeNode* node;
};

class QuadTreeNode
{
	struct Subdivision
	{
		std::unique_ptr<QuadTreeNode> node;
		AABB aabb;

		bool CanInsert(const PolygonWithQuadTreeData toPutIn) const
		{
			return aabb.CheckCollision(toPutIn.moveableAABB.GetmovedAABB());
		}

		QuadTreeNode* Insert(const PolygonWithQuadTreeData& newPolygon, QuadTreeNode& outer)
		{
			if (node == nullptr)
			{
				node = std::make_unique<QuadTreeNode>(aabb, outer.maxDepth - 1);
				node->parent = &outer;
			}

			return node->Insert(newPolygon);
		}
	} subdivisions[4];

	std::vector<const PolygonWithQuadTreeData*> elements;
	AABB borders;

	int maxDepth;
	QuadTreeNode* parent = nullptr;

	QuadTreeNode* InsertHere(const PolygonWithQuadTreeData& newPolygon)
	{
		elements.push_back(&newPolygon);

		return this;
	}

public:
	QuadTreeNode(const AABB& newBorders, int newMaxDepth) : borders(newBorders), maxDepth(newMaxDepth)
	{
		// Z are values we already have
		// Y are values we need to compute for the sub quads
		// X are other points vertices
		//
		// X---Y---Z
		// |   |   |
		// Y---Y---Y
		// |   |   |
		// Z---Y---X

		float middleX = borders.pMin.x + (borders.pMax.x - borders.pMin.x) * 0.5f;
		float middleY = borders.pMin.y + (borders.pMax.y - borders.pMin.y) * 0.5f;

		Vec2 middleLeft = Vec2(borders.pMin.x, middleY);
		Vec2 middleBottom = Vec2(middleX, borders.pMin.y);
		Vec2 middle = Vec2(middleX, middleY);
		Vec2 middleRight = Vec2(borders.pMax.x, middleY);
		Vec2 middleTop = Vec2(middleX, borders.pMax.y);

		subdivisions[0].aabb = AABB(middleLeft, middleTop);
		subdivisions[1].aabb = AABB(borders.pMin, middle);
		subdivisions[2].aabb = AABB(middleBottom, middleRight);
		subdivisions[3].aabb = AABB(middle, borders.pMax);
	}

	bool CanInsert(const PolygonWithQuadTreeData& toPutIn) const
	{
		return borders.CheckCollision(toPutIn.moveableAABB.GetmovedAABB());
	}

	GETTER(elements)
	GETTER(subdivisions)

	QuadTreeNode* Insert(const PolygonWithQuadTreeData& newPolygon)
	{
		if (maxDepth == 0)
		{
			return InsertHere(newPolygon);
		}

		Subdivision* collidingSubdivision = nullptr;

		for (auto& subDiv : subdivisions)
		{
			if (subDiv.CanInsert(newPolygon))
			{
				if (collidingSubdivision == nullptr)
					collidingSubdivision = &subDiv;
				else
				{
					// intersects with multiple subdivisions
					return InsertHere(newPolygon);
				}
			}
		}

		// intersects with only 1 subdivision
		if (collidingSubdivision != nullptr)
		{
			QuadTreeNode* node = collidingSubdivision->Insert(newPolygon, *this);
			assert(node != nullptr);
			return node;
		}
		
		return nullptr;
	}

	// Removes in this node (and not children)
	void RemoveHere(const PolygonWithQuadTreeData& toRemove)
	{
		elements.erase(std::remove(elements.begin(), elements.end(), &toRemove), elements.end());
		TryRemoveNode();
	}

	void TryRemoveNode()
	{
		if (parent == nullptr)
			return;

		if (elements.size() != 0)
			return;

		for (const Subdivision& subdiv : Getsubdivisions())
		{
			if (subdiv.node != nullptr)
				return;
		}

		QuadTreeNode* thisParent = parent;

		for (Subdivision& subdiv : parent->subdivisions)
		{
			if (subdiv.node.get() == this)
			{
				subdiv.node = nullptr;
			}
		}

		thisParent->TryRemoveNode();
	}
};

class CQuadTree : public IBroadPhase
{
	QuadTreeNode treeRoot = QuadTreeNode(AABB(Vec2(-40, -40), Vec2(40, 40)), 5);

	std::vector<std::unique_ptr<PolygonWithQuadTreeData>> polys;

public:
	void RemoveObjectFromTree(PolygonWithQuadTreeData& objectData)
	{
		if (objectData.node == nullptr)
			return;

		objectData.node->RemoveHere(objectData);
		objectData.node = nullptr;
	}

	void AddObjectToTree(PolygonWithQuadTreeData& objectData)
	{
		QuadTreeNode* node = treeRoot.Insert(objectData);
		objectData.node = node;
	}

	virtual void OnObjectAdded(const CPolygonPtr& newPolygon) override
	{
		std::unique_ptr<PolygonWithQuadTreeData> polyWithData = std::make_unique<PolygonWithQuadTreeData>();
		PolygonWithQuadTreeData* polyDataPtr = polyWithData.get();
		polyWithData->poly = newPolygon;
		polyWithData->poly->onTransformUpdatedCallback = [this, polyDataPtr](const CPolygon& poly)
		{
			RemoveObjectFromTree(*polyDataPtr);
			UpdateAABBTransformFromPolygon(polyDataPtr->moveableAABB, poly);
			AddObjectToTree(*polyDataPtr);

		};
		polyWithData->moveableAABB = PolyToMoveableAABB(newPolygon);
		AddObjectToTree(*polyWithData);

		assert(polyWithData->node != nullptr); // quad tree too small
		assert(polyWithData->poly != nullptr);

		polys.push_back(std::move(polyWithData));
	}

	virtual void OnObjectRemoved(const CPolygonPtr& polygon) override
	{
		//RemoveObjectFromTree(polygon);
		polygon->onTransformUpdatedCallback = nullptr;
	}

	void AddCollidingPairsChildren(const QuadTreeNode& node, const QuadTreeNode& subNode, std::vector<SPolygonPair>& pairsToCheck)
	{
		for (const PolygonWithQuadTreeData* polyWithData : node.Getelements())
		{
			for (const PolygonWithQuadTreeData* polyWithDataSub : subNode.Getelements())
			{
				pairsToCheck.push_back(SPolygonPair(polyWithData->poly, polyWithDataSub->poly));
			}
		}

		for (const auto& subdiv : subNode.Getsubdivisions())
		{
			if (subdiv.node != nullptr)
				AddCollidingPairsChildren(node, *subdiv.node, pairsToCheck);
		}
	}

	void AddCollidingPairs(const QuadTreeNode& node, std::vector<SPolygonPair>& pairsToCheck)
	{
		// Brute Force for those in the same node
		auto& polygonsToCheck = node.Getelements();
		pairsToCheck.reserve(polygonsToCheck.size() * polygonsToCheck.size()); // ideal sizee : n * log(n), but log long to compute
		size_t size = polygonsToCheck.size();
		for (size_t i = 0; i < size; ++i)
		{
			for (size_t j = i + 1; j < size; ++j)
			{
				pairsToCheck.push_back(SPolygonPair(polygonsToCheck[i]->poly, polygonsToCheck[j]->poly));
			}
		}

		for (const auto& subdiv : node.Getsubdivisions())
		{
			if (subdiv.node != nullptr)
				AddCollidingPairsChildren(node, *subdiv.node, pairsToCheck);
		}

		for (const auto& subdiv : node.Getsubdivisions())
		{
			if (subdiv.node != nullptr)
				AddCollidingPairs(*subdiv.node, pairsToCheck);
		}
	}

	virtual void GetCollidingPairsToCheck(std::vector<SPolygonPair>& pairsToCheck) override
	{
		AddCollidingPairs(treeRoot, pairsToCheck);

		if (gVars->bDebug)
		{
			gVars->pRenderer->DisplayText("QuadTree");
			DisplayDebugForNode(treeRoot, 1);
		}
	}

	void DisplayDebugForNode(const QuadTreeNode& treeNode, float r)
	{
		for (const auto& subDiv : treeNode.Getsubdivisions())
		{
			gVars->pRenderer->DrawAABB(subDiv.aabb, r, 0, 0);
			if (subDiv.node != nullptr)
				DisplayDebugForNode(*subDiv.node, Max(0.1, r-0.1));
		}
	}
};

#endif

