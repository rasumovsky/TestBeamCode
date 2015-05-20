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
#include <iostream>
#include <string>
#include <map>

class ChipDimension {
  
 public:
  
  ChipDimension();
  ~ChipDimension();
  
  // Accessors:
  int getNCol(std::string chipName);
  int getNRow(std::string chipName);
  double getColPitch(std::string chipName);
  double getRowPitch(std::string chipName);
  double getThickness(std::string chipName);
  double getColPosition(std::string chipName, int col);
  double getRowPosition(std::string chipName, int row);
  double getPosition(std::string chipName, std::string parameter, int index);
  int getColFromPos(std::string chipName, double colPos);
  int getRowFromPos(std::string chipName, double rowPos);
  int getIndexFromPos(std::string chipName, std::string parameter, double pos);
  bool isInChip(std::string chipName, int row, int col);
  
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
