////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: TestBeamTracks.cxx                                                  //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 14/05/2015                                                          //
//                                                                            //
//  This program runs through the test beam data to understand general        //
//  characteristics of the data and to define quality cuts. The program also  //
//  makes a preliminary efficiency measurement based on an occupancy ratio.   //
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

// Store masked pixel locations:
std::vector<std::pair<int,int> > maskFEI4;
std::vector<std::pair<int,int> > maskT3MAPS;

/**
   Checks the mask lists above to see whether the queried hit should be ignored.
   @param row - the row number of the hit.
   @param col - the column number of the hit.
   @param chipName - "FEI4" or "T3MAPS"
   @returns - true iff the hit should be masked.
 */
bool isMasked(int row, int col, TString chipName) {
  std::vector<std::pair<int,int> > currMask;
  if (chipName.EqualTo("FEI4")) currMask = maskFEI4;
  else if (chipName.EqualTo("T3MAPS")) currMask = maskT3MAPS;
  else return true;
  for (int i_c = 0; i_c < (int)currMask.size(); i_c++) {
    if ((currMask[i_c].first == row) && (currMask[i_c].second == col)) {
      return true;
    }
  }
  return false;
}

/**
   Check if the mapper expects a hit in one chip to be matched with any of the
   hits in the other chip.
   @param hitList - a list of hits in one chip
   @param singleHit - a single hit in the other chip.
   @returns - true if the single hit is matched with at least one of the hits
   in the other chip.
*/
bool isHitMatched(TString chipName, std::vector<std::pair<int,int> > hitList,
		  std::pair<int,int> singleHit) {
  
  // These are the nominal positions:
  int rowNom; int colNom; int rowSigma; int colSigma;
  if (chipName.EqualTo("T3MAPS")) {
    rowNom = mapper->getT3MAPSfromFEI4("rowVal", singleHit.first);
    colNom = mapper->getT3MAPSfromFEI4("colVal", singleHit.second);
    rowSigma = mapper->getT3MAPSfromFEI4("rowSigma", singleHit.first);
    colSigma = mapper->getT3MAPSfromFEI4("colSigma", singleHit.second);
  }
  else {
    rowNom = mapper->getFEI4fromT3MAPS("rowVal", singleHit.first);
    colNom = mapper->getFEI4fromT3MAPS("colVal", singleHit.second);
    rowSigma = mapper->getFEI4fromT3MAPS("rowSigma", singleHit.first);
    colSigma = mapper->getFEI4fromT3MAPS("colSigma", singleHit.second);
  }
  
  // loop over T3MAPS hits, see if any are around the nominal +/- sigma position
  for (int i = 0; i < (int)hitList.size(); i++) {
    if (hitList[i].first  >= (rowNom - rowSigma) &&
	hitList[i].first  <= (rowNom + rowSigma) &&
	hitList[i].second >= (colNom - colSigma) &&
	hitList[i].second <= (colNom + colSigma)) {
      return true;
    }
  }
  return false;
}

/**
   Check whether a hit CAN be matched in the other chip.
   @param chipName - the name of the chip to match
   @param singleHit -the hit to be matched in the chipName chip.
   @returns true iff singleHit is matched in the chipName chip.
*/
bool canMatchHit(TString chipName, std::pair<int,int> singleHit) {
  // Check if the hit can be matched in the other chip.
  int rowNom; int colNom;
  if (chipName.EqualTo("T3MAPS")) {
    rowNom = mapper->getT3MAPSfromFEI4("rowVal", singleHit.first);
    colNom = mapper->getT3MAPSfromFEI4("colVal", singleHit.second);
  }
  if (chipName.EqualTo("FEI4")) {
    rowNom = mapper->getFEI4fromT3MAPS("rowVal", singleHit.first);
    colNom = mapper->getFEI4fromT3MAPS("colVal", singleHit.second);
  }
  return chips->isInChip((string)chipName, rowNom, colNom);
}

