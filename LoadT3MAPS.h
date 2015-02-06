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

class LoadT3MAPS 
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
  LoadT3MAPS( std::string inFileName, std::string outFileName );
  ~LoadT3MAPS();
  
  // Member functions:
  int getNEvents();
  TTree* getTree();
  void closeFiles();
};

#endif
