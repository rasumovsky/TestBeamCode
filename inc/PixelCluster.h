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
  
 public:
  
  PixelCluster(PixelHit *hit);
  ~PixelCluster();
  
  // Mutators:
  void addHit(PixelHit *hit);
  void addCluster(PixelCluster *cluster);

  // Accessors:
  int getNHits();
  PixelHit* getHit(int hitIndex);
  bool containsHit(PixelHit *hit);
  bool clusterIsAdjacent(PixelCluster *cluster);
  bool isClusterMatched();
  bool reTestClusterMatching();

private:
  
  // Member objects:
  std::vector<PixelHit*> clusteredHits;
  bool clusterMatched;
};

#endif
