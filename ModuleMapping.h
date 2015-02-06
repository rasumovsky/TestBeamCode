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

// look at CommonHead.hh to see what might be missing...
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TString.h"

#incude "LoadT3MAPS.h"

using namespace std;

class ModuleMapping {

 private:
  
  // Pixel ranges for FEI4 and T3MAPS:
  int nRowFEI4;
  int nColFEI4;
  int nRowT3MAPS;
  int nColT3MAPS;
  
  // For T3MAPS data:
  LoadT3MAPS *T3MAPS;
  TTree *myTreeT3MAPS;
  
  // For FEI4 data:
  TFile *fileFEI4;
  TTree *myTreeFEI4;
  
  TCanvas *can;
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
  
 public:
  
  ModuleMapping(TString inputT3MAPS, TString inputFEI4, TString fileDir);
  ModuleMapping(TString fileDir);
  ~ModuleMapping();
  
  void closeInputFiles();
  void saveMapParameters(TString outputDir);
  void loadMapParameters(TString inputDir);
  void printMapParameters();
  
  void prepareTrees();
  void makeCombinedMap();
  void makeGeoMap(int rowT3MAPS_1, int colT3MAPS_1, 
		  int rowT3MAPS_2, int colT3MAPS_2);
  
  bool mapExists();
  int getFEI4fromT3MAPS(TString pos, int valT3MAPS);
  int getT3MAPSfromFEI4(TString pos, int valFEI4);
  double getMapVar(int varIndex);
  double getMapRMS(int varIndex);
  void setMapVar(int varIndex, double newVal);
  void setMapRMS(int varIndex, double newVal);
  
};

#endif
