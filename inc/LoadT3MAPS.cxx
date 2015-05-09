////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: LoadT3MAPS.cxx                                                      //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 04/02/2015                                                          //
//                                                                            //
//  This class loads the T3MAPS output file and creates a TTree to store      //
//  relevant event information including:                                     //
//         - timestamp_start   (beginning of integration period)              //
//         - timestamp_stop    (end of integration period)                    //
//         - hit_row           (vector of hit rows)                           //
//         - hit_column        (vector of hit columns)                        //
//                                                                            //
//  The class will produce an output root file.                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "LoadT3MAPS.h"

/**
   Initialize the T3MAPS data class. Loads an input textfile, reads the data 
   into a TTree, then saves the TTree in a TFile.
   @param inFileName - the input text file with the T3MAPS hit tables.
   @param outFileName - the name of the file in which the TTree will be saved. 
*/
LoadT3MAPS::LoadT3MAPS(std::string inFileName, std::string outFileName) {
  std::cout << "\nLoadT3MAPS: Initializing..." << "\n\tLoading " << inFileName
	    << "\n\tReturning " << outFileName << std::endl;
  
  nEvents = 0;
    
  int currLineIndex = 0;
  std::string currText;
  char *outFileNameC = (char*)outFileName.c_str();
  // Load output file, configure output TTree:
  outputT3MAPS = new TFile(outFileNameC,"recreate");
  treeT3MAPS = new TTree("TreeT3MAPS","TreeT3MAPS");
  treeT3MAPS->Branch("nHits", &nHits, "nHits/I");
  treeT3MAPS->Branch("timestamp_start", &timestamp_start, "timestamp_start/D");
  treeT3MAPS->Branch("timestamp_stop", &timestamp_stop, "timestamp_stop/D");
  treeT3MAPS->Branch("hit_row", "std::vector<int>", &hit_row);
  treeT3MAPS->Branch("hit_column", "std::vector<int>", &hit_column);
  
  // Open input text file from T3MAPS run:
  char *inFileNameC = (char*)inFileName.c_str();
  ifstream historyFile(inFileNameC);
  if (historyFile.is_open()) {
    while (getline(historyFile, currText) ) {
      
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
	timestamp_start = atoi((char*)currText.c_str());
      }
      // stop time recorded:
      else if (currLineIndex == 4) {
	timestamp_stop = atoi((char*)currText.c_str());
      }
      
      // get hit table information:
      else if (currLineIndex > 4 && currLineIndex < 23) {
	int currRow = currLineIndex - 5;
	
	std::vector<std::string> hitRows = delimString(currText, " ");
	
	// iterate over the columns that were hit in each row:
	for (std::vector<std::string>::iterator it = hitRows.begin(); 
	     it != hitRows.end(); ++it) {
	  int currColumn = atoi(it->c_str());// + 1;
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
    }
  }
  
  historyFile.close();
  treeT3MAPS->Write();
  outputT3MAPS->Close();
  
  std::cout << "\nLoadT3MAPS: Initialized successfully!" << std::endl;
  return;
}

/**
   Returns the number of events in the data.
*/
int LoadT3MAPS::getNEvents() {
  return nEvents;
}

/**
   Returns the TTree produced from the input textfile.
*/
TTree* LoadT3MAPS::getTree() {
  return treeT3MAPS;
}

/**
   Close the input files and delete TTree from memory.
*/
void LoadT3MAPS::closeFiles() {
  outputT3MAPS->Close();
}

/**
   Splits a line of text up into the interpretable chunks.
   @param line - the line of input text.
   @param delim - the delimiter for the text (typically " " or ",").
   @returns - a vetor of strings that were separated by the delim.
 */
std::vector<std::string> LoadT3MAPS::delimString(std::string line, 
						  std::string delim) {
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
