#include "PGVIPrune.h"

namespace momdp 
{

	void PGVIPrune::prune(void)
	{
		DEBUG_TRACE(cout << "PGVIPrune" << endl;);

		pruneLowerBound(); // bounds->pruneAlpha->prune();
		pruneUpperBound();  // bounds->pruneBVpair->prune();

		if(solver->numBackups/pruneInterval>= currentRound)
		{
			DEBUG_TRACE(cout << "currentRound " << currentRound << endl;);
			currentRound++;
			//tranverse the entire tree to nullify sub-optimal branches 
			//	in the tree
			//SYL_NO_NULLIFY  280409 added if statement
			if (problem->XStates->size() == 1)
			{
				nullifySubOptimalBranches();
			}
			//check for delta-dominance, and prune the alpha planes
			//	which do not hold any certificate

			//bounds->pruneAlpha->updateCertsAndUses(bounds->numBackups);
			//bounds->pruneAlpha->pruneNotCertedAndNotUsed();

			//SYL_NO_PRUNE	
			if (problem->XStates->size() == 1)
			{
				pruneDynamicDeltaVersion();  // calls wrapper in PGVIPrune class
				// bounds->pruneAlpha->pruneDynamicDeltaVersion(bounds->numBackups);
			}


		} 
	}

	void PGVIPrune::pruneLowerBound()
	{
		FOR (stateidx, pgviSolver->lowerBoundSet->set.size()) 
		{
			pgviSolver->lowerBoundSet->set[stateidx]->pruneEngine->prune();
		}
	}
	void PGVIPrune::pruneUpperBound()
	{
		FOR (stateidx, pgviSolver->upperBoundSet->set.size()) 
		{
			pgviSolver->upperBoundSet->set[stateidx]->pruneEngine->prune();
		}
	}

	void PGVIPrune::pruneDynamicDeltaVersion()
	{
		DEBUG_TRACE( cout << "PGVIPrune::pruneDynamicDeltaVersion" << endl;);

		double curTime = GlobalResource::getInstance()->getInstance()->getRunTime();	
		double sumPruneTime = 0;
		int overPruneLocal = 0, underPruneLocal = 0; 
		int overPruneSum = 0;
		int underPruneSum = 0; 

		int sumNumPrune=0;

		FOR (stateidx, pgviSolver->lowerBoundSet->set.size()) 
		{
			sumPruneTime  += pgviSolver->lowerBoundSet->set[stateidx]->pruneEngine->pruneTime;
			sumNumPrune  += pgviSolver->lowerBoundSet->set[stateidx]->pruneEngine->numPrune;
		}

		if (firstPass) 
		{
			// first time in this function - record that as the elapsed time
			DEBUG_TRACE( cout << "firstPass" << endl;);
			elapsed = curTime;
			firstPass = false;
		}

	
		// below follows the settings in appl-0.3w
		//		if((curTime-elapsed) > 10 && sumPruneTime < (curTime-elapsed-10) * 0.05)  //curTime-elapsed-10
		//		if(curTime > 5 && sumPruneTime < curTime * 0.1)
		
		if((curTime-elapsed) > 5 && sumPruneTime < (curTime-elapsed-5) * 0.1)  
		{
			//cout << "elapsed : " << elapsed << " curTime : " << curTime << " (curTime-elapsed) : " << (curTime-elapsed) << " (curTime-elapsed-5) : " << (curTime-elapsed-5) << " sumPruneTime : " << sumPruneTime << " sumNumPrune : " << sumNumPrune << endl;
			FOR (stateidx, pgviSolver->lowerBoundSet->set.size()) 
			{
				DEBUG_TRACE( cout << "stateidx " << stateidx << endl;);
				pgviSolver->lowerBoundSet->set[stateidx]->pruneEngine->pruneDynamicDeltaVersion(solver->numBackups, overPruneLocal, underPruneLocal);
				overPruneSum += overPruneLocal;
				underPruneSum += underPruneLocal;

			}


			//	cout << "overPruneSum : " << overPruneSum << endl;
			//	cout << "underPruneSum : " << underPruneSum << endl;

			// adjust bglobal_delta accordingly
			updateDeltaVersion2(overPruneSum, underPruneSum);  

			//	cout << "bglobal_delta : " << bglobal_delta << endl;

			// set global_delta for each pruneAlpha accordingly
			FOR (stateidx, pgviSolver->lowerBoundSet->set.size()) 
			{
				pgviSolver->lowerBoundSet->set[stateidx]->pruneEngine->setDelta(bglobal_delta);
			}
		}
	}


