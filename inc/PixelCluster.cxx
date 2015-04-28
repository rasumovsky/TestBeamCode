////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: PixelCluster.cxx                                                    //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 14/04/2015                                                          //
//                                                                            //
//  This class stores a list of pixel hits that have been grouped together to //
//  form a single cluster.                                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "PixelCluster.h"

/**
   Initialize a cluster:
 */
PixelCluster::PixelCluster(std::string newChipName) {
  clearCluster();
  chipName = newChipName;
  return;
}

/** 
    Add another cluster to this cluster.
    @param cluster - the cluster to merge.
*/
void PixelCluster::addCluster(PixelCluster *cluster) {
  
  std::vector<PixelHit*> newClusterHits = cluster->getHits();
  std::vector<PixelHit*> newClusterMasks = cluster->getMasks();
  
  // Iterate over hits:
  for (std::vector<PixelHit*>::iterator iterHit = newClusterHits.begin();
       iterHit != newClusterHits.end(); iterHit++) {
    addHit(iterHit);// addHit also adds the TOT and charge.
  }
  // Iterate over masked pixels:
  for (std::vector<PixelHit*>::iterator iterMask = newClusterMasks.begin();
       iterMask != newClusterMasks.end(); iterMask++) {
    addMask(iterMask);
  }
}

/**
   Add a hit to the cluster.
   @param hit - the pixel hit to merge.
*/
void PixelCluster::addHit(PixelHit *hit) {
  // Only proceed if the hit is not already included in the cluster.
  if (!containsHit(hit)) {
    if (hit.getTOT() > 15) {
      std::cout << "PixelCluster::addHit Error - Invalid TOT." << std::endl;
    }
    // Don't add TOT=14 (delayed) or TOT=15 (no hit) in the sum:
    if (hit.getTOT() >= 0 && hit.getTOT() < 14) {
      clusterTOTSum += hit->getTOT();
      clusterHits.push_back(hit);
    }
    // Add to list of delayed hits:
    else if (hit.getTOT() == 14) {
      std::cout << "PixelCluster: found delayed hit." << std::endl;
    }
  }
}

/**
   Add a masked pixel to the cluster.
   @param mask - the mask (hit object) to add to the cluster.
*/
void PixelCluster::addMask(PixelHit *mask) {
  if (!containsHit(mask)) {
    clusterMasks.push_back(mask);
  }
}

/**
   Clear the PixelCluster data contents.
*/
void PixelCluster::clearCluster() {
  clusterTOTSum = 0.0;
  clusterPathLength = 0.0;
  clusterHits.clear();
  clusterMasks.clear();
  clusterCentroid.first = 0.0;
  clusterCentroid.second = 0.0;
  clusterHist = NULL;
  tracklet = NULL;
}

/**
   Fill the cluster histogram.
*/
void PixelCluster::fillHistogram() {
  
  float colLo(80), colHi(0), rowLo(336), rowHi(0);
  
  // loop over the hits:
  for (std::vector<PixelHit*>::iterator iterHit = clusterHits.begin();
       iterHit != clusterHits.end(); iterHit++) {
    if (iterHits->getCol() < colLo) colLo = iterHit->getCol();
    if (iterHits->getCol() > colHi) colHi = iterHit->getCol();
    if (iterHits->getRow() < rowLo) rowLo = iterHit->getRow();
    if (iterHits->getRow() > rowHi) rowHi = iterHit->getRow();
  }
  
  // loop over the masks:
  for (std::vector<PixelHit*>::iterator iterMask = clusterMasks.begin();
       iterMask != clusterMasks.end(); iterMask++) {
    if (iterMask->getCol() < colLo) colLo = iterMask->getCol();
    if (iterMask->getCol() > colHi) colHi = iterMask->getCol();
    if (iterMask->getRow() < rowLo) rowLo = iterMask->getRow();
    if (iterMask->getRow() > rowHi) rowHi = iterMask->getRow();
  }
  
  colLo -= 1.5;
  colHi += 1.5;
  rowLo -= 1.5;
  rowHi += 1.5;
  
  clusterHist = new TH2F("clusterHist", "clusterHist",
			 (colHi - colLo), colLo, colHi,
			 (rowHi - rowLo), rowLo, rowHi);
  
  // Fill histogram with hit and weight according to TOT:
  for (std::vector<PixelHit*>::iterator iterHit = clusterHits.begin();
       iterHit != clusterHits.end(); iterHit++) {
    double currWeight = iterHit->getTOT + 1;
    clusterHist->Fill(iterHit->getCol(), iterHit->getRow(), currWeight);
  }
  
  // Fill histogram with mask and weight -1:
  for (std::vector<PixelHit*>::iterator iterMask = clusterMasks.begin();
       iterMask != clusterMasks.end(); iterMask++) {
    clusterHist->Fill(iterMask->getCol(), iterMask->getRow(), -1);
  }
}

