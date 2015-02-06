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
//        row_FEI4 = mapVar[0] * row_T3MAPS + mapVar[1];                      //
//        col_FEI4 = mapVar[2] * col_T3MAPS + mapVar[3];                      //
//                                                                            //
//  TODO: plotting and saving of histograms                                   //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "ModuleMapping.h"

/**
   Initialize the class using the input files to calculate new mapping:
*/
ModuleMapping::ModuleMapping(TTree *tT3MAPS, TTree *tFEI4, TString fileDir) {
  
  std::cout << "Initializing ModuleMapping..." << std::endl;
  // Settings for FEI4 and T3MAPS chip layouts:
  setChipDimensions();
  for (int i = 0; i < 4; i++) createdMap[i] = false;
  
  // if this doesn't work, maybe try double pointer?? **TTree... see bookmark
  myTreeT3MAPS = tT3MAPS;
  myTreeFEI4 = tFEI4;
  
  prepareTrees();// Load the two trees  
  makeCombinedMap();//Construct mapping T3MAPS<-->FEI4
  saveMapParameters(outputDir);//Save parameters to file
  
  std::cout << "Successfully initialized ModuleMapping!" << std::endl;
}

/**
   Initialize the class by using the results of a previous run:
 */
ModuleMapping::ModuleMapping(TString fileDir) {
  
  std::cout << "Initializing ModuleMapping..." << std::endl;
  // Settings for FEI4 and T3MAPS chip layouts:
  setChipDimensions();
  
  loadMapParameters(fileDir);// Load map from file
  std::cout << "Successfully initialized ModuleMapping!" << std::endl;
}

/**
Create output text file with map parameters:
*/
void ModuleMapping::saveMapParameters(TString outputDir) {
  ofstream outputFile;
  outputFile.open(Form("%s/mapParameters.txt",outputDir.Data()));
  for (int i = 0; i < 4; i++) {
    outputFile << "parameter" << i << " " << mapVar[i] << " " 
	       << mapRMS[i] << std::endl;
  }
  outputFile.close();
}


/**
Load parameters from previous mapping:
*/
void ModuleMapping::loadMapParameters(TString inputDir) {
  ifstream inputFile;
  inputFile.open(Form("%s/mapParameters.txt",inputDir.Data()));
  int lineIndex = 0;
  while (!inputFile.eof()) {
    inputFile >> paramname >> tempMapVar >> tempMapRMS;
    mapVar[lineIndex] = tempMapVar;
    mapRMS[lineIndex] = tempMapRMS;
    createdMap[lineIndex] = true;
    lineIndex++;
  }
  inputFile.close();
  printMapParameters();
}

/**
 */
void ModuleMapping::printMapParameters() {
  
  std::cout << "  ModuleMapping::makeCombinedMap Precise map ok." << std::endl;
  std::cout << "    Printing map parameters:" << std::endl;
  for (int i = 0; i < 4; i++) {
    std::cout << "      parameter(" << i << ") = " << mapVar[i] 
	      << " +/- " << mapRMS[i] << std::endl;
  }
  std::cout << "  Interpretation:" << std::endl;
  std::cout << "    row_FEI4 = " << mapVar[0] << " * row_T3MAPS + " 
	    << mapVar[1]<< std::endl;
  std::cout << "    col_FEI4 = " << mapVar[2] << " * col_T3MAPS + " 
	    << mapVar[3] << std::endl;
}

/**
   Load the TTrees for T3MAPS and FEI4 from file, and set the branch addresses.
*/
void ModuleMapping::prepareTrees() {
  
  std::cout << "  ModuleMapping::prepareTrees() reading hit files" << std::endl;
  
  // Set the T3MAPS TTree addresses:
  myTreeT3MAPS->SetBranchAddress("timestamp_start", &t_T3MAPS_timestamp_start);
  myTreeT3MAPS->SetBranchAddress("timestamp_stop", &t_T3MAPS_timestamp_stop);
  myTreeT3MAPS->SetBranchAddress("hit_row", &t_T3MAPS_hit_row);
  myTreeT3MAPS->SetBranchAddress("hit_column", &t_T3MAPS_hit_column);
  
  // Load the FEI4 data tree:
  myTreeFEI4->SetBranchAddress("timestamp_start", &t_FEI4_timestamp_start);
  myTreeFEI4->SetBranchAddress("timestamp_stop", &t_FEI4_timestamp_stop);
  myTreeFEI4->SetBranchAddress("row", &t_FEI4_hit_row);
  myTreeFEI4->SetBranchAddress("column", &t_FEI4_hit_column);
}

/**
   This method uses several pairs of T3MAPS pixels to derive a linear one-to-one
   mapping from T3MAPS(row,column) <--> FEI4(row,column). It averages the
   results of individual calls to makeGeoMap() to get a more accurate mapping.
 */
