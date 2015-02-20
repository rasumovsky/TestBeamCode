////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: LinearMapMaker.cxx                                                  //
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
//    1. LinearMapMaker() initialize, either by loading or not.               //
//    if not from file:                                                       //
//      3. addHitToMap() to add hits for calculation                          //
//           ->this should be done inside some loop over TTrees in main class //
//      4. createMapFromHits() uses the added hits to create a new map.       //
//      5. You're ready to map!                                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "LinearMapMaker.h"

/**
   Initialize the class either using the results of a previous run or using the
   intputs from a loop over TTrees.
*/
LinearMapMaker::LinearMapMaker(TString fileDir, TString option) {
  
  std::cout << "Initializing LinearMapMaker..." << std::endl;
  // Settings for FEI4 and T3MAPS chip layouts:
  myChips = new ChipDimension();
  
  if (option.Contains("FromFile")) {
    loadMapParameters(fileDir);// Load map from file
  }
  else {
    for (int i = 0; i < 4; i++) hasMap[i] = false;
  }
  
  // set relative orientations of FEI4 and T3MAPS:
  sameOrientation = false;
  
  rowHist2dFEI4.clear();
  colHist2dFEI4.clear();
  TH2D *rowHist;
  TH2D *colHist;
  
  // Initialize the 2D histograms for row correlations:
  for (int i_r = 1; i_r <= myChips->getChipSize("T3MAPS","nRows"); i_r++) {
    
    rowHist = new TH2D(Form("rowHistFEI4_c%i",i_r), Form("rowHistFEI4_c%i",i_r),
		       myChips->getChipSize("FEI4","nRows"),
		       0.5,
		       0.5+((double)myChips->getChipSize("FEI4","nRows")),
		       myChips->getChipSize("FEI4","nColumns"),
		       0.5,
		       0.5+((double)myChips->getChipSize("FEI4","nColumns")));
    rowHist2dFEI4.push_back(rowHist);
  }
  
  // Initialize the 2D histograms for column correlations:
  for (int i_c = 1; i_c <= myChips->getChipSize("T3MAPS","nColumns"); i_c++) {
    
    colHist = new TH2D(Form("colHistFEI4_c%i",i_c), Form("colHistFEI4_c%i",i_c),
		       myChips->getChipSize("FEI4","nRows"),
		       0.5,
		       0.5+((double)myChips->getChipSize("FEI4","nRows")),
		       myChips->getChipSize("FEI4","nColumns"),
		       0.5,
		       0.5+((double)myChips->getChipSize("FEI4","nColumns")));
    colHist2dFEI4.push_back(colHist);
  }
  
  std::cout << "Successfully initialized LinearMapMaker!" << std::endl;
}

/**
   Add the hit to the histograms corresponding to the T3MAPS row and column.
 */
void LinearMapMaker::addPairToMap(PixelHit *hitFEI4, PixelHit *hitT3MAPS) {
  
  // row 1 corresponds to entry 0 in the vector.
  int rowIdx = hitT3MAPS->getRow() - 1;
  int colIdx = hitT3MAPS->getCol() - 1;
  
  if (rowIdx < 0 || rowIdx >= myChips->getChipSize("T3MAPS","nRows")) {
    std::cout << "Row index error! rowIdx=" << rowIdx << std::endl; 
  }
  else if (colIdx < 0 || colIdx >= myChips->getChipSize("T3MAPS","nColumns")) {
    std::cout << "Column index error! colIdx=" << colIdx << std::endl; 
  }
  else { // only fill if it falls within defined vector element:
    (rowHist2dFEI4[rowIdx])->Fill(hitFEI4->getRow(), hitFEI4->getCol());
    (colHist2dFEI4[colIdx])->Fill(hitFEI4->getRow(), hitFEI4->getCol());
  }
}

TF1* LinearMapMaker::createFit(TString name, TGraphErrors *g, double xlow, 
			       double xhigh) {
  
  TF1 *fit = new TF1(name,"[0]*x+[1]", xlow, xhigh);
  fit->SetParameter(0,1);
  fit->SetParameter(1,1);
  g->Fit(fit);
  return fit;
}

/**
   Extract linear map from linear fits to data.
 */
