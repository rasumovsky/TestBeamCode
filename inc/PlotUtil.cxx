////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: PlotUtil.cxx                                                        //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 10/02/2015                                                          //
//                                                                            //
//  This class stores ROOT plotting code that is reused in many places.       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "PlotUtil.h"

/**
   Initialize a hit:
 */
PlotUtil::PlotUtil(TString newOutDir, int canWidth=800, int canHeight=800) {
  outputDirectory = newOutDir;
  can = new TCanvas("can","can",canWidth,canHeight);
  can->cd();
  return;
}

void PlotUtil::setOutputDirectory(TString newOutDir) {
  outputDirectory = newOutDir;
}
