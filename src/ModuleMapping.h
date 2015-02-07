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
#include "TH2F.h"
#include "TString.h"

#include "ChipDimension.h"

class ModuleMapping {
  
 public:
  
  ModuleMapping(TString inputT3MAPS, TString inputFEI4, TString fileDir);
  ModuleMapping(TString fileDir);
  ~ModuleMapping();
  
  void loadMapParameters(TString inputDir);
  void saveMapParameters(TString outputDir);
  void printMapParameters(void);
  
  void prepareTrees(void);
  void makeCombinedMap(void);
  void makeGeoMap(int rowT3MAPS_1, int colT3MAPS_1, 
		  int rowT3MAPS_2, int colT3MAPS_2);
  
  bool mapExists(void);
  int getFEI4fromT3MAPS(TString pos, int valT3MAPS);
  int getT3MAPSfromFEI4(TString pos, int valFEI4);
  double getMapVar(int varIndex);
  double getMapRMS(int varIndex);
  void setMapVar(int varIndex, double newVal);
  void setMapRMS(int varIndex, double newVal);

 private:
  
  ChipDimension *myChips;
  TTree *myTreeT3MAPS;
  TTree *myTreeFEI4;
  
  TH1F *histMapValues[4];
  TH1F *histRowFEI4Pix[2];
  TH1F *histColFEI4Pix[2];
  TH2F *hist2dFEI4Pix[2];
  
  // T3MAPS tree variables:
  double t_T3MAPS_timestamp_start;
  double t_T3MAPS_timestamp_stop;
  std::vector<int> t_T3MAPS_hit_row;
  std::vector<int> t_T3MAPS_hit_column;
  
  // FEI4 tree variables:
  double t_FEI4_timestamp_start;
  double t_FEI4_timestamp_stop;
  int t_FEI4_hit_row;
  int t_FEI4_hit_column;
    
  // Array to store linear constants.
  double mapVar[4];
  double mapRMS[4];
  bool createdMap[4];
  
};

#endif
