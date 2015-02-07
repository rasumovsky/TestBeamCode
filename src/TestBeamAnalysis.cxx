////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: TestBeamAnalysis.cxx                                                //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 04/02/2015                                                          //
//                                                                            //
//  This class loads the FE-I4 and T3MAPS data structures and then correlates //
//  hits between the two chips for an efficiency test.                        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "TestBeamAnalysis.h"

int main( int argc, char **argv ) {
  
  // Check arguments:
  if (argc < 4) {
    printf("\nUsage: %s <inputT3MAPS> <inputFEI4> <options>\n\n",argv[0]); 
    exit(0);
  }
  
  TString inputT3MAPS = argv[1];
  TString inputFEI4 = argv[2];
  options = argv[3];

  PixelHit *hit = new PixelHit(1,1,true);
  //PixelHit hit(1,1,true);
  
  PixelCluster *cluster = new PixelCluster(hit);
  
  // Root macros:
  //SetAtlasStyle();
  
  // in this program, first call LoadT3MAPS, then call Map, then HitMatch
  /**
  // Code to load the input data:
  outputT3MAPS = inputT3MAPS.ReplaceAll(".txt",".root");
  T3MAPS = new LoadT3MAPS(inputT3MAPS, outputT3MAPS);
  myTreeT3MAPS = T3MAPS->getTree();

  // Load the FEI4 data tree:
  fileFEI4 = new TFile(inputFEI4);
  myTreeFEI4 = (TTree*)fileFEI4->Get("Table");

  // must go in the header:
  
  // For T3MAPS data:
  LoadT3MAPS *T3MAPS;
  TTree *myTreeT3MAPS;
  
  // For FEI4 data:
  TFile *fileFEI4;
  TTree *myTreeFEI4;
  */  
  ///// FOR FEI4, chiech that they have the same event number before reco clusters and hits.
  
  return 0;
}


///// ORIGINALL FROM MODULE MAPPING:::
 TTree *myTreeT3MAPS;
  TTree *myTreeFEI4;
  
  // T3MAPS tree variables:
  double t_T3MAPS_timestamp_start;
  double t_T3MAPS_timestamp_stop;
  std::vector<int> t_T3MAPS_hit_row;
  std::vector<int> t_T3MAPS_hit_column;
  
  // FEI4 tree variables:
  double t_FEI4_timestamp_start;
  double t_FEI4_timestamp_stop;
  int t_FEI4_hit_row;
  int t_FEI4_hit_column;

/**
   Load the TTrees for T3MAPS and FEI4 from file, and set the branch addresses.
*/
void ModuleMapping::prepareTrees() {
  
  std::cout << "  ModuleMapping::prepareTrees() reading hit files" << std::endl;
  
  // Set the T3MAPS TTree addresses:
  myTreeT3MAPS->SetBranchAddress("timestamp_start", &t_T3MAPS_timestamp_start);
  myTreeT3MAPS->SetBranchAddress("timestamp_stop", &t_T3MAPS_timestamp_stop);
  myTreeT3MAPS->SetBranchAddress("hit_row", &t_T3MAPS_hit_row);
  myTreeT3MAPS->SetBranchAddress("hit_column", &t_T3MAPS_hit_column);
  
  // Load the FEI4 data tree:
  myTreeFEI4->SetBranchAddress("timestamp_start", &t_FEI4_timestamp_start);
  myTreeFEI4->SetBranchAddress("timestamp_stop", &t_FEI4_timestamp_stop);
  myTreeFEI4->SetBranchAddress("row", &t_FEI4_hit_row);
  myTreeFEI4->SetBranchAddress("column", &t_FEI4_hit_column);
}

/**
   This method uses several pairs of T3MAPS pixels to derive a linear one-to-one
   mapping from T3MAPS(row,column) <--> FEI4(row,column). It averages the
   results of individual calls to makeGeoMap() to get a more accurate mapping.
 */
void ModuleMapping::makeCombinedMap() {
  
  std::cout << "  ModuleMapping::makeCombinedMap Starting..." << std::endl;
  
  int currRow1 = 1;
  int currCol1 = 1;
  
  int currRow2 = myChips->getChipSize("T3MAPS","nRows");
  int currCol2 = myChips->getChipSize("T3MAPS","nColumns");
  
  for (int i = 0; i < 4; i++) {
    histMapValues[i] = new TH1F(Form("histMapValues%i",i),
				Form("histMapValues%i",i)
				40,-2,2);
  }
  
  // Scan edge pixels of T3MAPS and create a map for each.
  while (currRow1 < myChips->getChipSize("T3MAPS","nRows") ||
	 currCol1 < myChips->getChipSize("T3MAPS","nColumns")) {
    
    // Make a new map of the geometries with the current points:
    makeGeoMap( currRow1, currCol1, currRow2, currCol2 );
    
    // Record the values:
    for (int i = 0; i < 4; i++) {
      histMapValues[i]->Fill(mVar[i]);
    }
    
    // Advance each of the points around T3MAPS edge:
    if (currRow1 < myChips->getChipSize("T3MAPS","nRows")) {
      currRow1++;
      currRow2--;
    }
    else {
      currCol1++;
      currCol2--;
    }
  }
  
  // Save the precision mapping constants and the error.
  for (int i = 0; i < 4; i++) {
    mVar[i] = histMapValues[i]->GetMean();
    mRMS[i] = histMapValues[i]->GetRMS();
  }
  
  printMapParameters();
}

/**
   This method uses two T3MAPS pixels to derive a linear one-to-one mapping 
   from T3MAPS(row,column) <--> FEI4(row,column).
 */
