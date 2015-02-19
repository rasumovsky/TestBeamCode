////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: MapMaker.h                                                          //
//  Class: MapMaker.cxx                                                       //
//                                                                            //
//  Author: Andrew Hard                                                       //
//  Email: ahard@cern.ch                                                      //
//  Date: 18/02/2015                                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef MapMaker_h
#define MapMaker_h

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

class MapMaker {
  
 public:
  
  MapMaker(TString fileDir, TString option);
  ~MapMaker();
  
  // Mutators:
  void loadMapParameters(TString inputDir);
  void saveMapParameters(TString outputDir);
  void setMapRMS(int varIndex, double newVal);
  void setMapVar(int varIndex, double newVal);
  void addPairToMap(PixelHit *hitFEI4, PixelHit *hitT3MAPS);
  void createMapFromHits();
  
  // Accessors:
  bool mapExists();
  int getFEI4fromT3MAPS(TString valName, int valT3MAPS);
  int getT3MAPSfromFEI4(TString valName, int valFEI4);
  double getMapRMS(int varIndex);
  double getMapVar(int varIndex);
  void printMapParameters();
  TH2D* getHitPlot();
  
 private:
  
  ChipDimension *myChips;
   
  TH2D *hist2dFEI4Pix;
      
  // Array to store linear constants.
  double mVar[4];
  double mRMS[4];
  bool hasMap[4];
    
};

#endif
