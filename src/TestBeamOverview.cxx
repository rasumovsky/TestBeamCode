////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: TestBeamOverview.cxx                                                //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 11/05/2015                                                          //
//                                                                            //
//  
//    "CutFullEvt" - Ignores events where T3MAPS has 10% of pixels showing    //
//                   hits for the integration period.                         //
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
  if (argc < 4) {
    std::cout << "\nUsage: " << argv[0] << " <inputT3MAPS> <inputFEI4> <option>"
	      << std::endl; 
    exit(0);
  }
  
  TString inputT3MAPS = argv[1];
  TString inputFEI4 = argv[2];
  TString options = argv[3];
  TString mapFileDir = "../TestBeamOverview";
  
  // IMPORTANT JOB SETTINGS:
  int noiseThresholdFEI4 = 300;
  int noiseThresholdT3MAPS = 15;
  double integrationTime = 0.5;
  
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
  TH1F *hitPerEvtFEI4 = new TH1F("hitPerEvtFEI4","hitPerEvtFEI4",100,0,100);
  
  TH1F *hitPerPeriodFEI4 = new TH1F("hitPerPeriodFEI4",
				    "hitPerPeriodFEI4",100,0,100);
  
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
    
    if (hitsInT3MAPS >= 6) {
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
      std:: cout << "Begin: " << std::setprecision(11) << cT->timestamp_start
		 << "\tEnd: " << std::setprecision(11) << cT->timestamp_stop
		 << std::endl;
    }  
  }
  
  int currEvtFEI4 = 0;
  int hitsInFEI4 = 0;
  int hitsInPeriod = 0;
  double previousTime = 0;
  
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
  std::vector<int> BusyFEI4Rows; BusyFEI4Rows.clear();
  std::vector<int> BusyFEI4Cols; BusyFEI4Cols.clear();
  std::vector<int> BusyT3MAPSRows; BusyT3MAPSRows.clear();
  std::vector<int> BusyT3MAPSCols; BusyT3MAPSCols.clear();
  
  // Fill the FEI4 hit per pixel plots:
  for (int i_r = 1; i_r <= chips->getNRow("FEI4"); i_r++) {
    for (int i_c = 1; i_c <= chips->getNCol("FEI4"); i_c++) {
      int currNHits = (int)occFEI4->GetBinContent(i_r, i_c);
      hitPerPixFEI4->Fill(currNHits);
      if (currNHits > noiseThresholdFEI4) {
	BusyFEI4Rows.push_back(i_r);
	BusyFEI4Cols.push_back(i_c);
      }
    }
  }
  
  // Fill the T3MAPS hit per pixel plots:
  for (int i_r = 1; i_r <= chips->getNRow("T3MAPS"); i_r++) {
    for (int i_c = 1; i_c <= chips->getNCol("T3MAPS"); i_c++) {
      int currNHits = (int)occT3MAPS->GetBinContent(i_r, i_c);
      hitPerPixT3MAPS->Fill(currNHits);
      if (currNHits > noiseThresholdT3MAPS) {
	BusyT3MAPSRows.push_back(i_r);
	BusyT3MAPSCols.push_back(i_c);
      }
    }
  }
  
  // Start plotting the results:
  PlotUtil::plotTH2D(occFEI4, "row_{FEI4}", "column_{FEI4}", "hits", "../TestBeamOutput/TestBeamOverview/occupancyFEI4");
  PlotUtil::plotTH2D(occT3MAPS, "row_{T3MAPS}", "column_{T3MAPS}", "hits", "../TestBeamOutput/TestBeamOverview/occupancyT3MAPS");
  PlotUtil::plotTH1F(hitPerEvtFEI4, "hits per event", "entries", "../TestBeamOutput/TestBeamOverview/hitsPerEvtFEI4", true);
  PlotUtil::plotTH1F(hitPerEvtT3MAPS, "hits per event", "entries", "../TestBeamOutput/TestBeamOverview/hitsPerEvtT3MAPS", true);
  PlotUtil::plotTH1F(hitPerPeriodFEI4, "hits per integration period", "entries", "../TestBeamOutput/TestBeamOverview/hitsPerPeriodFEI4", true);
  
  std::cout << "The T3MAPS mean hits/integration is "
	    <<  hitPerEvtT3MAPS->GetMean() << std::endl;
  std::cout << "The FEI4 mean hits/integration is "
	    << hitPerPeriodFEI4->GetMean() << std::endl;


  //PlotUtil::plotTH1F(clusterPerEvtT3MAPS, "clusters per event", "entries", "../TestBeamOutput/TestBeamOverview/clustersPerEvtT3MAPS");
  //PlotUtil::plotTH1F(clusterPerEvtFEI4, "clusters per event", "entries", "../TestBeamOutput/TestBeamOverview/clustersPerEvtFEI4");
  
  PlotUtil::plotTH1F(hitPerPixT3MAPS, "hits per pixel", "entries", "../TestBeamOutput/TestBeamOverview/hitsPerPixT3MAPS", true);
  PlotUtil::plotTH1F(hitPerPixFEI4, "hits per pixel", "entries", "../TestBeamOutput/TestBeamOverview/hitsPerPixFEI4", true);
  
  // Then print counters:
  std::cout << "\nTestBeamStudies: Printing hit counters." << std::endl;
  std::cout << "\tT3MAPS total = " << nHitsT3MAPS_total << std::endl;
  std::cout << "\tFEI4 total = " << nHitsFEI4_total << std::endl;
  std::cout << "\nTestBeamStudies: Printing noisy T3MAPS regions." << std::endl;

  // Then print noisy pixels:
  for (int i_n = 0; i_n < (int)BusyT3MAPSRows.size(); i_n++) {
    std::cout << "\tRow = " << BusyT3MAPSRows[i_n] << ", \tCol = "
	      << BusyT3MAPSCols[i_n] << std::endl;
  }
  std::cout << "\nTestBeamStudies: Printing noisy FEI4 regions." << std::endl;
  for (int i_n = 0; i_n < (int)BusyFEI4Rows.size(); i_n++) {
    std::cout << "\tRow = " << BusyFEI4Rows[i_n] << ", \tCol = "
	      << BusyFEI4Cols[i_n] << std::endl;
  }
  
  std::cout << "TestBeamStudies: Finished analysis." << std::endl;
}
