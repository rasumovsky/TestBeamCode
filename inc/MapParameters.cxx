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
//    1. MapParameters() initialize, either by loading or not.                //
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
  chips = new ChipDimension();
  
  // Load map from file:
  if (option.Contains("FromFile")) {
    loadMapParameters(fileDir);
  }
  else {
    hasMap = false;
  }
  
  // Set counters to zero:
  nBkgHits = 0;
  nSigHits = 0;
  
  // Define the relative orientation possibilities for the chips:
  rowSign[0] =  1.0;  colSign[0] =  1.0;
  rowSign[1] =  1.0;  colSign[1] = -1.0;
  rowSign[2] = -1.0;  colSign[2] =  1.0;
  rowSign[3] = -1.0;  colSign[3] = -1.0;
    
  // Set the constant slope parameters of the map:
  for (int i_h = 3; i_h >= 0; i_h--) {
    setOrientation(i_h);
    setMapVar(0, getRowSlope());
    setMapErr(0, getRowSlope() * 0.05);// 5% of calculated slope
    setMapVar(2, getColSlope());
    setMapErr(2, getColSlope() * 0.05);// 5% of calculated slope
  }
  
  // Calculate the min and max possibilities:
  double rMin1 = (-1.0 * getRowSlope() * 
		  chips->getRowPosition("T3MAPS",chips->getNRow("T3MAPS")));
  double rMax1 = chips->getRowPosition("FEI4",chips->getNRow("FEI4"));
  double rMax2 = rMax1 - rMin1;
  double cMin1 = (-1.0 * getColSlope() *
		  chips->getColPosition("T3MAPS",chips->getNCol("T3MAPS")));
  double cMax1 = chips->getColPosition("FEI4",chips->getNCol("FEI4"));
  double cMax2 = cMax1 - cMin1;
  h2Sig[0] = new TH2D("h2Sig0","h2Sig0",nRBin,rMin1,rMax1,nCBin,cMin1,cMax1);
  h2Bkg[0] = new TH2D("h2Bkg0","h2Bkg0",nRBin,rMin1,rMax1,nCBin,cMin1,cMax1);
  h2Diff[0] = new TH2D("h2Diff0","h2Diff0",nRBin,rMin1,rMax1,nCBin,cMin1,cMax1);
  h2Sig[1] = new TH2D("h2Sig1","h2Sig1",nRBin,rMin1,rMax1,nCBin,0.0,cMax2);
  h2Bkg[1] = new TH2D("h2Bkg1","h2Bkg1",nRBin,rMin1,rMax1,nCBin,0.0,cMax2);
  h2Diff[1] = new TH2D("h2Diff1","h2Diff1",nRBin,rMin1,rMax1,nCBin,0.0,cMax2);
  h2Sig[2] = new TH2D("h2Sig2","h2Sig2",nRBin,0.0,rMax2,nCBin,cMin1,cMax1);
  h2Bkg[2] = new TH2D("h2Bkg2","h2Bkg2",nRBin,0.0,rMax2,nCBin,cMin1,cMax1);
  h2Diff[2] = new TH2D("h2Diff2","h2Diff2",nRBin,0.0,rMax2,nCBin,cMin1,cMax1);
  h2Sig[3] = new TH2D("h2Sig3","h2Sig3",nRBin,0.0,rMax2,nCBin,0.0,cMax2);
  h2Bkg[3] = new TH2D("h2Bkg3","h2Bkg3",nRBin,0.0,rMax2,nCBin,0.0,cMax2);
  h2Diff[3] = new TH2D("h2Diff3","h2Diff3",nRBin,0.0,rMax2,nCBin,0.0,cMax2);
  
  std::cout << "MapParameters: Successfully initialized!" << std::endl;
}

/**
   Add the hit to the histograms corresponding to the T3MAPS row and column.
   @param hitFEI4 - the hit in FEI4 to add to the map construction.
   @param hitT3MAPS - the hit in T3MAPS to add to the map construction.
*/
void MapParameters::addPairToMap(PixelHit *hitFEI4, PixelHit *hitT3MAPS) {
  
  // Check that hits are in the chips:
  if (!chips->isInChip("T3MAPS", hitT3MAPS->getRow(), hitT3MAPS->getCol()) ||
      !chips->isInChip("FEI4", hitFEI4->getRow(), hitFEI4->getCol())) {
    std::cout << "MapParameters: Index Error!" << std::endl;
    hitFEI4->printHit();
    hitT3MAPS->printHit();
    exit(0);
  }
  
  // Only fill if it falls within defined chip area:
  else {
    for (int i_h = 0; i_h < 4; i_h++) {
      h2Sig[i_h]->Fill(getRowOffset(hitFEI4->getRow(),hitT3MAPS->getRow(),i_h),
		       getColOffset(hitFEI4->getCol(),hitT3MAPS->getCol(),i_h));
    }
  }
  nSigHits++;
}