void ModuleMapping::makeCombinedMap() {
  
  std::cout << "  ModuleMapping::makeCombinedMap Starting..." << std::endl;
  
  int currRow1 = 1;
  int currCol1 = 1;
  
  int currRow2 = nRowT3MAPS;
  int currCol2 = nColT3MAPS;
  
  for (int i = 0; i < 4; i++) {
    histMapValues[i] = new TH1F(Form("histMapValues%i",i),
				Form("histMapValues%i",i)
				40,-2,2);
  }
  
  // Scan edge pixels of T3MAPS and create a map for each.
  while (currRow1 < nRowT3MAPS || currCol1 < nColT3MAPS) {
    
    // Make a new map of the geometries with the current points:
    makeGeoMap( currRow1, currCol1, currRow2, currCol2 );
    
    // Record the values:
    for (int i = 0; i < 4; i++) {
      histMapValues[i]->Fill(mapVar[i]);
    }
    
    // Advance each of the points around T3MAPS edge:
    if (currRow1 < nRowT3MAPS) {
      currRow1++;
      currRow2--;
    }
    else {
      currCol1++;
      currCol2--;
    }
  }
  
  // Save the precision mapping constants and the error.
  for (int i = 0; i < 4; i++) {
    mapVar[i] = histMapValues[i]->GetMean();
    mapRMS[i] = histMapValues[i]->GetRMS();
  }
  
  printMapParameters();
}

/**
   This method uses two T3MAPS pixels to derive a linear one-to-one mapping 
   from T3MAPS(row,column) <--> FEI4(row,column).
 */
void ModuleMapping::makeGeoMap(int rowT3MAPS_1, int colT3MAPS_1, 
			       int rowT3MAPS_2, int colT3MAPS_2) {
  
  std::cout << "  ModuleMapping::makeGeoMap() Maps T3MAPS-->FEI4" << std::endl;
  
  for (int i = 0; i < 2; i++) {
    histRowFEI4Pix[i] = new TH1F(Form("FEI4Pix%iRow",i),Form("FEI4Pix%iRow",i),
				 nRowFEI4,0.5,((double)nRowFEI4)+0.5);
    histColFEI4Pix[i] = new TH1F(Form("FEI4Pix%iCol",i),Form("FEI4Pix%iCol",i),
				 nColFEI4,0.5,((double)nColFEI4)+0.5);
    hist2dFEI4Pix[i] = new TH2F(Form("FEI4Pix%i2d",i),Form("FEI4Pix%i2d",i),
				nRowFEI4,0.5,((double)nRowFEI4)+0.5,
				nColFEI4,0.5,((double)nColFEI4)+0.5);
  }
  
  // Count the number of coincidences found.
  int nHitsPix1 = 0;
  int nHitsPix2 = 0;
  
  // Prepare FEI4 tree for loop inside T3MAPS tree's loop.
  Long64_t entriesFEI4 = myTreeFEI4->GetEntries();
  Long64_t eventFEI4 = 0;
  myTreeFEI4->GetEntry(eventFEI4);
  
  // Loop over T3MAPS tree.
  Long64_t entriesT3MAPS = myTreeT3MAPS->GetEntries();
  for (Long64_t eventT3MAPS = 0; eventT3MAPS < entriesT3MAPS; eventT3MAPS++) {
    
    myTreeT3MAPS->GetEntry(eventT3MAPS);
    
    // Check to see if either of the mapping pixels were hit:
    bool hitPix1 = false;
    bool hitPix2 = false;
    for (int hitT3MAPS = 0; hitT3MAPS < t_T3MAPS_hit_row.size(); hitT3MAPS++) {
      
      if (t_T3MAPS_hit_row(hitT3MAPS) == rowT3MAPS_1 && 
	  t_T3MAPS_hit_column(hitT3MAPS) == colT3MAPS_1) {
	hitPix1 = true;
      }
      else if (t_T3MAPS_hit_row(hitT3MAPS) == rowT3MAPS_2 && 
	  t_T3MAPS_hit_column(hitT3MAPS) == colT3MAPS_2) {
	hitPix2 = true;
      }
    }
    
    // Only consider events with hits in the mapping pixels:
    if (!hitPix1 && !hitPix2) {
      continue;
    }
    
    // also advance FEI4 tree.
    while (t_FEI4_timestamp_start < t_T3MAPS_timestamp_stop && 
	   eventFEI4 < entriesFEI4) {

      eventFEI4++;
      myTreeFEI4->GetEntry(eventFEI4);
      
      // Only consider events with timestamp inside that of T3MAPS
      // Same as looking for time coincidence hits in FEI4 and T3MAPS
      if (t_FEI4_timestamp_start >= t_T3MAPS_timestamp_start &&
	  t_FEI4_timestamp_stop <= t_T3MAPS_timestamp_stop) {
	
	if (hitPix1) {
	  histRowFEI4Pix[0]->Fill(t_FEI4_hit_row);
	  histColFEI4Pix[0]->Fill(t_FEI4_hit_column);
	  hist2dFEI4Pix[0]->Fill(t_FEI4_hit_row, t_FEI4_hit_column);
	  nHitsPix1++;
	}
	
	if (hitPix2) {
	  histRowFEI4Pix[1]->Fill(t_FEI4_hit_row);
	  histColFEI4Pix[1]->Fill(t_FEI4_hit_column);
	  hist2dFEI4Pix[1]->Fill(t_FEI4_hit_row, t_FEI4_hit_column);
	  nHitsPix2++;
	}
      }
    }
  }// End of loop over events
  
  // make plot of FEI4 row and col for both T3MAPS pixels (4 histograms)
    
  // Use max bin to get the row and column numbers in T3MAPS.
  int rowFEI4_1 = histRowFEI4Pix1->GetMaximumBin();
  int colFEI4_1 = histColFEI4Pix1->GetMaximumBin();
  int rowFEI4_2 = histRowFEI4Pix2->GetMaximumBin();
  int colFEI4_2 = histColFEI4Pix2->GetMaximumBin();
  
  // Calculate the linear mapping constants:
  double mapVar[0] = ((double)(rowFEI4_2-rowFEI4_1)) 
    / ((double)(rowT3MAPS_2-rowT3MAPS_1));
  double mapVar[2] = ((double)(colFEI4_2-colFEI4_1))
    / ((double)(colT3MAPS_2-colT3MAPS_1));
  double mapVar[1] = ((double)rowFEI4_1) - (mapVar[0] * (double)rowT3MAPS_1);
  double mapVar[3] = ((double)colFEI4_1) - (mapVar[2] * (double)colT3MAPS_1);
  
  for (int i = 0; i < 4; i++) createdMap[i] = true;
    
  std::cout << "  makeGeoMap Finished successfully!" << std::endl;
  std::cout << "    matched hits for T3MAPS pixel1: " << nHitsPix1 << endl;
  std::cout << "    matched hits for T3MAPS pixel2: " << nHitsPix2 << endl;
}

