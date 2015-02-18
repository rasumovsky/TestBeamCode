////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: TestBeamAnalysis.h                                                  //
//  Class: TestBeamAnalysis.cxx                                               //
//                                                                            //
//  Author: Andrew Hard                                                       //
//  Email: ahard@cern.ch                                                      //
//  Date: 09/02/2015                                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

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

#include "ChipDimension.h"
//#include "LoadT3MAPS.h"
#include "MatchMaker.h"
#include "PixelCluster.h"
#include "PixelHit.h"
#include "TreeFEI4.h"
#include "TreeT3MAPS.h"

using namespace std;

ChipDimension *myChips;

TString options;

TH1F *histMapValues[4];

TCanvas *can;

TreeFEI4 *cF;
TreeT3MAPS *cT;