/**
   Add the hit to the background histograms.
   @param hitFEI4 - the hit in FEI4 to add to the map construction.
   @param hitT3MAPS - the hit in T3MAPS to add to the map construction.
*/
void MapParameters::addPairToBkg(PixelHit *hitFEI4, PixelHit *hitT3MAPS) {
 
  // Check that hits are in the chips:
  if (!chips->isInChip("T3MAPS", hitT3MAPS->getRow(), hitT3MAPS->getCol()) ||
      !chips->isInChip("FEI4", hitFEI4->getRow(), hitFEI4->getCol())) {
    std::cout << "MapParameters: Index Error!" << std::endl;
    hitFEI4->printHit();
    hitT3MAPS->printHit();
    exit(0);
  }
  
  // Only fill if it falls within defined chip area:
  else {
    for (int i_h = 0; i_h < 4; i_h++) {
      h2Bkg[i_h]->Fill(getRowOffset(hitFEI4->getRow(),hitT3MAPS->getRow(),i_h),
		       getColOffset(hitFEI4->getCol(),hitT3MAPS->getCol(),i_h));
    }
  }
  nBkgHits++;
}

/**
   Extract linear map from linear fits to data.
*/
void MapParameters::createMapFromHits() {
  std::cout << "MapParameters: Create map from " << nBkgHits << " bkg hits and "
	    << nSigHits << " sig hits." << std::endl;
  
  // Initialize the plotting utility
  PlotUtil::setAtlasStyle();
  
  TH1F *projRowOff[4];
  TH1F *projColOff[4];
  // Loop over 4 orientations:
  for (int i_h = 0; i_h < 4; i_h++) {
    PlotUtil::plotTH2D(h2Sig[i_h], "row offset [mm]", "column offset [mm]", "hits", Form("../TestBeamOutput/MapParameters/sig_paraOff%d",i_h));
    PlotUtil::plotTH2D(h2Bkg[i_h], "row offset [mm]", "column offset [mm]", "hits", Form("../TestBeamOutput/MapParameters/bkg_paraOff%d",i_h));
    for (int i_x = 1; i_x <= h2Diff[i_h]->GetNbinsX(); i_x++) {
      for (int i_y = 1; i_y <= h2Diff[i_h]->GetNbinsY(); i_y++) {
	double para_weight = ((h2Sig[i_h]->GetBinContent(i_x,i_y) /
			       h2Sig[i_h]->Integral()) - 
			      (h2Bkg[i_h]->GetBinContent(i_x,i_y) / 
			       h2Bkg[i_h]->Integral()));
	h2Diff[i_h]->SetBinContent(i_x, i_y, para_weight);
      }
    }
    
    PlotUtil::plotTH2D(h2Diff[i_h], "row offset [mm]", "column offset [mm]", "Sig-Bkg", Form("../TestBeamOutput/MapParameters/diff_paraOff%d",i_h));
        
    // Slope parameters derived from chip geometry:
    projRowOff[i_h] = (TH1F*)h2Sig[i_h]->ProjectionX();
    projColOff[i_h] = (TH1F*)h2Sig[i_h]->ProjectionY();
      
    // Offset parameters from the maps:
    mVar[i_h][1] = projRowOff[i_h]->GetMean();
    mErr[i_h][1] = projRowOff[i_h]->GetRMS();
    mVar[i_h][3] = projColOff[i_h]->GetMean();
    mErr[i_h][3] = projColOff[i_h]->GetRMS();
    
    // Plot the profiles. 
    PlotUtil::plotTH1F(projRowOff[i_h], "row offset [mm]", "hits", Form("../TestBeamOutput/MapParameters/projRowOff%d",i_h));
    PlotUtil::plotTH1F(projColOff[i_h], "col offset [mm]", "hits", Form("../TestBeamOutput/MapParameters/projColOff%d",i_h));
  }
  hasMap = true;
}

/**
   Load parameters from previous mapping from file.
   @param inputDir - the directory containing the map parameters.
*/
void MapParameters::loadMapParameters(TString inputDir) {
  ifstream inputFile;
  inputFile.open(Form("%s/MapParameters/mapParameters.txt",inputDir.Data()));
  if (!inputFile) {
    std::cout << "MapParameters: Map file nonexistent!" << std::endl;
    exit(0);
  }
  
  std::string pName;
  double tempVar[4]; double tempErr[4];
  int lineIndex = 0;
  while (!inputFile.eof()) {
    inputFile >> pName >> tempVar[0] >> tempErr[0] >> tempVar[1] >> tempErr[1]
	      >> tempVar[2] >> tempErr[2] >> tempVar[3] >> tempErr[3];
    for (int i_h = 0; i_h < 4; i_h++) {
      mVar[i_h][lineIndex] = tempVar[i_h];
      mErr[i_h][lineIndex] = tempErr[i_h];
    }
    lineIndex++;
  }
  hasMap = true;
  inputFile.close();
  printMapParameters();
}

