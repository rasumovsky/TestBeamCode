////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: TestBeamAnalysis.h                                                  //
//  Class: TestBeamAnalysis.cxx                                               //
//                                                                            //
//  Author: Andrew Hard                                                       //
//  Email: ahard@cern.ch                                                      //
//  Date: 04/02/2015                                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef TestBeamAnalysis_h
#define TestBeamAnalysis_h

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

#incude "LoadT3MAPS.h"

using namespace std;

class TestBeamAnalysis {
 private:
  TString option;
  
  // For T3MAPS data:
  LoadT3MAPS *T3MAPS;
  TTree *myTreeT3MAPS;
  
  // For FEI4 data:
  TFile *fileFEI4;
  TTree *myTreeFEI4;
  
  TCanvas *can;
  
  // T3MAPS tree variables:
  double t_T3MAPS_timestamp_start;
  double t_T3MAPS_timestamp_stop;
  std::vector<int> t_T3MAPS_hit_row;
  std::vector<int> t_T3MAPS_hit_column;
  
  // FEI4 tree variables:
  double t_FEI4_timestamp_start;
  double t_FEI4_timestamp_stop;
  std::vector<int> t_FEI4_hit_row;
  std::vector<int> t_FEI4_hit_column;
  
  // row and column mapping: REPLACE WITH ARRAY!
  double varR1;
  double varR2;
  double varC1;
  double varC2;
  
  // Array to store linear constants.
  // bit1 is for R/C, bit2 is for 1,2
  double mapVar[2][2];
  bool createdMap[2][2];
  
 public:
  
  void prepareTrees();
  void makeGeoMap( int rowT3MAPS_1, int colT3MAPS_1, 
		  int rowT3MAPS_2, int colT3MAPS_2 );
  
  bool mapExists();
  double getMapVar(string varName);
  int getFEI4fromT3MAPS( std::string pos, int valT3MAPS );
  int getT3MAPSfromFEI4( std::string pos, int valFEI4 );
  
  void setMapVar(string varName, double newVal);
  
};

#endif
