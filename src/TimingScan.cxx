////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: TimingScan.cxx                                                      //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 14/05/2015                                                          //
//                                                                            //
//  This program cross-checks the timing offset between two chips by          //
//  maximizing the efficiency measurement in TestBeamTracks.cxx.              //
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
   @returns - 0. Prints plots to TestBeamOutput/TimingScan/ directory.
*/
int main(int argc, char **argv) {
  // Check arguments:
  if (argc < 2) {
    std::cout << "\nUsage: " << argv[0] << std::endl; 
    exit(0);
  }
  TString option = argv[1];
  
  // Set the output plot style:
  PlotUtil::setAtlasStyle();  
  
  // Graphs for results:
  TGraph *gEffT3MAPS = new TGraph();
  TGraph *gEffFEI4 = new TGraph();
  
  // Counters and readers:
  double effT3MAPS = 0;
  double effFEI4 = 0;
  int point = 0;
  
  // Keep track of highest efficiency and associated timing:
  double timingMax = -999.9;
  double effMax = 0.0;
  
  // Loop over timing offsets:
  for (double timing = -4.0; timing <= 4.0; timing += 0.1) {
    
    // Run the track efficiency program:
    system(Form("./bin/TestBeamTracks %s %f",option.Data(),timing));
    
    // Read the output file:
    ifstream eFile;
    eFile.open(Form("../TestBeamOutput/TestBeamTracks/eff_t%2.2f.txt",timing));
    while (!eFile.eof()) eFile >> effT3MAPS >> effFEI4;
    
    // Add efficiencies to graphs:
    gEffT3MAPS->SetPoint(point, timing, effT3MAPS);
    gEffFEI4->SetPoint(point, timing, effFEI4); 
    point++;
    
    // Update the maximum efficiency
    if (effT3MAPS >= effMax) {
      timingMax = timing;
      effMax = effT3MAPS;
    }
  }
  
  // Finally, plot the data:
   TCanvas *can = new TCanvas("can","can",800,600);
  can->cd();
  gEffT3MAPS->SetLineWidth(2);
  gEffFEI4->SetLineWidth(2);
  gEffT3MAPS->SetLineColor(kRed);
  gEffT3MAPS->SetMarkerColor(kRed);
  gEffFEI4->SetLineColor(kBlue);
  gEffFEI4->SetMarkerColor(kBlue);
  gEffT3MAPS->GetXaxis()->SetTitle("Chip #Deltatiming [s]");
  gEffT3MAPS->GetYaxis()->SetTitle("% hits matched in other chip");
  gEffT3MAPS->Draw("ALP");
  gEffFEI4->Draw("LPSAME");
  
  TLegend leg(0.24, 0.7, 0.44, 0.8);
  leg.SetBorderSize(0);
  leg.SetFillColor(0);
  leg.SetTextSize(0.03);
  leg.AddEntry(gEffT3MAPS, "T3MAPS", "LP");
  leg.AddEntry(gEffFEI4, "FEI4", "LP");
  leg.Draw("SAME");
  
  TLine *line = new TLine();
  line->SetLineStyle(2);
  line->SetLineWidth(1);
  line->SetLineColor(kBlack);
  line->DrawLine(timingMax, gEffT3MAPS->GetYaxis()->GetXmin(),
		 timingMax, gEffT3MAPS->GetYaxis()->GetXmax());
  can->Print("../TestBeamOutput/TimingScan/timeEffScan.eps");
  can->Clear();
  
  std::cout << "\nTimingScan: Finished analysis." << std::endl;
  std::cout << "\t Efficiency maximized for timing=" << timingMax << std::endl;
  return 0;
}