/**
   Create output text file with map parameters.
   @param outputDir - the directory in which the map parameters will be saved.
*/
void MapParameters::saveMapParameters(TString outputDir) {
  ofstream outputFile;
  outputFile.open(Form("%s/MapParameters/mapParameters.txt",outputDir.Data()));
  for (int i_p = 0; i_p < 4; i_p++) {
    outputFile << "parameter" << i_p;
    for (int i_h = 0; i_h < 4; i_h++) {
      outputFile << " " << mVar[i_h][i_p] << " " << mErr[i_h][i_p];
    }
    outputFile << std::endl;
  }
  outputFile.close();
}

/**
   Set the orientation either parallel or perpendicular.
   @param orientation - the orientation of the two modules.
*/
void MapParameters::setOrientation(int newOrientation) {
  orientation = newOrientation;
}

/**
   Set the error on the 4 parameters for the linear maps. Index = 0,1,2,3
   @param varIndex - the index of the variable of interest.
   @param value - the new value of the variable.
*/
void MapParameters::setMapErr(int varIndex, double value) {
  mErr[orientation][varIndex] = value;
}

/**
   Set the parameters for the linear maps.
   @param varIndex - the index of the variable of interest.
   @param value - the new value of the variable.
*/
void MapParameters::setMapVar(int varIndex, double value) {
  mVar[orientation][varIndex] = value;
}

/**
   Set the hasMap boolean.
   @param doesExist - true iff map exists.
*/
void MapParameters::setMapExists(bool doesExist) {
  hasMap = doesExist;
}

/**
   Checks that all four parameters necessary for a complete T3MAPS<-->FEI4
   mapping have been assigned.
*/
bool MapParameters::mapExists() {
  return hasMap;
}

/**
   Converts T3MAPS row or col number to the corresponding FEI4 row or col.
   Returns -1 if the returned FEI4 row or column is outside defined range.
   @param valName - the value name.
   @param valT3MAPS - the value in T3MAPS.
*/
int MapParameters::getFEI4fromT3MAPS(TString valName, int valT3MAPS) {
  if (!mapExists()) {
    std::cout << "MapParameters: No map exists!" << std::endl;
    exit(0);
  }
  
  // Spell out map parameter definitions for easier reading:
  std::string param; double p0, p1, e0, e1, sign;
  if (valName.Contains("row")) { 
    p0 = mVar[orientation][0];  p1 = mVar[orientation][1];
    e0 = mErr[orientation][0];  e1 = mErr[orientation][1];
    sign = rowSign[orientation];
    param = "row"; }
  else if (valName.Contains("col")) { 
    p0 = mVar[orientation][2];  p1 = mVar[orientation][3];
    e0 = mErr[orientation][2];  e1 = mErr[orientation][3];
    sign = colSign[orientation];
    param = "col"; }
  else {
    std::cout << "MapParameters: Bad val name: "<< std::endl;
    exit(0);
  }
  
  double positionT = chips->getPosition("T3MAPS", param, valT3MAPS);
  
  // Here is the linear mapping function:
  double positionF = (p0 * sign * positionT) + p1;
  double posPlusSigma = (p0 * sign * positionT) + (p1 + e1);
  
  // Convert position back to index:
  int index = chips->getIndexFromPos("FEI4", param, positionF);
  int indexPlusSigma = chips->getIndexFromPos("FEI4", param, posPlusSigma);
  int indexSigma = std::abs(indexPlusSigma - index);
  if (valName.Contains("Val")) return index;
  else if (valName.Contains("Sigma")) return indexSigma;
  else exit(0);
}

