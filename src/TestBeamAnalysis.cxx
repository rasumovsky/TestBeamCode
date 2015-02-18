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
  
  TString mapFileName = "combined_map_output.txt";
  
  // Root macros:
  //SetAtlasStyle();
  
  // LoadT3MAPS and load the FEI4 TTree
  TFile *fileT3MAPS = new TFile(inputT3MAPS);
  TTree *myTreeT3MAPS = (TTree*)fileT3MAPS->Get("TreeT3MAPS");
  cT = new TreeT3MAPS(myTreeT3MAPS);
  
  TFile *fileFEI4 = new TFile(inputFEI4);
  TTree *myTreeFEI4 = (TTree*)fileFEI4->Get("Table");
  cF = new TreeFEI4(myTreeFEI4);
  
  /**
     Need to update this map-making section. First of all, it is unwise to use
     the outside rows for mapping. Choose an interior rectangle. 
     
     Then, we want to have protections in place in case the chosen pixel is 
     never hit, or has zero corresponding hits. Have the map util return 
     something like -1, and check that in this code to prevent non-existent
     maps from influencing the meta-map creation.
     
     
  */
  
  // Load the chip sizes (but use defaults!)
  myChips = new ChipDimension();
    
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
    if (currR1 < myChips->getChipSize("T3MAPS","nRows")) {
      currR1++;
      currR2--;
    }
    else {
      currC1++;
      currC2--;
    }
  }
  /*
  // T3MAPS tree variables:
  double t_T3MAPS_timestamp_start;
  double t_T3MAPS_timestamp_stop;
  std::vector<int> t_T3MAPS_hit_row;
  std::vector<int> t_T3MAPS_hit_column;
  
  // FEI4 tree variables:
  int t_FEI4_event_number;
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
  myTreeFEI4->SetBranchAddress("event_number", &t_FEI4_event_number);
  myTreeFEI4->SetBranchAddress("timestamp_start", &t_FEI4_timestamp_start);
  myTreeFEI4->SetBranchAddress("timestamp_stop", &t_FEI4_timestamp_stop);
  myTreeFEI4->SetBranchAddress("row", &t_FEI4_hit_row);
  myTreeFEI4->SetBranchAddress("column", &t_FEI4_hit_column);
  */
  
  // Prepare FEI4 tree for loop inside T3MAPS tree's loop.
  Long64_t entriesFEI4 = cF->fChain->GetEntries();
  Long64_t eventFEI4 = 0;
  cF->fChain->GetEntry(eventFEI4);
  
  // Loop over T3MAPS tree.
  Long64_t entriesT3MAPS = cT->fChain->GetEntries();
  for (Long64_t eventT3MAPS = 0; eventT3MAPS < entriesT3MAPS; eventT3MAPS++) {
    
    cT->fChain->GetEntry(eventT3MAPS);
    
    
    // Now also advance the FEI4 tree.
    while (cF->timestamp_start < cT->timestamp_stop && 
	   eventFEI4 < entriesFEI4) {
      
      eventFEI4++;
      cF->fChain->GetEntry(eventFEI4);
      
      // Only consider events with timestamp inside that of T3MAPS
      // Same as looking for time coincidence hits in FEI4 and T3MAPS
      if (cF->timestamp_start >= cT->timestamp_start &&
	  cF->timestamp_stop <= cT->timestamp_stop) {
	
	PixelHit *currFEI4Hit = new PixelHit(cF->row,
					     cF->column,
					     false);
	
	// Loop over T3MAPS hits to see if any of the mapping pixels were hit:
	for (int i_h = 0; i_h < (int)cT->hit_row->size(); i_h++) {
	  
	  PixelHit *currT3MAPSHit = new PixelHit((*cT->hit_row)[i_h], 
						 (*cT->hit_column)[i_h],
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
      histMapValues[i_p]->Fill(simpleMaps[i_m]->getMapVar(i_p));
    }
  }
  
  // Create a new "meta map" to be used for the efficiency analysis:
  ModuleMapping *combinedMap = new ModuleMapping("NA","none");
  for (int i_p = 0; i_p < 4; i_p++) {
    combinedMap->setMapVar(i_p,histMapValues[i_p]->GetMean());
    combinedMap->setMapRMS(i_p,histMapValues[i_p]->GetRMS());
  }
  combinedMap->printMapParameters();
  combinedMap->saveMapParameters(mapFileName);
  
  // Map has been loaded, may proceed to efficiency analysis.
  
  // in each event, run the MatchMaker class.
  // be sure FEI4 hits have same event number and are within T3MAPS time.
  // get results...
  
  
  map<string,int> nHitSum;
  nHitSum["FEI4_total"] = 0;
  nHitSum["FEI4_matched"] = 0;
  nHitSum["T3MAPS_total"] = 0;
  nHitSum["T3MAPS_matched"] = 0;
  
  ///// FOR FEI4, chiech that they have the same event number before reco clusters and hits.
  // --> THIS PART IS TRICKY!!!
  
  // Loop over TTrees in tandem again
  eventFEI4 = 0;
  cF->fChain->GetEntry(eventFEI4);
  for (Long64_t eventT3MAPS = 0; eventT3MAPS < entriesT3MAPS; eventT3MAPS++) {
    
    cT->fChain->GetEntry(eventT3MAPS);
    
    MatchMaker *myMatch;
    
    // Now also advance the FEI4 tree.
    int prevEvent = -1;
    while (cF->timestamp_start < cT->timestamp_stop && 
	   eventFEI4 < entriesFEI4) {
      
      eventFEI4++;
      cF->fChain->GetEntry(eventFEI4);
      
      if (cF->event_number != prevEvent) {
	
	// Save information on previous match.
	if (prevEvent != -1) {
	  myMatch->buildAndMatchClusters();
	  
	  nHitSum["FEI4_total"] += myMatch->getNHits("FEI4","");
	  nHitSum["FEI4_matched"] += myMatch->getNHits("FEI4","matched");
	  nHitSum["T3MAPS_total"] += myMatch->getNHits("T3MAPS","");
	  nHitSum["T3MAPS_matched"] += myMatch->getNHits("T3MAPS","matched");
	  
	}
	
	myMatch = new MatchMaker(combinedMap);
	// Loop over T3MAPS hits.
	for (int i_h = 0; i_h < (int)cT->hit_row->size(); i_h++) {
	  PixelHit *currT3MAPSHit = new PixelHit((*cT->hit_row)[i_h],
						 (*cT->hit_column)[i_h],
						 false);
	  myMatch->addHitInT3MAPS(currT3MAPSHit);
	}
	
	// Only consider events with timestamp inside that of T3MAPS
	// Same as looking for time coincidence hits in FEI4 and T3MAPS
	if (cF->timestamp_start >= cT->timestamp_start &&
	    cF->timestamp_stop <= cT->timestamp_stop) {
	  
	  PixelHit *currFEI4Hit = new PixelHit(cF->row,
					       cF->column,
					       false);
	  myMatch->addHitInFEI4(currFEI4Hit);
	}
      }
      else {// just add new FEI4 hit.
	// Only consider events with timestamp inside that of T3MAPS
	// Same as looking for time coincidence hits in FEI4 and T3MAPS
	if (cF->timestamp_start >= cT->timestamp_start &&
	    cF->timestamp_stop <= cT->timestamp_stop) {
	  
	  PixelHit *currFEI4Hit = new PixelHit(cF->row,
					       cF->column,
					       false);
	  myMatch->addHitInFEI4(currFEI4Hit);
	}
      }
      
      prevEvent = cF->event_number;
    }
  }
  
  // Print the results:
  cout << "TestBeamAnalysis is complete. Results:" << endl;
  cout << "  FEI4 Hits:" << endl;
  cout << "    Matched = " << nHitSum["FEI4_matched"] << endl;
  cout << "    Total   = " << nHitSum["FEI4_total"] << endl;
  cout << "    Ratio   = " << (((double)nHitSum["FEI4_matched"]) /
			       ((double)nHitSum["FEI4_total"])) << endl << endl;
  cout << "  T3MAPS Hits:" << endl;
  cout << "    Matched = " << nHitSum["T3MAPS_matched"] << endl;
  cout << "    Total   = " << nHitSum["T3MAPS_total"] << endl;
  cout << "    Ratio   = " << (((double)nHitSum["T3MAPS_matched"]) /
			       ((double)nHitSum["T3MAPS_total"])) << endl;
  return 1;
}
