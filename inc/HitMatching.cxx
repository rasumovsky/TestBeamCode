////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: HitMatching.cxx                                                     //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 05/02/2015                                                          //
//                                                                            //
//  This class uses the ModuleMapping information to look for matches between //
//  clusters in FEI4 and T3MAPS. Procedure for use:                           //
//      1. HitMatching(mapper)                                                //
//      2. AddHitInFEI4 & AddHitInT3MAPS                                      //
//      3. matchHits();                                                       //
//      4. matchClusters();                                                   //
//            WARNING! matchHits() must be called before matchClusters().     //
//            Otherwise, the PixelCluster class will not be configured. You   //
//            Could also run reTestClusterMatch()                             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "HitMatching.h"

//--------------------------------------//
// HitMatching: For the initialization, load table values fr.
HitMatching::HitMatching(ModuleMapping *mapper) {
  std::cout << std::endl << "HitMatching::Initializing..." << std::endl;
  
  clustersFEI4.clear();
  hitsFEI4.clear();
  nMatchedClustersFEI4 = 0;
  nMatchedHitsFEI4 = 0;
      
  clustersT3MAPS.clear();
  hitsT3MAPS.clear();
  nMatchedClustersT3MAPS = 0;
  nMatchedHitsT3MAPS = 0;
  
  // make sure this assignment works!
  myMapper = mapper;
  
  return;
}

/**
   Matches all FEI4 and T3MAPS hits that have been added.
 */
void HitMatching::matchHits() {
  // loop over T3MAPS hits, call isHitMatchedInFEI4:
  for (int i = 0; i < (int)hitsT3MAPS.size(); i++) {
    if (isHitMatchedInFEI4(hitsT3MAPS[iT3MAPS])) {
      nMatchedHitsT3MAPS++;
    }
  }
  // loop over FEI4 hits, call isHitMatchedInT3MAPS:
  for (int j = 0; j < (int)hitsFEI4.size(); j++) {
    if (isHitMatchedInT3MAPS(hitsFEI4[j])) {
      nMatchedHitsFEI4++;
    }
  }
}

/**
   Add a single pixel hit in the FEI4 chip.
 */
void HitMatching::AddHitInFEI4(PixelHit *hit) {
  if (hit->getRow() > 0 && hit->getRow() < 336 && 
      hit->getCol() > 0 && hit->getCol() < 336) {
    hitsFEI4.push_back(hit);
  }
  else {
    std::cout << "HitMatching::AddHitInFEI4 Error! Pixel out of bounds" 
	      << std::endl;
  }
}

/**
   Add a single pixel hit in the T3MAPS chip.
*/
void HitMatching::AddHitInT3MAPS(PixelHit *hit) {
  if (hit->getRow() > 0 && hit->getRow() < 16 && 
      hit->getCol() > 0 && hit->getCol() < 64) {
    hitsT3MAPS.push_back(hit);
  }
  else {
    std::cout << "HitMatching::AddHitInT3MAPS Error! Pixel out of bounds" 
	      << std::endl;
  }
}

/**
   For a given hit in T3MAPS, searches for a corresponding hit in FEI4.
*/
bool HitMatching::isHitMatchedInFEI4(PixelHit *hit) {
  
  // These are the nominal positions:
  int rowNomFEI4 = mapper->GetFEI4fromT3MAPS("rowVal",hit->getRow());
  int colNomFEI4 = mapper->GetFEI4fromT3MAPS("colVal",hit->getCol());
  
  /// WARNING!!@!!! THIS STILL HAS TO BE IMPLEMENTED IN MODULEMAPPING!
  int rowSigmaFEI4 = mapper->GetFEI4fromT3MAPS("rowSigma",hit->getRow());
  int colSigmaFEI4 = mapper->GetFEI4fromT3MAPS("colSigma",hit->getCol());
  
  // loop over FEI4 hits, see if any are around the nominal +/- sigma position
  for (int i = 0; i < (int)hitsFEI4.size(); i++) {
    if (hitsFEI4(i)->getRow() >= (rowNomFEI4 - rowSigmaFEI4) &&
	hitsFEI4(i)->getRow() <= (rowNomFEI4 + rowSigmaFEI4) &&
	hitsFEI4(i)->getCol() >= (colNomFEI4 - colSigmaFEI4) &&
	hitsFEI4(i)->getCol() <= (colNomFEI4 + colSigmaFEI4)) {
      hit->setMatched(true);
      return true;
    }
  }
  return false;
}

