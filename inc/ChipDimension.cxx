////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: ChipDimension.cxx                                                   //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 06/02/2015                                                          //
//                                                                            //
//  This class stores the dimensions of the FEI4 and T3MAPS chips. Mutators   //
//  are provided but probably don't ever need to be used.                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "ChipDimension.h"

/**
   Initialize the class with default values:
 */
ChipDimension::ChipDimension() {
  nRows["FEI4"] = 336;
  nColumns["FEI4"] = 80;
  nRows["T3MAPS"] = 16;
  nColumns["T3MAPS"] = 64;
  return;
}

/**
   Mutator to change the dimensions of the given chip ("FEI4" or "T3MAPS"):
 */
void ChipDimension::setChipSize(std::string chip, std::string pos, int val) {
  if (pos == "nColumns") nColumns[chip] = val;
  else if (pos == "nRows") nRows[chip] = val;
}

/**
   Returns the number of columns or rows for the "FEI4" or the "T3MAPS" chip.
 */
int ChipDimension::getChipSize(std::string chip, std::string pos) {
  if (pos == "nColumns") return nColumns[chip];
  else if (pos == "nRows") return nRows[chip];
  else return -1;
}

/**
   Checks to see whether a given row and column are compatible with given chip.
 */
bool ChipDimension::isInChip(std::string chip, int row, int col) {
  if (row > 0 && row < nRows[chip] && col > 0 && col < nColumns[chip]) {
    return true;
  }
  else {
    return false;
  }
}
