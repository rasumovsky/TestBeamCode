////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: FormatT3MAPS.cxx                                                    //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 20/02/2015                                                          //
//                                                                            //
//  Efficiently converts T3MAPS .txt file to TTree.                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "FormatT3MAPS.h"

int main(int argc, char **argv) {
  
  // Check arguments:
  if (argc < 2) {
    printf("\nUsage: %s <inputT3MAPS.txt> \n\n",argv[0]); 
    exit(0);
  }
  
  std::system("mkdir -vp formOutput/*");
  std::string inputT3MAPS = argv[1];
  
  std::cout << "Running tool to split up single text file." << std::endl;
  SplitT3MAPS *sT = new SplitT3MAPS(inputT3MAPS, 500);
  int numFiles = sT->getNFiles();
  std::cout << "Success! Split into " << numFiles << " files." << std::endl;
  
  std::vector<std::string> inFileList = sT->getFileNames();
  std::vector<std::string> outFileList;
  outFileList.clear();

  // Loop over files, and create a TTree for each:
  for (int i_f = 0; i_f < numFiles; i_f++) {
    
    std::string currInFile = inFileList[i_f];
    char buffer[100];
    int n = sprintf(buffer,"formOutput/ttree_output_%i.root",i_f+1);
    std::string currOutFile = std::string(buffer);
    
    LoadT3MAPS *lT = new LoadT3MAPS(currInFile, currOutFile);
  }
  
  std::system("hadd -f T3MAPS_new.root formOutput/*.root");
  std::system("rm -rf formOutput");
  std::cout << "\nFile location is T3MAPS_new.root!\n" << std::endl;
}