/** 
    Fit the tracklet in the chip. Borrowed from TrackletFitter/Cluster.C 
    written by Simon Viel.
*/
void PixelCluster::fitTracklet() {
  
  bool vertical = false;
  
  // Check that cluster histogram has been created and filled:
  if (!clusterHist) {
    fillHistogram();
  }

  // Least-squares method
  int counter = 0;
  float weight(0);
  float sumCol(0), sumRow(0), sumColCol(0), sumColRow(0);
  
  // Iterate over hits:
  for (std::vector<PixelHit*>::iterator iterHit = clusterHits.begin();
       iterHit != clusterHits.end(); iterHit++) {
    
    // Use the weighted average of TOT (update to charge eventually):
    currWeight = iterHit->getTOT() + 1;
    
    counter += currWeight;
    
    sumCol += iterHit->getCol() * currWeight;
    sumRow += iterHit->getRow() * currWeight;
    sumColCol += iterHit->getCol() * iterHit->getCol() * currWeight;
    sumColRow += iterHit->getCol() * iterHit->getRow() * currWeight;
  }
  
  float meanCol = sumCol / counter;
  float meanRow = sumRow / counter;
  float slope = (sumColRow - sumCol * meanRow) / (sumColCol - sumCol * meanCol);
  if (!isfinite(slope)) {
    vertical = true;
    slope = FLT_MAX;
  }
  float intercept = meanRow - slope * meanCol;
  
  // Set center of the charge deposition:
  setClusterCentroid(meanRow, meanCol);
  
  // Evaluate tracklet entry and exit points
  float entryCol, entryRow, exitCol, exitRow;
  if (vertical) {
    entryRow = clusterHist->GetYaxis()->GetBinUpEdge(1);
    exitRow = clusterHist->GetYaxis()->GetBinLowEdge(clusterHist->GetNbinsY());
    entryCol = clusterHist->GetMean(1);
    exitCol = entryCol;
  }
  else {
    entryRow = clusterHist->GetYaxis()->GetBinUpEdge(1);
    exitRow = clusterHist->GetYaxis()->GetBinLowEdge(clusterHist->GetNbinsY());
    entryCol = (entryRow - intercept) / slope;
    exitCol = (exitRow - intercept) / slope;
  }
    
  // correction if out of bounds ("mostly horizontal" tracklet)
  if (entryCol < clusterHist->GetXaxis()->GetBinUpEdge(1)) {
    entryCol = clusterHist->GetXaxis()->GetBinUpEdge(1);
    exitCol = clusterHist->GetXaxis()->GetBinLowEdge(clusterHist->GetNbinsX());
    entryRow = intercept + slope * entryCol;
    exitRow = intercept + slope * exitCol;
  }
  else if (entryCol > 
	   clusterHist->GetXaxis()->GetBinLowEdge(clusterHist->GetNbinsX())) {
    entryCol = clusterHist->GetXaxis()->GetBinLowEdge(clusterHist->GetNbinsX());
    exitCol = clusterHist->GetXaxis()->GetBinUpEdge(1);
    entryRow = intercept + slope * entryCol;
    exitRow = intercept + slope * exitCol;
  }
  
  // Store the result in a linear function:
  tracklet = new TF1("tracklet", "pol1", entryCol, exitCol);
  tracklet->SetParameter(0, intercept);
  tracklet->SetParameter(1, slope);
  
  // Path length of tracklet in sensor on the fly:
  // this is for planar modules (thickness of 230 for 3D modules)
  ChipDimension *cd = new ChipDimension();
  float thickness = cd->getThickness(chipName);// microns
  float row_pitch =  cd->getRowPitch(chipName);// microns
  float col_pitch = cd->getColPitch(chipName); // microns
  
  clusterPathLength = sqrt(pow((exitRow - entryRow) * rowPitch, 2) +
			   pow((exitCol - entryCol) * colPitch, 2) +
			   pow(thickness, 2));
}

