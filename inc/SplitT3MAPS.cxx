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
SplitT3MAPS::SplitT3MAPS(std::string inFileName, int scansPerFile) {
  std::cout << std::endl << "SplitT3MAPS::Initializing..." << std::endl;
  
  fileNames.clear();
  int currLineIndex = 0;
  int fileIndex = 1;
  std::string currText;
  
  // Load output file:
  std::ofstream outputFile;
  std::string outputFileName = "formOutput/split_output_1.txt";
  outputFile.open((char*)outputFileName.c_str());
  fileNames.push_back(outputFileName);

  // Open input text file from T3MAPS run:
  std::ifstream historyFile((char*)inFileName.c_str());
  if (historyFile.is_open()) {
    while (getline(historyFile, currText)) {
      
      if (currLineIndex % 100 == 0) { std::cout << currText << std::endl; }
      currLineIndex++;
      outputFile << currText << std::endl;
      
      if ( (((double)currLineIndex)/24.0) == scansPerFile * fileIndex) {
	fileIndex++;
	outputFile.close();
	char buffer[100];
	int n = sprintf(buffer,"formOutput/split_output_%i.txt",fileIndex);
	outputFile.open(buffer);
	fileNames.push_back(std::string(buffer));
      }
    }
  }
  
  historyFile.close();
  nFiles = fileIndex;
  return;
}

/**
   Returns the number of output files that were generated.
 */
int SplitT3MAPS::getNFiles() {
  return nFiles;
}

/**
   Get a list of all the files that were produced.
*/
std::vector<std::string> SplitT3MAPS::getFileNames() {
  return fileNames;
}
