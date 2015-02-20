////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: LinearMapMaker.h                                                    //
//  Class: LinearMapMaker.cxx                                                 //
//                                                                            //
//  Author: Andrew Hard                                                       //
//  Email: ahard@cern.ch                                                      //
//  Date: 19/02/2015                                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef LinearMapMaker_h
#define LinearMapMaker_h

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

class LinearMapMaker {
  
 public:
  
  LinearMapMaker(TString fileDir, TString option);
  ~LinearMapMaker();
  
  // Mutators:
  void addPairToMap(PixelHit *hitFEI4, PixelHit *hitT3MAPS);
  void createMapFromHits();
  void loadMapParameters(TString inputDir);
  void saveMapParameters(TString outputDir);
  void setMapRMS(int varIndex, double newVal);
  void setMapVar(int varIndex, double newVal);
  
  // Accessors:
  bool mapExists();
  int getFEI4fromT3MAPS(TString valName, int valT3MAPS);
  int getT3MAPSfromFEI4(TString valName, int valFEI4);
  double getMapRMS(int varIndex);
  double getMapVar(int varIndex);
  void printMapParameters();
  
 private:
  
  ChipDimension *myChips;
  
  // A 2D histogram for every row in T3MAPS
  std::vector<TH2D*> rowHist2dFEI4;
  
  // A 2D histogram for every column in T3MAPS
  std::vector<TH2D*> colHist2dFEI4;
  
  // Array to store linear constants.
  double mVar[4];
  double mRMS[4];
  bool hasMap[4];
  
  bool sameOrientation;
  
  TF1* createFit(TString name, TGraphErrors *g, double xlow, double xhigh);

};

#endif
