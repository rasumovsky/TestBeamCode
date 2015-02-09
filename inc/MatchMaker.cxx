////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: MatchMaker.cxx                                                     //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 05/02/2015                                                          //
//                                                                            //
//  This class uses the ModuleMapping information to look for matches between //
//  clusters in FEI4 and T3MAPS. Procedure for use:                           //
//      1. MatchMaker(mapper)                                                //
//      2. AddHitInFEI4 & AddHitInT3MAPS                                      //
//      3. buildAndMatchClusters();                                           //
//            Now calls matchHits(), so that user doesn't have to do so.      //
//            Otherwise, the PixelCluster class will not be configured. You   //
//            Could also run reTestClusterMatch()                             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "MatchMaker.h"

/**
   All that is necessary for initializing the class is a mapping from the FEI4
   chip to the T3MAPS chip. 
*/
MatchMaker::MatchMaker(ModuleMapping *mapper) {
  std::cout << std::endl << "MatchMaker::Initializing..." << std::endl;
  
  clustersFEI4.clear();
  hitsFEI4.clear();
  nMatchedClusters["FEI4"] = 0;
  nMatchedHits["FEI4"] = 0;
      
  clustersT3MAPS.clear();
  hitsT3MAPS.clear();
  nMatchedClusters["T3MAPS"] = 0;
  nMatchedHits["T3MAPS"] = 0;
  
  // make sure this assignment works!
  myMapper = mapper;
  myChips = new ChipDimension();
  
  return;
}

/**
   Add a single pixel hit in the FEI4 chip.
 */
void MatchMaker::AddHitInFEI4(PixelHit *hit) {
  if (myChips->isInChip("FEI4", hit->getRow(), hit->getCol())) {
    hitsFEI4.push_back(hit);
  }
  else {
    std::cout << "MatchMaker::AddHitInFEI4 Error! Pixel out of bounds" 
	      << std::endl;
  }
}

/**
   Add a single pixel hit in the T3MAPS chip.
*/
void MatchMaker::AddHitInT3MAPS(PixelHit *hit) {
  if (myChips->isInChip("T3MAPS", hit->getRow(), hit->getCol())) {
    hitsT3MAPS.push_back(hit);
  }
  else {
    std::cout << "MatchMaker::AddHitInT3MAPS Error! Pixel out of bounds" 
	      << std::endl;
  }
}

/**
   Matches all FEI4 and T3MAPS hits that have been added.
 */
void MatchMaker::matchHits() {
  // loop over T3MAPS hits, call isHitMatchedInFEI4:
  for (int i = 0; i < (int)hitsT3MAPS.size(); i++) {
    if (isHitMatchedInFEI4(hitsT3MAPS[i])) {
      nMatchedHits["T3MAPS"]++;
    }
  }
  // loop over FEI4 hits, call isHitMatchedInT3MAPS:
  for (int j = 0; j < (int)hitsFEI4.size(); j++) {
    if (isHitMatchedInT3MAPS(hitsFEI4[j])) {
      nMatchedHits["FEI4"]++;
    }
  }
}

/**
   For a given hit in T3MAPS, searches for a corresponding hit in FEI4.
*/
bool MatchMaker::isHitMatchedInFEI4(PixelHit *hit) {
  
  // These are the nominal positions:
  int rowNomFEI4 = myMapper->getFEI4fromT3MAPS("rowVal",hit->getRow());
  int colNomFEI4 = myMapper->getFEI4fromT3MAPS("colVal",hit->getCol());
  int rowSigmaFEI4 = myMapper->getFEI4fromT3MAPS("rowSigma",hit->getRow());
  int colSigmaFEI4 = myMapper->getFEI4fromT3MAPS("colSigma",hit->getCol());
  
  // loop over FEI4 hits, see if any are around the nominal +/- sigma position
  for (int i = 0; i < (int)hitsFEI4.size(); i++) {
    if (hitsFEI4[i]->getRow() >= (rowNomFEI4 - rowSigmaFEI4) &&
	hitsFEI4[i]->getRow() <= (rowNomFEI4 + rowSigmaFEI4) &&
	hitsFEI4[i]->getCol() >= (colNomFEI4 - colSigmaFEI4) &&
	hitsFEI4[i]->getCol() <= (colNomFEI4 + colSigmaFEI4)) {
      hit->setMatched(true);
      return true;
    }
  }
  return false;
}

/**
   For a given hit in FEI4, searches for a corresponding hit in T3MAPS.
 */