/**
   Checks that all four parameters necessary for a complete T3MAPS<-->FEI4
   mapping have been assigned.
*/
bool ModuleMapping::mapExists() {
  if (createdMap[0] && createdMap[1] && createdMap[2] && createdMap[3]) {
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
int ModuleMapping::getFEI4fromT3MAPS(TString pos, int valT3MAPS) {
  if (mapExists()) {
    if (pos == "row") {
      int row = (int)(mapVar[0] * ((double)valT3MAPS) + mapVar[1]);
      if (row > 0 && row < nRowFEI4) {
	return row;
      }
      else {
	return -1;
      }
    }
    else if (pos == "col") {
      int col = (int)(mapVar[2] * ((double)valT3MAPS) + mapVar[3]);
      if (col > 0 && col < nColFEI4) {
	return col;
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
int ModuleMapping::getT3MAPSfromFEI4(TString pos, int valFEI4) {
  if (mapExists()) {
    if (pos == "row") {
      int row = (int)((((double)valFEI4) - mapVar[1]) / mapVar[0]);
      if (row > 0 && row < nRowT3MAPS) {
	return row;
      }
      else {
	return -1;
      }
    }
    else if (pos == "col") {
      int col = (int)((((double)valFEI4) - mapVar[3]) / mapVar[2]);
      if (col > 0 && col < nColT3MAPS) {
	return col;
      }
      else { 
	return -1;
      }
    }
  }
  return -1;
}

/**
   Returns the parameters for the linear maps.
       row_FEI4 = [0] * row_T3MAPS + [1];
       col_FEI4 = [2] * col_T3MAPS + [3];
 */
double ModuleMapping::getMapVar(int varIndex) {
  if (mapExists()) {
    return mapVar[varIndex];
  }
  else {
    std::cout << "    ModuleMapping::getMapVar No map exists!" << std::endl;
    return -1;
  }
}

/**
   Returns the error on the 4 parameters for the linear maps. Index = 0,1,2,3.
 */
double ModuleMapping::getMapRMS(int varIndex) {
  if (mapExists()) {
    return mapRMS[varIndex];
  }
  else {
    std::cout << "    ModuleMapping::getMapVar No map exists!" << std::endl;
    return -1;
  }
}

/**
   Sets a parameter for the linear map. Index = 0,1,2,3.
   newVal is the new value of the specified mapping parameter.
       row_FEI4 = [0] * row_T3MAPS + [1];
       col_FEI4 = [2] * col_T3MAPS + [3];
 */
void ModuleMapping::setMapVar(int varIndex, double newVal) {
  if (varIndex >= 0 && varIndex < 4) {
    mapVar[varIndex] = newVal;
    createdMap[varIndex] = true;
  }
  else {
    std::cout << "    ModuleMapping::setMapVar Improper varIndex!" << std::endl;
  }
}

/**
   Sets the RMS for a parameter for the linear map. Index = 0,1,2,3.
*/
void ModuleMapping::setMapRMS(int varIndex, double newVal) {
  if (varIndex >= 0 && varIndex < 4) {
    mapRMS[varIndex] = newVal;
  }
  else {
    std::cout << "    ModuleMapping::setMapVar Improper varIndex!" << std::endl;
  }
}

/**
   Set the standard number of rows and columns for pixels in FEI4 and T3MAPS.
   Row and column numbers start at 1 and go up to the values below.
 */
void ModuleMapping::setChipDimensions(void) {
  nRowFEI4 = 336;
  nColFEI4 = 80;
  nRowT3MAPS = 16;
  nColT3MAPS = 64;
}
