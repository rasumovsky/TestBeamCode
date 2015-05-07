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
    hasMap = false;
  }
  
  // Set relative orientations of FEI4 and T3MAPS:
  orientation = 0;
  h2Sig[0] = new TH2D("h2Sig0", "h2Sig0", 50, -3.2, 16.8, 50, -4.5, 20.0);
  h2Bkg[0] = new TH2D("h2Bkg0", "h2Bkg0", 50, -3.2, 16.8, 50, -4.5, 20.0);
  h2Sig[1] = new TH2D("h2Sig1", "h2Sig1", 50, -3.2, 16.8, 50, 0.0, 24.5);
  h2Bkg[1] = new TH2D("h2Bkg1", "h2Bkg1", 50, -3.2, 16.8, 50, 0.0, 24.5);
  h2Sig[2] = new TH2D("h2Sig2", "h2Sig2", 50, 0.0, 20.0, 50, -4.5, 20.0);
  h2Bkg[2] = new TH2D("h2Bkg2", "h2Bkg2", 50, 0.0, 20.0, 50, -4.5, 20.0);
  h2Sig[3] = new TH2D("h2Sig3", "h2Sig3", 50, 0.0, 20.0, 50, 0.0, 24.5);
  h2Bkg[3] = new TH2D("h2Bkg3", "h2Bkg3", 50, 0.0, 20.0, 50, 0.0, 24.5);
  
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
    
    double rowOffAlign = (rowPosFEI4 - ((myChips->getRowPitch("FEI4") / myChips->getRowPitch("T3MAPS")) * rowPosT3MAPS));
    double colOffAlign = (colPosFEI4 - ((myChips->getColPitch("FEI4") / myChips->getColPitch("T3MAPS")) * colPosT3MAPS));
    double rowOffAntiAlign = (rowPosFEI4 - (-1.0 * (myChips->getRowPitch("FEI4") / myChips->getRowPitch("T3MAPS")) * rowPosT3MAPS));
    double colOffAntiAlign = (colPosFEI4 - (-1.0 * (myChips->getColPitch("FEI4") / myChips->getColPitch("T3MAPS")) * colPosT3MAPS));
        
    h2Sig[0]->Fill(rowOffAlign, colOffAlign);
    h2Sig[1]->Fill(rowOffAlign, colOffAntiAlign);
    h2Sig[2]->Fill(rowOffAntiAlign, colOffAlign);
    h2Sig[3]->Fill(rowOffAntiAlign, colOffAntiAlign);
  }
}

/**
   Add the hit to the background histograms.
   @param hitFEI4 - the hit in FEI4 to add to the map construction.
   @param hitT3MAPS - the hit in T3MAPS to add to the map construction.
*/
void MapParameters::addPairToBkg(PixelHit *hitFEI4, PixelHit *hitT3MAPS) {
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
    
    double rowOffAlign = (rowPosFEI4 - ((myChips->getRowPitch("FEI4") / myChips->getRowPitch("T3MAPS")) * rowPosT3MAPS));
    double colOffAlign = (colPosFEI4 - ((myChips->getColPitch("FEI4") / myChips->getColPitch("T3MAPS")) * colPosT3MAPS));
    double rowOffAntiAlign = (rowPosFEI4 - (-1.0 * (myChips->getRowPitch("FEI4") / myChips->getRowPitch("T3MAPS")) * rowPosT3MAPS));
    double colOffAntiAlign = (colPosFEI4 - (-1.0 * (myChips->getColPitch("FEI4") / myChips->getColPitch("T3MAPS")) * colPosT3MAPS));
        
    h2Bkg[0]->Fill(rowOffAlign, colOffAlign);
    h2Bkg[1]->Fill(rowOffAlign, colOffAntiAlign);
    h2Bkg[2]->Fill(rowOffAntiAlign, colOffAlign);
    h2Bkg[3]->Fill(rowOffAntiAlign, colOffAntiAlign);
  }
}

