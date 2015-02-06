////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: HitMatching.cxx                                                     //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 05/02/2015                                                          //
//                                                                            //
//  This class loads the T3MAPS output file and creates a TTree to store      //
//  relevant event information including:                                     //
//         - timestamp_start   (beginning of integration period)              //
//         - timestamp_stop    (end of integration period)                    //
//         - hit_row           (vector of hit rows)                           //
//         - hit_column        (vector of hit columns)                        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "HitMatching.h"

//--------------------------------------//
// HitMatching: For the initialization, load table values fr.
HitMatching::HitMatching(ModuleMapping *mapper) {
  std::cout << std::endl << "HitMatching::Initializing..." << std::endl;

  hitsFEI4.clear();
  nPixHitsFEI4 = 0;
  nClustersFEI4 = 0;
  
  hitsT3MAPS.clear();
  nPixHitsT3MAPS = 0;
  nClustersT3MAPS = 0;  
  
  // make sure this assignment works!
  myMapper = mapper;
  
  return;
}

/**
   Add a single pixel hit in the FEI4 chip.
 */
void HitMatching::AddHitInFEI4(PixelHit hit) {
  if (hit.getRow() > 0 && hit.getRow() < 336 && 
      hit.getCol() > 0 && hit.getCol() < 336) {
    hitsFEI4.push_back(hit);
    nPixHitsFEI4++;
  }
  else {
    std::cout << "HitMatching::AddHitInFEI4 Error! Pixel out of bounds" 
	      << std::endl;
  }
}

/**
   Add a single pixel hit in the T3MAPS chip.
*/
void HitMatching::AddHitInT3MAPS(PixelHit hit) {
  if (hit.getRow() > 0 && hit.getRow() < 16 && 
      hit.getCol() > 0 && hit.getCol() < 64) {
    hitsT3MAPS.push_back(hit);
    nPixHitsT3MAPS++;
  }
  else {
    std::cout << "HitMatching::AddHitInT3MAPS Error! Pixel out of bounds" 
	      << std::endl;
  }
}

/**
   For a given hit in T3MAPS, searches for a corresponding hit in FEI4.
*/
bool HitMatching::isHitMatchedInFEI4(PixelHit hit) {
  
  // These are the nominal positions:
  int rowNomFEI4 = mapper->GetFEI4fromT3MAPS("rowVal",hit.getRow());
  int colNomFEI4 = mapper->GetFEI4fromT3MAPS("colVal",hit.getCol());
  
  /// WARNING!!@!!! THIS STILL HAS TO BE IMPLEMENTED IN MODULEMAPPING!
  int rowSigmaFEI4 = mapper->GetFEI4fromT3MAPS("rowSigma",hit.getRow());
  int colSigmaFEI4 = mapper->GetFEI4fromT3MAPS("colSigma",hit.getCol());
  
  // loop over FEI4 hits, see if any are around the nominal +/- sigma positions
  for (int i = 0; i < nPixHitsFEI4; i++) {
    if (hitsFEI4(i).getRow() >= (rowNomFEI4 - rowSigmaFEI4) &&
	hitsFEI4(i).getRow() <= (rowNomFEI4 + rowSigmaFEI4) &&
	hitsFEI4(i).getCol() >= (colNomFEI4 - colSigmaFEI4) &&
	hitsFEI4(i).getCol() <= (colNomFEI4 + colSigmaFEI4)) {
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
  int rowNomT3MAPS = mapper->GetT3MAPSfromFEI4("rowVal",hit.getRow());
  int colNomT3MAPS = mapper->GetT3MAPSfromFEI4("colVal",hit.getCol());
  
  /// WARNING!!@!!! THIS STILL HAS TO BE IMPLEMENTED IN MODULEMAPPING!
  int rowSigmaT3MAPS = mapper->GetT3MAPSfromFEI4("rowSigma",hit.getRow());
  int colSigmaT3MAPS = mapper->GetT3MAPSfromFEI4("colSigma",hit.getCol());
  
  // loop over FEI4 hits, see if any are around the nominal +/- sigma positions
  for (int i = 0; i < nPixHitsFEI4; i++) {
    if (hitsT3MAPS(i).getRow() >= (rowNomT3MAPS - rowSigmaT3MAPS) &&
	hitsT3MAPS(i).getRow() <= (rowNomT3MAPS + rowSigmaT3MAPS) &&
	hitsT3MAPS(i).getCol() >= (colNomT3MAPS - colSigmaT3MAPS) &&
	hitsT3MAPS(i).getCol() <= (colNomT3MAPS + colSigmaT3MAPS)) {
      return true;
    }
  }
  return false;
}

void buildFEI4Clusters() {

  nClustersFEI4 = 0;
  clusterIndicesFEI4.clear();
  
  // loop over hits in FEI4:
  for (int hitIdx1 = 0; hitIdx1 < rowHitsFEI4; hitIdx1++) {
    
    if (notInFEI4Cluster) {
      
    }
    
  }
  
}
