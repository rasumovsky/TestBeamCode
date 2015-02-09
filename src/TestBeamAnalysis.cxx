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
  
  // Root macros:
  SetAtlasStyle();
  
  // LoadT3MAPS and load the FEI4 TTree
  outputT3MAPS = inputT3MAPS.ReplaceAll(".txt",".root");
  T3MAPS = new LoadT3MAPS(inputT3MAPS, outputT3MAPS);
  myTreeT3MAPS = T3MAPS->getTree();
  
  // Load the FEI4 data tree:
  fileFEI4 = new TFile(inputFEI4);
  myTreeFEI4 = (TTree*)fileFEI4->Get("Table");

  // Initialize several ModuleMapping instances
  int currR1 = 1;
  int currC1 = 1;
  int currR2 = myChips->getChipSize("T3MAPS","nRows");
  int currC2 = myChips->getChipSize("T3MAPS","nColumns");
  
  for (int i = 0; i < 4; i++) {
    histMapValues[i] = new TH1F(Form("histMapValues%i",i),
				Form("histMapValues%i",i),
				40,-2,2);
  }
  
  int const nMaps = (myChips->getChipSize("T3MAPS","nRows") +
		     myChips->getChipSize("T3MAPS","nColumns"));
  
  ModuleMapping *simpleMaps[nMaps];
  for (int i = 0; i < nMaps; i++) {
    
    simpleMaps[i] = new ModuleMapping("NA","new");
    simpleMaps[i]->designatePixelPair(currR1, currC1, currR2, currC2);
    
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
  
  // T3MAPS tree variables:
  double t_T3MAPS_timestamp_start;
  double t_T3MAPS_timestamp_stop;
  std::vector<int> t_T3MAPS_hit_row;
  std::vector<int> t_T3MAPS_hit_column;
  
  // FEI4 tree variables:
  int t_event_number;
  double t_FEI4_timestamp_start;
  double t_FEI4_timestamp_stop;
  int t_FEI4_hit_row;
  int t_FEI4_hit_column;
  
  // Load the TTrees for T3MAPS and FEI4 from file, set the branch addresses.
  std::cout << "  Setting branch addresses for input TTrees." << std::endl;
  
  // Set the T3MAPS TTree addresses:
  myTreeT3MAPS->SetBranchAddress("timestamp_start", &t_T3MAPS_timestamp_start);
  myTreeT3MAPS->SetBranchAddress("timestamp_stop", &t_T3MAPS_timestamp_stop);
  myTreeT3MAPS->SetBranchAddress("hit_row", &t_T3MAPS_hit_row);
  myTreeT3MAPS->SetBranchAddress("hit_column", &t_T3MAPS_hit_column);
  
  // Load the FEI4 data tree:
  myTreeFEI4->SetBranchAddress("event_number", &event_number);
  myTreeFEI4->SetBranchAddress("timestamp_start", &t_FEI4_timestamp_start);
  myTreeFEI4->SetBranchAddress("timestamp_stop", &t_FEI4_timestamp_stop);
  myTreeFEI4->SetBranchAddress("row", &t_FEI4_hit_row);
  myTreeFEI4->SetBranchAddress("column", &t_FEI4_hit_column);
  
  // Prepare FEI4 tree for loop inside T3MAPS tree's loop.
  Long64_t entriesFEI4 = myTreeFEI4->GetEntries();
  Long64_t eventFEI4 = 0;
  myTreeFEI4->GetEntry(eventFEI4);
  
  // Loop over T3MAPS tree.
  Long64_t entriesT3MAPS = myTreeT3MAPS->GetEntries();
  for (Long64_t eventT3MAPS = 0; eventT3MAPS < entriesT3MAPS; eventT3MAPS++) {
    
    myTreeT3MAPS->GetEntry(eventT3MAPS);
    
    
    // Now also advance the FEI4 tree.
    while (t_FEI4_timestamp_start < t_T3MAPS_timestamp_stop && 
	   eventFEI4 < entriesFEI4) {
      
      eventFEI4++;
      myTreeFEI4->GetEntry(eventFEI4);
      
      // Only consider events with timestamp inside that of T3MAPS
      // Same as looking for time coincidence hits in FEI4 and T3MAPS
      if (t_FEI4_timestamp_start >= t_T3MAPS_timestamp_start &&
	  t_FEI4_timestamp_stop <= t_T3MAPS_timestamp_stop) {
	
	// Loop over T3MAPS hits to see if any of the mapping pixels were hit:
	for (int i_h = 0; i_h < t_T3MAPS_hit_row.size(); i_h++) {
	  
	  PixelHit *currT3MAPSHit = new PixelHit(t_T3MAPS_hit_row, 
						 t_T3MAPS_hit_column,
						 false);
	  
	  for (int i_m = 0; i_m < nMaps; i_m++) {
	    
	    if (simpleMaps[i_m]->isPixelHit(0,currT3MAPSHit)) {
	      simpleMaps[i_m]->addHitToMap(0,currFEI4Hit);
	    }
	    
	    if (simpleMaps[i_m]->isPixelHit(1,currT3MAPSHit)) {
	      simpleMaps[i_m]->addHitToMap(1,currFEI4Hit);
	    }
	  }
	}
      }
    }
  }// End of loop over events
  
  // Collect the results of multiple maps:
  TH1F *histMapValues[4];
  for (int i_p = 0; i_p < 4; i_p++) {
    histMapValues[i_p] = new TH1F(Form("histMapValues%i",i_p),
				  Form("histMapValues%i",i_p),
				  40,-2,2);
  }
  
  // Make plot of FEI4 row and col for both T3MAPS pixels (4 histograms):
  for (int i_m = 0; i_m < nMaps; i_m++) {
    simpleMaps[i_m]->createMapFromHits();
    
    // save map parameters in histograms:
    for (int i_p = 0; i_p < 4; i_p++) {
      histMapValues[i_p]->Fill(getMapVar(i_p));
    }
  }
  
  // Create a new "meta map" to be used for the efficiency analysis:
  ModuleMapping *combinedMap = new ModuleMapping("NA","none");
  for (int i_p = 0; i_p < 4; i_p++) {
    combinedMap->SetMapVar(i_p,histMapValues[i_p]->GetMean());
    combinedMap->SetMapRMS(i_p,histMapValues[i_p]->GetRMS());
  }
  combinedMap->PrintMapParameters();
  combinedMap->SaveMapParameters();
  
  // Map has been loaded, may proceed to actual analysis.

// Loop over TTrees in tandem again
  // in each event, run the MatchMaker class.
  // be sure FEI4 hits have same event number and are within T3MAPS time.
  // get results...
  
  ///// FOR FEI4, chiech that they have the same event number before reco clusters and hits.
  

}
