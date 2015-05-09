////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: TestBeamAnalysis.cxx                                                //
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
//    "CutFullEvt" - Ignores events where T3MAPS has 10% of pixels showing    //
//                   hits for the integration period.                         //
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

/*
void PlotFourGraphs(TGraph *graphs[4], TString gnames[4], TString xname,
		    TString yname, TString sname) {
  TCanvas *c = new TCanvas("c","c",800,800);
  c->cd();
  for (int i_g = 0; i_g < 4; i_g++) {
    graphs[i_g]->GetXaxis()->SetTitle(xname);
    graphs[i_g]->GetYaxis()->SetTitle(yname);
    leg.Add(graphs[i_g],gnames[i_g],"l");
    graphs[i_g]->SetLineColor(i_g+2);
    if (i_g == 0) graphs[i_g]->Draw("ALP");
    else graphs[i_g]->Draw("LPSAME");
  }
  leg.Draw("SAME");
  c->Print(sname);
  c->Clear();
}
*/


int main(int argc, char **argv) {
  
  // Check arguments:
  if (argc < 4) {
    std::cout << "\nUsage: " << argv[0]
	      << " <inputT3MAPS> <inputFEI4> <options>" << std::endl; 
    exit(0);
  }
  
  PlotUtil::setAtlasStyle();
  
  // HERE IS THE TIME OFFSET INFORMATION:
  double timeOffsetMin = -5.0;
  double timeOffsetMax = 5.0;
  double timeOffsetInterval = 0.4;
  int nTimeBins = (int)((timeOffsetMax - timeOffsetMin)/timeOffsetInterval);
  
  TString inputT3MAPS = argv[1];
  TString inputFEI4 = argv[2];
  TString options = argv[3];
  TString mapFileDir = "../TestBeamOutput";
  
  // LoadT3MAPS and load the FEI4 TTree
  TFile *fileT3MAPS = new TFile(inputT3MAPS);
  TTree *myTreeT3MAPS = (TTree*)fileT3MAPS->Get("TreeT3MAPS");
  TreeT3MAPS *cT = new TreeT3MAPS(myTreeT3MAPS);
  
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
  TH2D *occOverFEI4 = new TH2D("occOverlapFEI4", "occOverlapFEI4",
			       chips->getNRow("FEI4"), -0.5,
			       (chips->getNRow("FEI4") - 0.5),
			       chips->getNCol("FEI4"), 0.5,
			       (chips->getNCol("FEI4") - 0.5));
  TH2D *occOnlyHitFEI4 = new TH2D("occOnlyHitFEI4", "occOnlyHitFEI4",
				  chips->getNRow("FEI4"), -0.5,
				  (chips->getNRow("FEI4") - 0.5),
				  chips->getNCol("FEI4"), 0.5,
				  (chips->getNCol("FEI4") - 0.5));
  TH2D *occT3MAPS = new TH2D("occT3MAPS", "occT3MAPS",
			     chips->getNRow("T3MAPS"), -0.5,
			     (chips->getNRow("T3MAPS") - 0.5),
			     chips->getNCol("T3MAPS"), -0.5,
			     (chips->getNCol("T3MAPS") - 0.5));
  
  // For analysis of scanning window:
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
  TH1F *histMax = new TH1F("histMax", "histMax", 50, 0.0, 0.001);
  TH1F *histDev = new TH1F("histMax", "histMax", 100, -0.001, 0.001);
  
  std::vector<std::string> significantPoint; significantPoint.clear();
  
  // Loop over scan timing offsets:
  int graphPoint = 0;
  std::cout << "TestBeamStudies: Beginning loop over time offset." << std::endl;
  for (double timeOffset = timeOffsetMin; timeOffset < timeOffsetMax; timeOffset+=timeOffsetInterval) {
    std::cout << "TestBeamStudies: timeOffset=" << timeOffset << std::endl;    
        
    MapParameters *mapper = new MapParameters("","");
    
    // Define the map from T3MAPS <--> FEI4
    std::cout << "TestBeamStudies: Entering loop to define maps." << std::endl;
    
    // Prepare FEI4 tree for loop inside T3MAPS tree's loop.
    Long64_t entriesFEI4 = cF->fChain->GetEntries();
    Long64_t eventFEI4 = 0;
    cF->fChain->GetEntry(eventFEI4);
    std::cout << "TestBeamStudies: FEI4 entries = " << entriesFEI4 << std::endl;
    
    // Loop over T3MAPS tree.
    Long64_t entriesT3MAPS = cT->fChain->GetEntries();
    cout << "Entries in T3MAPS = " << entriesT3MAPS << endl;
    for (Long64_t eventT3MAPS = 0; eventT3MAPS < entriesT3MAPS; eventT3MAPS++) {
      
      cT->fChain->GetEntry(eventT3MAPS);
            
      // Cut on events where T3MAPS is saturated:
      if (options.Contains("CutFullEvt") && cT->nHits > 100) continue;
      
      if (graphPoint == 0) {
	for (int i_h = 0; i_h < (int)cT->hit_row->size(); i_h++) {
	  occT3MAPS->Fill((*cT->hit_row)[i_h], (*cT->hit_column)[i_h]);
	}    
      }

      // Advance position in the FEI4 tree:
      while (cF->timestamp_start < (cT->timestamp_stop+timeOffset) &&
	     eventFEI4 < entriesFEI4) {
	
	// Fill FEI4 occupancy plot:
	if (graphPoint == 0) occFEI4->Fill(cF->row, cF->column);
	
	// Only consider events with timestamp inside that of T3MAPS
	if (cF->timestamp_start >= (cT->timestamp_start+timeOffset) &&
	    cF->timestamp_stop <= (cT->timestamp_stop+timeOffset)) {
	  
	  
	  // Cut on events with no T3MAPS hits:
	  if (cT->nHits > 0) {
	    
	    // Fill overlapping FEI4 hit occupancy plot:
	    if (graphPoint == 0) { 
	      occOverFEI4->Fill(cF->row-1, cF->column-1);
	    }
	    
	    PixelHit *currFEI4Hit = new PixelHit(cF->row-1, cF->column-1,
						 cF->LVL1ID, cF->tot, false);
	    // Loop over T3MAPS hits:
	    for (int i_h = 0; i_h < (int)cT->hit_row->size(); i_h++) {
	      PixelHit *currT3MAPSHit = new PixelHit((*cT->hit_row)[i_h], 
						     (*cT->hit_column)[i_h],
						     1, 1, false);
	      mapper->addPairToMap(currFEI4Hit, currT3MAPSHit);
	    }
	  }
	  // If timestamps don't match up, use as background estimate:
	  else {	  
	    PixelHit *currFEI4Hit = new PixelHit(cF->row-1, cF->column-1,
						 cF->LVL1ID, cF->tot, false);
	    // Loop over T3MAPS hits:
	    for (int i_h = 0; i_h < (int)cT->hit_row->size(); i_h++) {
	      PixelHit *currT3MAPSHit = new PixelHit((*cT->hit_row)[i_h], 
						     (*cT->hit_column)[i_h],
						     1, 1, false);
	      mapper->addPairToBkg(currFEI4Hit, currT3MAPSHit);
	    }
	  }
	}
	
	// then advance to the next entry
	eventFEI4++;
	cF->fChain->GetEntry(eventFEI4);
	
      }// End of loop over FEI4 hits
    }// End of loop over T3MAPS events
    std::cout << "TestBeamStudies: Ending loop to define maps." << std::endl;
    
    mapper->createMapFromHits();
    //mapper->printMapParameters();
    mapper->saveMapParameters(mapFileDir);
    
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
	  if (diffVal > 0.0007) {
	    significantPoint.push_back((std::string)Form("time%f_orient%d_x%d_y%d",timeOffset,i_h,i_x,i_y));
	  }
	}
      }
      tempDiffHist->Delete();
    }
    graphPoint++;
    delete mapper;
  }
  std::cout << "TestBeamStudies: Finished timing scan." << std::endl;
  
  for (int i_h = 0; i_h < 4; i_h++) {
    for (int i_p = 0; i_p < 4; i_p++) {
      PlotUtil::plotTGraph(graphMapVar[i_h][i_p], "time offset [s]",
			   Form("parameter %d",i_p),
			   Form("../TestBeamOutput/mapVal%d_orient%d",i_p,i_h));
      PlotUtil::plotTGraph(graphMapErr[i_h][i_p], "time offset [s]",
			   Form("error %d",i_p),
			   Form("../TestBeamOutput/mapErr%d_orient%d",i_p,i_h));
    }
    PlotUtil::plotTGraph(graphDiffMax[i_h], "time offset [s]",
			 "Difference Maximum",
			 Form("../TestBeamOutput/mapDiffMax_orient%d",i_h));
  }
  
  // Plot occupancy for FEI4 and T3MAPS:
  PlotUtil::plotTH2D(occFEI4, "row_{FEI4}", "column_{FEI4}", "hits", 
		    "../TestBeamOutput/occupancyFEI4");
  PlotUtil::plotTH2D(occOverFEI4, "row_{FEI4}", "column_{FEI4}", "hits", 
		    "../TestBeamOutput/occupancyOverlappingFEI4");
  PlotUtil::plotTH2D(occT3MAPS, "row_{T3MAPS}", "column_{T3MAPS}", "hits", 
		    "../TestBeamOutput/occupancyT3MAPS");
  PlotUtil::plotTH2D(occOnlyHitFEI4, "row_{FEI4}", "column_{FEI4}", "hits", 
		     "../TestBeamOutput/occupancyOnlyFEI4");
  
  PlotUtil::plotTH1F(histMax, "maximum value of (s-b)", "entries", 
		     "../TestBeamOutput/histMax");
  PlotUtil::plotTH1F(histDev, "(s-b)", "entries", 
		     "../TestBeamOutput/histDeviations",true);
  
  // Then print the most significant point:
  std::cout << "Printing a list of significant points" << std::endl;
  for (int i_s = 0; i_s < (int)significantPoint.size(); i_s++) {
    std::cout << significantPoint[i_s] << std::endl;
  }
  std::cout << "TestBeamStudiesTwo: Finished analysis." << std::endl;
}
