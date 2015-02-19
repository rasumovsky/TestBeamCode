////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: MapMaker.cxx                                                        //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 05/02/2015                                                          //
//                                                                            //
//  This class loads the FE-I4 and T3MAPS data structures and then creates a  //
//  one-to-one map between T3MAPS and FEI4 pixels.                            //
//                                                                            //
//  Linear map format:                                                        //
//        row_FEI4 = mVar[0] * row_T3MAPS + mVar[1];                          //
//        col_FEI4 = mVar[2] * col_T3MAPS + mVar[3];                          //
//                                                                            //
//  TODO: plotting and saving of histograms                                   //
//                                                                            //
//  NOTE: looping over TTrees should not be done here!!! moving those methods //
//  to TestBeamAnalysis for the time being. Will need other methods to add    //
//  information as it is necessary for filling histograms etc.                //
//                                                                            //
//  Typical run:                                                              //
//    1. MapMaker() initialize, either by loading or not.                     //
//    if not from file:                                                       //
//      2. designatePixelPair() in T3MAPS to use for calculating map.         //
//      3. addHitToMap() to add hits for calculation                          //
//           ->this should be done inside some loop over TTrees in main class //
//      4. createMapFromHits() uses the added hits to create a new map.       //
//      5. You're ready to map!                                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "MapMaker.h"

/**
   Initialize the class either using the results of a previous run or using the
   intputs from a loop over TTrees.
*/
MapMaker::MapMaker(TString fileDir, TString option) {
  
  std::cout << "Initializing MapMaker..." << std::endl;
  // Settings for FEI4 and T3MAPS chip layouts:
  myChips = new ChipDimension();
  
  if (option.Contains("FromFile")) {
    loadMapParameters(fileDir);// Load map from file
  }
  else {
    for (int i = 0; i < 4; i++) hasMap[i] = false;
  }
  
  // Initialize the 2D histogram for correlation calculations.
  hist2dFEI4Pix = new TH2D("FEI4Pix2d",
			   "FEI4Pix2d",
			   2*myChips->getChipSize("FEI4","nRows"),
			   -0.5-((double)myChips->getChipSize("FEI4","nRows")),
			   0.5+((double)myChips->getChipSize("FEI4","nRows")),
			   2*myChips->getChipSize("FEI4","nColumns"),
			   -0.5-((double)myChips->getChipSize("FEI4",
							      "nColumns")),
			   0.5+((double)myChips->getChipSize("FEI4",
							     "nColumns")));
  
  std::cout << "Successfully initialized MapMaker!" << std::endl;
}

/**
Load parameters from previous mapping:
*/
void MapMaker::loadMapParameters(TString inputDir) {
  ifstream inputFile;
  inputFile.open(Form("%s/mapParameters.txt",inputDir.Data()));
  std::string paramName; double tempMapVar; double tempMapRMS;
  int lineIndex = 0;
  while (!inputFile.eof()) {
    inputFile >> paramName >> tempMapVar >> tempMapRMS;
    mVar[lineIndex] = tempMapVar;
    mRMS[lineIndex] = tempMapRMS;
    hasMap[lineIndex] = true;
    lineIndex++;
  }
  inputFile.close();
  printMapParameters();
}

/**
Create output text file with map parameters:
*/
void MapMaker::saveMapParameters(TString outputDir) {
  ofstream outputFile;
  outputFile.open(Form("%s/mapParameters.txt",outputDir.Data()));
  for (int i = 0; i < 4; i++) {
    outputFile << "parameter" << i << " " << mVar[i] << " " 
	       << mRMS[i] << std::endl;
  }
  outputFile.close();
}

/**
   Sets the RMS for a parameter for the linear map. Index = 0,1,2,3.
*/
void MapMaker::setMapRMS(int varIndex, double newVal) {
  if (varIndex >= 0 && varIndex < 4) {
    mRMS[varIndex] = newVal;
  }
  else {
    std::cout << "    MapMaker::setMapVar Improper varIndex!" << std::endl;
  }
}

/**
   Sets a parameter for the linear map. Index = 0,1,2,3.
   newVal is the new value of the specified mapping parameter.
 */
void MapMaker::setMapVar(int varIndex, double newVal) {
  if (varIndex >= 0 && varIndex < 4) {
    mVar[varIndex] = newVal;
    hasMap[varIndex] = true;
  }
  else {
    std::cout << "    MapMaker::setMapVar Improper varIndex!" << std::endl;
  }
}

/**
   Add a hit to be used for either pixel 0 or pixel 1 in defining the map.
 */
void MapMaker::addPairToMap(PixelHit *hitFEI4, PixelHit *hitT3MAPS) {
  hist2dFEI4Pix->Fill(hitFEI4->getRow()-hitT3MAPS->getRow(),
		      hitFEI4->getCol()-hitT3MAPS->getCol());
}

/**
   Uses the hits added from the addHitToMap method to calculate new parameters
   for the mapping. This is where the algebra happens.
 */
