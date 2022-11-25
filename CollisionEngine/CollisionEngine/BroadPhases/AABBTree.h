#ifndef _AABB_TREE_H_
#define _AABB_TREE_H_

#include <cassert>
#include "BroadPhase.h"

#include "Polygon.h"
#include "GlobalVariables.h"
#include "World.h"

class PolygonWithAABB
{
	CPolygonPtr polygon;

	void OnPolygonSet()
	{
		moveableAABB = PolyToMoveableAABB(polygon);
	}

public:
	MoveableAABB moveableAABB;

	using Type = PolygonWithAABB;

	GETTER(polygon)
	SETTER(polygon, OnPolygonSet)

	PolygonWithAABB() = default;
	PolygonWithAABB(const CPolygonPtr& poly)
	{
		Setpolygon(poly);
	}

	static void Update(PolygonWithAABB& object)
	{
		UpdateAABBTransformFromPolygon(object.moveableAABB, *object.polygon);
	}

	static float ComputeCost(const PolygonWithAABB& object1)
	{
		return 2.f * object1.moveableAABB.GetmovedAABB().GetArea();
	}

	static float ComputeMergeCost(const PolygonWithAABB& object1, const PolygonWithAABB& object2)
	{
		AABB mergedAABB = object1.moveableAABB.GetmovedAABB().Merge(object2.moveableAABB.GetmovedAABB());
		return 2.f * mergedAABB.GetArea();
	}

	inline bool operator==(const PolygonWithAABB& rhs) const
	{
		return polygon == rhs.polygon;
	}
};

template<typename I_BOUNDING_VOLUME>
class BoundingVolumeHierarchyNode
{
public:
	typename I_BOUNDING_VOLUME::Type object;

	BoundingVolumeHierarchyNode* parent = nullptr;

	BoundingVolumeHierarchyNode* left = nullptr;
	BoundingVolumeHierarchyNode* right = nullptr;

	using TNodePtr = BoundingVolumeHierarchyNode<I_BOUNDING_VOLUME>*;

	inline BoundingVolumeHierarchyNode*& GetDirectChildRef(BoundingVolumeHierarchyNode* child)
	{
		return left == child ? left : right;
	}

	inline bool IsLeaf() const
	{
		return left == nullptr && right == nullptr; // right should already be null if left is null, but just checking
	}

	inline bool IsRoot() const
	{
		return parent == nullptr;
	}

	static TNodePtr CreateParentNode()
	{
		return new BoundingVolumeHierarchyNode<I_BOUNDING_VOLUME>();
	}

	void InsertLeaf(BoundingVolumeHierarchyNode* nodeToInsert, BoundingVolumeHierarchyNode*& rootRef)
	{
		assert(nodeToInsert->parent == nullptr);
		assert(nodeToInsert->left == nullptr);
		assert(nodeToInsert->right == nullptr);

		auto addDirectChild = [nodeToInsert, &rootRef](BoundingVolumeHierarchyNode* sibling)
		{
			TNodePtr newParent = CreateParentNode();

			newParent->parent = sibling->parent;
			newParent->object.moveableAABB.SetbaseAABB(nodeToInsert->object.moveableAABB.GetmovedAABB().Merge(sibling->object.moveableAABB.GetmovedAABB()));
			newParent->left = sibling;
			newParent->right = nodeToInsert;
			nodeToInsert->parent = newParent;


			if (sibling->parent == nullptr)
			{
				rootRef = newParent;
			}
			else
			{
				sibling->parent->GetDirectChildRef(sibling) = newParent;
			}
			//fixUpwardsTree(nodeToInsert->parent);

			sibling->parent = newParent;
		};

		if (!IsLeaf())
		{
			float newParentCost = I_BOUNDING_VOLUME::ComputeMergeCost(object, nodeToInsert->object);

			float minPushCost = newParentCost - I_BOUNDING_VOLUME::ComputeCost(object);

			auto computeChildCost = [nodeToInsert](BoundingVolumeHierarchyNode* child) -> float
			{
				if (child == nullptr)
					return 0.f;

				float cost;

				cost = I_BOUNDING_VOLUME::ComputeMergeCost(child->object, nodeToInsert->object);

				if (!child->IsLeaf())
				{
					cost -= I_BOUNDING_VOLUME::ComputeCost(child->object);
				}

				return cost;
			};

			float leftCost = computeChildCost(left);
			float rightCost = computeChildCost(right);

			if ((newParentCost < leftCost && newParentCost < rightCost) || (left == nullptr) || (right == nullptr))
			{
				addDirectChild(this);
				return;
			}

			if (leftCost < rightCost)
			{
				left->InsertLeaf(nodeToInsert, rootRef);
			}
			else
			{
				right->InsertLeaf(nodeToInsert, rootRef);
			}
		}
		else
		{
			addDirectChild(this);
		}
	}

	BoundingVolumeHierarchyNode*& GetSibling() 
	{
		assert(!IsRoot());
		return parent->left == this ? right : left;
	}

