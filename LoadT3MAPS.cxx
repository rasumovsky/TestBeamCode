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
////////////////////////////////////////////////////////////////////////////////

#include "LoadT3MAPS.h"

//--------------------------------------//
// LoadT3MAPS: For the initialization, load table values fr.
LoadT3MAPS::LoadT3MAPS( std::string inFileName, std::string outFileName ) {
  std::cout << std::endl << "LoadT3MAPS::Initializing..." << std::endl;
  
  nEvents = 0;
  
  int currLineIndex = 0;
  std::string currText;

  // Load output file, configure output TTree:
  outputT3MAPS = new TFile(outFileName,"recreate");
  treeT3MAPS = new TTree("TreeT3MAPS","TreeT3MAPS");
  treeT3MAPS.Branch("timestamp_start", &timestamp_start, "timestamp_start/D");
  treeT3MAPS.Branch("timestamp_stop", &timestamp_stop, "timestamp_stop/D");
  treeT3MAPS.Branch("hit_row", &hit_row);
  treeT3MAPS.Branch("hit_column", &hit_column);
  
  // Open input text file from T3MAPS run:
  ifstream historyFile(inFileName);
  if (historyFile.is_open()) {
    while (getline(historyFile, currText) ) {
      std::cout << currText << std::endl;
      
      // Start counting the line numbers (one run is 0-23)
      std::size_t foundText = currText.find("BEGIN SCAN");
      if (foundText!=std::string::npos) {
	currLineIndex = 0;
	hit_row.clear();
	hit_column.clear();
      }
      
      // start time recorded:
      if (currLineIndex == 2) timestamp_start = atoi(currText.c_str());
      
      // stop time recorded:
      else if (currLineIndex == 4) timestamp_stop = atoi(currText.c_str());
      
      // get hit table information:
      else if (currLineIndex > 4 && currLineIndex < 23) {
	int currRow = currLineIndex - 4;
	
	std::vector<std::string> hitColumns = delimString(currText, " ");
	
	// iterate over the columns that were hit in each row:
	for (std::vector<string>::iterator it = hitColumns.begin(); 
	     it != hitColumns.end(); ++it) {
	  int currColumn = atoi(*it->c_str());
	  hit_row.push_back(currRow);
	  hit_column.push_back(currColumn);
	}
      }
      
      // end scan, save event information:
      else if (currLineIndex == 23) {
	treeT3MAPS.Fill();
	nEvents++;
      }
            
      // increment the line number:
      currLineIndex++;
    }
  }
  
  historyFile.close();
  
  //TDirectory *savedir = gDirectory;
  //fOutputFile->cd();
  treeT3MAPS.Write();
  //treeT3MAPS.SetDirectory(0);
  //gDirectory = savedir;
  outputT3MAPS->Close();
  
  return;
}

//--------------------------------------//
// delimString:
std::vector<std::string> LoadT3MAPS::delimString( std::string line, 
						 std::string delim ) {
  // vector to return (for table)
  std::vector<std::string> result;
  result.clear();
  
  // the positions in the string of delims:
  std::size_t found = line.find(delim);
  std::size_t found_prev = 0;
  
  while (found!=std::string::npos) {
    std::string token = s.substr(found_prev, found);
    result.push_back(token);
    found_prev = found;
    found = line.find(delim,found+1);
  }
  
  return result;  
}

//--------------------------------------//
// getNHits:
int LoadT3MAPS::getNEvents() {
  return nEvents;
}

//--------------------------------------//
// getEvent:
TTree* LoadT3MAPS::getTree() {
  return treeT3MAPS;
}

//--------------------------------------//
// closeFiles:
void LoadT3MAPS::closeFiles() {
  //treeT3MAPS->Delete();
  outputT3MAPS->Close();
}
