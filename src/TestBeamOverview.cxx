////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: TestBeamOverview.cxx                                                //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 11/05/2015                                                          //
//                                                                            //
//  This program runs through the test beam data to understand general        //
//  characteristics of the data and to define quality cuts. The program also  //
//  makes a preliminary efficiency measurement based on an occupancy ratio.   //
//                                                                            //
//  Program options:                                                          //
//    "CutFullEvt" - Ignores events where T3MAPS has 10% of pixels showing    //
//                   hits for the integration period.                         //
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

int main(int argc, char **argv) {
  // Check arguments:
  if (argc < 2) {
    std::cout << "\nUsage: " << argv[0] << " <option>" << std::endl; 
    exit(0);
  }
  
  TString options = argv[1];
  TString mapFileDir = "../TestBeamOverview";
  
  // IMPORTANT JOB SETTINGS:
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
  ChipDimension *chips = new ChipDimension();
  
  // Book histograms:
  TH2D *occFEI4 = new TH2D("occFEI4", "occFEI4", 
			   chips->getNRow("FEI4"), -0.5,
			   (chips->getNRow("FEI4") - 0.5),
			   chips->getNCol("FEI4"), -0.5,
			   (chips->getNCol("FEI4") - 0.5));
  
  TH2D *occT3MAPS = new TH2D("occT3MAPS", "occT3MAPS",
			     chips->getNRow("T3MAPS"), -0.5,
			     (chips->getNRow("T3MAPS") - 0.5),
			     chips->getNCol("T3MAPS"), -0.5,
			     (chips->getNCol("T3MAPS") - 0.5));
  
  TH1F *hitPerEvtT3MAPS = new TH1F("hitPerEvtT3MAPS","hitPerEvtT3MAPS",20,0,20);
  TH1F *hitPerEvtFEI4 = new TH1F("hitPerEvtFEI4","hitPerEvtFEI4",100,0,400);
  
  TH1F *hitPerPeriodFEI4 = new TH1F("hitPerPeriodFEI4",
				    "hitPerPeriodFEI4",100,0,400);
  
  //TH1F *clusterPerEvtT3MAPS = new TH1F("clustersPerEvtT3MAPS",
  //				       "clustersPerEvtT3MAPS",20,0,20);
  //TH1F *clusterPerEvtFEI4 = new TH1F("clustersPerEvtFEI4",
  //				     "clustersPerEvtFEI4",100,0,100);
  
  TH1F *hitPerPixT3MAPS = new TH1F("hitPerPixT3MAPS",
				   "hitPerPixT3MAPS",30,0,30);
  TH1F *hitPerPixFEI4 = new TH1F("hitPerPixFEI4",
				 "hitPerPixFEI4",600,0,600);
  
  // Counters:
  int nHitsT3MAPS_total = 0;
  int nHitsFEI4_total = 0;
  
  // Store masked pixel locations:
  std::vector<std::pair<int,int> > maskFEI4; maskFEI4.clear();
  std::vector<std::pair<int,int> > maskT3MAPS; maskT3MAPS.clear();
  
  int plotNum = 0;
  // Loop over T3MAPS tree.
  Long64_t entriesT3MAPS = cT->fChain->GetEntries();
  std::cout << "TestBeamOverview: T3MAPS entries = " << entriesT3MAPS
	    << std::endl;
  for (Long64_t eventT3MAPS = 0; eventT3MAPS < entriesT3MAPS; eventT3MAPS++) {
    
    cT->fChain->GetEntry(eventT3MAPS);
    
    int hitsInT3MAPS = 0;
    for (int i_h = 0; i_h < (int)cT->hit_row->size(); i_h++) {
      occT3MAPS->Fill((*cT->hit_row)[i_h], (*cT->hit_column)[i_h]);
      nHitsT3MAPS_total++;
      hitsInT3MAPS++;
    }    
    hitPerEvtT3MAPS->Fill(hitsInT3MAPS);
    
    if (hitsInT3MAPS >= 5) {
      plotNum++;
      TH2D *evtPlotT3MAPS = new TH2D("evtPlotT3MAPS", "evtPlotT3MAPS",
				     chips->getNRow("T3MAPS"), -0.5,
				     (chips->getNRow("T3MAPS") - 0.5),
				     chips->getNCol("T3MAPS"), -0.5,
				     (chips->getNCol("T3MAPS") - 0.5));
      for (int i_h = 0; i_h < (int)cT->hit_row->size(); i_h++) {
	evtPlotT3MAPS->Fill((*cT->hit_row)[i_h], (*cT->hit_column)[i_h]);
      }
      PlotUtil::plotTH2D(evtPlotT3MAPS, "row_{T3MAPS}", "column_{T3MAPS}", "hits", Form("../TestBeamOutput/TestBeamOverview/evtPlot%dT3MAPS",plotNum));
      std::cout << "Plot number " << plotNum << " has timestamps:" << std::endl;
      std:: cout << "Begin: " << std::setprecision(12) << cT->timestamp_start
		 << "\tEnd: " << std::setprecision(12) << cT->timestamp_stop
		 << std::endl;
    }  
  }// end of T3MAPS loop
  
  // Counters and histograms for the FEI4 tree loop:
  int currEvtFEI4 = 0;
  int hitsInFEI4 = 0;
  int hitsInPeriod = 0;
  double previousTime = 0;  
  TH2D *matchEvtFEI4 = new TH2D("matchEvtFEI4", "matchEvtFEI4",
				chips->getNRow("FEI4"), -0.5,
				(chips->getNRow("FEI4") - 0.5),
				chips->getNCol("FEI4"), -0.5,
				(chips->getNCol("FEI4") - 0.5));
  
  // Loop over FEI4 tree:
  Long64_t entriesFEI4 = cF->fChain->GetEntries();
  std::cout << "TestBeamOverview: FEI4 entries = " << entriesFEI4 << std::endl;
  for (Long64_t eventFEI4 = 0; eventFEI4 < entriesFEI4; eventFEI4++) {
    
    cF->fChain->GetEntry(eventFEI4);
    
    if (cF->event_number != currEvtFEI4) {
      hitPerEvtFEI4->Fill(hitsInFEI4);
      hitsInFEI4 = 0;
    }
    else {
      hitsInFEI4++;
    }
    
    // integration time with +/- 1 second window
    if (cF->timestamp_start > 1430686885 && cF->timestamp_stop < 1430686888) {
      matchEvtFEI4->Fill(cF->row-1, cF->column-1);
    }
    
    //TH1F *hitPerPeriodFEI4 = new TH1F("hitPerPeriodFEI4",
    if (cF->timestamp_start >= previousTime && 
	cF->timestamp_stop < previousTime + integrationTime) {
      hitsInPeriod++;
    }
    else {
      hitPerPeriodFEI4->Fill(hitsInPeriod);
      previousTime = cF->timestamp_start;
      hitsInPeriod = 0;
    }
    
    // Fill FEI4 occupancy plot:
    occFEI4->Fill(cF->row-1, cF->column-1);
    nHitsFEI4_total++;
  }
  
  std::cout << "TestBeamOverview: Ending loops over data." << std::endl;
  
  // Save the BusyT3MAPS pixels:
  ofstream fBusyFEI4;
  ofstream fBusyT3MAPS;
  if (options.Contains("RunII")) {
    fBusyT3MAPS.open("../TestBeamOutput/TestBeamOverview/busyT3MAPS_RunII.txt");
    fBusyFEI4.open("../TestBeamOutput/TestBeamOverview/busyFEI4_RunII.txt");
  }
  else {
    fBusyT3MAPS.open("../TestBeamOutput/TestBeamOverview/busyT3MAPS_RunI.txt");
    fBusyFEI4.open("../TestBeamOutput/TestBeamOverview/busyFEI4_RunI.txt");
  }
  
  // Fill the FEI4 hit per pixel plots, get mask list:
  for (int i_r = 1; i_r <= chips->getNRow("FEI4"); i_r++) {
    for (int i_c = 1; i_c <= chips->getNCol("FEI4"); i_c++) {
      int currNHits = (int)occFEI4->GetBinContent(i_r, i_c);
      hitPerPixFEI4->Fill(currNHits);
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
      int currNHits = (int)occT3MAPS->GetBinContent(i_r, i_c);
      hitPerPixT3MAPS->Fill(currNHits);
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
  
  // Start plotting the results:
  PlotUtil::plotTH2D(occFEI4, "row_{FEI4}", "column_{FEI4}", "hits", "../TestBeamOutput/TestBeamOverview/occupancyFEI4");
  PlotUtil::plotTH2D(occT3MAPS, "row_{T3MAPS}", "column_{T3MAPS}", "hits", "../TestBeamOutput/TestBeamOverview/occupancyT3MAPS");
  PlotUtil::plotTH1F(hitPerEvtFEI4, "hits per event", "entries", "../TestBeamOutput/TestBeamOverview/hitsPerEvtFEI4", true);
  PlotUtil::plotTH1F(hitPerEvtT3MAPS, "hits per event", "entries", "../TestBeamOutput/TestBeamOverview/hitsPerEvtT3MAPS", true);
  PlotUtil::plotTH1F(hitPerPeriodFEI4, "hits per integration period", "entries", "../TestBeamOutput/TestBeamOverview/hitsPerPeriodFEI4", true);
  
  //PlotUtil::plotTH1F(clusterPerEvtT3MAPS, "clusters per event", "entries", "../TestBeamOutput/TestBeamOverview/clustersPerEvtT3MAPS");
  //PlotUtil::plotTH1F(clusterPerEvtFEI4, "clusters per event", "entries", "../TestBeamOutput/TestBeamOverview/clustersPerEvtFEI4");
  
  PlotUtil::plotTH1F(hitPerPixT3MAPS, "hits per pixel", "entries", "../TestBeamOutput/TestBeamOverview/hitsPerPixT3MAPS", true);
  PlotUtil::plotTH1F(hitPerPixFEI4, "hits per pixel", "entries", "../TestBeamOutput/TestBeamOverview/hitsPerPixFEI4", true);
  
  PlotUtil::plotTH2D(matchEvtFEI4, "row_{FEI4}", "column_{FEI4}", "matched hits", "../TestBeamOutput/TestBeamOverview/matchedInFEI4");
  matchEvtFEI4->Fill(cF->row-1, cF->column-1);

  // Then print counters:
  std::cout << "\nTestBeamOverview: Printing hit counters." << std::endl;
  std::cout << "\tT3MAPS total = " << nHitsT3MAPS_total << std::endl;
  std::cout << "\tFEI4 total = " << nHitsFEI4_total << std::endl;
  std::cout << "The T3MAPS mean hits/integration is "
	    <<  hitPerEvtT3MAPS->GetMean() << std::endl;
  std::cout << "The FEI4 mean hits/integration is "
	    << hitPerPeriodFEI4->GetMean() << std::endl;




  //**********//
  // Start Part Two of the analysis, with cuts implemented:
  std::cout << "\n\nTestBeamOverview: Part Two - Apply Cuts" << std::endl;

  TH2D *cutOccFEI4 = new TH2D("cutOccFEI4", "cutOccFEI4", 
			      chips->getNRow("FEI4"), -0.5,
			      (chips->getNRow("FEI4") - 0.5),
			      chips->getNCol("FEI4"), -0.5,
			      (chips->getNCol("FEI4") - 0.5));
  TH2D *cutOccT3MAPS = new TH2D("cutOccT3MAPS", "cutOccT3MAPS", 
				chips->getNRow("T3MAPS"), -0.5,
				(chips->getNRow("T3MAPS") - 0.5),
				chips->getNCol("T3MAPS"), -0.5,
				(chips->getNCol("T3MAPS") - 0.5));
  int nPassCutsT3MAPS = 0;
  int nPassCutsFEI4 = 0;
  
  // Store live time info for T3MAPS:
  int nIntegrationPeriods = 0;
  double startTime = 0;
  double stopTime = 0;

  // Loop over T3MAPS tree, again.
  std::cout << "TestBeamOverview: Second loop over T3MAPS." << std::endl;
  for (Long64_t eventT3MAPS = 0; eventT3MAPS < entriesT3MAPS; eventT3MAPS++) {
    cT->fChain->GetEntry(eventT3MAPS);
    
    // Remove events with 12 or more hits in one integration period.
    if ((*cT->hit_row).size() >= 12) continue;

    if (nIntegrationPeriods == 0) startTime = cT->timestamp_start; 
    stopTime = cT->timestamp_stop;
    nIntegrationPeriods++;
    
    // Loop over hits in the current T3MAPS integration period:
    for (int i_h = 0; i_h < (int)cT->hit_row->size(); i_h++) {
      
      // Cut masked pixels inside loop over hits:
      bool maskCut = false;
      for (int i_c = 0; i_c < (int)maskT3MAPS.size(); i_c++) {
	if (maskT3MAPS[i_c].first == (*cT->hit_row)[i_h] &&
	    maskT3MAPS[i_c].second == (*cT->hit_column)[i_h]) {
	  maskCut = true;
	  break;
	}
      }
      if (maskCut) continue;
      
      cutOccT3MAPS->Fill((*cT->hit_row)[i_h], (*cT->hit_column)[i_h]);
      nPassCutsT3MAPS++;
    }// End of loop over T3MAPS hits in each event
  }// End of loop over T3MAPS events
  
   // Loop over FEI4 tree, again:
  std::cout << "TestBeamOverview: Second loop over FEI4." << std::endl;
  for (Long64_t eventFEI4 = 0; eventFEI4 < entriesFEI4; eventFEI4++) {
    cF->fChain->GetEntry(eventFEI4);
    
    // Exclude noisy column 79. 
    if (cF->column >= 80) continue;
    
    // Only match hits with Row > 75, Col > 40
    if (cF->column <= 40) continue;
    if (cF->row >= 75) continue;
    
    // Cut masked channels:
    bool maskCut = false;
    for (int i_c = 0; i_c < (int)maskFEI4.size(); i_c++) {
      if (maskFEI4[i_c].first == (cF->row-1) &&
	  maskFEI4[i_c].second == (cF->column-1) ) {
	maskCut = true;
	break;
      }
    }
    if (maskCut) continue;
    
    // Fill FEI4 occupancy plot:
    cutOccFEI4->Fill(cF->row-1, cF->column-1);
    nPassCutsFEI4++;
  }// End of loop over FEI4 events.
  
  std::cout << "TestBeamOverview: Finished analysis." << std::endl;
  std::cout << "T3MAPS hits passing cuts = " << nPassCutsT3MAPS << " \t"
	    <<  100.0 * ((double)nPassCutsT3MAPS) / ((double)nHitsT3MAPS_total)
	    << "%" << std::endl;
  std::cout << "FEI4 hits passing cuts = " << nPassCutsFEI4 << " \t"
	    <<  100.0 * ((double)nPassCutsFEI4) / ((double)nHitsFEI4_total)
	    << "%" << std::endl;
  
  int totalPixT3MAPS = chips->getNRow("T3MAPS") * chips->getNCol("T3MAPS");
  //int totalPixFEI4 = chips->getNRow("FEI4") * chips->getNCol("FEI4");
  // Must account for the fact that much of the chip is ignored.
  int totalPixFEI4 = (75 * (chips->getNCol("FEI4")-41));

  //int usedPixFEI4 = totalPixFEI4 - ((int)maskFEI4.size());
  int usedPixFEI4 = totalPixFEI4;
  int usedPixT3MAPS = totalPixT3MAPS - ((int)maskT3MAPS.size());
  double meanHitsPerGoodPixT3MAPS = (((double)nPassCutsT3MAPS) /
			       ((double)usedPixT3MAPS));
  double meanHitsPerGoodPixFEI4 = (((double)nPassCutsFEI4) /
				   ((double)usedPixFEI4));
  std::cout << "\nT3MAPS mean hits/good pixel = " << meanHitsPerGoodPixT3MAPS
	    << std::endl;
  std::cout << "FEI4 mean hits/good pixel = " << meanHitsPerGoodPixFEI4
	    << std::endl;
  
  // Calculate live time fraction for T3MAPS:  
  double liveFraction = (((double)nIntegrationPeriods * integrationTime) /
			 (stopTime - startTime));
  std::cout << "\nT3MAPS live time fraction = " << liveFraction << std::endl;
  
  // Calculate ratio of FEI4 to T3MAPS size. 
  double pixSizeT3MAPS = (chips->getColPitch("T3MAPS") *
			  chips->getRowPitch("T3MAPS"));
  double pixSizeFEI4 = (chips->getColPitch("FEI4") * 
			chips->getRowPitch("FEI4"));
  double ratioFEI4OverT3MAPS = (pixSizeFEI4 / pixSizeT3MAPS);
  std::cout << "FEI4 pixel size / T3MAPS pixel size = " << ratioFEI4OverT3MAPS 
	    << std::endl;
  
  // Calculate the expected T3MAPS occupancy:
  double expOccT3MAPS = meanHitsPerGoodPixFEI4 * liveFraction * (1.0 / ratioFEI4OverT3MAPS);
  std::cout << "Expect " << expOccT3MAPS
	    << " hits per pixel in T3MAPS for 100% efficiency." << std::endl;
  std::cout << "Observe " << (100 * meanHitsPerGoodPixT3MAPS / expOccT3MAPS)
	    << "% efficiency." << std::endl;
  
  PlotUtil::plotTH2D(cutOccFEI4, "row_{FEI4}", "column_{FEI4}", "hits", "../TestBeamOutput/TestBeamOverview/cutOccupancyFEI4");
  PlotUtil::plotTH2D(cutOccT3MAPS, "row_{T3MAPS}", "column_{T3MAPS}", "hits", "../TestBeamOutput/TestBeamOverview/cutOccupancyT3MAPS");
}
