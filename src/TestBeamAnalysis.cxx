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
  
  TString mapFileDir = "../TestBeamOutput";
    
  // LoadT3MAPS and load the FEI4 TTree
  TFile *fileT3MAPS = new TFile(inputT3MAPS);
  TTree *myTreeT3MAPS = (TTree*)fileT3MAPS->Get("TreeT3MAPS");
  cT = new TreeT3MAPS(myTreeT3MAPS);
  
  TFile *fileFEI4 = new TFile(inputFEI4);
  TTree *myTreeFEI4 = (TTree*)fileFEI4->Get("Table");
  cF = new TreeFEI4(myTreeFEI4);
  
  // Load the chip sizes (but use defaults!)
  myChips = new ChipDimension();
  
  LinearMapMaker *mapper = new LinearMapMaker("","");
  
  // Book histograms:
  TH2D *occFEI4 = new TH2D("occFEI4","occFEI4",
			   myChips->getChipSize("FEI4","nRows"),
			   0.5,
			   0.5+myChips->getChipSize("FEI4","nRows"),
			   myChips->getChipSize("FEI4","nColumns"),
			   0.5,
			   0.5+myChips->getChipSize("FEI4","nColumns"));
  
  
  TH2D *occOverFEI4 = new TH2D("occOverlapFEI4","occOverlapFEI4",
			       myChips->getChipSize("FEI4","nRows"),
			       0.5,
			       0.5+myChips->getChipSize("FEI4","nRows"),
			       myChips->getChipSize("FEI4","nColumns"),
			       0.5,
			       0.5+myChips->getChipSize("FEI4","nColumns"));
  
  TH2D *occT3MAPS = new TH2D("occT3MAPS","occT3MAPS",
			     myChips->getChipSize("T3MAPS","nRows"),
			     0.5,
			     0.5+myChips->getChipSize("T3MAPS","nRows"),
			     myChips->getChipSize("T3MAPS","nColumns"),
			     0.5,
			     0.5+myChips->getChipSize("T3MAPS","nColumns"));
  
  TH2D *occDiffFEI4 = new TH2D("occDiffFEI4","occDiffFEI4",
			       myChips->getChipSize("FEI4","nRows"),
			       0.5,
			       0.5+myChips->getChipSize("FEI4","nRows"),
			       myChips->getChipSize("FEI4","nColumns"),
			       0.5,
			       0.5+myChips->getChipSize("FEI4","nColumns"));
  
  TH1F *hPI_T3MAPS = new TH1F("hPI_T3MAPS", "hPI_T3MAPS", 21, -0.5, 20.5);
  TH1F *hPE_FEI4 = new TH1F("hPE_FEI4", "hPE_FEI4", 51, -0.5, 50.5);
  TH1F *hP_OverFEI4 = new TH1F("hPI_OverFEI4", "hPI_OverFEI4", 51, -0.5, 50.5);
  
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
    
    // Cut on events with no T3MAPS hits:
    if (cT->nHits == 0) {
      continue;
    }
    
    // Cut on events where T3MAPS is fully occupied:
    if (options.Contains("CutFullEvt") && cT->nHits > 5) {
      continue;
    }
        
    // Fill occupancy plot:
    hPI_T3MAPS->Fill(cT->nHits);
    
    for (int i_h = 0; i_h < (int)cT->hit_row->size(); i_h++) {
      occT3MAPS->Fill((*cT->hit_row)[i_h], (*cT->hit_column)[i_h]);
    }    
    
    int nHits_OverFEI4 = 0;
    int nHits_EvtFEI4 = 0;
    int currEvent = 0;
    
    //--------------------//
    // Advance position in the FEI4 tree:
    while (cF->timestamp_start < cT->timestamp_stop && eventFEI4<entriesFEI4) {
            
      // Fill FEI4 occupancy plot:
      occFEI4->Fill(cF->row, cF->column);
      
      // Fill histogram of hits per events:
      if (cF->event_number != currEvent) {
	currEvent = cF->event_number;
	if (currEvent != 0) {
	  hPE_FEI4->Fill(nHits_EvtFEI4);
	}
	nHits_EvtFEI4 = 0;
      }
      
      // Only consider events with timestamp inside that of T3MAPS
      if (cF->timestamp_start >= cT->timestamp_start &&
	  cF->timestamp_stop <= cT->timestamp_stop) {
	
	// Fill overlapping FEI4 hit occupancy plot:
	occOverFEI4->Fill(cF->row, cF->column);
	occDiffFEI4->Fill(cF->row, cF->column, 1.0);
	nHits_OverFEI4++;
		
	PixelHit *currFEI4Hit = new PixelHit(cF->row, cF->column, false);
	
	// Loop over T3MAPS hits to see if any of the mapping pixels were hit:
	for (int i_h = 0; i_h < (int)cT->hit_row->size(); i_h++) {
	  
	  PixelHit *currT3MAPSHit = new PixelHit((*cT->hit_row)[i_h], 
						 (*cT->hit_column)[i_h],
						 false);
	  
	  mapper->addPairToMap(currFEI4Hit, currT3MAPSHit);
	}
      }
      else {
	occDiffFEI4->Fill(cF->row, cF->column, -0.2);
      }
      
      // then advance to the next entry
      eventFEI4++;
      cF->fChain->GetEntry(eventFEI4);
      
    }// end of loop over FEI4 entries
    
    hP_OverFEI4->Fill(nHits_OverFEI4);
    
  }// End of loop over events
  cout << "TestBeamAnalysis: Ending loop to define maps." << endl;
      
  mapper->createMapFromHits();
  mapper->printMapParameters();
  mapper->saveMapParameters(mapFileDir);
  
  // Plot occupancy for FEI4 and T3MAPS:
  PlotUtil *plotter = new PlotUtil("../TestBeamOutput",800,800);
  plotter->plotTH2D(occFEI4, "row_{FEI4}", "column_{FEI4}", "hits", 
		    "occupancyFEI4");
  plotter->plotTH2D(occOverFEI4, "row_{FEI4}", "column_{FEI4}", "hits", 
		    "occupancyOverlappingFEI4");
  plotter->plotTH2D(occT3MAPS, "row_{T3MAPS}", "column_{T3MAPS}", "hits", 
		    "occupancyT3MAPS");
  plotter->plotTH2D(occDiffFEI4, "row_{FEI4}", "column_{FEI4}", "weights", 
		    "occupancyDifferenceFEI4");
  
  plotter->plotTH1F(hPI_T3MAPS, "hits/integration", "entries", "hitsT3MAPS");
  plotter->plotTH1F(hPE_FEI4, "hits/event", "entries", "hitsFEI4");
  plotter->plotTH1F(hP_OverFEI4, "hits/integration", "entries", "hitsOverFEI4");
    
  
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