void MapMaker::createMapFromHits() {
  mVar[0] = 1;
  mVar[2] = 1;
  mVar[1] = hist2dFEI4Pix->ProjectionX()->GetMean();
  mVar[3] = hist2dFEI4Pix->ProjectionY()->GetMean();
  for (int i = 0; i < 4; i++) hasMap[i] = true;
}

/**
   Checks that all four parameters necessary for a complete T3MAPS<-->FEI4
   mapping have been assigned.
*/
bool MapMaker::mapExists() {
  if (hasMap[0] && hasMap[1] && hasMap[2] && hasMap[3]) {
    return true;
  }
  else {
    return false;
  }
}

/**
   Converts T3MAPS row or col number to the corresponding FEI4 row or col.
   Returns -1 if the returned FEI4 row or column is outside defined range.
*/
int MapMaker::getFEI4fromT3MAPS(TString valName, int valT3MAPS) {
  if (mapExists()) {
    if (valName.Contains("row")) {
      int rowVal = (int)(mVar[0] * ((double)valT3MAPS) + mVar[1]);
      int rowSigma = (int)((mVar[0]+mRMS[0]) * ((double)valT3MAPS) 
			   + (mVar[1]+mRMS[1])) - rowVal;
      if (rowSigma < 0) rowSigma = -1 * rowSigma;
      if (myChips->isInChip("FEI4",rowVal,1)) {
	if (valName.Contains("Val")) return rowVal;
	else if (valName.Contains("Sigma")) return rowSigma;
      }
      else {
	return -1;
      }
    }
    else if (valName.Contains("col")) {
      int colVal = (int)(mVar[2] * ((double)valT3MAPS) + mVar[3]);
      int colSigma = (int)((mVar[2]+mRMS[2]) * ((double)valT3MAPS) + (mVar[3]+mRMS[3])) - colVal;
      if (colSigma < 0) colSigma = -1 * colSigma;
      if (myChips->isInChip("FEI4",1,colVal)) {
	if (valName.Contains("Val")) return colVal;
	else if (valName.Contains("Sigma")) return colSigma;
      }
      else {
	return -1;
      }
    }
  }
  return -1;
}

/**
   Converts FEI4 row or col number to the corresponding T3MAPS row or col.
   Returns -1 if the returned T3MAPS row or column is outside defined range.
 */
int MapMaker::getT3MAPSfromFEI4(TString valName, int valFEI4) {
  if (mapExists()) {
    if (valName.Contains("row")) {
      int rowVal = (int)((((double)valFEI4) - mVar[1]) / mVar[0]);
      int rowSigma = (int)((((double)valFEI4) - (mVar[1]+mRMS[1])) / (mVar[0]-mRMS[0]));
      if (rowSigma < 0) rowSigma = -1 * rowSigma;
      if (myChips->isInChip("T3MAPS",rowVal,1)) {
	if (valName.Contains("Val")) return rowVal;
	else if (valName.Contains("Sigma")) return rowSigma;
      }
      else {
	return -1;
      }
    }
    else if (valName.Contains("col")) {
      int colVal = (int)((((double)valFEI4) - mVar[3]) / mVar[2]);
      int colSigma = (int)((((double)valFEI4) - (mVar[3]+mRMS[3])) / (mVar[2]-mRMS[2]));
      if (myChips->isInChip("T3MAPS",1,colVal)) {
	if (valName.Contains("Val")) return colVal;
	if (valName.Contains("Sigma")) return colSigma;
      }
      else { 
	return -1;
      }
    }
  }
  return -1;
}

/**
   Returns the error on the 4 parameters for the linear maps. Index = 0,1,2,3.
 */
double MapMaker::getMapRMS(int varIndex) {
  if (mapExists()) {
    return mRMS[varIndex];
  }
  else {
    std::cout << "    MapMaker::getMapVar No map exists!" << std::endl;
    return -1;
  }
}

/**
   Returns the parameters for the linear maps.
 */
double MapMaker::getMapVar(int varIndex) {
  if (mapExists()) {
    return mVar[varIndex];
  }
  else {
    std::cout << "    MapMaker::getMapVar No map exists!" << std::endl;
    return -1;
  }
}

/**
   Print the parameters from the most recent mapping.
 */
void MapMaker::printMapParameters() {
  
  std::cout << "  MapMaker::makeCombinedMap Precise map ok." << std::endl;
  std::cout << "    Printing map parameters:" << std::endl;
  for (int i = 0; i < 4; i++) {
    std::cout << "      parameter(" << i << ") = " << mVar[i] 
	      << " +/- " << mRMS[i] << std::endl;
  }
  std::cout << "  Interpretation:" << std::endl;
  std::cout << "    row_FEI4 = " << mVar[0] << " * row_T3MAPS + " 
	    << mVar[1]<< std::endl;
  std::cout << "    col_FEI4 = " << mVar[2] << " * col_T3MAPS + " 
	    << mVar[3] << std::endl;
}

TH2D* MapMaker::getHitPlot() {
  return hist2dFEI4Pix;
}
