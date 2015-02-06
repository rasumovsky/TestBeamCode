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

#include "PixelHit.h"

class HitMatching 
{
 private:
  
  // All wrong. Create a hit class, with ???
  // not sure now, maybe this is fine...
  // better just use vectors, loop over them, check if they are 'in range'
  
  // need to add mask file somehow...
  // simple class, load text from file. create array. done.
  // but maybe not. after all, we are starting with FEI4 hits and are not really
  // masking T3MAPS (or should we?)

  // Member objects:
  vector<PixelHit> hitsFEI4;
  vector<PixelHit> hitsT3MAPS;
  
  int nPixHitsT3MAPS;
  int nPixHitsFEI4;
  
  int nClustersT3MAPS;
  int nClustersFEI4;
  
  vector<vector<int>> clusterIndicesT3MAPS;
  vector<vector<int>> clusterIndicesFEI4;
  
  
  ModuleMapping *mapper;
  
 public:
  
  // Constructor:
  HitMatching( std::string inFileName, std::string outFileName );
  ~HitMatching();
  
  // Member functions:
  void AddHitInFEI4(PixelHit hit);
  void AddHitInT3MAPS(PixelHit hit);
  
  void buildFEI4Clusters();

  bool isHitMatchedInFEI4(PixelHit hit);
  bool isHitMatchedInT3MAPS(PixelHit hit);
  
  int getNPixHitsT3MAPS(string option);
  int getNPixHitsFEI4(string option);
  int getNClustersT3MAPS(string option);
  int getNCLustersFEI4(string option);
};

#endif
