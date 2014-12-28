#include "PackingSetManager.h"
#define RECORDPACKINGTIME 


PackingSetManager::PackingSetManager()
{
	delta = 0.5;
#ifdef RECORDPACKINGTIME
	packingSetTimer.start();
	packingSetTimer.pause();
#endif
}

PackingSetManager::~PackingSetManager(void)
{
}

void PackingSetManager::resetDelta(double newDelta)
{
	delta = newDelta;
	return ;
}

int PackingSetManager::getPackingSetSize()
{
	return packingSet.size();
}

void PackingSetManager::printPackingSetSize()
{
	FILE *fp = fopen("packing.log", "w");
	for (int i = 0; i < packingSet.size(); i++)
	{
		fprintf(fp, "%d\n", packingSet[i].size());
	}
	fclose(fp);
}

int PackingSetManager::getUniquePackingSize()
{
	return uniquePacking.size();
}

double PackingSetManager::getDelta()
{
	return delta;
}

void PackingSetManager::addBeliefsToPackingSet(list<cacherow_stval> beliefs, vector <BeliefCache *> beliefCacheSet, int numBackups)
{

#ifdef RECORDPACKINGTIME
	packingSetTimer.resume();
#endif

	int i = 0;
	for (list<cacherow_stval>::iterator iter = beliefs.begin(); iter != beliefs.end(); iter++)
	{							
		if (i < packingSet.size())
		{
			if (calculateMinimumDistanceBetweenBeliefAndPacking(*iter, i, beliefCacheSet) >= delta)
			{
				packingSet[i].push_back(*iter);
				lastBackupTimeIndexSet[i].push_back(numBackups - 1 - i);
			}
			else
			{
				lastBackupTimeIndexSet[i][minimumDistanceIndex] = numBackups - 1 - i;
			}
		}
		else
		{
			list<cacherow_stval> tempPacking;
			tempPacking.push_back(*iter);
			packingSet.push_back(tempPacking);
			
			vector<int> tempLastBackupTimeIndex;
			tempLastBackupTimeIndex.push_back(numBackups - 1 - i);
			lastBackupTimeIndexSet.push_back(tempLastBackupTimeIndex);
		}
		i++;
	}

#ifdef RECORDPACKINGTIME
	packingSetTimer.pause();
#endif

	return ;
}

void PackingSetManager::addBeliefsToPacking(list<cacherow_stval> beliefs, vector <BeliefCache *> beliefCacheSet)
{

#ifdef RECORDPACKINGTIME
	packingSetTimer.resume();
#endif

	for (list<cacherow_stval>::iterator iter = beliefs.begin(); iter != beliefs.end(); iter++)
	{
		if (uniquePacking.size() == 0)
		{
			uniquePacking.push_back(*iter);
		}
		else
		{
			if (calculateMinimumDistanceBetweenBeliefAndPacking(*iter, uniquePacking, beliefCacheSet) >= 1e-1)
			{
				uniquePacking.push_back(*iter);
			}
		}
	}

#ifdef RECORDPACKINGTIME
	packingSetTimer.pause();
#endif

	return ;
}

double PackingSetManager::calculateMinimumDistanceBetweenBeliefAndPacking(cacherow_stval &elem, int i, vector <BeliefCache *> beliefCacheSet)
{
	if (packingSet.size() == 0 || packingSet.size() - 1 < i)
	{
		//printf("Warning in calculating minimum distance between belief and packing[%d] because packing[%d] never exists.\n", i, i);
		return 2.0;
	}

	double minimumDistance = 99e+20;
	//int minimumDistanceIndex = -1;
	int k = 0; //track the index of the belief in the packing[i] with the minimum distance to elem

	BeliefTreeNode *belief1 = beliefCacheSet[elem.sval]->getRow(elem.row)->REACHABLE;

	for (list<cacherow_stval>::iterator iter = packingSet[i].begin(); iter != packingSet[i].end(); iter++)
	{
		double distance = 0.0;
		
		BeliefTreeNode *belief2 = beliefCacheSet[iter->sval]->getRow(iter->row)->REACHABLE;

		if (elem.sval == iter->sval)
		{
			bool isTheEndofDataInBelief2 = false;
			int j = 0; //point to the jth data in Belief2 
			for (int n = 0; n < belief1->s->bvec->data.size(); n++)
			{		
				if (isTheEndofDataInBelief2 == true || belief1->s->bvec->data[n].index < belief2->s->bvec->data[j].index)
				{
					distance += fabs(belief1->s->bvec->data[n].value); 
				}
				else if(belief1->s->bvec->data[n].index == belief2->s->bvec->data[j].index)
				{
					distance += fabs(belief1->s->bvec->data[n].value - belief2->s->bvec->data[j].value);
					if (j == belief2->s->bvec->data.size() - 1)
					{
						isTheEndofDataInBelief2 = true;
					}
					else
					{
						j++;
					}
				}
				else
				{
					while (belief2->s->bvec->data[j].index < belief1->s->bvec->data[n].index)
					{
						distance += fabs(belief2->s->bvec->data[j].value);
						if (j == belief2->s->bvec->data.size() - 1)
						{
							isTheEndofDataInBelief2 = true;
							break;
						}
						else
						{
							j++;
						}
					}
				}
			}
		}
		else
		{
			for (int n = 0; n < belief1->s->bvec->data.size(); n++)
			{
				distance += fabs(belief1->s->bvec->data[n].value);
			}

			for (int n = 0; n < belief2->s->bvec->data.size(); n++)
			{
				distance += fabs(belief2->s->bvec->data[n].value);
			}
		}
		if (distance < minimumDistance)
		{
			minimumDistance = distance;
			minimumDistanceIndex = k;
			minimumCacheRow = *iter;
		}
		k++;
	}

	return minimumDistance;
}