	void PGVIPrune::setDelta(double newDelta)
	{
		DEBUG_TRACE(cout << "PGVIPrune::setDelta newDelta " << newDelta << endl;);
		bglobal_delta = newDelta;
		FOR (stateidx, pgviSolver->lowerBoundSet->set.size()) 
		{
			pgviSolver->lowerBoundSet->set[stateidx]->pruneEngine->setDelta(newDelta);
		}
	}

	void PGVIPrune::updateDeltaVersion2(int overPrune, int underPrune)
	{
		double overPruneThreshold = solver->solverParams-> overPruneThreshold;
		double lowerPruneThreshold = solver->solverParams-> lowerPruneThreshold;
		
		double fOverPrune = overPrune;
		
		DEBUG_TRACE(cout << "PGVIPrune::updateDeltaVersion2" << endl;);
		DEBUG_TRACE(cout << "overPruneThreshold" << overPruneThreshold << endl;);
		DEBUG_TRACE(cout << "lowerPruneThreshold" << lowerPruneThreshold << endl;);
		DEBUG_TRACE(cout << "fOverPrune" << fOverPrune << endl;);
		DEBUG_TRACE(cout << "underPrune" << underPrune << endl;);
		


		if(solver->solverParams->dynamicDeltaPercentageMode)
		{
			DEBUG_TRACE(cout << "dynamicDeltaPercentageMode" << endl;);
			unsigned int total_planes = 0;

			FOR (stateidx, pgviSolver->lowerBoundSet->set.size()) 
			{
				total_planes += pgviSolver->lowerBoundSet->set[stateidx]->planes.size();
			}
			fOverPrune = ((double)overPrune)/total_planes;
			DEBUG_TRACE(cout << "fOverPrune" << fOverPrune << endl;);
		}

		
		
		//cout  << "overPruneThreshold : "<< overPruneThreshold<< endl; 
		//cout  << "lowerPruneThreshold : "<< lowerPruneThreshold<< endl;
		//cout  << "overPrune : "<< fOverPrune << endl; 		 		
	
		// 0 is increase delta
		// 1 is stay
		// 2 is decrease delta
		
		//int state = 2;
		switch(state)
		{
			case 0:
				DEBUG_TRACE(cout << "state 0" << endl;);
				if(fOverPrune < overPruneThreshold)
				{
					state = 1;
				}
				else
				{
					increaseDelta();
					
				}
				break;
			case 1:
				DEBUG_TRACE(cout << "state 1" << endl;);
				if(fOverPrune < lowerPruneThreshold)
				{
					state = 2;
					decreaseDelta();
					
				}
				else if(fOverPrune > overPruneThreshold)
				{
					state = 0;
					increaseDelta();
					
				}
				break;
			case 2:
				DEBUG_TRACE(cout << "state 2" << endl;);
				if(fOverPrune > overPruneThreshold)
				{
					state = 0;
					increaseDelta();

				}
				else
				{
					decreaseDelta();
				}
				break;
		}
		state = 1;

	}
	void PGVIPrune::increaseDelta()
	{
		DEBUG_TRACE(cout << "increaseDelta" << endl;);
		if (bglobal_delta < 2.0+1e-7) 
		{	// 2 is the maximum distance between 2 belief points.
			bglobal_delta *= 2;
		}
		//cout << "Increase Delta to : " << global_delta << endl;
	}

