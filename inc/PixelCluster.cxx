////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: PixelCluster.cxx                                                    //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 06/02/2015                                                          //
//                                                                            //
//  This class stores a list of pixel hits that have been grouped together in //
//  to form a single cluster.                                                 //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "PixelCluster.h"

/**
   Initialize a cluster:
 */
PixelCluster::PixelCluster(PixelHit *hit) {
  clusteredHits.clear();
  clusterMatched = false;
  addHit(hit);
  return;
}

/**
   Add a hit to the cluster.
*/
void PixelCluster::addHit(PixelHit *hit) {
  clusteredHits.push_back(hit);
  if (hit->isHitMatched()) {
    clusterMatched = true;
  }
}

/** 
    Add another cluster to this cluster.
*/
void PixelCluster::addCluster(PixelCluster *cluster) {
  for (int i = 0; i < cluster->getNHits(); i++) {
    addHit(cluster->getHit(i));
  }
}

/**
   Get the number of hits in a cluster.
*/
int PixelCluster::getNHits() {
  return clusteredHits.size();
}

/**
   Get a specific hit based on the index from a cluster.
*/
PixelHit* PixelCluster::getHit(int hitIndex) {
  return clusteredHits[hitIndex];
}

/**
   Check if a hit is contained in the cluster.
*/
bool PixelCluster::containsHit(PixelHit *hit) {
  for (int i = 0; i < (int)clusteredHits.size(); i++) {
    if ((clusteredHits[i])->getRow() == hit->getRow() &&
	(clusteredHits[i])->getCol() == hit->getCol()) {
      return true;
    }
  }
  return false;
}

/**
   Check if another cluster is next to this cluster.
 */
bool PixelCluster::clusterIsAdjacent(PixelCluster *cluster) {

  // Loop over this cluster
  for (int i = 0; i < (int)clusteredHits.size(); i++) {
    PixelHit *currHit1 = clusteredHits[i];
    
    // Loop over input cluster:
    for (int j = 0; j < cluster->getNHits(); j++) {
      PixelHit *currHit2 = cluster->getHit(j);
      if (currHit1->hitIsAdjacent(currHit2)) {
	return true;
      }
    }
  }
  return false;
}

/**
   Check if any of the hits in this cluster are matched to any hits in the 
   other chip.
 */
bool PixelCluster::isClusterMatched() {
  return clusterMatched;
}

/**
   Check if any of the hits in this cluster are matched to any hits in the 
   other chip. This is a bit more thorough, and allows for hits matched after
   the clustering took place...
 */
bool PixelCluster::reTestClusterMatching() {
  for (int i = 0; i < (int)clusteredHits.size(); i++) {
    if (clusteredHits[i]->isHitMatched()) {
      return true;
    }
  }
  return false;
}

