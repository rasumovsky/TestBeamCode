////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: LoadT3MAPS.h                                                        //
//  Class: LoadT3MAPS.cxx                                                     //
//                                                                            //
//  Author: Andrew Hard                                                       //
//  Email: ahard@cern.ch                                                      //
//  Date: 03/02/2015                                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef LoadT3MAPS_h
#define LoadT3MAPS_h

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

class LoadT3MAPS 
{
  
 public:
  
  LoadT3MAPS( std::string inFileName, std::string outFileName );
  virtual ~LoadT3MAPS() {};
  
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
  //std::vector<int> hit_row;
  //std::vector<int> hit_column;
  vector<int> hit_row;
  vector<int> hit_column;
  
  std::vector<std::string> delimString( std::string line, std::string delim );  
  
};

#endif
