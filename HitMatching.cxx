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
HitMatching::HitMatching() {
  std::cout << std::endl << "HitMatching::Initializing..." << std::endl;

  hitsFEI4 = {{false}};
  nPixHitsFEI4 = 0;
  nClustersFEI4 = 0;
  
  hitsT3MAPS = {{false}};
  nPixHitsT3MAPS = 0;
  nClustersT3MAPS = 0;  
  
  return;
}

/**
   Add a single pixel hit in the FEI4 chip.
 */
void HitMatching::AddHitInFEI4(int row, int col) {
  if (row > 0 && row < 336 && col > 0 && col < 336) {
    nPixHitsFEI4++;
    hitsFEI4[row][col] = true;
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
    nPixHitsT3MAPS++;
    hitsT3MAPS[row][col] = true;
  }
  else {
    std::cout << "HitMatching::AddHitInT3MAPS Error! Pixel out of bounds" 
	      << std::endl;
  }
}