	void PGVIPrune::decreaseDelta()
	{
		DEBUG_TRACE(cout << "decreaseDelta" << endl;);
		bglobal_delta /= 2; // on the left follows the settings in appl-0.3w *= 0.75;
		//bglobal_delta /= 2;
		//cout << "Decrease Delta to : " << global_delta << endl;
	}

	void PGVIPrune::nullifySubOptimalBranches()
	{
		DEBUG_TRACE( cout << "PGVIPrune::nullifySubOptimalBranches" << endl;);

		BeliefForest* globalRoot = pgviSolver->sampleEngine->getGlobalNode();

		BeliefTreeNode* currRoot;
		//unsigned int numSampleRoots = globalRoot->getGlobalRootNumSampleroots();
		unsigned int numSampleRoots = globalRoot->sampleRootEdges.size();

		FOR (r, numSampleRoots) 
		{
			//FOR (r, globalRoot->getGlobalRootNumSampleroots()) {
			SampleRootEdge* eR = globalRoot->sampleRootEdges[r];
			if (NULL != eR) 
			{
				currRoot = eR->sampleRoot;
				uncheckAllSubNodes(currRoot);
				nullifySubOptimalCerts(currRoot);
			}
		}
	}


	//Function: nullifySubOptimalCerts
	//Functionality:
	//	tranverse the reachability-tree with root 'cn', and update
	//	the 'count' of non-suboptimal paths which leads to the node
	//	for each node in the tree. (i.e. a node with reachableCount==0
	//	is a suboptimal path
	//Note:
	//	note that a node which becomes suboptimal may become a valid
	//	node again later, as it maybe reached from other paths as
	//	the tree grows
	//Parameters:
	//	cn: the root for the reachability-tree that we are going to
	//		explore
	//Returns:
	//	NA
	void PGVIPrune::nullifySubOptimalCerts(BeliefTreeNode* cn)
	{
		double ubVal, lbVal;
		DEBUG_TRACE ( cout << "PGVIPrune::nullifySubOptimalCerts" << endl; );
		if(cn->checked==false)
		{
			DEBUG_TRACE ( cout << "cn->checked==false" << endl; );
			//check the checked
			cn->checked = true;

			//get ubVal and lbVal
			
			ubVal=pgviSolver->beliefCacheSet[cn->cacheIndex.sval]->getRow(cn->cacheIndex.row)->UB;
			lbVal=pgviSolver->beliefCacheSet[cn->cacheIndex.sval]->getRow(cn->cacheIndex.row)->LB;
			//ubVal=beliefCache->getRow(cn->cacheIndex)->UB;
			//lbVal=beliefCache->getRow(cn->cacheIndex)->LB;

			DEBUG_TRACE ( cout << "ubVal " << ubVal << endl; );
			DEBUG_TRACE ( cout << "lbVal " << lbVal << endl; );

			//check for each sub entry E of cn
			FOR(a, cn->getNodeNumActions()) 
			{
				if(cn->Q[a].ubVal <lbVal - 0.0001)
				{
					DEBUG_TRACE ( cout << "cn->Q[a].ubVal " << cn->Q[a].ubVal << endl; );
					//update the entry cn.Q[a] as uninitialized
					nullifyEntry(&cn->Q[a]);
				}
				nullifySubOptimalCerts(&cn->Q[a]);
			}
		}
	}


