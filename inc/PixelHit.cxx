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
   Initialize the hit class with hit properties.
   @param row - hit pixel row.
   @param col - hit pixel column.
   @param LVL1 - hit trigger.
   @param TOT - hit time over threshold (converted to charge).
   @param match - true if the hit is matched.
*/
PixelHit::PixelHit(int row, int col, int LVL1, int TOT, bool match) {
  setRow(row);
  setCol(col);
  setLVL1(LVL1);
  setTOT(TOT);
  setMatched(match);
  return;
}

/**
   Set the row location of the hit.
   @param row - the input hit row.
*/
void PixelHit::setRow(int row) {
  hitRow = row;
}

/**
   Set the column location of the hit.
   @param col - the input hit column.
*/
void PixelHit::setCol(int col) {
  hitCol = col;
}

/**
   Set the LVL1 value of the hit.
   @param LVL1 - the hit's LVL1 value.
*/ 
void PixelHit::setLVL1(int LVL1) {
  hitLVL1 = LVL1;
}

/**
   Set the time over threshold of the hit.
   @param TOT - the input time over threshold.
*/
void PixelHit::setTOT(int TOT) {
  hitTOT = TOT;
  // set charge at the same time as TOT:
  setCharge(calculateQ(TOT));
}

/**
   Set the matching status of the hit.
   @param match - true iff the hit is matched.
*/
void PixelHit::setMatched(bool match) {
  hitMatched = match;
}

/**
   Checks if two hits are equivalent.
   @param hit - the input hit to test for equality with this hit.
   @returns - true iff the hits have all of the same properties.
*/
bool PixelHit::equalTo(PixelHit* hit) {
  return (hitRow == hit->getRow() && hitCol == hit->getCol() &&
	  hitLVL1 == hit->getLVL1() && hitTOT == hit->getTOT());
}

/**
   Get the row location of the hit.
   @returns - the hit row.
*/
int PixelHit::getRow() {
  return hitRow;
}

/**
   Get the column location of the hit.
   @returns - the hit column.
*/
int PixelHit::getCol() {
  return hitCol;
}

/**
   Get the LVL1 value of the hit.
   @returns - the hit LVL1 value.
*/
int PixelHit::getLVL1() {
  return hitLVL1;
}

/**
   Get the time over threshold of the hit.
   @returns - the hit time over threshold.
*/
int PixelHit::getTOT() {
  return hitTOT;
}

/**
   Get the column distance between two hits.
   @param hit - the hit to measure against this one.
   @returns - the column separation of the two hits.
*/
int PixelHit::hitDistanceCol(PixelHit* hit) {
  return fabs(hitCol - hit->getCol());
}

/**
   Get the row distance between two hits.
   @param hit - the hit to measure against this one.
   @returns - the row separation of the two hits.
*/
double PixelHit::hitDistanceRow(PixelHit* hit) {
  return fabs(hitRow - hit->getRow());
}

/**
   Get the matching status of the hit.
   @returns - true iff hit is matched.
*/
bool PixelHit::isHitMatched() {
  return hitMatched;
}
/**
   Check if two hits are adjacent.
   @param hit - the hit to measure against this one.
   @returns - true iff the two hits are next to each other or are in the same
   position on the chip. 
*/
bool PixelHit::hitIsAdjacent(PixelHit* hit) {
  if (hit->getRow() >= hitRow-1 && hit->getRow() <= hitRow+1 &&
      hit->getCol() >= hitCol-1 && hit->getCol() <= hitCol+1) {
    return true;
  }
  else {
    return false;
  }
}

/**
   Check if two hits are in the same position.
   @param hit - the hit to measure against this one.
   @returns - true iff the two hits have the same row and col positons.
*/
bool PixelHit::hitIsSamePos(PixelHit* hit) {
  return (hitRow == hit->getRow() && hitCol == hit->getCol());
}

/**
   Print the pixel values.
*/
void PixelHit::printHit() {
  std::cout << "Printing PixelHit properties. \nRow = " << getRow()
	    << "\n Column = " << getCol() << "\nLVL1 = " << getLVL1()
	    << "\nTOT = " << getTOT() << std::endl;
}
