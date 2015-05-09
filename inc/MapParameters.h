////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: MapParameters.h                                                    //
//  Class: MapParameters.cxx                                                 //
//                                                                            //
//  Author: Andrew Hard                                                       //
//  Email: ahard@cern.ch                                                      //
//  Date: 19/02/2015                                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef MapParameters_h
#define MapParameters_h

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "TF1.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH1F.h"
#include "TH2D.h"
#include "TString.h"
#include "TTree.h"

#include "ChipDimension.h"
#include "PixelHit.h"
#include "PlotUtil.h"

class MapParameters {
  
 public:
  
  MapParameters(TString fileDir, TString option);
  virtual ~MapParameters() {};
  
  static const int nBins = 50;
  
  // Mutators:
  void addPairToMap(PixelHit *hitFEI4, PixelHit *hitT3MAPS);
  void addPairToBkg(PixelHit *hitFEI4, PixelHit *hitT3MAPS);
  void createMapFromHits();
  void loadMapParameters(TString inputDir);
  void saveMapParameters(TString outputDir);
  void setOrientation(int orientation);

  // Accessors:
  bool mapExists();
  int getFEI4fromT3MAPS(TString valName, int valT3MAPS);
  int getT3MAPSfromFEI4(TString valName, int valFEI4);
  double getMapErr(int varIndex);
  double getMapVar(int varIndex);
  void printMapParameters();
  TH2D *getParamPlot(TString name);
  
 private:
  
  ChipDimension *myChips;
    
  // Array to store linear constants.
  double mVar[4][4];
  double mErr[4][4];
  bool hasMap;
  
  int orientation;// 0 = ++, 1 = +-, 2 = -+, 3 = -- in (r,c)
  TH2D *h2Sig[4];
  TH2D *h2Bkg[4];
  TH2D *h2Diff[4];
};

#endif
