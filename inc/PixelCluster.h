////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: PixelCluster.h                                                      //
//  Class: PixelCluster.cxx                                                   //
//                                                                            //
//  Author: Andrew Hard                                                       //
//  Email: ahard@cern.ch                                                      //
//  Date: 06/02/2015                                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef PixelCluster_h
#define PixelCluster_h

#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include "PixelHit.h"

class PixelCluster 
{
 private:
  
  // Member objects:
  std::vector<PixelHit> clusteredHits;
  int nClusteredHits;
  
 public:
  
  // Constructor:
  PixelCluster(PixelHit hit);
  ~PixelCluster();
  
  // Mutators:
  void addHit(PixelHit hit);
    
  // Accessors:
  int getNHits();
  PixelHit getHit(int hitIndex);
  bool containsHit(PixelHit hit);
  std::vector<PixelHit>::iterator iterator();
};

#endif