void LinearMapMaker::createMapFromHits() {
  
  std::cout << "LinearMapPaker::createMapFromHits()" << std::endl;
  
  // Initialize the plotting utility
  PlotUtil *plotter = new PlotUtil("../TestBeamOutput",800,800);
  
  // loop over row histograms and add points to corresponding graph:
  double r_T3MAPS[100] = {0};
  double e_r_T3MAPS[100] = {0};
  double r2r_FEI4[100] = {0};
  double r2c_FEI4[100] = {0};
  double e_r2r_FEI4[100] = {0};
  double e_r2c_FEI4[100] = {0};
  
  for (int i_r = 0; i_r < myChips->getChipSize("T3MAPS","nRows"); i_r++) {
    r_T3MAPS[i_r] = i_r+1;
    e_r_T3MAPS[i_r] = 0.5;
    r2r_FEI4[i_r] = rowHist2dFEI4[i_r]->ProjectionX()->GetMean();
    r2c_FEI4[i_r] = rowHist2dFEI4[i_r]->ProjectionY()->GetMean();
    e_r2r_FEI4[i_r] = rowHist2dFEI4[i_r]->ProjectionX()->GetRMS();
    e_r2c_FEI4[i_r] = rowHist2dFEI4[i_r]->ProjectionY()->GetRMS();
    plotter->plotTH2D(rowHist2dFEI4[i_r], "row", "column", "hits",
		      Form("hit2D_row%i",i_r));
  }
  
  // loop over column histograms and add points to corresponding graph:
  double c_T3MAPS[100] = {0};
  double e_c_T3MAPS[100] = {0};
  double c2r_FEI4[100] = {0};
  double c2c_FEI4[100] = {0};
  double e_c2r_FEI4[100] = {0};
  double e_c2c_FEI4[100] = {0};
  
  for (int i_c = 0; i_c < myChips->getChipSize("T3MAPS","nColumns"); i_c++) {
    c_T3MAPS[i_c] = i_c+1;
    e_c_T3MAPS[i_c] = 0.5;
    c2r_FEI4[i_c] = colHist2dFEI4[i_c]->ProjectionX()->GetMean();
    c2c_FEI4[i_c] = colHist2dFEI4[i_c]->ProjectionY()->GetMean();
    e_c2r_FEI4[i_c] = colHist2dFEI4[i_c]->ProjectionX()->GetRMS();
    e_c2c_FEI4[i_c] = colHist2dFEI4[i_c]->ProjectionY()->GetRMS();
    plotter->plotTH2D(colHist2dFEI4[i_c], "row", "column", "hits",
		      Form("hit2D_col%i",i_c));
  }
  
  // create TGraphs:
  TGraphErrors *g_r2r = new TGraphErrors(myChips->getChipSize("T3MAPS","nRows"), r_T3MAPS, r2r_FEI4, e_r_T3MAPS, e_r2r_FEI4);
  TGraphErrors *g_r2c = new TGraphErrors(myChips->getChipSize("T3MAPS","nRows"), r_T3MAPS, r2c_FEI4, e_r_T3MAPS, e_r2c_FEI4);
  TGraphErrors *g_c2r = new TGraphErrors(myChips->getChipSize("T3MAPS","nColumns"), c_T3MAPS, c2r_FEI4, e_c_T3MAPS, e_c2r_FEI4);
  TGraphErrors *g_c2c = new TGraphErrors(myChips->getChipSize("T3MAPS","nColumns"), c_T3MAPS, c2c_FEI4, e_c_T3MAPS, e_c2c_FEI4);
  
  // extract constants from fits:
  TF1 *fit_r2r = createFit("fit_r2r", g_r2r, 1, 
			   myChips->getChipSize("T3MAPS","nRows"));
  TF1 *fit_r2c = createFit("fit_r2c", g_r2c, 1, 
			   myChips->getChipSize("T3MAPS","nRows"));
  TF1 *fit_c2r = createFit("fit_c2r", g_c2r, 1, 
			   myChips->getChipSize("T3MAPS","nColumns"));
  TF1 *fit_c2c = createFit("fit_c2c", g_c2c, 1,
			   myChips->getChipSize("T3MAPS","nColumns"));
  
  // Plot the linear relations:
  plotter->plotTGraphErrFit(g_r2r, fit_r2r, "row_{T3MAPS}", "row_{FEI4}",
			    "fit_r2r");
  plotter->plotTGraphErrFit(g_r2c, fit_r2c, "row_{T3MAPS}", "column_{FEI4}", 
			    "fit_r2c");
  plotter->plotTGraphErrFit(g_c2r, fit_c2r, "column_{T3MAPS}", "row_{FEI4}",
			    "fit_c2r");
  plotter->plotTGraphErrFit(g_c2c, fit_c2c, "column_{T3MAPS}", "column_{FEI4}",
			    "fit_c2c");
  
  // Choose the fit variables:
  if ((fabs(fit_r2r->GetParameter(0)) > fabs(fit_r2c->GetParameter(0))) &&
      (fabs(fit_c2c->GetParameter(0)) > fabs(fit_c2r->GetParameter(0)))) {
    sameOrientation = true;
    mVar[0] = fit_r2r->GetParameter(0);//slope
    mVar[1] = fit_r2r->GetParameter(1);//offset
    mVar[2] = fit_c2c->GetParameter(0);//slope
    mVar[3] = fit_c2c->GetParameter(1);//offset
    std::cout << "  T3MAPS and FEI4 have same orientation +/-pi" << std::endl;
  }
  else if ((fabs(fit_r2r->GetParameter(0)) < fabs(fit_r2c->GetParameter(0))) &&
	   (fabs(fit_c2c->GetParameter(0)) < fabs(fit_c2r->GetParameter(0)))) {
    sameOrientation = false;
    mVar[0] = fit_r2c->GetParameter(0);//slope
    mVar[1] = fit_r2c->GetParameter(1);//offset
    mVar[2] = fit_c2r->GetParameter(0);//slope
    mVar[3] = fit_c2r->GetParameter(1);//offset
    std::cout << "  T3MAPS and FEI4 have pi/2 relative rotation" << std::endl;
  }
  else {
    sameOrientation = true;
    mVar[0] = fit_r2r->GetParameter(0);//slope
    mVar[1] = fit_r2r->GetParameter(1);//offset
    mVar[2] = fit_c2c->GetParameter(0);//slope
    mVar[3] = fit_c2c->GetParameter(1);//offset
    std::cout << "  WARNING! orientation is ambiguous!" << std::endl;
  }
  
  for (int i = 0; i < 4; i++) hasMap[i] = true;
}

