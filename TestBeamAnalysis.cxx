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

int TestBeamAnalysis::main( int argc, char **argv ) {
  
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
  
  // in this program, first call LoadT3MAPS, then call Map, then HitMatch

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
  
