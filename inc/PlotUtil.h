////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: PlotUtil.h                                                          //
//  Class: PlotUtil.cxx                                                       //
//                                                                            //
//  Author: Andrew Hard                                                       //
//  Email: ahard@cern.ch                                                      //
//  Date: 10/02/2015                                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef PlotUtil_h
#define PlotUtil_h

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

#include <string>
#include <sstream>
#include <algorithm>
#include <list>
#include <map>
#include <cmath>

/// ROOT headers
#include <TROOT.h>
#include <TObject.h>
#include <TMath.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TTree.h>
#include <TString.h>
#include <TH2.h>
#include <THStack.h>
#include <TBox.h>
#include <TLegend.h>
#include <TPad.h>
#include <TF1.h>
#include <TProfile.h>
#include <TDirectory.h>
#include <TStyle.h>
#include <TLine.h>
#include <TFrame.h>
#include <TAxis.h>
#include <TPaveText.h>
#include <TLatex.h>
#include <TSystem.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TGraph2D.h>
#include <TMultiGraph.h>
#include <TGraphAsymmErrors.h>
#include <TPaveStats.h>
#include <TPolyLine3D.h>
#include <TH2Poly.h>
#include <TGaxis.h>

namespace PlotUtil {
  
  TStyle* atlasStyle();
  
  void setAtlasStyle();
  
  void animateTH2D(TH2D *h2, TString xname, TString yname, TString zname, 
		    TString sname);
  void finishAnimation(TString sname);
  
  void plotTH1F(TH1F *h, TString xname, TString yname, TString sname, double x1,
		double x2, double y1, double y2, bool log);

  void plotTH1F(TH1F *h, TString xname, TString yname, TString sname, double x1,
		double x2, double y1, double y2);
  
  void plotTH1F(TH1F *h, TString xname, TString yname, TString sname, bool log);
  void plotTH1F(TH1F *h, TString xname, TString yname, TString sname);
  
  void plotTH1FAndFit(TH1F *h, TF1 *f, TString xname, TString yname,
		      TString sname, bool log);
  
  void plotTwoTH1Fs(TH1F *h1, TH1F *h2, TString xname, TString yname, 
		    TString sname, bool normalize);

  void plotTH2D(TH2D *h, TString xname, TString yname, TString zname, 
		TString sname, double x1, double x2, double y1, double y2,
		double z1, double z2);
  
  void plotTH2D(TH2D *h, TString xname, TString yname, TString zname, 
		TString sname);

  void plotTGraph(TGraph *g, TString xname, TString yname, TString sname);
  
  void plotTGraphErrFit(TGraphErrors *g, TF1 *fit, TString xname, TString yname,
			TString sname);
  
  /*
  void plotMultiTH1F(TH1F *h[], TString labels[], int size, TString xname, 
		     TString yname, TString sname, double x1, double x2,
		     double y1, double y2);
  */
  
};

#endif