/**
   Set the centroid of the cluster.
   @param row - the new centroid row.
   @param col - the new centroid column.
*/
void setClusterCentroid(double row, double col) {
  clusterCentroid.first = row;
  clusterCentroid.second = col;
}

/**
   Print the cluster histogram.
   @param fileName - the output file location.
*/
void PixelCluster::drawClusterHist(TString fileName) {
  
  TCanvas *can = new TCanvas("can","can",800,600);
  
  clusterHist->SetTitle("");
  clusterHist->GetXaxis()->SetTitle("Column");
  clusterHist->GetYaxis()->SetTitle("Row");
  clusterHist->GetZaxis()->SetTitle("ToT");
  clusterHist->SetMinimum(-1);
  clusterHist->SetMaximum(14);
  
  clusterHist->Draw("colz");
  tracklet->Draw("Lsame");
  
  TGraph *centroidGraph = new TGraph(1);
  centroidGraph->SetPoint(0, centreOfCharge.second, centreOfCharge.first);
  centroidGraph->SetMarkerStyle(2);
  centroidGraph->SetMarkerSize(3);
  centroidGraph->Draw("Psame");
  can->Print(fileName);
}

/**
   Get the center of the cluster.
*/
std::pair<double,double> PixelCluster::getClusterCentroid() {
  return clusterCentroid;
}

/**
   Get the minimum separation between two clusters.
   @param cluster - the cluster to which the distance will be measured.
*/
double PixelCluster::getClusterSeparation(PixelCluster *cluster) {
  double minSeparation = 10000.0;
  //loop over hits in this cluster
  // Iterate over hits in input cluster:
  std::vector<PixelHit*> currClusterHits = cluster->getHits();
  for (std::vector<PixelHit*>::iterator iterHit1 = currClusterHits.begin();
       iterHit1 != currClusterHits.end(); iterHit1++) {
    
    // Iterate over hits in this cluster:
    for (std::vector<PixelHit*>::iterator iterHit2 = clusterHits.begin();
	 iterHit2 != clusterHits.end(); iterHit2++) {
      
      double currSeparation = sqrt((iterHit1->hitDistanceRow(iterHit2) * 
				    iterHit1->hitDistanceRow(iterHit2)) +
				   (iterHit1->hitDistanceCol(iterHit2) * 
				    iterHit1->hitDistanceCol(iterHit2)));
      if (currSeparation < minSeparation) {
	minSeparation = currSeparation;
      }
    }
  }
  return minSeparation;
}

/**
   Get the histogram of hits in the cluster.
*/
TH2F* PixelCluster::getClusterHist() {
  if (clusterHist) {
    return clusterHist;
  }
  else {
    fillClusterHist();
    return clusterHist;
  }
}

/**
   Get the number of hits in the cluster.
*/
int PixelCluster::getNHits() {
  return clusterHits.size();
}

/**
   Get the number of masked pixels in the cluster.
*/
int PixelCluster::getNMasked() {
  return clusterMasks.size();
}

