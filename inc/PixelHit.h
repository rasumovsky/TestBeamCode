////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: PixelHit.h                                                          //
//  Class: PixelHit.cxx                                                       //
//                                                                            //
//  Author: Andrew Hard                                                       //
//  Email: ahard@cern.ch                                                      //
//  Date: 06/02/2015                                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef PixelHit_h
#define PixelHit_h

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cmath>

class PixelHit 
{
  
 public:
  
  PixelHit(int row, int col, int LVL1, int TOT, bool match);
  ~PixelHit();
  
  // Mutators:
  void setRow(int row);
  void setCol(int col);
  void setLVL1(int LVL1);
  void setTOT(int TOT);
  void setMatched(bool matched);
  
  // Accessors:
  bool equalTo(PixelHit* hit);
  int getRow();
  int getCol();
  int getLVL1();
  int getTOT();
  int hitDistanceCol(PixelHit* hit);
  int hitDistanceRow(PixelHit* hit);
  bool isHitMatched();
  bool hitIsAdjacent(PixelHit* hit);
  bool hitIsSamePos(PixelHit* hit);
  void printHit();
  
 private:
  
  // Member objects:
  int hitRow;
  int hitCol;
  int hitLVL1;
  int hitTOT;
  bool hitMatched;
  
};

#endif
