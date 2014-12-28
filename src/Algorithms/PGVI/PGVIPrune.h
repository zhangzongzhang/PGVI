#ifndef PGVIPrune_H
#define PGVIPrune_H

#include "PointBasedAlgorithm.h"
#include "Bound.h"
#include "Prune.h"
#include "BoundSet.h"
#include "AlphaPlane.h"
#include "BeliefValuePair.h"
#include "PGVI.h"

namespace momdp 
{
	class PGVIPrune :public Prune
	{
	private:
		PGVI* pgviSolver;
		int pruneInterval; //numBackups between each 2 prunings
		int currentRound; 

		double elapsed;

		// for pruning
		double bglobal_delta;
		int state;
		bool firstPass;

	public:
		PGVIPrune( PGVI* _solver) :Prune(_solver)
		{
			pgviSolver = _solver;
			pruneInterval = 50;
			currentRound = 1;

			state = 2;
			firstPass = true;
			setDelta(pgviSolver->solverParams->delta);

		}

		~PGVIPrune(void)
		{
		}

		virtual void prune(void);
		void pruneLowerBound();
		void pruneUpperBound();
		void nullifySubOptimalBranches();
		void nullifySubOptimalCerts(BeliefTreeNode* cn);
		void nullifySubOptimalCerts(BeliefTreeQEntry* e);
		void nullifyEntry(BeliefTreeQEntry* e);
		void uncheckAllSubNodes(BeliefTreeNode* cn);
		void uncheckEntry(BeliefTreeQEntry* e);
		void pruneDynamicDeltaVersion();
		void updateDeltaVersion2(int overPrune, int underPrune);
		void increaseDelta();
		void decreaseDelta();

		void setDelta(double newDelta);

		static void onLowerBoundBackup (PointBasedAlgorithm* solver, BeliefTreeNode * node, SharedPointer<AlphaPlane> backupResult)
		{
			// updating certs, etc
		}

		// Migrated from AlphaPlane
		bool hasMaxMetaAt(SharedPointer<AlphaPlane> alphaPlane, int index)
		{
			PGVIAlphaPlaneTuple* dataTuple = (PGVIAlphaPlaneTuple *) alphaPlane->solverData;
			FOREACH(AlphaPlaneMaxMeta* , entry, dataTuple->maxMeta)
			{
				if((*entry)->cacheIndex == index)
				{
					return true;
				}
			}
			return false;
		}




	};
}

#endif