/**
   Extract linear map from linear fits to data.
*/
void MapParameters::createMapFromHits() {
  std::cout << "MapParameters: Create map from imported hits." << std::endl;
  
  // Initialize the plotting utility
  PlotUtil::setAtlasStyle();
  
  TH1F *projRowOff[4];
  TH1F *projColOff[4];

  // Loop over 4 orientations:
  for (int i_h = 0; i_h < 4; i_h++) {
    PlotUtil::plotTH2D(h2Sig[i_h], "row offset [mm]", "column offset [mm]",
		       "hits", Form("../TestBeamOutput/sig_paraOff%d",i_h));
    
    PlotUtil::plotTH2D(h2Bkg[i_h], "row offset [mm]", "column offset [mm]",
		       "hits", Form("../TestBeamOutput/bkg_paraOff%d",i_h));
    
    h2Diff[i_h] = new TH2D(Form("h2Diff%d",i_h), Form("h2Diff%d",i_h),
			   50, -3.2, 16.8, 50, -4.5, 20.0);
    
    for (int i_x = 1; i_x <= h2Diff[i_h]->GetNbinsX(); i_x++) {
      for (int i_y = 1; i_y <= h2Diff[i_h]->GetNbinsY(); i_y++) {
	double para_weight = ((h2Sig[i_h]->GetBinContent(i_x,i_y) /
			       h2Sig[i_h]->Integral()) - 
			      (h2Bkg[i_h]->GetBinContent(i_x,i_y) / 
			       h2Bkg[i_h]->Integral()));
	h2Diff[i_h]->SetBinContent(i_x, i_y, para_weight);
      }
    }
    
    PlotUtil::plotTH2D(h2Diff[i_h], "row offset [mm]", "column offset [mm]",
		       "hits", Form("../TestBeamOutput/diff_paraOff%d",i_h));
    
    /*
      double valRowOff_para = histRowOff_para->GetMean();
      double valColOff_para = histColOff_para->GetMean();
      double errRowOff_para = histRowOff_para->GetMeanError();
      double errColOff_para = histColOff_para->GetMeanError();
    */
    
    // Multiplicative factors
    double rowFactor = (i_h == 0 || i_h == 1) ? 1.0 : -1.0;
    double colFactor = (i_h == 0 || i_h == 2) ? 1.0 : -1.0;
    
    // For the parallel orientation:
    projRowOff[i_h] = (TH1F*)h2Sig[i_h]->ProjectionX();
    projColOff[i_h] = (TH1F*)h2Sig[i_h]->ProjectionY();
    mVar[i_h][0] = rowFactor * (myChips->getRowPitch("FEI4") / 
				myChips->getRowPitch("T3MAPS"));
    mErr[i_h][0] = 0.05 * (mVar[i_h][0]);// 5% of calculated slope?
    mVar[i_h][1] = projRowOff[i_h]->GetMean();
    mErr[i_h][1] = projRowOff[i_h]->GetRMS();
    mVar[i_h][2] = colFactor * (myChips->getColPitch("FEI4") / 
				myChips->getColPitch("T3MAPS"));
    mErr[i_h][2] = 0.05 * (mVar[i_h][2]);// 5% of calculated slope?
    mVar[i_h][3] = projColOff[i_h]->GetMean();
    mErr[i_h][3] = projColOff[i_h]->GetRMS();
    
    // Plot the profiles. 
    PlotUtil::plotTH1F(projRowOff[i_h], "row offset [mm]", "hits",
		       Form("../TestBeamOutput/projRowOff%d",i_h));
    PlotUtil::plotTH1F(projColOff[i_h], "col offset [mm]", "hits",
		       Form("../TestBeamOutput/projColOff%d",i_h));
  }
  hasMap = true;
}

/**
   Load parameters from previous mapping from file.
   @param inputDir - the directory containing the map parameters.
*/
void MapParameters::loadMapParameters(TString inputDir) {
  ifstream inputFile;
  inputFile.open(Form("%s/mapParameters.txt",inputDir.Data()));
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
  outputFile.open(Form("%s/mapParameters.txt",outputDir.Data()));
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
  if (mapExists()) {
    // Row value:
    if (valName.Contains("row")) {
      int rowVal = (int)(mVar[orientation][0] * ((double)valT3MAPS) + mVar[orientation][1]);
      int rowSigma = (int)((mVar[orientation][0]+mErr[orientation][0]) * ((double)valT3MAPS) + (mVar[orientation][1]+mErr[orientation][1])) - rowVal;
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
      int colVal = (int)(mVar[orientation][2] * ((double)valT3MAPS) + mVar[orientation][3]);
      int colSigma = (int)((mVar[orientation][2]+mErr[orientation][2]) * ((double)valT3MAPS) + (mVar[orientation][3]+mErr[orientation][3])) - colVal;
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
      int rowVal = (int)((((double)valFEI4) - mVar[orientation][1]) / mVar[orientation][0]);
      int rowSigma = (int)((((double)valFEI4) - (mVar[orientation][1]+mErr[orientation][1])) / (mVar[orientation][0]-mErr[orientation][0]));
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
      int colVal = (int)((((double)valFEI4) - mVar[orientation][3]) / mVar[orientation][2]);
      int colSigma = (int)((((double)valFEI4) - (mVar[orientation][3]+mErr[orientation][3])) / (mVar[orientation][2]-mErr[orientation][2]));
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