bool MatchMaker::isHitMatchedInT3MAPS(PixelHit *hit) {
  
  // These are the nominal positions:
  int rowNomT3MAPS = myMapper->getT3MAPSfromFEI4("rowVal",hit->getRow());
  int colNomT3MAPS = myMapper->getT3MAPSfromFEI4("colVal",hit->getCol());
  int rowSigmaT3MAPS = myMapper->getT3MAPSfromFEI4("rowSigma",hit->getRow());
  int colSigmaT3MAPS = myMapper->getT3MAPSfromFEI4("colSigma",hit->getCol());
  
  // loop over T3MAPS hits, see if any are around the nominal +/- sigma position
  for (int i = 0; i < (int)hitsT3MAPS.size(); i++) {
    if (hitsT3MAPS[i]->getRow() >= (rowNomT3MAPS - rowSigmaT3MAPS) &&
	hitsT3MAPS[i]->getRow() <= (rowNomT3MAPS + rowSigmaT3MAPS) &&
	hitsT3MAPS[i]->getCol() >= (colNomT3MAPS - colSigmaT3MAPS) &&
	hitsT3MAPS[i]->getCol() <= (colNomT3MAPS + colSigmaT3MAPS)) {
      hit->setMatched(true);
      return true;
    }
  }
  return false;
}

/**
   Private method to assist the merging of clusters. Must be called in a while-
   loop to ensure that merging is complete and not partial (see the 
   buildFEI4Clusters() and buildT3MAPSClusters() methods for examples.
 */
std::vector<PixelCluster*> MatchMaker::mergeClusters(std::vector<PixelCluster*> 
						     inList) {
  
  std::vector<PixelCluster*> result;
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

/**
   Combine individual hits in FEI4 and T3MAPS into clusters and also check the
   matching of the clusters.
 */
void MatchMaker::buildAndMatchClusters() {
  matchHits();
  buildFEI4Clusters();
  buildT3MAPSClusters();
}

/**
   Combine individual hits in FEI4 into clusters.
*/
void MatchMaker::buildFEI4Clusters() {
  
  clustersFEI4.clear();
  nMatchedClusters["FEI4"] = 0;
  
  // loop over hits in FEI4 and add a new cluster for each:
  for (int hitIndex = 0; hitIndex < (int)hitsFEI4.size(); hitIndex++) {
    PixelCluster *currCluster = new PixelCluster(hitsFEI4[hitIndex]);
    clustersFEI4.push_back(currCluster);
  }
  
  // Keep merging clusters until they can't be merged any further:
  int old_size = 1; int new_size = 0;
  while (new_size < old_size) {
    old_size = (int)clustersFEI4.size();
    clustersFEI4 = mergeClusters(clustersFEI4);
    new_size = clustersFEI4.size();
  }
  
  // Finally, count the number of matched clusters.
  for (int i = 0; i < (int)clustersFEI4.size(); i++) {
    if (clustersFEI4[i]->isClusterMatched()) {
      nMatchedClusters["FEI4"]++;
    }
  }
}

/**
   Combine individual hits in T3MAPS into clusters.
*/
void MatchMaker::buildT3MAPSClusters() {
  
  clustersT3MAPS.clear();
  nMatchedClusters["T3MAPS"] = 0;
  
  // loop over hits in T3MAPS and add a new cluster for each:
  for (int hitIndex = 0; hitIndex < (int)hitsT3MAPS.size(); hitIndex++) {
    PixelCluster *currCluster = new PixelCluster(hitsT3MAPS[hitIndex]);
    clustersT3MAPS.push_back(currCluster);
  }
  
  // Keep merging clusters until they can't be merged any further:
  int old_size = 1; int new_size = 0;
  while (new_size < old_size) {
    old_size = (int)clustersT3MAPS.size();
    clustersT3MAPS = mergeClusters(clustersT3MAPS);
    new_size = clustersT3MAPS.size();
  }
  
  // Finally, count the number of matched clusters.
  for (int i = 0; i < (int)clustersT3MAPS.size(); i++) {
    if (clustersT3MAPS[i]->isClusterMatched()) {
      nMatchedClusters["T3MAPS"]++;
    }
  }
}

/**
   Retrieve the number of hits in a chip, either total or matched.
*/
int MatchMaker::getNHits(std::string chip, std::string type = "") {
  if (type=="matched") {
    return nMatchedHits[chip];
  }
  else {
    if (chip == "FEI4") return hitsFEI4.size();
    else return hitsT3MAPS.size();
  }
}

/**
   Retrieve the number of clusters in a chip, either total or matched.
*/
int MatchMaker::getNClusters(std::string chip, std::string type = "") {
  if (type=="matched") {
    return nMatchedClusters[chip];
  }
  else {
    if (chip == "FEI4") return clustersFEI4.size();
    else return clustersT3MAPS.size();
  }
}

