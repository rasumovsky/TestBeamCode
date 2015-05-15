////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: TestBeamStudies.cxx                                                 //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 09/02/2015                                                          //
//                                                                            //
//  This class loads the FE-I4 and T3MAPS data structures and then correlates //
//  hits between the two chips for an efficiency test.                        //
//                                                                            //
//  Need to include plotting utility. This will also require implementation   //
//  of SetAtlasStyle(); Perhaps it would be useful to create a plotting class.//
//                                                                            //
//  Options:                                                                  //
//    "RunI", "RunII", "NoScan"                                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// C++ includes:
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
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

// Store masked pixel locations:
std::vector<std::pair<int,int> > maskFEI4;// maskFEI4.clear();
std::vector<std::pair<int,int> > maskT3MAPS;// maskT3MAPS.clear();

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
   Get the fourier transform of a 1D histogram and isolate the magnitude
   associated with a particular frequency.
   @param h - the TH1F 1D histogram object
   @param frequency - the frequency (in Hz) to isolate.
   @returns - the magnitude of the FFT for the specified frequency.
*/
double GetFFTValue(TH1F *h, double frequency) {
  //double intervalSize;
  int n = h->GetNbinsX()-1;
  TH1 *hm = 0; 
  TVirtualFFT::SetTransform(0);
  hm = h->FFT(hm, "MAG");
  hm->SetTitle("magnitude of the 1st FFT");
  hm->Scale(1.0/hm->Integral());
  double position = (frequency *
		     (h->GetXaxis()->GetXmax() - h->GetXaxis()->GetXmin()));
  int i_b = 0;
  for (i_b = 1; i_b <= hm->GetNbinsX(); i_b++) {
    if ((hm->GetBinCenter(i_b)+(0.5*hm->GetBinWidth(i_b)) > position) &&
	(hm->GetBinCenter(i_b)-(0.5*hm->GetBinWidth(i_b)) <= position)) {
      break;
    }
  }
  double magnitude = hm->GetBinContent(i_b);
  return magnitude;
}

/**
   The main method of TestBeamStudies scans the time offset between the two
   chips (FEI4 and T3MAPS) and produces plots to help identify the mapping.
   @param options - "RunI" or "RunII" to specify the dataset, or "NoScan" to 
   avoid scanning all of the different time offsets for the chip clocks.
 */
