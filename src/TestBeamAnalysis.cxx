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

#include "TestBeamAnalysis.h"

int main(int argc, char **argv) {
  
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
  
  /*
  // Initialize several ModuleMapping instances
  int currR1 = 3;
  int currC1 = 3;
  int currR2 = myChips->getChipSize("T3MAPS","nRows")-2;
  int currC2 = myChips->getChipSize("T3MAPS","nColumns")-2;
  
  for (int i = 0; i < 4; i++) {
    histMapValues[i] = new TH1F(Form("histMapValues%i",i),
				Form("histMapValues%i",i),
				40,-2,2);
  }
  
  int const nMaps = (myChips->getChipSize("T3MAPS","nRows") +
		     myChips->getChipSize("T3MAPS","nColumns") - 8);
  
  ModuleMapping *simpleMaps[nMaps];
  for (int i = 0; i < nMaps; i++) {
    
    simpleMaps[i] = new ModuleMapping("NA","new");
    simpleMaps[i]->designatePixelPair(currR1, currC1, currR2, currC2);
    
    // Advance each of the points around T3MAPS edge:
    if (currR1 < myChips->getChipSize("T3MAPS","nRows")-2) {
      currR1++;
      currR2--;
    }
    else {
      currC1++;
      currC2--;
    }
  }
  */
  
  MapMaker *mapper = new MapMaker("NA","new");
  
  cout << "TestBeamAnalysis: Beginning loop to define maps." << endl;
  
  // Prepare FEI4 tree for loop inside T3MAPS tree's loop.
  Long64_t entriesFEI4 = cF->fChain->GetEntries();
  Long64_t eventFEI4 = 0;
  cF->fChain->GetEntry(eventFEI4);
  cout << "Entries in FEI4 = " << entriesFEI4 << endl;
  
  // Loop over T3MAPS tree.
  Long64_t entriesT3MAPS = cT->fChain->GetEntries();
  cout << "Entries in T3MAPS = " << entriesT3MAPS << endl;
  for (Long64_t eventT3MAPS = 0; eventT3MAPS < entriesT3MAPS; eventT3MAPS++) {
    
    cT->fChain->GetEntry(eventT3MAPS);
    
    // Cut on events where T3MAPS is fully occupied.
    if (options.Contains("CutFullEvt") && cT->nHits >= 50) {
      continue;
    }
    
    //cout << "  T3MAPS event " << eventT3MAPS << endl;
    
    // Now also advance the FEI4 tree.
    while (cF->timestamp_start < cT->timestamp_stop && 
	   eventFEI4 < entriesFEI4) {
      
      //cout << "    FEI4 event " << eventFEI4 << endl;
      
      // Only consider events with timestamp inside that of T3MAPS
      // Same as looking for time coincidence hits in FEI4 and T3MAPS
      if (cF->timestamp_start >= cT->timestamp_start &&
	  cF->timestamp_stop <= cT->timestamp_stop) {
	
	//printf("\tMATCH! FEI4( %2.2f, %2.2f ) \tT3MAPS( %2.2f, %2.2f )\n",cF->timestamp_start, cF->timestamp_stop, cT->timestamp_start, cT->timestamp_stop);
	
	PixelHit *currFEI4Hit = new PixelHit(cF->row, cF->column, false);
	
	// Loop over T3MAPS hits to see if any of the mapping pixels were hit:
	for (int i_h = 0; i_h < (int)cT->hit_row->size(); i_h++) {
	  
	  PixelHit *currT3MAPSHit = new PixelHit((*cT->hit_row)[i_h], 
						 (*cT->hit_column)[i_h],
						 false);
	  
	  /*
	  for (int i_m = 0; i_m < nMaps; i_m++) {
	    
	    if (simpleMaps[i_m]->isPixelHit(0,currT3MAPSHit)) {
	      simpleMaps[i_m]->addHitToMap(0,currFEI4Hit);
	      //cout << "\t  For T3MAPS(" << simpleMaps[i_m]->getPixPos("row",0) << ", " << simpleMaps[i_m]->getPixPos("col",0) << "), Add FEI4(" << currFEI4Hit->getRow() << ", " << currFEI4Hit->getCol() << ") " << endl;
	    }
	    
	    if (simpleMaps[i_m]->isPixelHit(1,currT3MAPSHit)) {
	      simpleMaps[i_m]->addHitToMap(1,currFEI4Hit);
	      //cout << "\t  For T3MAPS(" << simpleMaps[i_m]->getPixPos("row",1) << ", " << simpleMaps[i_m]->getPixPos("col",1) << "), Add FEI4(" << currFEI4Hit->getRow() << ", " << currFEI4Hit->getCol() << ") " << endl;
	    }	  
	  }
	  */
	  
	  mapper->addPairToMap(currFEI4Hit, currT3MAPSHit);
	  
	}
      }
      
      // then advance to the next entry
      eventFEI4++;
      cF->fChain->GetEntry(eventFEI4);
      
    }// end of loop over FEI4 entries
  }// End of loop over events
  cout << "TestBeamAnalysis: Ending loop to define maps." << endl;
  
  // Initialize the plotting utility
  PlotUtil *plotter = new PlotUtil("output/",800,800);
    
  // Collect the results of multiple maps:
  /*
  TH1F *histMapValues[4];
  for (int i_p = 0; i_p < 4; i_p++) {
    histMapValues[i_p] = new TH1F(Form("histMapValues%i",i_p),
				  Form("histMapValues%i",i_p),
				  40,-2,2);
  }
  
  // Make plot of FEI4 row and col for both T3MAPS pixels (4 histograms):
  for (int i_m = 0; i_m < nMaps; i_m++) {
    simpleMaps[i_m]->createMapFromHits();
    plotter->plotTH2D( simpleMaps[i_m]->getHitPlot(0), "row", "col", "hits",
    		       Form("hit2D_pix%d_%d",0,i_m) );
    plotter->plotTH2D( simpleMaps[i_m]->getHitPlot(1), "row", "col", "hits",
    		       Form("hit2D_pix%d_%d",1,i_m) );
    
    // save map parameters in histograms:
    for (int i_p = 0; i_p < 4; i_p++) {
      histMapValues[i_p]->Fill(simpleMaps[i_m]->getMapVar(i_p));
    }
  }
  */
  
  mapper->createMapFromHits();
  plotter->plotTH2D(mapper->getHitPlot(), "row", "col", "hits", "hitDiff2D");
  
  /*
  // Create a new "meta map" to be used for the efficiency analysis:
  ModuleMapping *combinedMap = new ModuleMapping("NA","none");
  for (int i_p = 0; i_p < 4; i_p++) {
    combinedMap->setMapVar(i_p,histMapValues[i_p]->GetMean());
    combinedMap->setMapRMS(i_p,histMapValues[i_p]->GetRMS());
    plotter->plotTH1F(histMapValues[i_p], Form("parameter %d",i_p), "Entries",
    		      Form("global_param%d",i_p));
  }
  combinedMap->printMapParameters();
  combinedMap->saveMapParameters(mapFileName);
  */
  
  mapper->printMapParameters();
  mapper->saveMapParameters(mapFileName);
  
  // Map has been loaded, may proceed to efficiency analysis.
  
  // in each event, run the MatchMaker class.
  // be sure FEI4 hits have same event number and are within T3MAPS time.
  // get results...
  /*
  
  map<string,int> nHitSum;
  nHitSum["FEI4_total"] = 0;
  nHitSum["FEI4_matched"] = 0;
  nHitSum["T3MAPS_total"] = 0;
  nHitSum["T3MAPS_matched"] = 0;
  
  ///// FOR FEI4, chiech that they have the same event number before reco clusters and hits.
  // --> THIS PART IS TRICKY!!!
  
  // Loop over TTrees in tandem again
  cout << "TestBeamAnalysis: Beginning loop to measure efficiency." << endl;
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
  cout << "TestBeamAnalysis: Ending loop to measure efficiency." << endl;
  
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

  */

}
