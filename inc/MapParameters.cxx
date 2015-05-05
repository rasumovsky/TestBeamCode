////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: MapParameters.cxx                                                  //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 19/02/2015                                                          //
//                                                                            //
//  This class loads the FE-I4 and T3MAPS data structures and then creates a  //
//  one-to-one map between T3MAPS and FEI4 pixels.                            //
//                                                                            //
//  Linear map format:                                                        //
//        row_FEI4 = mVar[0] * row_T3MAPS + mVar[1];                          //
//        col_FEI4 = mVar[2] * col_T3MAPS + mVar[3];                          //
//                                                                            //
//  Typical run:                                                              //
//    1. MapParameters() initialize, either by loading or not.               //
//    if not from file:                                                       //
//      3. addHitToMap() to add hits for calculation                          //
//           ->this should be done inside some loop over TTrees in main class //
//      4. createMapFromHits() uses the added hits to create a new map.       //
//      5. You're ready to map!                                               //
//                                                                            //
//  The conceptual shift in this mapper is to use row and column pitch        //
//  information for each chip to fix the slope parameters and only allow two  //
//  free parameters for the maps. These will be chosen using a plot.          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "MapParameters.h"

/**
   Initialize the class either using the results of a previous run or using the
   intputs from a loop over TTrees.
   @param fileDir - the directory of the input/output files.
   @param option - the job options.
*/
MapParameters::MapParameters(TString fileDir, TString option) {
  std::cout << "MapParameters: Initializing..." << std::endl;
  
  // Settings for FEI4 and T3MAPS chip layouts:
  myChips = new ChipDimension();
  
  // Load map from file:
  if (option.Contains("FromFile")) {
    loadMapParameters(fileDir);
  }
  else {
    for (int i = 0; i < 4; i++) hasMap[i] = false;
  }
  
  // Set relative orientations of FEI4 and T3MAPS:
  sameOrientation = false;
  h2_para = new TH2D("h2_para","h2_para",100,-3200,16800,100,-4500,20000);
  h2_perp = new TH2D("h2_perp","h2_perp",100,-900,16800,100,-16000,20000);
  
  std::cout << "MapParameters: Successfully initialized!" << std::endl;
}

/**
   Add the hit to the histograms corresponding to the T3MAPS row and column.
   @param hitFEI4 - the hit in FEI4 to add to the map construction.
   @param hitT3MAPS - the hit in T3MAPS to add to the map construction.
*/
void MapParameters::addPairToMap(PixelHit *hitFEI4, PixelHit *hitT3MAPS) {
  // row 1 corresponds to entry 0 in the vector, did have -1 in T3MAPS
  int rowFEI4 = hitFEI4->getRow();
  int colFEI4 = hitFEI4->getCol();
  int rowT3MAPS = hitT3MAPS->getRow();
  int colT3MAPS = hitT3MAPS->getCol();
    
  if (rowT3MAPS < 0 || rowT3MAPS >= myChips->getNRow("T3MAPS")) {
    std::cout << "MapParameters: T3MAPS Row index error = " << rowT3MAPS
	      << std::endl;
    exit(0);
  }
  else if (colT3MAPS < 0 || colT3MAPS >= myChips->getNCol("T3MAPS")) {
    std::cout << "MapParameters: T3MAPS Column index error =" << colT3MAPS
	      << std::endl; 
    exit(0);
  }
  
  else if (rowFEI4 < 0 || rowFEI4 >= myChips->getNRow("FEI4")) {
  std::cout << "MapParameters: FEI4 Row index error = " << rowFEI4
	      << std::endl;
    exit(0);
  }
  else if (colFEI4 < 0 || colFEI4 >= myChips->getNCol("FEI4")) {
    std::cout << "MapParameters: FEI4 Column index error =" << colFEI4
	      << std::endl; 
    exit(0);
  }
  else { // only fill if it falls within defined vector element:
    double rowPosFEI4 = myChips->getRowPosition("FEI4", rowFEI4);
    double colPosFEI4 = myChips->getColPosition("FEI4", colFEI4);
    double rowPosT3MAPS = myChips->getRowPosition("T3MAPS", rowT3MAPS);
    double colPosT3MAPS = myChips->getColPosition("T3MAPS", colT3MAPS);
    
    double para_rowOffset = (rowPosFEI4 - ((myChips->getRowPitch("FEI4") / myChips->getRowPitch("T3MAPS")) * rowPosT3MAPS));
    double para_colOffset = (colPosFEI4 - ((myChips->getColPitch("FEI4") / myChips->getColPitch("T3MAPS")) * colPosT3MAPS));
    
    double perp_rowOffset = (rowPosFEI4 - ((myChips->getRowPitch("FEI4") / myChips->getColPitch("T3MAPS")) * colPosT3MAPS));
    double perp_colOffset = (colPosFEI4 - ((myChips->getColPitch("FEI4") / myChips->getRowPitch("T3MAPS")) * rowPosT3MAPS));
    
    h2_para->Fill(para_rowOffset, para_colOffset);
    h2_perp->Fill(perp_rowOffset, perp_colOffset);
  }
}

