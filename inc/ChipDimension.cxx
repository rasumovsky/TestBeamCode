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
   Initialize the class with default values for FEI4 and T3MAPS.
 */
ChipDimension::ChipDimension() {
  std::cout << "ChipDimesion: Initializing..." << std::endl;
  setNCol("FEI4", 80);
  setNRow("FEI4", 336);
  setColPitch("FEI4", 250.0);//micrometers
  setRowPitch("FEI4", 50.0);//micrometers
  setThickness("FEI4", 200.0);//micrometers
  setNCol("T3MAPS", 18);
  setNRow("T3MAPS", 64);
  setColPitch("T3MAPS", 225.0);//micrometers
  setRowPitch("T3MAPS", 22.0);//micrometers
  setThickness("T3MAPS", 13.0);//micrometers
  std::cout << "ChipDimesion: Initialized with default values." << std::endl;
  return;
}

/**
   Returns the number of columns for the given chip.
   @param chipName - the name of the chip ("FEI4" or "T3MAPS").
   @returns - the number of columns in the chip.
 */
int ChipDimension::getNCol(std::string chipName) {
  if (chipName.compare("FEI4") == 0 || chipName.compare("T3MAPS") == 0) {
    return numCol[chipName];
  }
  else {
    std::cout << "ChipDimension: Bad chip name " << chipName << std::endl;
    return 0;
  }
}

/**
   Returns the number of rows for the given chip.
   @param chipName - the name of the chip ("FEI4" or "T3MAPS").
   @returns - the number of rows in the chip.
 */
int ChipDimension::getNRow(std::string chipName) {
  if (chipName.compare("FEI4") == 0 || chipName.compare("T3MAPS") == 0) {
    return numRow[chipName];
  }
  else {
    std::cout << "Chip Dimension: Bad chip name " << chipName << std::endl;
    return 0;
  }
}

/**
   Returns the column pitch for the given chip.
   @param chipName - the name of the chip ("FEI4" or "T3MAPS").
   @returns - the column pitch.
 */
double ChipDimension::getColPitch(std::string chipName) {
  if (chipName.compare("FEI4") == 0 || chipName.compare("T3MAPS") == 0) {
    return colPitch[chipName];
  }
  else {
    std::cout << "ChipDimension: Bad chip name " << chipName << std::endl;
    return 0.0;
  }
}

/**
   Returns the row pitch for the given chip.
   @param chipName - the name of the chip ("FEI4" or "T3MAPS").
   @returns - the row pitch.
 */
double ChipDimension::getRowPitch(std::string chipName) {
  if (chipName.compare("FEI4") == 0 || chipName.compare("T3MAPS") == 0) {
    return rowPitch[chipName];
  }
  else {
    std::cout << "ChipDimension: Bad chip name " << chipName << std::endl;
    return 0.0;
  }
}

/**
   Returns the thickness for the given chip.
   @param chipName - the name of the chip ("FEI4" or "T3MAPS").
   @returns - the chip thickness.
*/
double ChipDimension::getThickness(std::string chipName) {
  if (chipName.compare("FEI4") == 0 || chipName.compare("T3MAPS") == 0) {
    return thickness[chipName];
  }
  else {
    std::cout << "ChipDimension: Bad chip name " << chipName << std::endl;
    return 0.0;
  }
}

/**
   Returns the distance away from column 0, in micrometers. 
   @param chipName - the name of the chip ("FEI4" or "T3MAPS").
   @param col - the column number.
   @returns - the distance away from row 0 in micrometers.
*/
double ChipDimension::getColPosition(std::string chipName, int col) {
  return ((double)col * getColPitch(chipName));
}

/**
   Returns the distance away from row 0, in micrometers.
   @param chipName - the name of the chip ("FEI4" or "T3MAPS").
   @param row - the row number.
   @returns - the distance away from column 0 in micrometers.
*/
double ChipDimension::getRowPosition(std::string chipName, int row) {
  return ((double)row * getRowPitch(chipName));
}

/**
   Checks to see whether a given row and column are compatible with given chip.
   @param chipName - the name of the chip ("FEI4" or "T3MAPS").
   @param row - the the chip row.
   @param col - the column of the chip, starting at 1
*/
bool ChipDimension::isInChip(std::string chipName, int row, int col) {
  if (chipName.compare("FEI4") == 0 || chipName.compare("T3MAPS") == 0) {
    if (row >= 0 && row < numRow[chipName] &&
	col >= 0 && col < numCol[chipName]) {
      return true;
    }
    else {
      return false;
    }
  }
  else {
    std::cout << "ChipDimension: Bad chip name " << chipName << std::endl;
    return false;
  }
}

/**
   Set the number of columns in the chip.
   @param chipName - the name of the chip ("FEI4" or "T3MAPS").
   @param val - the new column number value.
*/
void ChipDimension::setNCol(std::string chipName, int val) {
  numCol[chipName] = val;
}

/**
   Set the number of rows in the chip.
   @param chipName - the name of the chip ("FEI4" or "T3MAPS").
   @param val - the new row number value.
*/
void ChipDimension::setNRow(std::string chipName, int val) {
  numRow[chipName] = val;
}

/**
   Set the column pitch in the chip.
   @param chipName - the name of the chip ("FEI4" or "T3MAPS").
   @param val - the new column pitch value.
*/
void ChipDimension::setColPitch(std::string chipName, double val) {
  colPitch[chipName] = val;
}

/**
   Set the row pitch in the chip.
   @param chipName - the name of the chip ("FEI4" or "T3MAPS").
   @param val - the new row pitch value.
*/
void ChipDimension::setRowPitch(std::string chipName, double val) {
  rowPitch[chipName] = val;
}

/**
   Set the thickness of the chip.
   @param chipName - the name of the chip ("FEI4" or "T3MAPS").
   @param val - the new thickness value.
*/
void ChipDimension::setThickness(std::string chipName, double val) {
  thickness[chipName] = val;
}
