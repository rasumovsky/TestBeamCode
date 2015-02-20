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

void PlotUtil::plotTH1F(TH1F *h, TString xname, TString yname, TString sname, 
			double x1=0, double x2=0, double y1=0, double y2=0) {
  
  can->cd();
  can->Clear();
  h->Draw();
  h->GetXaxis()->SetTitle(xname);
  if (x1 != 0 || x2 != 0) h->GetXaxis()->SetRangeUser(x1,x2);
  h->GetYaxis()->SetTitle(yname);
  if (y1 != 0 || y2 != 0) h->GetYaxis()->SetRangeUser(y1,y2);
  can->Print(Form("%s/%s.eps",outputDirectory.Data(),sname.Data()));
  can->Print(Form("%s/%s.pdf",outputDirectory.Data(),sname.Data()));
  can->Clear();
}


void PlotUtil::plotTH1F(TH1F *h, TString xname, TString yname, TString sname) {
  plotTH1F(h, xname, yname, sname, 0, 0, 0, 0);
}

void PlotUtil::plotTH2D(TH2D *h2, TString xname, TString yname, TString zname,
			TString sname, double x1=0, double x2=0, double y1=0,
			double y2=0, double z1=0, double z2=0) {
  
  can->cd();
  can->Clear();
  gPad->SetRightMargin(0.15);
  h2->Draw("colz");
  h2->GetXaxis()->SetTitle(xname);
  if (x1 != 0 || x2 != 0) h2->GetXaxis()->SetRangeUser(x1,x2);
  h2->GetYaxis()->SetTitle(yname);
  if (y1 != 0 || y2 != 0) h2->GetYaxis()->SetRangeUser(y1,y2);
  h2->GetZaxis()->SetTitle(zname);
  if (z1 != 0 || z2 != 0) h2->GetZaxis()->SetRangeUser(z1,z2);
  can->Print(Form("%s/%s.eps",outputDirectory.Data(),sname.Data()));
  can->Clear();
}


void PlotUtil::plotTH2D(TH2D *h2, TString xname, TString yname, TString zname,
			TString sname) {
  plotTH2D(h2, xname, yname, zname, sname, 0, 0, 0, 0, 0, 0);
}

void PlotUtil::plotTGraphErrFit(TGraphErrors *g, TF1 *fit, TString xname,
				TString yname, TString sname) {
  can->cd();
  can->Clear();
  g->GetXaxis()->SetTitle(xname);
  g->GetYaxis()->SetTitle(yname);
  g->Draw("AP");
  fit->Draw("SAME");
  can->Print(Form("%s/%s.eps",outputDirectory.Data(),sname.Data()));
  can->Clear();
}