/**
   Extract linear map from linear fits to data.
*/
void MapParameters::createMapFromHits() {
  std::cout << "MapParameters: Create map from imported hits." << std::endl;
  
  // Initialize the plotting utility
  PlotUtil *plotter = new PlotUtil("../TestBeamOutput", 800, 800);
  plotter->plotTH2D(h2_para,"row offset","column offset","hits","paraOffset");
  plotter->plotTH2D(h2_perp,"row offset","column offset","hits","perpOffset");
  
  // Then set the parameters. 

}

/**
   Load parameters from previous mapping from file.
   @param inputDir - the directory containing the map parameters.
*/
void MapParameters::loadMapParameters(TString inputDir) {
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
   Create output text file with map parameters.
   @param outputDir - the directory in which the map parameters will be saved.
*/
void MapParameters::saveMapParameters(TString outputDir) {
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
   @param varIndex - the index of the variable.
   @param newVal - the new value of the map RMS for the specified variable.
*/
void MapParameters::setMapRMS(int varIndex, double newVal) {
  if (varIndex >= 0 && varIndex < 4) {
    mRMS[varIndex] = newVal;
  }
  else {
    std::cout << "MapParameters: setMapVar Improper varIndex!" << std::endl;
  }
}

/**
   Sets a parameter for the linear map. Index = 0,1,2,3.
   @param varIndex - the index of the variable.
   @param newVal - the new value of the map RMS for the specified variable.
*/
void MapParameters::setMapVar(int varIndex, double newVal) {
  if (varIndex >= 0 && varIndex < 4) {
    mVar[varIndex] = newVal;
    hasMap[varIndex] = true;
  }
  else {
    std::cout << "MapParameters: setMapVar Improper varIndex!" << std::endl;
  }
}

/**
   Checks that all four parameters necessary for a complete T3MAPS<-->FEI4
   mapping have been assigned.
*/
bool MapParameters::mapExists() {
  if (hasMap[0] && hasMap[1] && hasMap[2] && hasMap[3]) return true;  
  else return false;
}

/**
   Converts T3MAPS row or col number to the corresponding FEI4 row or col.
   Returns -1 if the returned FEI4 row or column is outside defined range.
   @param valName - the value name.
   @param valT3MAPS - the value in T3MAPS.
*/
int MapParameters::getFEI4fromT3MAPS(TString valName, int valT3MAPS) {
  if (mapExists()) {
    // Row value:
    if (valName.Contains("row")) {
      int rowVal = (int)(mVar[0] * ((double)valT3MAPS) + mVar[1]);
      int rowSigma = (int)((mVar[0]+mRMS[0]) * ((double)valT3MAPS) 
			   + (mVar[1]+mRMS[1])) - rowVal;
      if (rowSigma < 0) rowSigma = -1 * rowSigma;
      if (myChips->isInChip("FEI4", rowVal, 1)) {
	if (valName.Contains("Val")) return rowVal;
	else if (valName.Contains("Sigma")) return rowSigma;
      }
      else {
	std::cout << "MapParameters: Point is not in FEI4 chip!" << std::endl;
	return -1;
      }
    }
    // Column value:
    else if (valName.Contains("col")) {
      int colVal = (int)(mVar[2] * ((double)valT3MAPS) + mVar[3]);
      int colSigma = (int)((mVar[2]+mRMS[2]) * ((double)valT3MAPS)
			   + (mVar[3]+mRMS[3])) - colVal;
      if (colSigma < 0) colSigma = -1 * colSigma;
      if (myChips->isInChip("FEI4", 1, colVal)) {
	if (valName.Contains("Val")) return colVal;
	else if (valName.Contains("Sigma")) return colSigma;
      }
      else {
	std::cout << "MapParameters: Point is not in FEI4 chip!" << std::endl;
	return -1;
      }
    }
  }
  std::cout << "MapParameters: Bad valName!" << std::endl;
  return -1;
}

/**
   Converts FEI4 row or col number to the corresponding T3MAPS row or col.
   Returns -1 if the returned T3MAPS row or column is outside defined range.
   @param valName - the value name.
   @param valT3MAPS - the value in FEI4.
*/
int MapParameters::getT3MAPSfromFEI4(TString valName, int valFEI4) {
  if (mapExists()) {
    if (valName.Contains("row")) {
      int rowVal = (int)((((double)valFEI4) - mVar[1]) / mVar[0]);
      int rowSigma = (int)((((double)valFEI4) - (mVar[1]+mRMS[1]))
			   / (mVar[0]-mRMS[0]));
      if (rowSigma < 0) rowSigma = -1 * rowSigma;
      if (myChips->isInChip("T3MAPS", rowVal, 1)) {
	if (valName.Contains("Val")) return rowVal;
	else if (valName.Contains("Sigma")) return rowSigma;
      }
      else {
	std::cout << "MapParameters: Point is not in T3MAPS!" << std::endl;
	return -1;
      }
    }
    else if (valName.Contains("col")) {
      int colVal = (int)((((double)valFEI4) - mVar[3]) / mVar[2]);
      int colSigma = (int)((((double)valFEI4) - (mVar[3]+mRMS[3]))
			   / (mVar[2]-mRMS[2]));
      if (myChips->isInChip("T3MAPS", 1, colVal)) {
	if (valName.Contains("Val")) return colVal;
	if (valName.Contains("Sigma")) return colSigma;
      }
      else { 
	std::cout << "MapParameters: Point is not in T3MAPS!" << std::endl;
	return -1;
      }
    }
  }
  std::cout << "MapParameters: Bad valName!" << std::endl;
  return -1;
}

/**
   Returns the error on the 4 parameters for the linear maps. Index = 0,1,2,3
   @param varIndex - the index of the variable of interest.
*/
double MapParameters::getMapRMS(int varIndex) {
  if (mapExists()) {
    return mRMS[varIndex];
  }
  else {
    std::cout << "MapParameters::getMapVar No map exists!" << std::endl;
    return -1;
  }
}

/**
   Returns the parameters for the linear maps.
   @param varIndex - the index of the variable of interest.
*/
double MapParameters::getMapVar(int varIndex) {
  if (mapExists()) {
    return mVar[varIndex];
  }
  else {
    std::cout << "MapParameters::getMapVar No map exists!" << std::endl;
    return -1;
  }
}

/**
   Print the parameters from the most recent mapping.
*/
void MapParameters::printMapParameters() {
  std::cout << "MapParameters: Printing map parameters:" << std::endl;
  for (int i = 0; i < 4; i++) {
    std::cout << "\tparameter(" << i << ") = " << mVar[i] << " +/- " << mRMS[i] 
	      << std::endl;
  }
  std::cout << "MapParameters: Interpretation:" << std::endl;
  std::cout << "\trow_FEI4 = " << mVar[0] << " * row_T3MAPS + " << mVar[1] 
	    << std::endl;
  std::cout << "\tcol_FEI4 = " << mVar[2] << " * col_T3MAPS + " << mVar[3]
	    << std::endl;
}
