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
 
 public:
  
  HitMatching(ModuleMapping* mapper);
  ~HitMatching();
  
  // Mutators:
  void matchHits();
  void buildFEI4Clusters();
  //void buildT3MAPSClusters();
  void AddHitInFEI4(PixelHit *hit);
  void AddHitInT3MAPS(PixelHit *hit);
    
  // Accessors:
  bool isHitMatchedInFEI4(PixelHit *hit);
  bool isHitMatchedInT3MAPS(PixelHit *hit);
  int getNClustersFEI4();
  int getNClustersT3MAPS();
  int getNHitsFEI4();
  int getNHitsT3MAPS();
  
 private:
  
  std::vector<PixelCluster*> mergeClusters(std::vector<PixelCluster*> inList);
  
  int nMatchedHitsFEI4;
  int nMatchedHitsT3MAPS;
  std::vector<PixelHit*> hitsFEI4;
  std::vector<PixelHit*> hitsT3MAPS;
  
  int nMatchedClustersFEI4;
  int nMatchedClustersT3MAPS;
  std::vector<PixelCluster*> clustersFEI4;
  std::vector<PixelCluster*> clustersT3MAPS;
  
  ModuleMapping *myMapper;
 
};

#endif
