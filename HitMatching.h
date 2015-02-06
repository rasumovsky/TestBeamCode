////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: HitMatching.h                                                       //
//  Class: HitMatching.cxx                                                    //
//                                                                            //
//  Author: Andrew Hard                                                       //
//  Email: ahard@cern.ch                                                      //
//  Date: 05/02/2015                                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef HitMatching_h
#define HitMatching_h

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "TFile.h"
#include "TTree.h"

class HitMatching 
{
 private:
  
  // Member objects:
  int nEvents;
  TFile *outputT3MAPS; 
  TTree *treeT3MAPS;
  
  // variables stored in TTree:
  double timestamp_start;
  double timestamp_stop;
  vector<int> hit_row;
  vector<int> hit_column;

  // Member functions:
  delimString( std::string line, std::string delim );  
  
 public:
    
  // Constructor:
  HitMatching( std::string inFileName, std::string outFileName );
  ~HitMatching();
  
  // Member functions:
  int getNEvents();
  TTree* getTree();
  void closeFiles();
  
};

#endif
