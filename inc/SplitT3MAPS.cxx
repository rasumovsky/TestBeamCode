////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: SplitT3MAPS.cxx                                                     //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 19/02/2015                                                          //
//                                                                            //
//  This class takes the history file as an input and splits it up into       //
//  multiple smaller files.                                                   //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "SplitT3MAPS.h"

/**
   Initialize the T3MAPS data class. Loads an input textfile, reads the data 
   into a TTree, then saves the TTree in a TFile.
*/
SplitT3MAPS::SplitT3MAPS(std::string inFileName, std::string outFileName,
			 int scansPerJob, int job) {
  std::cout << std::endl << "SplitT3MAPS::Initializing..." << std::endl;
  
  nEvents = 0;
  
  gROOT->ProcessLine("#include <vector>");
  
  int totalLineIndex = 0;
  int currLineIndex = 0;
  std::string currText;
  char *outFileNameC = (char*)outFileName.c_str();
  // Load output file, configure output TTree:
  outputT3MAPS = new TFile(outFileNameC,"recreate");
  treeT3MAPS = new TTree("TreeT3MAPS","TreeT3MAPS");
  treeT3MAPS->Branch("nHits", &nHits, "nHits/I");
  treeT3MAPS->Branch("timestamp_start", &timestamp_start, "timestamp_start/D");
  treeT3MAPS->Branch("timestamp_stop", &timestamp_stop, "timestamp_stop/D");
  treeT3MAPS->Branch("hit_row", &hit_row);
  treeT3MAPS->Branch("hit_column", &hit_column);
  
  // Open input text file from T3MAPS run:
  char *inFileNameC = (char*)inFileName.c_str();
  ifstream historyFile(inFileNameC);
  if (historyFile.is_open()) {
    while (getline(historyFile, currText) &&
	   totalLineIndex < (24*job*scansPerJob)) {
      
      if (totalLineIdex < (24*(job-1)*scansPerJob) ) {
	totalLineIndex++;
	continue;
      }
      else {
	// Start counting the line numbers (one run is 0-23)
	std::size_t foundText = currText.find("BEGIN SCAN");
	if (foundText!=std::string::npos) {
	  currLineIndex = 0;
	  hit_row.clear();
	  hit_column.clear();
	  nHits = 0;
	  if (nEvents % 100 == 0) { std::cout << currText << std::endl; }
	}
	
	// start time recorded:
	if (currLineIndex == 2) { 
	  timestamp_start = strtod(currText.c_str(),"");
	}
	// stop time recorded:
	else if (currLineIndex == 4) {
	  timestamp_stop = strtod(currText.c_str(),"");
	}
	
	// get hit table information:
	else if (currLineIndex > 4 && currLineIndex < 23) {
	  int currRow = currLineIndex - 5;
	  
	  std::vector<std::string> hitColumns = delimString(currText, " ");
	  
	  // iterate over the columns that were hit in each row:
	  for (std::vector<std::string>::iterator it = hitColumns.begin(); 
	       it != hitColumns.end(); ++it) {
	    int currColumn = atoi(it->c_str()) + 1;
	    hit_row.push_back(currRow);
	    hit_column.push_back(currColumn);
	    nHits++;
	  }
	}
	
	// end scan, save event information:
	else if (currLineIndex == 23) {
	  treeT3MAPS->Fill();
	  nEvents++;
	}
	
	// increment the line number:
	currLineIndex++;
	totalLineIndex++;
      }
    }
  }
  
  historyFile.close();
  treeT3MAPS->Write();
  outputT3MAPS->Close();
  
  return;
}

/**
   Returns the number of events in the data.
*/
int SplitT3MAPS::getNEvents() {
  return nEvents;
}

/**
   Returns the TTree produced from the input textfile.
*/
TTree* SplitT3MAPS::getTree() {
  return treeT3MAPS;
}

/**
   Close the input files and delete TTree from memory.
*/
void SplitT3MAPS::closeFiles() {
  //treeT3MAPS->Delete();
  outputT3MAPS->Close();
}

/**
   Splits a line of text up into the interpretable chunks.
 */
std::vector<std::string> SplitT3MAPS::delimString( std::string line, 
						  std::string delim ) {
  // vector to return (for table)
  std::vector<std::string> result;
  result.clear();
  
  // the positions in the string of delims:
  std::size_t found = line.find(delim);
  std::size_t found_prev = 0;
  
  while (found!=std::string::npos) {
    std::string token = line.substr(found_prev, found);
    result.push_back(token);
    found_prev = found;
    found = line.find(delim,found+1);
  }
  
  return result;  
}
