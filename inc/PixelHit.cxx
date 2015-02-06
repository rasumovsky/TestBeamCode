////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: PixelHit.cxx                                                        //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 06/02/2015                                                          //
//                                                                            //
//  This class stores the basic information associated with a pixel hit that  //
//  is necessary for matching and clusterizing hits.                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "PixelHit.h"

/**
   Initialize a hit:
 */
PixelHit::PixelHit(int row, int col, bool match) {
  hitRow = row;
  hitCol = col;
  hitMatched = match;
  return;
}

void PixelHit::setRow(int row) {
  hitRow = row;
}

void PixelHit::setCol(int col) {
  hitCol = col;
}

void PixelHit::setMatched(bool match) {
  hitMatched = match;
}

int PixelHit::getRow() {
  return hitRow;
}

int PixelHit::getCol() {
  return hitCol;
}

bool PixelHit::isMatched() {
  return hitMatched;
}