/**
   For a given hit in FEI4, searches for a corresponding hit in T3MAPS.
 */
bool HitMatching::isHitMatchedInT3MAPS(PixelHit hit) {
  
  // These are the nominal positions:
  int rowNomT3MAPS = mapper->GetT3MAPSfromFEI4("rowVal",hit->getRow());
  int colNomT3MAPS = mapper->GetT3MAPSfromFEI4("colVal",hit->getCol());
  
  /// WARNING!!@!!! THIS STILL HAS TO BE IMPLEMENTED IN MODULEMAPPING!
  int rowSigmaT3MAPS = mapper->GetT3MAPSfromFEI4("rowSigma",hit->getRow());
  int colSigmaT3MAPS = mapper->GetT3MAPSfromFEI4("colSigma",hit->getCol());
  
  // loop over T3MAPS hits, see if any are around the nominal +/- sigma position
  for (int i = 0; i < (int)hitsT3MAPS.size(); i++) {
    if (hitsT3MAPS(i)->getRow() >= (rowNomT3MAPS - rowSigmaT3MAPS) &&
	hitsT3MAPS(i)->getRow() <= (rowNomT3MAPS + rowSigmaT3MAPS) &&
	hitsT3MAPS(i)->getCol() >= (colNomT3MAPS - colSigmaT3MAPS) &&
	hitsT3MAPS(i)->getCol() <= (colNomT3MAPS + colSigmaT3MAPS)) {
      hit->setMatched(true);
      return true;
    }
  }
  return false;
}

// NEED TO DO SOME SORT OF ITERATIVE MERGING...
vector<PixelCluster*> HitMatching::mergeClusters(vector<PixelCluster*> inList) {
  
  vector<PixelCluster*> result;
  result.clear();
  for (int i = 0; i < (int)inList.size(); i++) {
    // check to see if we can merge:
    bool couldMerge = false;
    for (int j = 0; j < (int)result.size(); j++) {
      if (result[j]->clusterIsAdjacent(inList[i])) {
	result[j]->addCluster(inList[i]);
	couldMerge = true;
	break;
      }
    }
    if (!couldMerge) {
      result.push_back(inList[i]);
    }
  }
  return result;
}

void HitMatching::buildFEI4Clusters() {
  
  clustersFEI4.clear();
  nMatchedClustersFEI4 = 0;
  
  // loop over hits in FEI4 and add a new cluster for each:
  for (int hitIndex = 0; hitIndex < (int)hitsFEI4.size(); hitIndex++) {
    PixelCluster *currCluster = new PixelCluster(hitsFEI4[hitIndex]);
    clustersFEI4.push_back(currCluster);
  }
  
  // Keep merging clusters until they can't be merged any further:
  int old_size = 1; int new_size = 0;
  while (new_size < old_size) {
    old_size = (int)tempClusters.size();
    clustersFEI4 = mergeClusters(clustersFEI4);
    new_size = clsutersFEI4.size();
  }
  
  // Finally, count the number of matched clusters.
  
  for (int i = 0; i < (int)clustersFEI4.size(); i++) {
    if (clustersFEI4[i]->isClusterMatched()) {
      nMatchedClustersFEI4++;
    }
  }
}
    
int HitMatching::getNClustersFEI4(std::string type = "") {
  if (type=="matched") return nMatchedClustersFEI4;
  else return clustersFEI4.size();
}

int HitMatching::getNClustersT3MAPS(std::string type = "") {
  if (type=="matched") return nMatchedClustersT3MAPS;
  else return clustersT3MAPS.size();
}

int HitMatching::getNHitsFEI4(std::string type = "") {
  if (type=="matched") return nMatchedHitsFEI4;
  else return hitsFEI4.size();
}

int HitMatching::getNHitsT3MAPS(std::string type "") {
  if (type=="matched") return nMatchedHitsT3MAPS;
  else return hitsT3MAPS.size();
}
