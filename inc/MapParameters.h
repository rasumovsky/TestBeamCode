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
  
  static const int nRBin = 89;
  static const int nCBin = 48;
  
  // Mutators:
  void addPairToMap(PixelHit *hitFEI4, PixelHit *hitT3MAPS);
  void addPairToBkg(PixelHit *hitFEI4, PixelHit *hitT3MAPS);
  void createMapFromHits();
  void loadMapParameters(TString inputDir);
  void saveMapParameters(TString outputDir);
  void setOrientation(int orientation);
  void setMapErr(int varIndex, double value);
  void setMapVar(int varIndex, double value);
  void setMapExists(bool doesExist);

  // Accessors:
  bool mapExists();
  int getFEI4fromT3MAPS(TString valName, int valT3MAPS);
  int getT3MAPSfromFEI4(TString valName, int valFEI4);
  double getColOffset(int colFEI4, int colT3MAPS, int orientation);
  double getRowOffset(int rowFEI4, int rowT3MAPS, int orientation);
  double getColSlope();
  double getRowSlope();
  double getMapErr(int varIndex);
  double getMapVar(int varIndex);
  void printMapParameters();
  TH2D *getParamPlot(TString name);
  
 private:
  
  ChipDimension *chips;
    
  // Array to store linear constants.
  double mVar[4][4];
  double mErr[4][4];
  bool hasMap;
  
  // Maps of parameters:
  TH2D *h2Sig[4];
  TH2D *h2Bkg[4];
  TH2D *h2Diff[4];
  
  // Hit counters:
  int nBkgHits;
  int nSigHits;
  
  // Chip relative orientation information:
  int orientation;// 0 = ++, 1 = +-, 2 = -+, 3 = -- in (r,c)
  double colSign[4];
  double rowSign[4];
  
};

#endif