/**
   The main method just requires an option to run. 
   @param option - "RunI" or "RunII" to select the desired dataset.
   @returns - 0. Prints plots to TestBeamOutput/TestBeamTracks/ directory.
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
  
  // Load T3MAPS data:
  TFile *fileT3MAPS = new TFile(inputT3MAPS);
  TTree *myTreeT3MAPS = (TTree*)fileT3MAPS->Get("TreeT3MAPS");
  TreeT3MAPS *cT = new TreeT3MAPS(myTreeT3MAPS);
  
  // Load FEI4 data:
  TFile *fileFEI4 = new TFile(inputFEI4);
  TTree *myTreeFEI4 = (TTree*)fileFEI4->Get("Table");
  TreeFEI4 *cF = new TreeFEI4(myTreeFEI4);
  
  // Load the chip sizes (but use defaults!)
  chips = new ChipDimension();
  
  //----------------------------------------//
  // Loop over trees initially to identify hot pixels and mask them.
  // Occupancy for masking:
  TH2D *totOccFEI4 = new TH2D("totOccFEI4", "totOccFEI4", 
			      chips->getNRow("FEI4"), -0.5,
			      (chips->getNRow("FEI4") - 0.5),
			      chips->getNCol("FEI4"), -0.5,
			      (chips->getNCol("FEI4") - 0.5));
  TH2D *totOccT3MAPS = new TH2D("totOccT3MAPS", "totOccT3MAPS", 
				chips->getNRow("T3MAPS"), -0.5,
				(chips->getNRow("T3MAPS") - 0.5),
				chips->getNCol("T3MAPS"), -0.5,
				(chips->getNCol("T3MAPS") - 0.5));
  
  // Loop over T3MAPS tree:
  Long64_t entriesT3MAPS = cT->fChain->GetEntries();
  std::cout << "TestBeamTracks: T3MAPS entries = " << entriesT3MAPS
	    << std::endl;
  for (Long64_t eventT3MAPS = 0; eventT3MAPS < entriesT3MAPS; eventT3MAPS++) {
    cT->fChain->GetEntry(eventT3MAPS);
    // Loop over hits in the event:
    for (int i_h = 0; i_h < (int)cT->hit_row->size(); i_h++) {
      totOccT3MAPS->Fill((*cT->hit_row)[i_h], (*cT->hit_column)[i_h]);
    }
  }// End of T3MAPS loop
  
  // Loop over FEI4 tree:
  Long64_t entriesFEI4 = cF->fChain->GetEntries();
  std::cout << "TestBeamOverview: FEI4 entries = " << entriesFEI4 << std::endl;
  for (Long64_t eventFEI4 = 0; eventFEI4 < entriesFEI4; eventFEI4++) {
    cF->fChain->GetEntry(eventFEI4);
    totOccFEI4->Fill(cF->row-1, cF->column-1);
  }// End of FEI4 loop
  
  // Save the BusyT3MAPS pixels:
  ofstream fBusyFEI4;
  ofstream fBusyT3MAPS;
  if (options.Contains("RunII")) {
    fBusyT3MAPS.open("../TestBeamOutput/TestBeamTracks/busyT3MAPS_RunII.txt");
    fBusyFEI4.open("../TestBeamOutput/TestBeamTracks/busyFEI4_RunII.txt");
  }
  else {
    fBusyT3MAPS.open("../TestBeamOutput/TestBeamTracks/busyT3MAPS_RunI.txt");
    fBusyFEI4.open("../TestBeamOutput/TestBeamTracks/busyFEI4_RunI.txt");
  }
  
  // Clear masks:
  maskFEI4.clear();
  maskT3MAPS.clear();
  
  // Fill the FEI4 hit per pixel plots, get mask list:
  for (int i_r = 1; i_r <= chips->getNRow("FEI4"); i_r++) {
    for (int i_c = 1; i_c <= chips->getNCol("FEI4"); i_c++) {
      // Exclude noisy column 79. 
      if (i_c >= 80) continue;
      // Only match hits with Row > 75, Col > 40
      if (i_c <= 40) continue;
      if (i_r >= 75) continue;
      
      int currNHits = (int)totOccFEI4->GetBinContent(i_r, i_c);
      if (currNHits >= noiseThresholdFEI4 || currNHits <= lowerThresholdFEI4) {
	std::pair<int,int> pairFEI4;
	pairFEI4.first = i_r-1;
	pairFEI4.second = i_c-1;
	maskFEI4.push_back(pairFEI4);
	fBusyFEI4 << i_r << " " << i_c << std::endl;
      }
    }
  }
  fBusyFEI4.close();
  
  // Fill the T3MAPS hit per pixel plots, get mask list:
  for (int i_r = 1; i_r <= chips->getNRow("T3MAPS"); i_r++) {
    for (int i_c = 1; i_c <= chips->getNCol("T3MAPS"); i_c++) {
      int currNHits = (int)totOccT3MAPS->GetBinContent(i_r, i_c);
      if (currNHits > noiseThresholdT3MAPS) {
	std::pair<int,int> pairT3MAPS;
	pairT3MAPS.first = i_r-1;
	pairT3MAPS.second = i_c-1;
	maskT3MAPS.push_back(pairT3MAPS);
	fBusyT3MAPS << i_r << " " << i_c << std::endl;
      }
    }
  }
  fBusyT3MAPS.close();
  std::cout << "TestBeamTracks: Found pixels to mask: " << maskT3MAPS.size() 
	    << " in T3MAPS and " << maskFEI4.size() << " in FEI4." << std::endl;
  
  //----------------------------------------//
  // Start Part Two of the analysis -- track by track matching!
  std::cout << "\n\nTestBeamTracks: Part Two - Track matching" << std::endl;
  
  int goodHitsT3MAPS_total = 0;
  int goodHitsT3MAPS_matchable = 0;
  int goodHitsT3MAPS_matched = 0;
  int goodHitsFEI4_total = 0;
  int goodHitsFEI4_matchable = 0;
  int goodHitsFEI4_matched = 0;
  
  // Instantiate the mapping utility:
  mapper = new MapParameters("","");
  int orientation = 0;
  mapper->setOrientation(orientation);
  // Orientation 0:
  if (orientation == 0) {
    if (options.Contains("RunII")) {
      mapper->setMapVar(1, 1.7); mapper->setMapErr(1, 0.75);//0.5);
      mapper->setMapVar(3, 13.0); mapper->setMapErr(3, 2.0);//1.0);
    }
    else {
      mapper->setMapVar(1, 2.6); mapper->setMapErr(1, 0.4);//0.2);
      mapper->setMapVar(3, 11.0); mapper->setMapErr(3, 2.0);//1.0);
    }
  }
  // Orientation 1:
  else if (orientation == 1) {
    if (options.Contains("RunII")) {
      mapper->setMapVar(1, 1.6); mapper->setMapErr(1, 0.5);
      mapper->setMapVar(3, 18.0); mapper->setMapErr(3, 1.0);
    }
    else {
      mapper->setMapVar(1, 2.8); mapper->setMapErr(1, 0.3);
      mapper->setMapVar(3, 18.0); mapper->setMapErr(3, 2.0);
    }
  }
  
  // Orientation 3:
  else if (orientation == 2) {
    if (options.Contains("RunII")) {
      mapper->setMapVar(1, 2.0); mapper->setMapErr(1, 0.6);
      mapper->setMapVar(3, 18.0); mapper->setMapErr(3, 2.0);
    }
    else {
      mapper->setMapVar(1, 3.75); mapper->setMapErr(1, 0.3);
      mapper->setMapVar(3, 18.0); mapper->setMapErr(3, 2.0);
    }
  }
  
  // Orientation 2:
  else if (orientation == 2) {
    if (options.Contains("RunII")) {
      mapper->setMapVar(1, 2.0); mapper->setMapErr(1, 0.6);
      mapper->setMapVar(3, 13.0); mapper->setMapErr(3, 2.0);
    }
    else {
      mapper->setMapVar(1, 4.0); mapper->setMapErr(1, 0.3);
      mapper->setMapVar(3, 11.0); mapper->setMapErr(3, 2.0);
    }
  }
  
  mapper->setMapExists(true);
  
  
  // Prepare FEI4 tree for loop inside T3MAPS tree's loop.
  Long64_t eventFEI4 = 0;
  cF->fChain->GetEntry(eventFEI4);
  
  // Define the map from T3MAPS <--> FEI4
  std::cout << "TestBeamTracks: Entering loop over events." << std::endl;
  for (Long64_t eventT3MAPS = 0; eventT3MAPS < entriesT3MAPS; eventT3MAPS++) {
    cT->fChain->GetEntry(eventT3MAPS);
    
    // Start quality cuts:
    // Remove T3MAPS events with 12 or more hits in one integration period.
    if ((*cT->hit_row).size() >= 12) continue;
    
    // Create list of good T3MAPS hits:
    std::vector<std::pair<int,int> > hitsInT3MAPS; hitsInT3MAPS.clear();
    for (int i_h = 0; i_h < (int)cT->hit_row->size(); i_h++) {
      // Check for masked T3MAPS pixels:
      if (!isMasked((*cT->hit_row)[i_h],(*cT->hit_column)[i_h],"T3MAPS")) {
	if ((*cT->hit_row)[i_h] > 0 && (*cT->hit_row)[i_h] < 17) {
	  
	  std::pair<int,int> newHitT3MAPS;
	  newHitT3MAPS.first = (*cT->hit_row)[i_h];
	  newHitT3MAPS.second = (*cT->hit_column)[i_h];
	  goodHitsT3MAPS_total++;
	
	  if (canMatchHit("FEI4", newHitT3MAPS)) {
	    hitsInT3MAPS.push_back(newHitT3MAPS);
	    goodHitsT3MAPS_matchable++;
	  }
	}
      }
    }
    
    // Advance position in the FEI4 tree, store good FEI4 candidates:
    std::vector<std::pair<int,int> > hitsInFEI4; hitsInFEI4.clear();
    while (cF->timestamp_start < (cT->timestamp_stop+timeOffset) &&
	   eventFEI4 < entriesFEI4) {
      
      // Exclude column 79, Row > 75, Col > 40, and masked pixels:
      if (cF->column < 80 && cF->column > 40 && cF->row < 75 &&
	  !isMasked(cF->row-1, cF->column-1, "FEI4")) {
	
	// Only consider events with timestamp inside that of T3MAPS
	if (cF->timestamp_start >= (cT->timestamp_start+timeOffset) &&
	    cF->timestamp_stop <= (cT->timestamp_stop+timeOffset)) {
	  
	  std::pair<int,int> newHitFEI4;
	  newHitFEI4.first = cF->row-1;
	  newHitFEI4.second = cF->column-1;
	  goodHitsFEI4_total++;
	
	  if (canMatchHit("T3MAPS", newHitFEI4)) {
	    hitsInFEI4.push_back(newHitFEI4);
	    goodHitsFEI4_matchable++;
	  }
	}
      }// if passes quality cuts
      
      // Then advance to the next FEI4 entry
      eventFEI4++;
      cF->fChain->GetEntry(eventFEI4);
    }// End of loop over FEI4 hits
    
    // Now have lists of T3MAPS and FEI4 hits. Check for matches.

    // Loop over FEI4 hits, see if matched in T3MAPS.
    for (int i_f = 0; i_f < (int)hitsInFEI4.size(); i_f++) {
      if (isHitMatched("T3MAPS", hitsInT3MAPS, hitsInFEI4[i_f])) {
	goodHitsFEI4_matched++;
      }
    }
    
    // Loop over T3MAPS hits, see if matched in FEI4.
    for (int i_t = 0; i_t < (int)hitsInT3MAPS.size(); i_t++) {
      if (isHitMatched("FEI4", hitsInFEI4, hitsInT3MAPS[i_t])) {
	goodHitsT3MAPS_matched++;
      }
    }
        
  }// End of loop over T3MAPS events
  std::cout << "TestBeamTracks: Ending loop over events." << std::endl;
  
  std::cout << "\nPrinting matching statistics." << std::endl;
  std::cout << "\tTotal Hits T3MAPS = " << goodHitsT3MAPS_total 
	    << "\tTotal Hits FEI4 = " << goodHitsFEI4_total << std::endl;
  std::cout << "\tT3MAPS (matched/matchable) = (" << goodHitsT3MAPS_matched
	    << " / " << goodHitsT3MAPS_matchable << " ) = "
	    << (((double)goodHitsT3MAPS_matched) / 
		((double)goodHitsT3MAPS_matchable)) << std::endl;
  
  std::cout << "\tFEI4 (matched/matchable) = (" << goodHitsFEI4_matched << " / "
	    << goodHitsFEI4_matchable << " ) = "
	    << (((double)goodHitsFEI4_matched)/((double)goodHitsFEI4_matchable))
	    << std::endl;
  
  std::cout << "\nTestBeamTracks: Finished analysis." << std::endl;
  return 0;
}
