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

class PixelHit 
{
 private:
  
  // Member objects:
  int hitRow;
  int hitCol;
  bool hitMatched;
  
 public:
    
  // Constructor:
  PixelHit(int row, int col, bool match);
  ~PixelHit();
  
  // Mutators:
  void setRow(int row);
  void setCol(int col);
  void setMatched(bool matched);
  
  // Accessors:
  int getRow();
  int getCol();
  bool isMatched();
};

#endif