void ModuleMapping::makeGeoMap(int rowT3MAPS_1, int colT3MAPS_1, 
			       int rowT3MAPS_2, int colT3MAPS_2) {
  
  std::cout << "  ModuleMapping::makeGeoMap() Maps T3MAPS-->FEI4" << std::endl;
  
  for (int i = 0; i < 2; i++) {
    histRowFEI4Pix[i] = new TH1F(Form("FEI4Pix%iRow",i),Form("FEI4Pix%iRow",i),
				 myChips->getChipSize("FEI4","nRows"),0.5,
				 ((double)myChips->getChipSize("FEI4","nRows"))
				 +0.5);
    histColFEI4Pix[i] = new TH1F(Form("FEI4Pix%iCol",i),Form("FEI4Pix%iCol",i),
				 myChips->getChipSize("FEI4","nColumns"),0.5,
				 ((double)myChips->getChipSize("FEI4",
							       "nColumns"))
				 +0.5);
    hist2dFEI4Pix[i] = new TH2F(Form("FEI4Pix%i2d",i),Form("FEI4Pix%i2d",i),
				myChips->getChipSize("FEI4","nRows"),0.5,
				((double)myChips->getChipSize("FEI4","nRows"))
				+0.5,
				myChips->getChipSize("FEI4","nColumns"),0.5,
				((double)myChips->getChipSize("FEI4",
							      "nColumns"))
				+0.5);
  }
  
  // Count the number of coincidences found.
  int nHitsPix1 = 0;
  int nHitsPix2 = 0;
  
  // Prepare FEI4 tree for loop inside T3MAPS tree's loop.
  Long64_t entriesFEI4 = myTreeFEI4->GetEntries();
  Long64_t eventFEI4 = 0;
  myTreeFEI4->GetEntry(eventFEI4);
  
  // Loop over T3MAPS tree.
  Long64_t entriesT3MAPS = myTreeT3MAPS->GetEntries();
  for (Long64_t eventT3MAPS = 0; eventT3MAPS < entriesT3MAPS; eventT3MAPS++) {
    
    myTreeT3MAPS->GetEntry(eventT3MAPS);
    
    // Check to see if either of the mapping pixels were hit:
    bool hitPix1 = false;
    bool hitPix2 = false;
    for (int hitT3MAPS = 0; hitT3MAPS < t_T3MAPS_hit_row.size(); hitT3MAPS++) {
      
      if (t_T3MAPS_hit_row(hitT3MAPS) == rowT3MAPS_1 && 
	  t_T3MAPS_hit_column(hitT3MAPS) == colT3MAPS_1) {
	hitPix1 = true;
      }
      else if (t_T3MAPS_hit_row(hitT3MAPS) == rowT3MAPS_2 && 
	  t_T3MAPS_hit_column(hitT3MAPS) == colT3MAPS_2) {
	hitPix2 = true;
      }
    }
    
    // Only consider events with hits in the mapping pixels:
    if (!hitPix1 && !hitPix2) {
      continue;
    }
    
    // also advance FEI4 tree.
    while (t_FEI4_timestamp_start < t_T3MAPS_timestamp_stop && 
	   eventFEI4 < entriesFEI4) {

      eventFEI4++;
      myTreeFEI4->GetEntry(eventFEI4);
      
      // Only consider events with timestamp inside that of T3MAPS
      // Same as looking for time coincidence hits in FEI4 and T3MAPS
      if (t_FEI4_timestamp_start >= t_T3MAPS_timestamp_start &&
	  t_FEI4_timestamp_stop <= t_T3MAPS_timestamp_stop) {
	
	if (hitPix1) {
	  histRowFEI4Pix[0]->Fill(t_FEI4_hit_row);
	  histColFEI4Pix[0]->Fill(t_FEI4_hit_column);
	  hist2dFEI4Pix[0]->Fill(t_FEI4_hit_row, t_FEI4_hit_column);
	  nHitsPix1++;
	}
	
	if (hitPix2) {
	  histRowFEI4Pix[1]->Fill(t_FEI4_hit_row);
	  histColFEI4Pix[1]->Fill(t_FEI4_hit_column);
	  hist2dFEI4Pix[1]->Fill(t_FEI4_hit_row, t_FEI4_hit_column);
	  nHitsPix2++;
	}
      }
    }
  }// End of loop over events
  
  // make plot of FEI4 row and col for both T3MAPS pixels (4 histograms)
    
  // Use max bin to get the row and column numbers in T3MAPS.
  int rowFEI4_1 = histRowFEI4Pix1->GetMaximumBin();
  int colFEI4_1 = histColFEI4Pix1->GetMaximumBin();
  int rowFEI4_2 = histRowFEI4Pix2->GetMaximumBin();
  int colFEI4_2 = histColFEI4Pix2->GetMaximumBin();
  
  // Calculate the linear mapping constants:
  double mVar[0] = ((double)(rowFEI4_2-rowFEI4_1)) 
    / ((double)(rowT3MAPS_2-rowT3MAPS_1));
  double mVar[2] = ((double)(colFEI4_2-colFEI4_1))
    / ((double)(colT3MAPS_2-colT3MAPS_1));
  double mVar[1] = ((double)rowFEI4_1) - (mVar[0] * (double)rowT3MAPS_1);
  double mVar[3] = ((double)colFEI4_1) - (mVar[2] * (double)colT3MAPS_1);
  
  for (int i = 0; i < 4; i++) hasMap[i] = true;
    
  std::cout << "  makeGeoMap Finished successfully!" << std::endl;
  std::cout << "    matched hits for T3MAPS pixel1: " << nHitsPix1 << endl;
  std::cout << "    matched hits for T3MAPS pixel2: " << nHitsPix2 << endl;
}
