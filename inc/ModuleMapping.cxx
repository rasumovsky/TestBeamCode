////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: ModuleMapping.cxx                                                   //
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
//    1. ModuleMapping() initialize, either by loading or not.                //
//    if not from file:                                                       //
//      2. designatePixelPair() in T3MAPS to use for calculating map.         //
//      3. addHitToMap() to add hits for calculation                          //
//           ->this should be done inside some loop over TTrees in main class //
//      4. createMapFromHits() uses the added hits to create a new map.       //
//      5. You're ready to map!
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "ModuleMapping.h"

/**
   Initialize the class either using the results of a previous run or using the
   intputs from a loop over TTrees.
*/
ModuleMapping::ModuleMapping(TString fileDir, TString option) {
  
  std::cout << "Initializing ModuleMapping..." << std::endl;
  // Settings for FEI4 and T3MAPS chip layouts:
  myChips = new ChipDimension();
  
  if (option.Contains("FromFile")) {
    loadMapParameters(fileDir);// Load map from file
  }
  else {
    for (int i = 0; i < 4; i++) hasMap[i] = false;
  }
  
  // Initialize the 2D histogram for correlation calculations.
  for (int i = 0; i < 2; i++) {
    hist2dFEI4Pix[i] = new TH2D(Form("FEI4Pix%i2d",i),Form("FEI4Pix%i2d",i),
				myChips->getNRow("FEI4"), -0.5,
				((double)myChips->getNRow("FEI4")) - 0.5,
				myChips->getNCol("FEI4"), -0.5,
				((double)myChips->getNCol("FEI4")) - 0.5);
  }
  
  //saveMapParameters(outputDir);//Save parameters to file
  std::cout << "Successfully initialized ModuleMapping!" << std::endl;
}

/**
Load parameters from previous mapping:
*/
void ModuleMapping::loadMapParameters(TString inputDir) {
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
void ModuleMapping::saveMapParameters(TString outputDir) {
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
void ModuleMapping::setMapRMS(int varIndex, double newVal) {
  if (varIndex >= 0 && varIndex < 4) {
    mRMS[varIndex] = newVal;
  }
  else {
    std::cout << "    ModuleMapping::setMapVar Improper varIndex!" << std::endl;
  }
}

/**
   Sets a parameter for the linear map. Index = 0,1,2,3.
   newVal is the new value of the specified mapping parameter.
 */
void ModuleMapping::setMapVar(int varIndex, double newVal) {
  if (varIndex >= 0 && varIndex < 4) {
    mVar[varIndex] = newVal;
    hasMap[varIndex] = true;
  }
  else {
    std::cout << "    ModuleMapping::setMapVar Improper varIndex!" << std::endl;
  }
}

/**
   Set the two pixel locations to be used for the mapping.
 */
void ModuleMapping::designatePixelPair(int row1, int col1, int row2, int col2) {
  T3MAPS_r1 = row1;
  T3MAPS_c1 = col1;
  T3MAPS_r2 = row2;
  T3MAPS_c2 = col2;
}

/**
   Add a hit to be used for either pixel 0 or pixel 1 in defining the map.
 */
void ModuleMapping::addHitToMap(int pixIndex, PixelHit *hit) {
  if (pixIndex >= 0 && pixIndex < 2) {
    hist2dFEI4Pix[pixIndex]->Fill(hit->getRow(),hit->getCol());
  }
}

/**
   Uses the hits added from the addHitToMap method to calculate new parameters
   for the mapping. This is where the algebra happens.
 */
void ModuleMapping::createMapFromHits() {
  // Use max bin to get row and column numbers in FEI4.
  /*
  int FEI4_r1 = hist2dFEI4Pix[0]->ProjectionX()->GetMaximumBin();
  int FEI4_c1 = hist2dFEI4Pix[0]->ProjectionY()->GetMaximumBin();
  int FEI4_r2 = hist2dFEI4Pix[1]->ProjectionX()->GetMaximumBin();
  int FEI4_c2 = hist2dFEI4Pix[1]->ProjectionY()->GetMaximumBin();
  */
  
  int FEI4_r1 = hist2dFEI4Pix[0]->ProjectionX()->GetMean();
  int FEI4_c1 = hist2dFEI4Pix[0]->ProjectionY()->GetMean();
  int FEI4_r2 = hist2dFEI4Pix[1]->ProjectionX()->GetMean();
  int FEI4_c2 = hist2dFEI4Pix[1]->ProjectionY()->GetMean();
  
  // Calculate the linear mapping constants:
  mVar[0] = ((double)(FEI4_r2-FEI4_r1)) / ((double)(T3MAPS_r2-T3MAPS_r1));
  mVar[2] = ((double)(FEI4_c2-FEI4_c1)) / ((double)(T3MAPS_c2-T3MAPS_c1));
  mVar[1] = ((double)FEI4_r1) - (mVar[0] * (double)T3MAPS_r1);
  mVar[3] = ((double)FEI4_c1) - (mVar[2] * (double)T3MAPS_c1);
  
  for (int i = 0; i < 4; i++) hasMap[i] = true;
}

/**
   Check if a hit is in one of the mapping pixels:
 */
bool ModuleMapping::isPixelHit(int pixIndex, PixelHit *hit) {
  if (pixIndex == 0) {
    if (hit->getRow() == T3MAPS_r1 && hit->getCol() == T3MAPS_c1) return true;
    else return false;
  }
  else if (pixIndex == 1) {
    if (hit->getRow() == T3MAPS_r2 && hit->getCol() == T3MAPS_c2) return true;
    else return false;
  }
  else return false;
}

/**
   Checks that all four parameters necessary for a complete T3MAPS<-->FEI4
   mapping have been assigned.
*/
bool ModuleMapping::mapExists() {
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
int ModuleMapping::getFEI4fromT3MAPS(TString valName, int valT3MAPS) {
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
int ModuleMapping::getT3MAPSfromFEI4(TString valName, int valFEI4) {
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
 */
int ModuleMapping::getPixPos(TString valName, int index) {
  if (valName == "row" && index == 0) { return T3MAPS_r1; }
  else if (valName == "row" && index == 1) { return T3MAPS_r2; }
  else if (valName == "col" && index == 0) { return T3MAPS_c1; }
  else if (valName == "col" && index == 1) { return T3MAPS_c2; }
  else {
    std::cout << "ModuleMapping::getMapPixPos ERROR! Bad args!" << std::endl;
    return -1;
  }
}

/**
   Returns the error on the 4 parameters for the linear maps. Index = 0,1,2,3.
 */
double ModuleMapping::getMapRMS(int varIndex) {
  if (mapExists()) {
    return mRMS[varIndex];
  }
  else {
    std::cout << "    ModuleMapping::getMapVar No map exists!" << std::endl;
    return -1;
  }
}

/**
   Returns the parameters for the linear maps.
 */
double ModuleMapping::getMapVar(int varIndex) {
  if (mapExists()) {
    return mVar[varIndex];
  }
  else {
    std::cout << "    ModuleMapping::getMapVar No map exists!" << std::endl;
    return -1;
  }
}

/**
   Print the parameters from the most recent mapping.
 */
void ModuleMapping::printMapParameters() {
  
  std::cout << "  ModuleMapping::makeCombinedMap Precise map ok." << std::endl;
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

TH2D* ModuleMapping::getHitPlot(int index) {
  return hist2dFEI4Pix[index];
}