int main(int argc, char **argv) {
  // Check arguments:
  if (argc < 2) {
    std::cout << "\nUsage: " << argv[0] << " <options>" << std::endl; 
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
  // For Fourier analysis:
  double frequency = options.Contains("RunII") ? 2.0 : 2.6;
  
  // Settings for the time offset and offset scan:
  double measuredOffset = 0.670;
  double timeOffsetMin = -5.0;
  double timeOffsetMax = 5.0;
  double timeOffsetInterval = 0.1;
  int nTimeBins = (int)((timeOffsetMax - timeOffsetMin)/timeOffsetInterval);
  
  // Clear mask lists:
  maskFEI4.clear();
  maskT3MAPS.clear();
  
  // Set the plot style:
  PlotUtil::setAtlasStyle();
  
  // Load the T3MAPS TTree:
  TFile *fileT3MAPS = new TFile(inputT3MAPS);
  TTree *myTreeT3MAPS = (TTree*)fileT3MAPS->Get("TreeT3MAPS");
  TreeT3MAPS *cT = new TreeT3MAPS(myTreeT3MAPS);
  
  // Load the FEI4 TTree:
  TFile *fileFEI4 = new TFile(inputFEI4);
  TTree *myTreeFEI4 = (TTree*)fileFEI4->Get("Table");
  TreeFEI4 *cF = new TreeFEI4(myTreeFEI4);
  
  // Load the chip sizes (but use defaults!)
  ChipDimension *chips = new ChipDimension();
  
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
    
  //----------------------------------------//
  // Loop over trees initially to identify hot pixels and mask them.
  // Loop over T3MAPS tree:
  Long64_t entriesT3MAPS = cT->fChain->GetEntries();
  std::cout << "TestBeamStudies: T3MAPS entries = " << entriesT3MAPS
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
    fBusyT3MAPS.open("../TestBeamOutput/TestBeamStudies/busyT3MAPS_RunII.txt");
    fBusyFEI4.open("../TestBeamOutput/TestBeamStudies/busyFEI4_RunII.txt");
  }
  else {
    fBusyT3MAPS.open("../TestBeamOutput/TestBeamStudies/busyT3MAPS_RunI.txt");
    fBusyFEI4.open("../TestBeamOutput/TestBeamStudies/busyFEI4_RunI.txt");
  }
  
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
  std::cout << "TestBeamStudies: Found pixels to mask: " << maskT3MAPS.size() 
	    << " in T3MAPS and " << maskFEI4.size() << " in FEI4." << std::endl;
  
  //----------------------------------------//
  // Initialize histograms, counters, and graphs for mapping & scanning:
  // Book histograms:
  TH2D *occFEI4 = new TH2D("occFEI4", "occFEI4", 
			   chips->getNRow("FEI4"), -0.5,
			   (chips->getNRow("FEI4") - 0.5),
			   chips->getNCol("FEI4"), -0.5,
			   (chips->getNCol("FEI4") - 0.5));
  TH2D *occOverlapFEI4 = new TH2D("occOverlapFEI4", "occOverlapFEI4",
				  chips->getNRow("FEI4"), -0.5,
				  (chips->getNRow("FEI4") - 0.5),
				  chips->getNCol("FEI4"), -0.5,
				  (chips->getNCol("FEI4") - 0.5));
  TH2D *occExcludeFEI4 = new TH2D("occExcludeFEI4", "occExcludeFEI4",
				  chips->getNRow("FEI4"), -0.5,
				  (chips->getNRow("FEI4") - 0.5),
				  chips->getNCol("FEI4"), -0.5,
				  (chips->getNCol("FEI4") - 0.5));
  TH2D *occT3MAPS = new TH2D("occT3MAPS", "occT3MAPS",
			     chips->getNRow("T3MAPS"), -0.5,
			     (chips->getNRow("T3MAPS") - 0.5),
			     chips->getNCol("T3MAPS"), -0.5,
			     (chips->getNCol("T3MAPS") - 0.5));
  
  // Hit counters:
  int nHitsT3MAPS_noCuts = 0;
  int nHitsT3MAPS_afterCuts = 0;
  int nHitsFEI4_total = 0;
  int nHitsFEI4_overlapping = 0;
  int nHitsFEI4_excluding = 0;
  
  // Graphs of map parameters and errors for timing scan analysis:
  TGraph *graphMapVar[4][4];
  TGraph *graphMapErr[4][4];
  TGraph *graphDiffMax[4];
  for (int i_h = 0; i_h < 4; i_h++) {
    for (int i_p = 0; i_p < 4; i_p++) {
      graphMapVar[i_h][i_p] = new TGraph();
      graphMapErr[i_h][i_p] = new TGraph();
    }
    graphDiffMax[i_h] = new TGraph();
  }
  TH1F *histMax = new TH1F("histMax", "histMax", 50, 0.0, 0.003);
  TH1F *histDev = new TH1F("histMax", "histMax", 100, -0.003, 0.003);
  
  // Graphs for Fast Fourier Transforms:
  TH1F *hTime[4][MapParameters::nRBin][MapParameters::nCBin];
  for (int i_h = 0; i_h < 4; i_h++) {
    for (int i_x = 0; i_x < MapParameters::nRBin; i_x++) {
      for (int i_y = 0; i_y < MapParameters::nCBin; i_y++) {
	hTime[i_h][i_x][i_y] = new TH1F(Form("hTime_%d_%d_%d",i_h,i_x,i_y),Form("hTime_%d_%d_%d",i_h,i_x,i_y),nTimeBins,timeOffsetMin,timeOffsetMax);
      }
    }
  }
  
  //----------------------------------------//
  // Loop over scan timing offsets, or consider measured time offset:
  int graphPoint = 0;
  double timeOffset = timeOffsetMin;
  // Only consider the measured offset if not doing a scan:
  if (options.Contains("NoScan")) {
    timeOffset = measuredOffset;
    timeOffsetMax = measuredOffset;
  }
  std::cout << "TestBeamStudies: Beginning loop over time offset." << std::endl;
  while (timeOffset <= timeOffsetMax) {
    std::cout << "TestBeamStudies: timeOffset=" << timeOffset << std::endl;    
    
    // Instantiate the mapping utility:
    MapParameters *mapper = new MapParameters("","");
    
    // Prepare FEI4 tree for loop inside T3MAPS tree's loop.
    Long64_t eventFEI4 = 0;
    cF->fChain->GetEntry(eventFEI4);
        
    // Define the map from T3MAPS <--> FEI4
    std::cout << "TestBeamStudies: Entering loop to define maps." << std::endl;
    for (Long64_t eventT3MAPS = 0; eventT3MAPS < entriesT3MAPS; eventT3MAPS++) {
      cT->fChain->GetEntry(eventT3MAPS);
            
      // For map definition, cut on events with no T3MAPS hits:
      //if (cT->nHits == 0) continue;
      
      // Add to total hit counter:
      if (graphPoint == 0) nHitsT3MAPS_noCuts += cT->nHits;
      
      // Start quality cuts:
      // Remove T3MAPS events with 12 or more hits in one integration period.
      if ((*cT->hit_row).size() >= 12) continue;
      
      // Create list of GOOD T3MAPS hits:
      std::vector<std::pair<int,int> > hitsInT3MAPS; hitsInT3MAPS.clear();
      for (int i_h = 0; i_h < (int)cT->hit_row->size(); i_h++) {
	// Check for masked T3MAPS pixels:
	if (!isMasked((*cT->hit_row)[i_h],(*cT->hit_column)[i_h],"T3MAPS")) {
	  if ((*cT->hit_row)[i_h] > 0 && (*cT->hit_row)[i_h] < 17) {
	    std::pair<int,int> newHitT3MAPS;
	    newHitT3MAPS.first = (*cT->hit_row)[i_h];
	    newHitT3MAPS.second = (*cT->hit_column)[i_h];
	    hitsInT3MAPS.push_back(newHitT3MAPS);
	    if (graphPoint == 0) {
	      occT3MAPS->Fill((*cT->hit_row)[i_h], (*cT->hit_column)[i_h]);
	    }
	    nHitsT3MAPS_afterCuts++;
	  }
	}
      }
            
      // Advance position in the FEI4 tree:
      while (cF->timestamp_start < (cT->timestamp_stop+timeOffset) &&
	     eventFEI4 < entriesFEI4) {
		
	// Exclude column 79, Row > 75, Col > 40, and masked pixels:
	if (cF->column < 80 && cF->column > 40 && cF->row < 75 &&
	    !isMasked(cF->row-1, cF->column-1, "FEI4")) {
	  
	  PixelHit *currFEI4Hit = new PixelHit(cF->row-1, cF->column-1,
					       cF->LVL1ID, cF->tot, false);
	  
	  // Fill FEI4 occupancy plot:
	  if (graphPoint == 0) {
	    occFEI4->Fill(currFEI4Hit->getRow(), currFEI4Hit->getCol());
	    nHitsFEI4_total++;
	  }
	  
	  // Only consider events with timestamp inside that of T3MAPS
	  if (cF->timestamp_start >= (cT->timestamp_start+timeOffset) &&
	      cF->timestamp_stop <= (cT->timestamp_stop+timeOffset)) {
	    
	    if (cT->nHits > 0) {

	      // Fill overlapping FEI4 hit occupancy plot:
	      if (graphPoint == 0) { 
		nHitsFEI4_overlapping++;
		occOverlapFEI4->Fill(currFEI4Hit->getRow(),
				     currFEI4Hit->getCol());
	      }
	      
	      // Loop over good T3MAPS hits:
	      for (int i_h = 0; i_h < (int)hitsInT3MAPS.size(); i_h++) {
		PixelHit *currT3MAPSHit = new PixelHit(hitsInT3MAPS[i_h].first,
						       hitsInT3MAPS[i_h].second,
						       1, 1, false);
		mapper->addPairToMap(currFEI4Hit, currT3MAPSHit);
		delete currT3MAPSHit;
	      }
	    }
	    // If timestamps don't match up, use as background estimate:
	    else {	
	      if (graphPoint == 0) { 
		nHitsFEI4_excluding++;
		occExcludeFEI4->Fill(currFEI4Hit->getRow(),
				     currFEI4Hit->getCol());
	      }
	      // Loop over possible T3MAPS hits:
	      
	      for (int i_r = 0; i_r < chips->getNRow("T3MAPS"); i_r++) {
		for (int i_c = 0; i_c < chips->getNCol("T3MAPS"); i_c++) {
		  PixelHit *missingT3MAPSHit = new PixelHit(i_r,i_c,1,1,false);
		  mapper->addPairToBkg(currFEI4Hit, missingT3MAPSHit);
		  delete missingT3MAPSHit;
		}
	      }
	    }
	  }
	  delete currFEI4Hit;
	}// if passes quality cuts
	
	// Then advance to the next FEI4 entry
	eventFEI4++;
	cF->fChain->GetEntry(eventFEI4);
	
      }// End of loop over FEI4 hits
    }// End of loop over T3MAPS events
    std::cout << "TestBeamStudies: Ending loop to define maps." << std::endl;
    
    mapper->createMapFromHits();
    if (options.Contains("NoScan")) {
      mapper->saveMapParameters("../TestBeamOutput");
      mapper->setOrientation(3);// Use what I think is correct orientation.
      mapper->printMapParameters();
    }
    
    // Loop over 4 orientations:
    for (int i_h = 0; i_h < 4; i_h++) {
      mapper->setOrientation(i_h);
      for (int i_p = 0; i_p < 4; i_p++) {
	graphMapVar[i_h][i_p]->SetPoint(graphPoint, timeOffset,
					mapper->getMapVar(i_p));
	graphMapErr[i_h][i_p]->SetPoint(graphPoint, timeOffset, 
					mapper->getMapErr(i_p));
      }

      TH2D *tempDiffHist = (TH2D*)mapper->getParamPlot("diff");
      graphDiffMax[i_h]->SetPoint(graphPoint, timeOffset,
				  tempDiffHist->GetMaximum()); 
      histMax->Fill(tempDiffHist->GetMaximum());
      
      for (int i_x = 1; i_x <= tempDiffHist->GetNbinsX(); i_x++) {
	for (int i_y = 1; i_y <= tempDiffHist->GetNbinsY(); i_y++) {
	  double diffVal = tempDiffHist->GetBinContent(i_x,i_y);
	  histDev->Fill(diffVal);
	  hTime[i_h][i_x-1][i_y-1]->SetBinContent(timeOffset, diffVal);
	}
      }
      tempDiffHist->Delete();
    }
    std::cout << "End check" << std::endl;
    graphPoint++;
    delete mapper;
    timeOffset+=timeOffsetInterval;
  
  }// End of timing scan.
  std::cout << "TestBeamStudies: Finished timing scan." << std::endl;
  
  //----------------------------------------//
  // Post-scan analysis:
  
  // Plot occupancy for FEI4 and T3MAPS:
  PlotUtil::plotTH2D(occFEI4, "row_{FEI4}", "column_{FEI4}", "hits", "../TestBeamOutput/TestBeamStudies/occupancyFEI4");
  PlotUtil::plotTH2D(occOverlapFEI4, "row_{FEI4}", "column_{FEI4}", "hits", "../TestBeamOutput/TestBeamStudies/occupancyOverlappingFEI4");
  PlotUtil::plotTH2D(occExcludeFEI4, "row_{FEI4}", "column_{FEI4}", "hits", "../TestBeamOutput/TestBeamStudies/occupancyExcludingFEI4");
  PlotUtil::plotTH2D(occT3MAPS, "row_{T3MAPS}", "column_{T3MAPS}", "hits", "../TestBeamOutput/TestBeamStudies/occupancyT3MAPS");
  
  // Plot deviations in map parameter histograms to find significant points:
  PlotUtil::plotTH1F(histMax, "maximum value of (s-b)", "entries", "../TestBeamOutput/TestBeamStudies/histMax");
  PlotUtil::plotTH1F(histDev, "(s-b)", "entries", "../TestBeamOutput/TestBeamStudies/histDeviations", true);
  
  // Plots only for the scan analysis:
  if (!options.Contains("NoScan")) {
    // Plots of map parameters for all orientations of the chips:
    for (int i_h = 0; i_h < 4; i_h++) {
      for (int i_p = 0; i_p < 4; i_p++) {
	PlotUtil::plotTGraph(graphMapVar[i_h][i_p], "time offset [s]", Form("parameter %d",i_p), Form("../TestBeamOutput/TestBeamStudies/mapVal%d_orient%d",i_p,i_h));
	PlotUtil::plotTGraph(graphMapErr[i_h][i_p], "time offset [s]", Form("error %d",i_p), Form("../TestBeamOutput/TestBeamStudies/mapErr%d_orient%d",i_p,i_h));
      }
      PlotUtil::plotTGraph(graphDiffMax[i_h], "time offset [s]", "Difference Maximum", Form("../TestBeamOutput/TestBeamStudies/mapDiffMax_orient%d",i_h));
    }
  }
  
  // Make subtraction plot:
  occOverlapFEI4->Scale(1.0/occOverlapFEI4->Integral());
  occExcludeFEI4->Scale(1.0/occExcludeFEI4->Integral());
  TH2D *occDiffFEI4 = new TH2D("occDiffFEI4", "occDiffFEI4",
			       (int)(((double)chips->getNRow("FEI4"))/8.0),
			       -0.5, (chips->getNRow("FEI4") - 0.5),
			       (int)(((double)chips->getNCol("FEI4"))/8.0),
			       -0.5, (chips->getNCol("FEI4") - 0.5));
  for (int i_x = 1; i_x <= occOverlapFEI4->GetNbinsX(); i_x++) {
    for (int i_y = 1; i_y <= occOverlapFEI4->GetNbinsY(); i_y++) {
      double value = (occOverlapFEI4->GetBinContent(i_x,i_y) -
		      occExcludeFEI4->GetBinContent(i_x,i_y));
      //occDiffFEI4->SetBinContent(i_x, i_y, value);
      occDiffFEI4->Fill(occOverlapFEI4->GetXaxis()->GetBinCenter(i_x),
			occOverlapFEI4->GetYaxis()->GetBinCenter(i_y), value);
    }
  }
  
  PlotUtil::plotTH2D(occDiffFEI4, "row_{FEI4}", "column_{FEI4}", "hits", "../TestBeamOutput/TestBeamStudies/occupancyDifferenceFEI4");
    
  // Then print counters:
  std::cout << "TestBeamStudies: Printing hit counters." << std::endl;
  std::cout << "\tT3MAPS before cuts = " << nHitsT3MAPS_noCuts << std::endl;
  std::cout << "\tT3MAPS after cuts = " << nHitsT3MAPS_afterCuts << std::endl;
  std::cout << "\tFEI4 total = " << nHitsFEI4_total << std::endl;
  std::cout << "\tFEI4 overlapping = " << nHitsFEI4_overlapping << std::endl;
  std::cout << "\tFEI4 excluding = " << nHitsFEI4_excluding << std::endl;
  
  // End of analysis.
  std::cout << "TestBeamStudies: Finished analysis." << std::endl;
  return 0;
}
