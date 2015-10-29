////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: Efficiency.cxx                                                      //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 26/05/2015                                                          //
//                                                                            //
//  This program calculates the true efficiency of the T3MAPS and FEI4 chips  //
//  using hit matching graphs for good and bad mappings.                      //
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

// Package includes:
#include "PlotUtil.h"

using namespace std;

/**
   The main method just requires an option to run. 
   @param option - "RunI" or "RunII" to select the desired dataset.
   @returns - 0. Prints plots to TestBeamOutput/Efficiency/ directory.
*/
int main(int argc, char **argv) {
  // Check arguments:
  if (argc < 2) {
    std::cout << "\nUsage: " << argv[0] << std::endl; 
    exit(0);
  }
  TString option = argv[1];
  
  TFile *fileSig;
  TFile *fileBkg;
  if (option.Contains("RunII")) {
    fileSig=new TFile("../TestBeamOutput/TestBeamScanner_SigII/graphFile.root");
    fileBkg=new TFile("../TestBeamOutput/TestBeamScanner_BkgII/graphFile.root");
  }
  else {
    fileSig=new TFile("../TestBeamOutput/TestBeamScanner_SigI/graphFile.root");
    fileBkg=new TFile("../TestBeamOutput/TestBeamScanner_BkgI/graphFile.root");
  }
  
  TH2D *hSig_T3MAPS = fileSig->Get("g2Eff_T3MAPS");
  TH2D *hSig_FEI4 = fileSig->Get("g2Eff_FEI4");
  
  TH2D *hBkg_T3MAPS = fileBkg->Get("g2Eff_T3MAPS");
  TH2D *hBkg_FEI4 = fileBkg->Get("g2Eff_FEI4");
  
  TH2D *hEff_T3MAPS = new TH2D("hEff_T3MAPS","hEff_T3MAPS",
			       hSig_T3MAPS->GetNbinsX(),
			       hSig_T3MAPS->GetXaxis()->GetXmin(),
			       hSig_T3MAPS->GetXaxis()->GetXmax(),
			       hSig_T3MAPS->GetNbinsY(),
			       hSig_T3MAPS->GetYaxis()->GetXmin(),
			       hSig_T3MAPS->GetYaxis()->GetXmax());
  
  TH2D *hEff_FEI4 = new TH2D("hEff_FEI4","hEff_FEI4",
			     hSig_FEI4->GetNbinsX(),
			     hSig_FEI4->GetXaxis()->GetXmin(),
			     hSig_FEI4->GetXaxis()->GetXmax(),
			     hSig_FEI4->GetNbinsY(),
			     hSig_FEI4->GetYaxis()->GetXmin(),
			     hSig_FEI4->GetYaxis()->GetXmax());
  
  for (int i_x = 1; i_x <= hSig_T3MAPS->GetNbinsX(); i_x++) {
    for (int i_y = 1; i_y <= hSig_T3MAPS->GetNbinsY(); i_y++) {
      
      double p1_T3MAPS = hSig_T3MAPS->GetBinContent(i_x, i_y);
      double p1_FEI4 = hSig_FEI4->GetBinContent(i_x, i_y);
      
      double p2_T3MAPS = hBkg_T3MAPS->GetBinContent(i_x, i_y);
      double p2_FEI4 = hBkg_FEI4->GetBinContent(i_x, i_y);
      
      // Subject to change:
      double value_T3MAPS = p1_T3MAPS - p2_T3MAPS;
      double value_FEI4 = p1_FEI4 - p2_FEI4;
      
      hEff_T3MAPS->SetBinContent(i_x, i_y, value_T3MAPS);
      hEff_FEI4->SetBinContent(i_x, i_y, value_FEI4);
    }
  }
       
  // Set the output plot style:
  PlotUtil::setAtlasStyle();  
  
  // Then plot 2D graphs:
  PlotUtil::plotTH2D(hEff_T3MAPS, "#Delta_{row} [mm]", "#Delta_{col} [mm]",
		     "Eff.", "../TestBeamOutput/Efficiency/eff_T3MAPS.eps");
  PlotUtil::plotTH2D(hEff_FEI4, "#Delta_{row} [mm]", "#Delta_{col} [mm]",
		     "Eff.", "../TestBeamOutput/Efficiency/eff_FEI4.eps");
  
  // Analysis is complete.
  std::cout << "\nEfficiency: Finished analysis." << std::endl;
  return 0;
}
