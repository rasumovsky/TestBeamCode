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
  int bin;
  for (int i_b = 1; i_b <= hm->GetNbinsX(); i_b++) {
    if ((hm->GetBinCenter(i_b)+(0.5*hm->GetBinWidth(i_b)) > position) &&
	(hm->GetBinCenter(i_b)-(0.5*hm->GetBinWidth(i_b)) <= position)) {
      bin = i_b;
      break;
    }
  }
  double magnitude = hm->GetBinContent(bin);
  return magnitude;
}

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
  double timeOffsetInterval = 0.1;
  int nTimeBins = (int)((timeOffsetMax - timeOffsetMin)/timeOffsetInterval);
  
  int saturationT3MAPS = 10;
  double frequency = 2.0;
  
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
			       chips->getNCol("FEI4"), - 0.5,
			       (chips->getNCol("FEI4") - 0.5));
  TH2D *occOnlyHitFEI4 = new TH2D("occOnlyHitFEI4", "occOnlyHitFEI4",
				  chips->getNRow("FEI4"), -0.5,
				  (chips->getNRow("FEI4") - 0.5),
				  chips->getNCol("FEI4"), - 0.5,
				  (chips->getNCol("FEI4") - 0.5));
  TH2D *occT3MAPS = new TH2D("occT3MAPS", "occT3MAPS",
			     chips->getNRow("T3MAPS"), -0.5,
			     (chips->getNRow("T3MAPS") - 0.5),
			     chips->getNCol("T3MAPS"), -0.5,
			     (chips->getNCol("T3MAPS") - 0.5));
  
  // Counters:
  int nHitsT3MAPS_noCuts = 0;
  int nHitsT3MAPS_afterCuts = 0;
  int nHitsFEI4_total = 0;
  int nHitsFEI4_overlapping = 0;
  
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
  TH1F *histMax = new TH1F("histMax", "histMax", 50, 0.0, 0.003);
  TH1F *histDev = new TH1F("histMax", "histMax", 100, -0.003, 0.003);
  
  
   // Graphs for FFTs:
  TH1F *hTime[4][MapParameters::nBins][MapParameters::nBins];
  for (int i_h = 0; i_h < 4; i_h++) {
    for (int i_x = 0; i_x < MapParameters::nBins; i_x++) {
      for (int i_y = 0; i_y < MapParameters::nBins; i_y++) {
	hTime[i_h][i_x][i_y] = new TH1F(Form("hTime_%d_%d_%d",i_h,i_x,i_y),Form("hTime_%d_%d_%d",i_h,i_x,i_y),nTimeBins,timeOffsetMin,timeOffsetMax);
      }
    }
  }
  
  
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
      
      // Cut on events with no T3MAPS hits:
      if (cT->nHits == 0) continue;

      if (graphPoint == 0) nHitsT3MAPS_noCuts += cT->nHits;
      
      // Cut on events where T3MAPS is saturated:
      if (options.Contains("CutFullEvt") && cT->nHits > saturationT3MAPS) {
	continue;
      }
      
      if (graphPoint == 0) {
	nHitsT3MAPS_afterCuts += cT->nHits;
	for (int i_h = 0; i_h < (int)cT->hit_row->size(); i_h++) {
	  occT3MAPS->Fill((*cT->hit_row)[i_h], (*cT->hit_column)[i_h]);
	}    
      }

      // Advance position in the FEI4 tree:
      while (cF->timestamp_start < (cT->timestamp_stop+timeOffset) &&
	     eventFEI4 < entriesFEI4) {
	
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
	  
	  // Fill overlapping FEI4 hit occupancy plot:
	  if (graphPoint == 0) { 
	    nHitsFEI4_overlapping++;
	    occOverFEI4->Fill(currFEI4Hit->getRow(), currFEI4Hit->getCol());
	  }
	  
	  // Loop over T3MAPS hits:
	  for (int i_h = 0; i_h < (int)cT->hit_row->size(); i_h++) {
	    PixelHit *currT3MAPSHit = new PixelHit((*cT->hit_row)[i_h], 
						   (*cT->hit_column)[i_h],
						   1, 1, false);
	    if (currT3MAPSHit->getRow() > 0 && currT3MAPSHit->getRow() < 17) {
	      mapper->addPairToMap(currFEI4Hit, currT3MAPSHit);
	    }
	    delete currT3MAPSHit;
	  }
	}
	// If timestamps don't match up, use as background estimate:
	else {	  

	  // Loop over T3MAPS hits:
	  for (int i_h = 0; i_h < (int)cT->hit_row->size(); i_h++) {
	    PixelHit *currT3MAPSHit = new PixelHit((*cT->hit_row)[i_h], 
						   (*cT->hit_column)[i_h],
						   1, 1, false);
	    if (currT3MAPSHit->getRow() > 0 && currT3MAPSHit->getRow() < 17) {
	      mapper->addPairToBkg(currFEI4Hit, currT3MAPSHit);
	    }
	    delete currT3MAPSHit;
	  }
	}
	
	// then advance to the next entry
	eventFEI4++;
	cF->fChain->GetEntry(eventFEI4);
	delete currFEI4Hit;
      }// End of loop over FEI4 hits
    }// End of loop over T3MAPS events
    std::cout << "TestBeamStudies: Ending loop to define maps." << std::endl;
    
    mapper->createMapFromHits();
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
	  hTime[i_h][i_x-1][i_y-1]->SetBinContent(timeOffset, diffVal);
	  if (diffVal > 0.0025) {
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
      PlotUtil::plotTGraph(graphMapVar[i_h][i_p], "time offset [s]", Form("parameter %d",i_p), Form("../TestBeamOutput/TestBeamStudies/mapVal%d_orient%d",i_p,i_h));
      PlotUtil::plotTGraph(graphMapErr[i_h][i_p], "time offset [s]", Form("error %d",i_p), Form("../TestBeamOutput/TestBeamStudies/mapErr%d_orient%d",i_p,i_h));
    }
    PlotUtil::plotTGraph(graphDiffMax[i_h], "time offset [s]", "Difference Maximum", Form("../TestBeamOutput/TestBeamStudies/mapDiffMax_orient%d",i_h));
  }
  
  // Plot occupancy for FEI4 and T3MAPS:
  PlotUtil::plotTH2D(occFEI4, "row_{FEI4}", "column_{FEI4}", "hits", "../TestBeamOutput/TestBeamStudies/occupancyFEI4");
  PlotUtil::plotTH2D(occOverFEI4, "row_{FEI4}", "column_{FEI4}", "hits", "../TestBeamOutput/TestBeamStudies/occupancyOverlappingFEI4");
  PlotUtil::plotTH2D(occT3MAPS, "row_{T3MAPS}", "column_{T3MAPS}", "hits", "../TestBeamOutput/TestBeamStudies/occupancyT3MAPS");
  PlotUtil::plotTH2D(occOnlyHitFEI4, "row_{FEI4}", "column_{FEI4}", "hits", "../TestBeamOutput/TestBeamStudies/occupancyOnlyFEI4");
  
  PlotUtil::plotTH1F(histMax, "maximum value of (s-b)", "entries", "../TestBeamOutput/TestBeamStudies/histMax");
  PlotUtil::plotTH1F(histDev, "(s-b)", "entries", "../TestBeamOutput/TestBeamStudies/histDeviations", true);
  
  // Fourier analysis:
  TH2D *hFFT[4];  
  hFFT[0] = new TH2D("hFFT0", "hFFT0", MapParameters::nBins, -3.2, 16.8, 
		     MapParameters::nBins, -4.5, 20.0);
  hFFT[1] = new TH2D("hFFT1", "hFFT1", MapParameters::nBins, -3.2, 16.8, 
		     MapParameters::nBins, -4.5, 20.0);
  hFFT[2] = new TH2D("hFFT2", "hFFT2", MapParameters::nBins, -3.2, 16.8, 
		     MapParameters::nBins, 0.0, 24.5);
  hFFT[3] = new TH2D("hFFT3", "hFFT3", MapParameters::nBins, -3.2, 16.8, 
		     MapParameters::nBins, 0.0, 24.5);
  
  // NOW TAKE CARE OF THE FFT:
  std::cout << "Starting the discrete fourier transforms" << std::endl;
  for (int i_h = 0; i_h < 4; i_h++) {
    for (int i_x = 0; i_x < MapParameters::nBins; i_x++) {
      for (int i_y = 0; i_y < MapParameters::nBins; i_y++) {
	double valueFFT = GetFFTValue(hTime[i_h][i_x][i_y], frequency);
	hFFT[i_h]->SetBinContent(i_x+1, i_y+1, valueFFT);
      }
    }
    
    PlotUtil::plotTH2D(hFFT[i_h], "row offset [mm]", "column offset [mm]",
		       "FFT Magnitude",
		       Form("../TestBeamOutput/FFTMagnitude_orient%d",i_h));
  }
  
  // Then print the most significant point:
  std::cout << "\nPrinting a list of significant points" << std::endl;
  for (int i_s = 0; i_s < (int)significantPoint.size(); i_s++) {
    std::cout << significantPoint[i_s] << std::endl;
  }
  
  // Then print counters:
  std::cout << "TestBeamStudies: Printing hit counters." << std::endl;
  std::cout << "T3MAPS before cuts = " << nHitsT3MAPS_noCuts << std::endl;
  std::cout << "T3MAPS after cuts = " << nHitsT3MAPS_afterCuts << std::endl;
  std::cout << "FEI4 total = " << nHitsFEI4_total << std::endl;
  std::cout << "FEI4 overlapping = " << nHitsFEI4_overlapping << std::endl;
  
  std::cout << "TestBeamStudies: Finished analysis." << std::endl;
}
