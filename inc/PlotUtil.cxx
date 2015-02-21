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
  SetAtlasStyle();
  can = new TCanvas("can","can",canWidth,canHeight);
  can->cd();
  return;
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

TStyle* PlotUtil::AtlasStyle() {
  TStyle *atlasStyle = new TStyle("ATLAS","Atlas style");

  // use plain black on white colors
  Int_t icol=0; // WHITE
  atlasStyle->SetFrameBorderMode(icol);
  atlasStyle->SetFrameFillColor(icol);
  atlasStyle->SetCanvasBorderMode(icol);
  atlasStyle->SetCanvasColor(icol);
  atlasStyle->SetPadBorderMode(icol);
  atlasStyle->SetPadColor(icol);
  atlasStyle->SetStatColor(icol);
  // don't use: white fill color for *all* objects:
  //atlasStyle->SetFillColor(icol); 

  // set the paper & margin sizes
  atlasStyle->SetPaperSize(20,26);

  // set margin sizes
  atlasStyle->SetPadTopMargin(0.05);
  atlasStyle->SetPadRightMargin(0.05);
  atlasStyle->SetPadBottomMargin(0.16);
  atlasStyle->SetPadLeftMargin(0.16);

  // set title offsets (for axis label)
  atlasStyle->SetTitleXOffset(1.1);
  atlasStyle->SetTitleYOffset(1.3);

  // use large fonts
  //Int_t font=72; // Helvetica italics
  Int_t font=42; // Helvetica
  Double_t tsize=0.05; // originally 0.05
  atlasStyle->SetTextFont(font);

  atlasStyle->SetTextSize(tsize);
  atlasStyle->SetLabelFont(font,"x");
  atlasStyle->SetTitleFont(font,"x");
  atlasStyle->SetLabelFont(font,"y");
  atlasStyle->SetTitleFont(font,"y");
  atlasStyle->SetLabelFont(font,"z");
  atlasStyle->SetTitleFont(font,"z");
  
  atlasStyle->SetLabelSize(tsize,"x");
  atlasStyle->SetTitleSize(tsize,"x");
  atlasStyle->SetLabelSize(tsize,"y");
  atlasStyle->SetTitleSize(tsize,"y");
  atlasStyle->SetLabelSize(tsize,"z");
  atlasStyle->SetTitleSize(tsize,"z");

  // use bold lines and markers
  atlasStyle->SetMarkerStyle(20);
  atlasStyle->SetMarkerSize(1.2);
  atlasStyle->SetHistLineWidth((Width_t)3.0);
  atlasStyle->SetLineStyleString(2,"[12 12]"); // postscript dashes

  // get rid of X error bars 
  //atlasStyle->SetErrorX(0.001);
  // get rid of error bar caps
  atlasStyle->SetEndErrorSize(0.);

  // do not display any of the standard histogram decorations
  atlasStyle->SetOptTitle(0);
  //atlasStyle->SetOptStat(1111);
  atlasStyle->SetOptStat(0);
  //atlasStyle->SetOptFit(1111);
  atlasStyle->SetOptFit(0);

  // put tick marks on top and RHS of plots
  atlasStyle->SetPadTickX(1);
  atlasStyle->SetPadTickY(1);
  
  atlasStyle->SetPalette(1);
  
  return atlasStyle;
}

void PlotUtil::SetAtlasStyle() {
  std::cout << "\nPlotUtil: Applying ATLAS style settings...\n" << std::endl;
  TStyle* atlasStyle = AtlasStyle();
  gROOT->SetStyle("ATLAS");
  gROOT->ForceStyle();
}

void PlotUtil::setOutputDirectory(TString newOutDir) {
  outputDirectory = newOutDir;
}
