////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: LocateT3MAPS.cxx                                                  //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 14/05/2015                                                          //
//                                                                            //
//  This program figures out the location of T3MAPS in FEI4 using a map that  //
//  was produced with TestBeamStudies.cxx.
//                                                                            //
//  Program options:                                                          //
//                                                                            //
//    "RunI" or "RunII" as an option will implement the proper cuts and load  //
//    the corresponding datasets.                                             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// C++ includes:
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>

// ROOT includes:
#include "TFile.h"
#include "TString.h"
#include "TTree.h"
#include "TVirtualFFT.h"

// Package includes:
#include "ChipDimension.h"
#include "MatchMaker.h"
#include "PixelCluster.h"
#include "PixelHit.h"
#include "TreeFEI4.h"
#include "TreeT3MAPS.h"
#include "PlotUtil.h"
#include "MapParameters.h"

using namespace std;

// Stores geometrical map of two chips:
MapParameters *mapper;

// Stores the chip geometry:
ChipDimension *chips = new ChipDimension();

/**
   The main method just requires an option to run. 
   @param option - "RunI" or "RunII" to select the desired dataset.
   @returns - 0. Prints plots to TestBeamOutput/LocateT3MAPS/ directory.
*/
int main(int argc, char **argv) {
  // Check arguments:
  if (argc < 2) {
    std::cout << "\nUsage: " << argv[0] << " <option>" << std::endl; 
    exit(0);
  }
  TString options = argv[1];
  
  // Fundamental job settings:
  TString inputT3MAPS = options.Contains("RunII") ?
    "../TestBeamData/TestBeamData_May9/T3MAPS_May9_RunI.root" :
    "../TestBeamData/TestBeamData_May3/T3MAPS_May3_RunI.root";
  TString inputFEI4 = options.Contains("RunII") ?
    "../TestBeamData/TestBeamData_May9/FEI4_May9_RunI.root" :
    "../TestBeamData/TestBeamData_May3/FEI4_May3_RunI.root";
  int lowerThresholdFEI4 = 10;
  int noiseThresholdFEI4 = options.Contains("RunII") ? 300 : 600;
  int noiseThresholdT3MAPS = options.Contains("RunII") ? 15 : 20;
  double integrationTime = options.Contains("RunII") ? 0.5 : 1.0;
  double timeOffset = 0.67;
  
  // Set the output plot style:
  PlotUtil::setAtlasStyle();  
  
  // Load the chip sizes (but use defaults!)
  chips = new ChipDimension();
  
  //----------------------------------------//
  // Occupancy for location:
  TH2D *locT3MAPS = new TH2D("locT3MAPS", "locT3MAPS", 
			     chips->getNRow("FEI4"), -0.5,
			     (chips->getNRow("FEI4") - 0.5),
			     chips->getNCol("FEI4"), -0.5,
			     (chips->getNCol("FEI4") - 0.5));
  
  TH2D *locT3MAPSErr = new TH2D("locT3MAPSErr", "locT3MAPSErr", 
				chips->getNRow("FEI4"), -0.5,
				(chips->getNRow("FEI4") - 0.5),
				chips->getNCol("FEI4"), -0.5,
				(chips->getNCol("FEI4") - 0.5));
  
  // Instantiate the mapping utility:
  mapper = new MapParameters("","");
  mapper->setOrientation(2);
  if (options.Contains("RunII")) {
    mapper->setMapVar(1, 1.7); mapper->setMapErr(1, 0.5);//0.75);
    mapper->setMapVar(3, 13.0); mapper->setMapErr(3, 1.0);//2.0);
  }
  else {
    mapper->setMapVar(1, 2.6); mapper->setMapErr(1, 0.2);//0.4);
    mapper->setMapVar(3, 11.0); mapper->setMapErr(3, 1.0);//2.0);
  }
  mapper->setMapExists(true);
  
  // Loop over T3MAPS pixels:
  //for (int i_r = 0; i_r < chips->getNRow("T3MAPS"); i_r++) {
  //for (int i_c = 0; i_c < chips->getNCol("T3MAPS"); i_c++) {
  //for (int i_r = 0; i_r < 1; i_r++) {
  //for (int i_c = 0; i_c < 1; i_c++) {
  for (int i_r = 10; i_r < 83; i_r++) {
  for (int i_c = 10; i_c < 16; i_c++) {
      
      //locT3MAPS->Fill(mapper->getFEI4fromT3MAPS("rowVal",i_r),
      //	      mapper->getFEI4fromT3MAPS("colVal",i_c));
      
      locT3MAPS->Fill(mapper->getT3MAPSfromFEI4("rowVal",mapper->getFEI4fromT3MAPS("rowVal",i_r)),mapper->getT3MAPSfromFEI4("colVal",mapper->getFEI4fromT3MAPS("colVal",i_c)));
      
      locT3MAPSErr->Fill(mapper->getFEI4fromT3MAPS("rowVal",i_r) +
			 mapper->getFEI4fromT3MAPS("rowSigma",i_r),
			 mapper->getFEI4fromT3MAPS("colVal",i_c) +
			 mapper->getFEI4fromT3MAPS("colSigma",i_c));
      locT3MAPSErr->Fill(mapper->getFEI4fromT3MAPS("rowVal",i_r) +
			 mapper->getFEI4fromT3MAPS("rowSigma",i_r),
			 mapper->getFEI4fromT3MAPS("colVal",i_c) -
			 mapper->getFEI4fromT3MAPS("colSigma",i_c));
      locT3MAPSErr->Fill(mapper->getFEI4fromT3MAPS("rowVal",i_r) -
			 mapper->getFEI4fromT3MAPS("rowSigma",i_r),
			 mapper->getFEI4fromT3MAPS("colVal",i_c) +
			 mapper->getFEI4fromT3MAPS("colSigma",i_c));
      locT3MAPSErr->Fill(mapper->getFEI4fromT3MAPS("rowVal",i_r) -
			 mapper->getFEI4fromT3MAPS("rowSigma",i_r),
			 mapper->getFEI4fromT3MAPS("colVal",i_c) -
			 mapper->getFEI4fromT3MAPS("colSigma",i_c));
      locT3MAPSErr->Fill(mapper->getFEI4fromT3MAPS("rowVal",i_r) +
			 mapper->getFEI4fromT3MAPS("rowSigma",i_r),
			 mapper->getFEI4fromT3MAPS("colVal",i_c));
      locT3MAPSErr->Fill(mapper->getFEI4fromT3MAPS("rowVal",i_r) -
			 mapper->getFEI4fromT3MAPS("rowSigma",i_r),
			 mapper->getFEI4fromT3MAPS("colVal",i_c));
      locT3MAPSErr->Fill(mapper->getFEI4fromT3MAPS("rowVal",i_r),
			 mapper->getFEI4fromT3MAPS("colVal",i_c) +
			 mapper->getFEI4fromT3MAPS("colSigma",i_c));
      locT3MAPSErr->Fill(mapper->getFEI4fromT3MAPS("rowVal",i_r),
			 mapper->getFEI4fromT3MAPS("colVal",i_c) -
			 mapper->getFEI4fromT3MAPS("colSigma",i_c));
    }
  }
  
  PlotUtil::plotTH2D(locT3MAPS, "row_{FEI4}", "col_{FEI4}", "", "../TestBeamOutput/LocateT3MAPS/nominalLocationT3MAPS.eps");
  PlotUtil::plotTH2D(locT3MAPSErr, "row_{FEI4}", "col_{FEI4}", "", "../TestBeamOutput/LocateT3MAPS/uncertaintyLocationT3MAPS.eps");
    
  std::cout << "\nLocateT3MAPS: Finished analysis." << std::endl;
  return 0;
}