/**
   Get the largest path length between hit pixels.
*/
double PixelCluster::getPathLength() {
  return clusterPathLength;
  /*
  clusterPathLength = 0.0;
  // Iterate over hits:
  for (std::vector<PixelHit*>::iterator iter1 = clusterHits.begin();
       iter1 != clusterHits.end(); iter1++) {
    
    // Iterate over hits again:
    for (std::vector<PixelHit*>::iterator iter2 = clusterHits.begin();
	 iter2 != clusterHits.end(); iter2++) {
      
      currDistance = sqrt((iter1->hitDistanceRow(iter2) *
			   iter1->hitDistanceRow(iter2)) +
			  (iter1->hitDistanceCol(iter2) *
			   iter1->hitDistanceCol(iter2)));
      
      if (currDistance > clusterPathLength) {
	clusterPathLength = currDistance;
      }
    }
  }
  return clusterPathLength;
  */
}

/**
   Get the number of masked pixels in the cluster.
*/
int PixelCluster::getTOTSum() {
  return clusterTOTSum;
}

/**
   Retrieve the tracklet stored as a linear function.
*/
TF1* PixelCluster::getTracklet() {
  if (tracklet) {
    return tracklet;
  }
  else {
    fitTracklet();
    return tracklet;
  }
}

/**
   Get the list of cluster hits.
*/
vector<PixelHit*> PixelCluster::getHits() {
  return clusterHits;
}

/**
   Get the list of cluster masks.
*/
vector<PixelHit*> PixelCluster::getMasks() {
  return clusterMasks;
}

/**
   Check if a hit is already contained in the cluster.
   @param hit - the hit to check.
*/
bool PixelCluster::containsHit(PixelHit *hit) {
  // Iterate over hits:
  for (std::vector<PixelHit*>::iterator iterHit = clusterHits.begin();
       iterHit != clusterHits.end(); iterHit++) {
    if (iterHit.equalTo(hit)) {
      return true;
    }
  }
  // Iterate over masked pixels:
  for (std::vector<PixelHit*>::iterator iterMask = clusterMasks.begin();
       iterMask != clusterMasks.end(); iterMask++) {
    if (iterMask.equalTo(hit)) {
      return true;
    }
  }
  return false;
}

/**
   Check if another cluster is next to this cluster.
   @param cluster - the cluster to check against this one.
*/
bool PixelCluster::isAdjacent(PixelCluster *cluster) {

  // Loop over this cluster
  for (int i = 0; i < (int)clusterHits.size(); i++) {
    PixelHit *currHit1 = clusterHits[i];
    
    // Loop over input cluster:
    for (int j = 0; j < cluster->getNHits(); j++) {
      PixelHit *currHit2 = cluster->getHit(j);
      if (currHit1->hitIsAdjacent(currHit2)) {
	return true;
      }
    }
  }
  return false;
}

/**
   Check if another cluster is overlapping with this cluster.
*/
bool PixelCluster::isOverlapping(PixelCluster *cluster) {
  // Iterate over hits:
  for (std::vector<PixelHit*>::iterator iterHit = clusterHits.begin();
       iterHit != clusterHits.end(); iterHit++) {
    if (cluster->containsHit(iterHit)) {
      return true;
    }
  }
  // Iterate over masked pixels:
  for (std::vector<PixelHit*>::iterator iterMask = clusterMasks.begin();
       iterMask != clusterMasks.end(); iterMask++) {
    if (cluster->containsHit(iterMask)) {
      return true;
    }
  }
  return false;
}

bool PixelCluster::clustersCanMerge(PixelCluster *cluster) {
  
  // Merge clusters if they share any of the same hits or masked pixels:
  if (isOverlapping(cluster)) {
    return true;
  }
  // Merge clusters if they have adjacent hits or masks:
  else if (isAdjacent(cluster)) {
    return true;
  }
  // Don't merge clusters that are missing all of these criteria.
  else {
    return false;
  }
}
