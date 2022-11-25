Source in CollisionEngine/
Build in Release/

BroadPhases implemented : 
- Brut 
- BoundingVolume (working with aabbs and circles)
- Grid
- QuadTree
- SweepAndPrune
- AABBTree WIP
All the broadphases are in the BroadPhases/ folder

Inputs : 
- f1 to reset scene
- f2 / f3 to change scenes
- f4 to display debug
- f5 to lock fps
- f6 / f7 to switch between different broad phases

a first AABB is computed from the original shape
a second one is computed from the first rotated aabb to prevent looping over all the points for a shape, which could be unoptimized

TODO : 
- Finish implement AABBTree 
- Implementing other broadphases
- Narrow phase 
- Physics Response