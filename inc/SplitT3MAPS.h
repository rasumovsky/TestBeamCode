////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: SplitT3MAPS.h                                                       //
//  Class: SplitT3MAPS.cxx                                                    //
//                                                                            //
//  Author: Andrew Hard                                                       //
//  Email: ahard@cern.ch                                                      //
//  Date: 19/02/2015                                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef SplitT3MAPS_h
#define SplitT3MAPS_h

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "TFile.h"
#include "TTree.h"
#include "TROOT.h"

using namespace std;

class SplitT3MAPS 
{
  
 public:
  
  SplitT3MAPS( std::string inFileName, std::string outFileName,
	       int scansPerJob, int job);
  virtual ~SplitT3MAPS() {};
  
  // Member functions:
  int getNEvents();
  TTree* getTree();
  void closeFiles();
  
 private:
  
  int nEvents;
  TFile *outputT3MAPS; 
  TTree *treeT3MAPS;
  
  // variables stored in TTree:
  int nHits;
  Double_t timestamp_start;
  Double_t timestamp_stop;
  vector<int> hit_row;
  vector<int> hit_column;
  
  std::vector<std::string> delimString( std::string line, std::string delim );  
  
};

#endif
