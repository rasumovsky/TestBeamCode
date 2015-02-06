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
  bool hitsFEI4[336][80];
  bool hitsT3MAPS[16][64];
  
  int nPixHitsT3MAPS;
  int nPixHitsFEI4;
  
  int nClustersT3MAPS;
  int nClustersFEI4;
  
  
 public:
  
  // Constructor:
  HitMatching( std::string inFileName, std::string outFileName );
  ~HitMatching();
  
  // Member functions:
  void AddHitInFEI4(int row, int col);
  void AddHitInT3MAPS(int row, int col);
  // Basically, we want to 
  void checkMatchInFEI4(int row, int col);
  void checkMatchInT3MAPS(int rowFEI4, int colFEI4);
};

#endif
