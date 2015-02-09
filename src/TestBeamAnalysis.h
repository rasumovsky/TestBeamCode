////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: TestBeamAnalysis.h                                                  //
//  Class: TestBeamAnalysis.cxx                                               //
//                                                                            //
//  Author: Andrew Hard                                                       //
//  Email: ahard@cern.ch                                                      //
//  Date: 04/02/2015                                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

//#ifndef TestBeamAnalysis_h
//#define TestBeamAnalysis_h

// look at CommonHead.hh to see what might be missing...
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "TCanvas.h"
#include "TFile.h"
#include "TH1F.h"
#include "TString.h"
#include "TTree.h"

#include "LoadT3MAPS.h"
#include "ChipDimension.h"
#include "PixelHit.h"
#include "PixelCluster.h"

using namespace std;

ChipDimension *myChips;

TString options;

// For T3MAPS data:
LoadT3MAPS *T3MAPS;
TTree *myTreeT3MAPS;

// For FEI4 data:
TFile *fileFEI4;
TTree *myTreeFEI4;

TCanvas *can;