/**
Load parameters from previous mapping:
*/
void LinearMapMaker::loadMapParameters(TString inputDir) {
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
void LinearMapMaker::saveMapParameters(TString outputDir) {
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
void LinearMapMaker::setMapRMS(int varIndex, double newVal) {
  if (varIndex >= 0 && varIndex < 4) {
    mRMS[varIndex] = newVal;
  }
  else {
    std::cout << "  LinearMapMaker::setMapVar Improper varIndex!" << std::endl;
  }
}

/**
   Sets a parameter for the linear map. Index = 0,1,2,3.
   newVal is the new value of the specified mapping parameter.
 */
void LinearMapMaker::setMapVar(int varIndex, double newVal) {
  if (varIndex >= 0 && varIndex < 4) {
    mVar[varIndex] = newVal;
    hasMap[varIndex] = true;
  }
  else {
    std::cout << "  LinearMapMaker::setMapVar Improper varIndex!" << std::endl;
  }
}

/**
   Checks that all four parameters necessary for a complete T3MAPS<-->FEI4
   mapping have been assigned.
*/
bool LinearMapMaker::mapExists() {
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
int LinearMapMaker::getFEI4fromT3MAPS(TString valName, int valT3MAPS) {
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
int LinearMapMaker::getT3MAPSfromFEI4(TString valName, int valFEI4) {
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
double LinearMapMaker::getMapRMS(int varIndex) {
  if (mapExists()) {
    return mRMS[varIndex];
  }
  else {
    std::cout << "    LinearMapMaker::getMapVar No map exists!" << std::endl;
    return -1;
  }
}

/**
   Returns the parameters for the linear maps.
 */
double LinearMapMaker::getMapVar(int varIndex) {
  if (mapExists()) {
    return mVar[varIndex];
  }
  else {
    std::cout << "    LinearMapMaker::getMapVar No map exists!" << std::endl;
    return -1;
  }
}

/**
   Print the parameters from the most recent mapping.
 */
void LinearMapMaker::printMapParameters() {
  
  std::cout << "  LinearMapMaker::makeCombinedMap Precise map ok." << std::endl;
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