	void RemoveLeaf(BoundingVolumeHierarchyNode*& refToParent)
	{
		assert(!IsRoot());

		BoundingVolumeHierarchyNode* grandParent = parent->parent;
		BoundingVolumeHierarchyNode* sibling = GetSibling();

		//// TODO : Assert sibling?
		//if (grandParent == nullptr)
		//{
		//	root = sibling;
		//	sibling->parent = nullptr;
		//	destroy(parent);
		//}

		//else
		//{
		//	grandParent->GetDirectChildRef(parent) = sibling;
		//	sibling->parent = grandParent;

		//	destroy(parent);

		//	fixUpwardsTree(grandParent);
		//}

		refToParent = sibling;
		if (sibling != nullptr)
			sibling->parent = grandParent;

		//destroy(parent);
		parent = nullptr;
		left = nullptr;
		right = nullptr;

		//if (grandParent != nullptr)
		//	grandParent->fixUpwardsTree();
	}

	void UpdateLeaf(BoundingVolumeHierarchyNode* root)
	{
		assert(IsLeaf());

		if (this == root)
		{
			I_BOUNDING_VOLUME::Update(object);
			return;
		}

		RemoveNode();

		I_BOUNDING_VOLUME::Update(object);
		
		InsertNode(root);
	}

	BoundingVolumeHierarchyNode* GetRoot() const
	{
		BoundingVolumeHierarchyNode* root = this;
		while (root != root->parent != nullptr)
		{
			root = root->parent;
		}
		return root;
	}

	void UpdateLeaf()
	{
		UpdateLeaf(GetRoot());
	}
};

class CAABBTree : public IBroadPhase
{
	using TNode = BoundingVolumeHierarchyNode<PolygonWithAABB>;
	using TNodePtr = std::shared_ptr<TNode>;
	TNode* treeRoot;
	
	std::unordered_map<CPolygonPtr, TNodePtr> polyToNodes;

public:
	void AddCollidingShapes(TNode* testedLeaf, TNode* treeNode, std::vector<SPolygonPair>& pairsToCheck)
	{
		if (treeNode == nullptr || testedLeaf == treeNode)
			return;

		if (treeNode->object.moveableAABB.GetmovedAABB().CheckCollision(testedLeaf->object.moveableAABB.GetmovedAABB()))
		{
			if (treeNode->IsLeaf())
			{
				if (testedLeaf->object.Getpolygon() == nullptr || treeNode->object.Getpolygon() == nullptr)
				{
					return;
				}

				// To not add multiple times the same element
				if (testedLeaf->object.Getpolygon().get() < treeNode->object.Getpolygon().get())
				{
					pairsToCheck.push_back(SPolygonPair(testedLeaf->object.Getpolygon(), treeNode->object.Getpolygon()));
				}
			}
			else
			{
				AddCollidingShapes(testedLeaf, treeNode->left, pairsToCheck);
				AddCollidingShapes(testedLeaf, treeNode->right, pairsToCheck);
			}
		}
	}

	virtual void GetCollidingPairsToCheck(std::vector<SPolygonPair>& pairsToCheck) override
	{
		if (treeRoot == nullptr)
			return;

		for (const auto& [poly, leafNode] : polyToNodes)
		{
			AddCollidingShapes(leafNode.get(), treeRoot, pairsToCheck);
		}
	}

	void AddLeaf(TNode& node)
	{
		if (treeRoot == nullptr)
		{
			treeRoot = &node;
			return;
		}

		treeRoot->InsertLeaf(&node, treeRoot);
	}

	void RemoveLeaf(TNode& node)
	{
		assert(node.IsLeaf());

		if (&node == treeRoot || node.parent == nullptr)
		{
			//treeRoot = nullptr;
			assert(node.parent == nullptr);
			return;
		}

		//if (node.parent != nullptr)
		//	return;

		TNode* grandParent = node.parent->parent;
		if (grandParent == nullptr)
		{
			node.RemoveLeaf(treeRoot);
		}
		else
		{
			TNode*& refToParent = grandParent->GetDirectChildRef(node.parent);
			node.RemoveLeaf(refToParent);
		}

		assert(node.parent == nullptr);
	}

	virtual void OnObjectAdded(const CPolygonPtr& polygon) override
	{
		TNodePtr nodePtr = std::make_unique<TNode>();
		nodePtr->object.Setpolygon(polygon);
		//treeRoot->InsertLeaf(nodePtr.get());
		AddLeaf(*nodePtr.get());
		TNode* nodeRawPtr = nodePtr.get();
		polygon->onTransformUpdatedCallback = ([this, polygon, nodeRawPtr, nodePtr](const CPolygon& poly)
		{
			RemoveLeaf(*nodeRawPtr);
			UpdateAABBTransformFromPolygon(nodeRawPtr->object.moveableAABB, poly);
			AddLeaf(*nodeRawPtr);
		});
		polyToNodes.insert(std::make_pair(polygon, nodePtr));
	}
	virtual void OnObjectRemoved(const CPolygonPtr& polygon) override
	{
		//polygon->onTransformUpdatedCallback = nullptr;
		//auto node = polyToNodes.extract(polygon);
		//RemoveLeaf(*node.mapped());
	}
};

#endif