////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: ModuleMapping.h                                                     //
//  Class: ModuleMapping.cxx                                                  //
//                                                                            //
//  Author: Andrew Hard                                                       //
//  Email: ahard@cern.ch                                                      //
//  Date: 05/02/2015                                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef ModuleMapping_h
#define ModuleMapping_h

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "TTree.h"
#include "TH1F.h"
#include "TH2D.h"
#include "TString.h"

#include "ChipDimension.h"
#include "PixelHit.h"

class ModuleMapping {
  
 public:
  
  ModuleMapping(TString fileDir, TString option);
  ~ModuleMapping();
  
  // Mutators:
  void loadMapParameters(TString inputDir);
  void saveMapParameters(TString outputDir);
  void setMapRMS(int varIndex, double newVal);
  void setMapVar(int varIndex, double newVal);
  void designatePixelPair(int row1, int col1, int row2, int col2);
  void addHitToMap(int pixIndex, PixelHit *hit);
  void createMapFromHits();
  
  // Accessors:
  bool mapExists();
  int getFEI4fromT3MAPS(TString pos, int valT3MAPS);
  int getT3MAPSfromFEI4(TString pos, int valFEI4);
  double getMapRMS(int varIndex);
  double getMapVar(int varIndex);
  void printMapParameters();
  
 private:
  
  ChipDimension *myChips;
   
  TH2D *hist2dFEI4Pix[2];
      
  // Array to store linear constants.
  double mVar[4];
  double mRMS[4];
  bool hasMap[4];
  
  int T3MAPS_r1; int T3MAPS_c1; int T3MAPS_r2; int T3MAPS_c2;
  
};

#endif
