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
  int getNCol(std::string chipName);
  int getNRow(std::string chipName);
  double getColPitch(std::string chipName);
  double getRowPitch(std::string chipName);
  double getThickness(std::string chipName);
  bool isInChip(std::string chip, int row, int col);
  
  // Mutators:
  void setNCol(std::string chipName, int val);
  void setNRow(std::string chipName, int val);
  void setColPitch(std::string chipName, double val);
  void setRowPitch(std::string chipName, double val);
  void setThickness(std::string chipName, double val);
  
 private:
  
  // Member objects:
  std::map<std::string,int> numCol;
  std::map<std::string,int> numRow;
  std::map<std::string,double> colPitch;
  std::map<std::string,double> rowPitch;
  std::map<std::string,double> thickness;
 
};

#endif
