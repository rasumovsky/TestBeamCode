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
#include "TF1.h"

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

double GetFFTValue(TH1F *h) {
  TH1 *hm = 0;
  TVirtualFFT::SetTransform(0);
  h->FFT(hm, "MAG");
  hm->SetTitle("Magnitude of the 1st transform");
  int bin = 0;
  // For "real" frequencies, divide x-axis range with range of your function.
  double frequency = 2 * TMath::Pi() * (1.0 / 2.6);
  double position =  (frequency *
		      (h->GetXaxis()->GetXmax() - h->GetXaxis()->GetXmin()) /
		      (hm->GetXaxis()->GetXmax() - hm->GetXaxis()->GetXmin()));
  double offset = hm->GetBinWithContent(position, bin);
  double magnitude = hm->GetBinContent(bin) / sqrt(hm->GetNbinsX());
  return magnitude;
}

int main(int argc, char **argv) {
  /*
  // Check arguments:
  if (argc < 4) {
    std::cout << "\nUsage: " << argv[0]
	      << " <inputT3MAPS> <inputFEI4> <options>" << std::endl; 
    exit(0);
  }
  */
  PlotUtil::setAtlasStyle();
    
  TCanvas *can = new TCanvas("can","can",800,800);
  can->Divide(2,2);
  can->cd(1);
  
  
  double intervalSize = 20;
  int n = 1000;

  TF1 *fsin = new TF1("fsin", "sin(2.6*2*3.1415*x)", 0, intervalSize);
  //fsin->Draw();
  TH1F *hsin = new TH1F("hsin","hsin",n+1,0,intervalSize);
  double x;
  for (int i = 0; i < n; i++) {
    x = ((double)i)/((double)n) * intervalSize;
    hsin->SetBinContent(i+1,fsin->Eval(x));
  }
  hsin->Draw();
  
  can->cd(2);
  
  TH1 *hm = 0;
  TVirtualFFT::SetTransform(0);
  hm = hsin->FFT(hm, "MAG");
  hm->SetTitle("Magnitude of the 1st transform");
  hm->Scale(1.0/hm->Integral());// GetNbinsX()));
  hm->Draw();
  
  // angular frequency of 2*pi = frequency of 1hz
  double position = (2.6*
		     (hsin->GetXaxis()->GetXmax()-hsin->GetXaxis()->GetXmin()));
  std::cout << "position = " << position << std::endl;

  int bin;		      
  for (int i_b = 1; i_b < hm->GetNbinsX(); i_b++) {
    std::cout << i_b << " " << hm->GetBinCenter(i_b) << " " << hm->GetBinWidth(i_b) << std::endl;
    if ((hm->GetBinCenter(i_b)+(0.5*hm->GetBinWidth(i_b)) > position) &&
	(hm->GetBinCenter(i_b)-(0.5*hm->GetBinWidth(i_b)) <= position)) {
      bin = i_b;
      break;
    }
  }
  
  double magnitude = hm->GetBinContent(bin);
  std::cout << "The bin content is: " << magnitude << " for bin " << bin << std::endl;
  can->Print("test.eps");
  
    
  // LoadT3MAPS and load the FEI4 TTree
  //TFile *fileT3MAPS = new TFile(inputT3MAPS);
  //TTree *myTreeT3MAPS = (TTree*)fileT3MAPS->Get("TreeT3MAPS");
  
  
  
    
  /*
  // Graphs for FFTs:
  TH1F *hTime[4][MapParameters::nBins][MapParameters::nBins];
  for (int i_h = 0; i_h < 4; i_h++) {
    for (int i_x = 0; i_x < MapParameters::nBins; i_x++) {
      for (int i_y = 0; i_y < MapParameters::nBins; i_y++) {
	hTime[i_h][i_x][i_y] = new TH1F(Form("hTime_%d_%d_%d",i_h,i_x,i_y),Form("hTime_%d_%d_%d",i_h,i_x,i_y),nTimeBins,timeOffsetMin,timeOffsetMax);
      }
    }
  }
  
  
	  // Add to plot for FFT:
	  hTime[i_h][i_x][i_y]->SetBinContent(timeOffset, diffVal);

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
	double valueFFT = GetFFTValue(hTime[i_h][i_x][i_y]);
	hFFT[i_h]->SetBinContent(i_x,i_y,valueFFT);
      }
    }
    
    PlotUtil::plotTH2D(hFFT[i_h], "row offset [mm]", "column offset [mm]",
		       "FFT Magnitude",
		       Form("../TestBeamOutput/FFTMagnitude_orient%d",i_h));
  }
  */
  return 0;
}