/**
   Converts FEI4 row or col number to the corresponding T3MAPS row or col.
   Returns -1 if the returned T3MAPS row or column is outside defined range.
   @param valName - the value name.
   @param valT3MAPS - the value in FEI4.
*/
int MapParameters::getT3MAPSfromFEI4(TString valName, int valFEI4) {
  if (!mapExists()) {
    std::cout << "MapParameters: No map exists!" << std::endl;
    exit(0);
  }
  
  // Spell out map parameter definitions for easier reading:
  std::string param; double p0, p1, e0, e1, sign;
  if (valName.Contains("row")) { 
    p0 = mVar[orientation][0];  p1 = mVar[orientation][1];
    e0 = mErr[orientation][0];  e1 = mErr[orientation][1];
    sign = rowSign[orientation];
    param = "row";
  }
  else if (valName.Contains("col")) { 
    p0 = mVar[orientation][2];  p1 = mVar[orientation][3];
    e0 = mErr[orientation][2];  e1 = mErr[orientation][3];
    sign = colSign[orientation];
    param = "col"; 
  }
  else {
    std::cout << "MapParameters: Bad val name: " << std::endl;
    exit(0);
  }
  
  double positionF = chips->getPosition("FEI4", param, valFEI4);
  
  // Here is the linear mapping function:
  // multiply by col signs?
  double positionT = sign * (positionF - p1) / p0;
  double posPlusSigma = sign * (positionF - (p1 + e1)) / p0;
  
  // Convert position back to index:
  int index = chips->getIndexFromPos("T3MAPS", param, positionT);
  int indexPlusSigma = chips->getIndexFromPos("T3MAPS", param, posPlusSigma);
  int indexSigma = std::abs(indexPlusSigma - index);
  if (valName.Contains("Val")) return index;
  else if (valName.Contains("Sigma")) return indexSigma;
  else exit(0);
}

/**
   Get the column offset parameter (in mm) using column indices and orientation.
   @param colFEI4 - the FEI4 column index.
   @param colT3MAPS - the T3MAPS column index.
   @param orientation - the chip orientation index.
   @returns - the column offset parameter value.
*/
double MapParameters::getColOffset(int colFEI4, int colT3MAPS, 
				   int orientation) {
  
  double colPosFEI4 = chips->getColPosition("FEI4", colFEI4);
  double colPosT3MAPS = (colSign[orientation] *
			 chips->getColPosition("T3MAPS", colT3MAPS));
  double colOffset = (colPosFEI4 - (getColSlope() * colPosT3MAPS));
  return colOffset;
}

/**
   Get the row offset parameter (in mm) using row indices and orientation.
   @param rowFEI4 - the FEI4 row index.
   @param rowT3MAPS - the T3MAPS row index.
   @param orientation - the chip orientation index.
   @returns - the row offset parameter value.
*/
double MapParameters::getRowOffset(int rowFEI4, int rowT3MAPS,
				   int orientation) {
  double rowPosFEI4 = chips->getRowPosition("FEI4", rowFEI4);
  double rowPosT3MAPS = (rowSign[orientation] *
			 chips->getRowPosition("T3MAPS", rowT3MAPS));
  double rowOffset = (rowPosFEI4 - (getRowSlope() * rowPosT3MAPS));
  return rowOffset;
}

/**
   Get the slope of the column mapping.
   @param orientation - the chip orientation index.
   @returns - the slope of the column mapping.
*/
double MapParameters::getColSlope() {
  double colSlope = (chips->getColPitch("FEI4") / 
		     chips->getColPitch("T3MAPS"));
  return colSlope;
}

/**
   Get the slope of the row mapping.
   @param orientation - the chip orientation index.
   @returns - the slope of the row mapping.
*/
double MapParameters::getRowSlope() {
  double rowSlope = (chips->getRowPitch("FEI4") / 
		     chips->getRowPitch("T3MAPS"));
  return rowSlope;
}

/**
   Returns the error on the 4 parameters for the linear maps. Index = 0,1,2,3
   @param varIndex - the index of the variable of interest.
*/
double MapParameters::getMapErr(int varIndex) {
  if (mapExists()) {
    return mErr[orientation][varIndex];
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
    return mVar[orientation][varIndex];
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
    std::cout << "\tparameter(" << i << ") = " << mVar[orientation][i]
	      << " +/- " << mErr[orientation][i] << std::endl;
  }
  std::cout << "MapParameters: Interpretation:" << std::endl;
  std::cout << "\trow_FEI4 = " << mVar[orientation][0] << " * row_T3MAPS + "
	    << mVar[orientation][1] << std::endl;
  std::cout << "\tcol_FEI4 = " << mVar[orientation][2] << " * col_T3MAPS + "
	    << mVar[orientation][3] << std::endl;
}

TH2D *MapParameters::getParamPlot(TString name) {
  if (mapExists()) {
    if (name.EqualTo("sig")) return h2Sig[orientation];
    else if (name.EqualTo("bkg")) return h2Bkg[orientation];
    else if (name.EqualTo("diff")) return h2Diff[orientation];
  }
  else {
    std::cout << "MapParameters::getParamPlot No map exists!" << std::endl;
    return NULL;
  }
}
