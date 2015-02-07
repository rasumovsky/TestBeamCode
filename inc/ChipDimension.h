////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: ChipDimension.h                                                     //
//  Class: ChipDimension.cxx                                                  //
//                                                                            //
//  Author: Andrew Hard                                                       //
//  Email: ahard@cern.ch                                                      //
//  Date: 06/02/2015                                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef ChipDimension_h
#define ChipDimension_h

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <map>

class ChipDimension 
{
  
 public:
  
  ChipDimension();
  ~ChipDimension();
  
  // Accessors:
  int getChipSize(std::string chip, std::string pos);
  bool isInChip(std::string chip, int row, int col);
  
  // Mutators:
  void setChipSize(std::string chip, std::string pos, int val);
  
 private:
  
  // Member objects:
  std::map<std::string,int> nRows;
  std::map<std::string,int> nColumns;

};

#endif
