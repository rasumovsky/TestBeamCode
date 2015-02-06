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
PixelCluster::PixelCluster(PixelHit hit) {
  clusteredHits.clear();
  nClusteredHits = 0;
  addHit(hit);
  return;
}

/**
   Add a hit to the cluster.
*/
void PixelCluster::addHit(PixelHit hit) {
  clusteredHits.push_back(hit);
  nClusteredHits++;
}

/**
   Get the number of hits in a cluster.
*/
int PixelCluster::getNHits() {
  return nClusteredHits;
}

/**
   Get a specific hit based on the index from a cluster.
*/
PixelHit PixelCluster::getHit(int hitIndex) {
  return clusteredHits[hitIndex];
}

/**
   Check if a hit is contained in the cluster.
*/
bool PixelCluster::containsHit(PixelHit hit) {
  for (int i = 0; i < (int)clusteredHits.size(); i++) {
    if ((clusteredHits[i]).getRow() == hit.getRow() &&
	(clusteredHits[i]).getCol() == hit.getCol()) {
      return true;
    }
  }
  return false;
}

/**
   Returns an iterator over the hits in the cluster.
*/
std::vector<PixelHit>::iterator PixelCluster::iterator() {
  std::vector<PixelHit>::iterator hitIter;
  hitIter = clusteredHits.begin();
  return hitIter;//clusteredHits.iterator();
}
