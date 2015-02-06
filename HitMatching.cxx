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

  rowHitsFEI4.clear();
  colHitsFEI4.clear();
  nPixHitsFEI4 = 0;
  nClustersFEI4 = 0;
  
  rowHitsT3MAPS.clear();
  colHitsT3MAPS.clear();
  nPixHitsT3MAPS = 0;
  nClustersT3MAPS = 0;  
  
  // make sure this assignment works!
  myMapper = mapper;
  
  return;
}

/**
   Add a single pixel hit in the FEI4 chip.
 */
void HitMatching::AddHitInFEI4(int row, int col) {
  if (row > 0 && row < 336 && col > 0 && col < 336) {
    rowHitsFEI4.push_back(row);
    colHitsFEI4.push_back(col);
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
void HitMatching::AddHitInT3MAPS(int row, int col) {
  if (row > 0 && row < 16 && col > 0 && col < 64) {
    rowHitsT3MAPS.push_back(row);
    colHitsT3MAPS.push_back(col);
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
bool isMatchedInFEI4(int rowT3MAPS, int colT3MAPS) {
  
  // These are the nominal positions:
  int rowNomFEI4 = mapper->GetFEI4fromT3MAPS("rowVal",rowT3MAPS);
  int colNomFEI4 = mapper->GetFEI4fromT3MAPS("colVal",colT3MAPS);
  
  /// WARNING!!@!!! THIS STILL HAS TO BE IMPLEMENTED IN MODULEMAPPING!
  int rowSigmaFEI4 = mapper->GetFEI4fromT3MAPS("rowSigma",rowT3MAPS);
  int colSigmaFEI4 = mapper->GetFEI4fromT3MAPS("colSigma",colT3MAPS);
  
  // loop over FEI4 hits, see if any are around the nominal +/- sigma positions
  for (int i = 0; i < nPixHitsFEI4; i++) {
    if (rowHitsFEI4(i) >= (rowNomFEI4 - rowSigmaFEI4) &&
	rowHitsFEI4(i) <= (rowNomFEI4 + rowSigmaFEI4) &&
	colHitsFEI4(i) >= (colNomFEI4 - colSigmaFEI4) &&
	colHitsFEI4(i) <= (colNomFEI4 + colSigmaFEI4)) {
      return true;
    }
  }
  return false;
}

/**
   For a given hit in FEI4, searches for a corresponding hit in T3MAPS.
 */
bool isMatchedInT3MAPS(int rowFEI4, int colFEI4) {
  
  // These are the nominal positions:
  int rowNomT3MAPS = mapper->GetT3MAPSfromFEI4("rowVal",rowFEI4);
  int colNomT3MAPS = mapper->GetT3MAPSfromFEI4("colVal",colFEI4);
  
  /// WARNING!!@!!! THIS STILL HAS TO BE IMPLEMENTED IN MODULEMAPPING!
  int rowSigmaT3MAPS = mapper->GetT3MAPSfromFEI4("rowSigma",rowFEI4);
  int colSigmaT3MAPS = mapper->GetT3MAPSfromFEI4("colSigma",colFEI4);
  
  // loop over FEI4 hits, see if any are around the nominal +/- sigma positions
  for (int i = 0; i < nPixHitsFEI4; i++) {
    if (rowHitsT3MAPS(i) >= (rowNomT3MAPS - rowSigmaT3MAPS) &&
	rowHitsT3MAPS(i) <= (rowNomT3MAPS + rowSigmaT3MAPS) &&
	colHitsT3MAPS(i) >= (colNomT3MAPS - colSigmaT3MAPS) &&
	colHitsT3MAPS(i) <= (colNomT3MAPS + colSigmaT3MAPS)) {
      return true;
    }
  }
  return false;
}