double PackingSetManager::calculateMinimumDistanceBetweenBeliefAndPacking(cacherow_stval &elem, list<cacherow_stval> packing, vector <BeliefCache *> beliefCacheSet)
{
	if (packing.size() == 0)
	{
		return 2.0;
	}

	double minimumDistance = 99e+20;

	BeliefTreeNode *belief1 = beliefCacheSet[elem.sval]->getRow(elem.row)->REACHABLE;

	for (list<cacherow_stval>::iterator iter = packing.begin(); iter != packing.end(); iter++)
	{
		double distance = 0.0;
		
		BeliefTreeNode *belief2 = beliefCacheSet[iter->sval]->getRow(iter->row)->REACHABLE;

		if (elem.sval == iter->sval)
		{
			bool isTheEndofDataInBelief2 = false;
			int j = 0; //point to the jth data in Belief2 
			for (int n = 0; n < belief1->s->bvec->data.size(); n++)
			{		
				if (isTheEndofDataInBelief2 == true || belief1->s->bvec->data[n].index < belief2->s->bvec->data[j].index)
				{
					distance += fabs(belief1->s->bvec->data[n].value); 
				}
				else if(belief1->s->bvec->data[n].index == belief2->s->bvec->data[j].index)
				{
					distance += fabs(belief1->s->bvec->data[n].value - belief2->s->bvec->data[j].value);
					if (j == belief2->s->bvec->data.size() - 1)
					{
						isTheEndofDataInBelief2 = true;
					}
					else
					{
						j++;
					}
				}
				else
				{
					while (belief2->s->bvec->data[j].index < belief1->s->bvec->data[n].index)
					{
						distance += fabs(belief2->s->bvec->data[j].value);
						if (j == belief2->s->bvec->data.size() - 1)
						{
							isTheEndofDataInBelief2 = true;
							break;
						}
						else
						{
							j++;
						}
					}
				}
			}
		}
		else
		{
			for (int n = 0; n < belief1->s->bvec->data.size(); n++)
			{
				distance += fabs(belief1->s->bvec->data[n].value);
			}

			for (int n = 0; n < belief2->s->bvec->data.size(); n++)
			{
				distance += fabs(belief2->s->bvec->data[n].value);
			}
		}
		if (distance < minimumDistance)
		{
			minimumDistance = distance;
		}
		}

	return minimumDistance;
}


double PackingSetManager::calculateWeightedMinimumDistanceBetweenBeliefAndPacking(cacherow_stval &elem, int i, vector <BeliefCache *> beliefCacheSet, int numBackups)
{

#ifdef RECORDPACKINGTIME
	packingSetTimer.resume();
#endif

	double weightedDistance;

	if (packingSet.size() > i) 
	{
		weightedDistance = calculateMinimumDistanceBetweenBeliefAndPacking(elem, i, beliefCacheSet);
		if (weightedDistance < delta)
		{
			double omega = (double)(numBackups - lastBackupTimeIndexSet[i][minimumDistanceIndex]) / numBackups;
			weightedDistance = delta * omega;
		}
	}
	else
	{
		weightedDistance = 2.0;
	}

#ifdef RECORDPACKINGTIME
	packingSetTimer.pause();
#endif

	return weightedDistance;
}

//The function is used to control the width of the search tree.
cacherow_stval PackingSetManager::getTheClosestBeliefInPacking(double deltaForConvergence, cacherow_stval &elem, int i, vector <BeliefCache *> beliefCacheSet)
{

#ifdef RECORDPACKINGTIME
	packingSetTimer.resume();
#endif

	cacherow_stval newIndexRow = elem;
	double distance = calculateMinimumDistanceBetweenBeliefAndPacking(elem, i, beliefCacheSet);
	if (distance <= deltaForConvergence) //change the belief (newIndexRow) only when its distance to the corresponding packing is not bigger than deltaForConvergence
	{
		newIndexRow = minimumCacheRow;
	}

#ifdef RECORDPACKINGTIME
	packingSetTimer.pause();
#endif

	return newIndexRow;
}

double PackingSetManager::getPackingSetTimerElapsed()
{
#ifdef RECORDPACKINGTIME
	return packingSetTimer.elapsed();
#else
	return 0.0;
#endif
}