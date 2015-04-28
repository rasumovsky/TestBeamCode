////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: PixelCluster.h                                                      //
//  Class: PixelCluster.cxx                                                   //
//                                                                            //
//  Author: Andrew Hard                                                       //
//  Email: ahard@cern.ch                                                      //
//  Date: 14/04/2015                                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef PixelCluster_h
#define PixelCluster_h

#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <cmath>

#include "PixelHit.h"
#inlcude "ChipDimension.h"

class PixelCluster 
{
  
 public:
  
  PixelCluster(std::string newChipName);
  ~PixelCluster();
  
  // Mutators:
  void addCluster(PixelCluster *cluster);
  void addHit(PixelHit *hit);
  void addMask(PixelHit *mask);
  void clearCluster();
  void fillHistogram();
  void fitTracklet();
  void setClusterCentroid();
  
  // Accessors:
  void drawClusterHist(TString fileName);
  std::pair<double,double> getClusterCentroid();
  double getClusterSeparation(PixelCluster *cluster);
  TH2F* getClusterHist();
  int getNHits();
  int getNMasked();
  double getPathLength();
  int getTOTSum();
  TF1* getTracklet();
  
  std::vector<PixelHit*> getHits();
  std::vector<PixelHit*> getMasks();
  bool containsHit(PixelHit *hit);
  bool isAdjacent(PixelCluster *cluster);
  bool isOverlapping(PixelCluster *cluster);
  bool clustersCanMerge(PixelCluster *cluster);

 private:
  
  // Member objects:
  std::string chipName;
  int clusterTOTSum;
  double clusterPathLength;
  std::vector<PixelHit*> clusterHits;
  std::vector<PixelHit*> clusterMasks;
  std::pair<double,double> clusterCentroid;
  
  TH2F *clusterHist;
  TF1 *tracklet;
};

#endif
