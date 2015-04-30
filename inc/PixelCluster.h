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

// C++ includes:
#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <cmath>

// ROOT includes:
#include "TCanvas.h"
#include "TF1.h"
#include "TGraph.h"
#include "TH1.h"
#include "TH2F.h"

// Package includes:
#include "PixelHit.h"
#include "ChipDimension.h"

class PixelCluster {
  
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
  void setClusterCentroid(double row, double col);
  
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
  bool isMatched();
  bool clustersCanMerge(PixelCluster *cluster);

 private:
  
  // Member objects:
  std::string chipName;
  int clusterTOTSum;
  double clusterPathLength;
  std::vector<PixelHit*> clusterHits;
  std::vector<PixelHit*> clusterMasks;
  std::pair<double,double> centerOfCharge;
  
  TH2F *clusterHist;
  TF1 *tracklet;
};

#endif
