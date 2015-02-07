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
#include <map>

#include "ChipDimension.h"
#include "PixelHit.h"

class HitMatching 
{
 
 public:
  
  HitMatching(ModuleMapping* mapper);
  ~HitMatching();
  
  // Mutators:
  void AddHitInFEI4(PixelHit *hit);
  void AddHitInT3MAPS(PixelHit *hit);
  void matchHits();
  void buildAndMatchClusters();
  
    
  // Accessors:
  bool isHitMatchedInFEI4(PixelHit *hit);
  bool isHitMatchedInT3MAPS(PixelHit *hit);
  int getNHits(std::string chip, std::string type);
  int getNClusters(std::string chip, std::string type);
  
 private:
  
  void buildFEI4Clusters();
  void buildT3MAPSClusters();
  std::vector<PixelCluster*> mergeClusters(std::vector<PixelCluster*> inList);
  
  std::map<std::string,int> nMatchedHits;
  std::vector<PixelHit*> hitsFEI4;
  std::vector<PixelHit*> hitsT3MAPS;
  
  std::map<std::string,int> nMatchedClusters;
  std::vector<PixelCluster*> clustersFEI4;
  std::vector<PixelCluster*> clustersT3MAPS;
  
  ModuleMapping *myMapper;
  ChipDimension *myChips;

};

#endif
