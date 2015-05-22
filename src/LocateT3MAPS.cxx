////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: LocateT3MAPS.cxx                                                    //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 14/05/2015                                                          //
//                                                                            //
//  This program figures out the location of T3MAPS in FEI4 using a map that  //
//  was produced with TestBeamStudies.cxx.                                    //
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
  if (argc < 1) {
    std::cout << "\nUsage: " << argv[0] << std::endl; 
    exit(0);
  }
    
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
    
  TH2D *locTempT3MAPS = new TH2D("locTempT3MAPS", "locTempT3MAPS", 
				 chips->getNRow("FEI4"), -0.5,
				 (chips->getNRow("FEI4") - 0.5),
				 chips->getNCol("FEI4"), -0.5,
				 (chips->getNCol("FEI4") - 0.5));
  
  TH2D *locTempT3MAPSErr = new TH2D("locTempT3MAPSErr", "locTempT3MAPSErr", 
				    chips->getNRow("FEI4"), -0.5,
				    (chips->getNRow("FEI4") - 0.5),
				    chips->getNCol("FEI4"), -0.5,
				    (chips->getNCol("FEI4") - 0.5));
  
  // Instantiate the mapping utility:
  mapper = new MapParameters("../TestBeamOutput","FromFile");
  mapper->setOrientation(1);
  //mapper->setMapVar(3, mapper->getMapVar(3)-3.0);

  double colSizeT = ((double)chips->getNCol("T3MAPS") *
		     chips->getColPitch("T3MAPS"));
  double rowSizeT = ((double)chips->getNRow("T3MAPS") *
		     chips->getRowPitch("T3MAPS"));
  double colSizeF = ((double)chips->getNCol("FEI4") * 
		     chips->getColPitch("FEI4"));
  double rowSizeF = ((double)chips->getNRow("FEI4") * 
		     chips->getRowPitch("FEI4"));
  double nMappedCol = colSizeT / chips->getColPitch("FEI4");
  double nMappedRow = rowSizeT / chips->getRowPitch("FEI4");

  std::cout << "\nThe size of T3MAPS is (col x row) " << colSizeT << " x "
	    << rowSizeT << " mm^2" << std::endl;
  std::cout << "The size of FEI4 is (col x row) " << colSizeF << " x "
	    << rowSizeF << " mm^2" << std::endl;
  std::cout << "So T3MAPS should take up " << nMappedCol << " columns x "
	    << nMappedRow << " rows in FEI4.\n" << std::endl;
  
  int minColFEI4 = chips->getNCol("FEI4") + 1;
  int maxColFEI4 = 0;
  int minRowFEI4 = chips->getNRow("FEI4") + 1;
  int maxRowFEI4 = 0;
  
  // Loop over T3MAPS pixels:
  for (int i_r = 0; i_r < chips->getNRow("T3MAPS"); i_r++) {
    for (int i_c = 0; i_c < chips->getNCol("T3MAPS"); i_c++) {
      int currRow = mapper->getFEI4fromT3MAPS("rowVal",i_r);
      int currCol = mapper->getFEI4fromT3MAPS("colVal",i_c);
      int currRowSigma = mapper->getFEI4fromT3MAPS("rowSigma",i_r);
      int currColSigma = mapper->getFEI4fromT3MAPS("colSigma",i_c);
      if (currCol < minColFEI4) minColFEI4 = currCol;
      if (currCol > maxColFEI4) maxColFEI4 = currCol;
      if (currRow < minRowFEI4) minRowFEI4 = currRow;
      if (currRow > maxRowFEI4) maxRowFEI4 = currRow;
      
      locTempT3MAPS->Fill(currRow, currCol);
      locTempT3MAPSErr->Fill(currRow + currRowSigma, currCol + currColSigma);
      locTempT3MAPSErr->Fill(currRow + currRowSigma, currCol - currColSigma);
      locTempT3MAPSErr->Fill(currRow - currRowSigma, currCol + currColSigma);
      locTempT3MAPSErr->Fill(currRow - currRowSigma, currCol - currColSigma);
      locTempT3MAPSErr->Fill(currRow + currRowSigma, currCol);
      locTempT3MAPSErr->Fill(currRow - currRowSigma, currCol);
      locTempT3MAPSErr->Fill(currRow, currCol + currColSigma);
      locTempT3MAPSErr->Fill(currRow, currCol - currColSigma);
    }
  }
  
  // Find range with uncertainties included:
  int minErrColFEI4 = chips->getNCol("FEI4") + 1;
  int maxErrColFEI4 = 0;
  int minErrRowFEI4 = chips->getNRow("FEI4") + 1;
  int maxErrRowFEI4 = 0;
  for (int i_x = 1; i_x < locTempT3MAPSErr->GetNbinsX(); i_x++) {
    for (int i_y = 1; i_y < locTempT3MAPSErr->GetNbinsY(); i_y++) {
      if (locTempT3MAPSErr->GetBinContent(i_x,i_y) > 0) {
	if (i_y < minErrColFEI4) minErrColFEI4 = i_y;
	if (i_y > maxErrColFEI4) maxErrColFEI4 = i_y;
	if (i_x < minErrRowFEI4) minErrRowFEI4 = i_x;
	if (i_x > maxErrRowFEI4) maxErrRowFEI4 = i_x;
      }
    }
  }
  
  // Then fill the normal histograms:
  for (int i_x = minErrRowFEI4; i_x <= maxErrRowFEI4; i_x++) {
    for (int i_y = minErrColFEI4; i_y <= maxErrColFEI4; i_y++) {
      locT3MAPS->SetBinContent(i_x, i_y, 2);
    }
  }
  
  for (int i_x = minRowFEI4; i_x <= maxRowFEI4; i_x++) {
    for (int i_y = minColFEI4; i_y <= maxColFEI4; i_y++) {
      locT3MAPS->SetBinContent(i_x, i_y, 1);
    }
  }
  
  std::cout << "LocateT3MAPS: Printing nominal location of T3MAPS in FEI4"
	    << std::endl;
  std::cout << "\tcolumn range = [" << minColFEI4 << ", " << maxColFEI4
	    << "]" << std::endl;
  std::cout << "\trow range = [" << minRowFEI4 << ", " << maxRowFEI4
	    << "]" << std::endl;
  
  
  std::cout << "LocateT3MAPS: Printing location + uncertainty of T3MAPS in FEI4"
	    << std::endl;
  std::cout << "\tcolumn range = [" << minErrColFEI4 << ", " << maxErrColFEI4
	    << "]" << std::endl;
  std::cout << "\trow range = [" << minErrRowFEI4 << ", " << maxErrRowFEI4
	    << "]" << std::endl;
  
  PlotUtil::plotTH2D(locT3MAPS, "row_{FEI4}", "col_{FEI4}", "", "../TestBeamOutput/LocateT3MAPS/nominalLocationT3MAPS.eps");
  //PlotUtil::plotTH2D(locTempT3MAPS, "row_{FEI4}", "col_{FEI4}", "", "../TestBeamOutput/LocateT3MAPS/nominalLocationT3MAPS.eps");
  //PlotUtil::plotTH2D(locTempT3MAPSErr, "row_{FEI4}", "col_{FEI4}", "", "../TestBeamOutput/LocateT3MAPS/uncertaintyLocationT3MAPS.eps");
  
  std::cout << "\nLocateT3MAPS: Finished analysis." << std::endl;
  return 0;
}
