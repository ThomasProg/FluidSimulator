#ifndef _BROAD_PHASE_H_
#define _BROAD_PHASE_H_

#include "PhysicEngine.h"

class IBroadPhase
{
public:
	virtual void OnObjectAdded(CPolygonPtr polygon) {} // = 0; // To call to add an object to the system
	virtual void OnObjectRemoved(CPolygonPtr polygon) {} //= 0; // To call to remove an object from the system
	virtual void OnObjectUpdated(CPolygonPtr polygon) {} // = 0; // To call when an object is moved
	virtual void GetCollidingPairsToCheck(std::vector<SPolygonPair>& pairsToCheck) = 0;
};

#endif