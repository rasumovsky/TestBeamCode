////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: SplitT3MAPS.h                                                       //
//  Class: SplitT3MAPS.cxx                                                    //
//                                                                            //
//  Author: Andrew Hard                                                       //
//  Email: ahard@cern.ch                                                      //
//  Date: 19/02/2015                                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef SplitT3MAPS_h
#define SplitT3MAPS_h

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

class SplitT3MAPS 
{
  
 public:
  
  SplitT3MAPS( std::string inFileName, int scanPerFile);
  virtual ~SplitT3MAPS() {};
  
  // Member functions:
  std::vector<std::string> getFileNames();
  int getNFiles();
  
 private:
  
  int nFiles;
  std::vector<std::string> fileNames;
};

#endif