	//Function: nullifySubOptimalCerts
	//Functionality:
	//	for the BeliefTreeQEntry (Action node of pomdp problem) 'e' given,
	//	check and nullify all its children nodes
	//	Basically, the method pass down the checking procedure
	//Parameters:
	//	e: the action node whose children nodes are to be checked
	//Returns:
	//	NA
	void PGVIPrune::nullifySubOptimalCerts(BeliefTreeQEntry* e)
	{
		FOR(x, e->getNumStateOutcomes()) {
			BeliefTreeObsState* xpt = e->stateOutcomes[x];
			if (xpt!=NULL) {
				FOR(o, xpt->getNumOutcomes()){//for all observations, nullify all subsequent BeliefTreeNodes
					if(xpt->outcomes[o]!=NULL){
						BeliefTreeNode* cn_p = xpt->outcomes[o]->nextState;//@
						if(cn_p != NULL){
							if((*cn_p).isFringe()==false){
								nullifySubOptimalCerts(cn_p);
							}
						}
					}
				}
			}
		} 
	}


	//Function: nullifyEntry
	//Functionality:
	//	To set the entire subtree of 'e' to be invalid, i.e. 'valid' attribute of
	//	'e' and its descendent-BeliefTreeQEntry-s are set to 'false', and all its
	//	descendent-BeliefTreeNode-s will have their validCount--
	//Parameters:
	//	e: the root entry which is suboptimal
	//Returns:
	//	NA
	void PGVIPrune::nullifyEntry(BeliefTreeQEntry* e)
	{
		//only do nullification if it hasn't been nullified before
		if(e->valid==true){
			e->valid = false;//nullify validness of entry
			FOR(x, e->getNumStateOutcomes()) 
			{
				BeliefTreeObsState* xpt = e->stateOutcomes[x];
				if (xpt!=NULL) 
				{
					FOR(o, xpt->getNumOutcomes())
					{
						//for all observations, nullify all subsequent BeliefTreeNodes
						if(xpt->outcomes[o]!=NULL)
						{
							BeliefTreeNode* cn_p = xpt->outcomes[o]->nextState;//@
							if(cn_p != NULL)
							{
								cn_p->count--;//decrease counter of cn

								DEBUG_TRACE ( cout << "nullifyEntry" << endl; );
								DEBUG_TRACE ( cout << "Node " << cn_p->cacheIndex.row << " count " << cn_p->count << endl; );

								if(cn_p->count==0)
								{
									FOR(a, cn_p->getNodeNumActions())
									{
										nullifyEntry(&cn_p->Q[a]);
									}
								}
							}
						}
					}
				} 
			}	
		}
	}



	//Function: uncheckAllSubNodes
	//Functionality:
	//	refresh the 'checked' record of every node in the subtree
	//	to be false. (so that later can be used for tranversing the tree)
	//Parameter:
	//	cn: the root node of the tree to be unchecked
	//Returns:
	//	NA
	void PGVIPrune::uncheckAllSubNodes(BeliefTreeNode* cn)
	{
		if (cn->checked==true) 
		{
			cn->checked = false;
			//uncheck all sub entries
			FOR(a, cn->getNodeNumActions()) 
			{
				uncheckEntry(&cn->Q[a]);
			}
		}
	}

	//Function: uncheckEntry
	//Functionality:
	//	refresh the 'checked' record of every node in the subtree
	//	of 'e' to be false. (for later's tranversing purpose)
	//Parameter:
	//	e: the action node whose subtree is to be unchecked
	//Returns:
	//	NA
	void PGVIPrune::uncheckEntry(BeliefTreeQEntry* e)
	{
		FOR(x, e->getNumStateOutcomes()) 
		{
			BeliefTreeObsState* xpt = e->stateOutcomes[x];
			if (xpt!=NULL) 
			{
				FOR(o, xpt->getNumOutcomes())
				{
					//for all observations, nullify all subsequent BeliefTreeNodes
					if(xpt->outcomes[o]!=NULL)
					{
						BeliefTreeNode* cn_p = xpt->outcomes[o]->nextState;//@
						if(cn_p != NULL)
						{
							uncheckAllSubNodes(cn_p);
						}
					}
				}
			}
		} 

	}
}

