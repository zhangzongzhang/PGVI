#ifndef PackingSetManager_H
#define PackingSetManager_H

#include <vector>
#include <list>
#include "BeliefTreeNode.h"
//#include "PointBasedAlgorithm.h"
#include "BeliefCache.h"
#include "CPTimer.h"

using namespace std;

namespace momdp{
	class PackingSetManager{
	public:
		PackingSetManager();
		virtual ~PackingSetManager(void);
		
		void addBeliefsToPackingSet(list<cacherow_stval> beliefs, vector <BeliefCache *> beliefCacheSet, int numBackups);
		
		double calculateMinimumDistanceBetweenBeliefAndPacking(cacherow_stval &elem, int i, vector <BeliefCache *> beliefCacheSet);
		// i represents the delta-packing at the ith level of the search tree

		void addBeliefsToPacking(list<cacherow_stval> beliefs, vector <BeliefCache *> beliefCacheSet);
		double calculateMinimumDistanceBetweenBeliefAndPacking(cacherow_stval &elem, list<cacherow_stval> packing, vector <BeliefCache *> beliefCacheSet);

		double calculateWeightedMinimumDistanceBetweenBeliefAndPacking(cacherow_stval &elem, int i, vector <BeliefCache *> beliefCacheSet, int numBackups);
		cacherow_stval getTheClosestBeliefInPacking(double deltaForConvergence, cacherow_stval &elem, int i, vector <BeliefCache *> beliefCacheSet);

		void resetDelta(double newDelta);
		int getPackingSetSize();
		void printPackingSetSize();
		int getUniquePackingSize();
		double getDelta();	
		double getPackingSetTimerElapsed();
		
	private:
		vector< list<cacherow_stval> > packingSet;
		vector< vector<int> > lastBackupTimeIndexSet;
		double delta; 
		int minimumDistanceIndex;
		cacherow_stval minimumCacheRow;

		list<cacherow_stval> uniquePacking;
		CPTimer packingSetTimer;
	};
};// namespace momdp

#endif