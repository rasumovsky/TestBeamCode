////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Name: PlotUtil.cxx                                                        //
//                                                                            //
//  Created: Andrew Hard                                                      //
//  Email: ahard@cern.ch                                                      //
//  Date: 06/05/2015                                                          //
//                                                                            //
//  This namespace stores ROOT plotting code that is reused in many places.   //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "PlotUtil.h"

TStyle* PlotUtil::atlasStyle() {
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

void PlotUtil::setAtlasStyle() {
  std::cout << "\nPlotUtil: Applying ATLAS style settings...\n" << std::endl;
  TStyle *atlasStyle = PlotUtil::atlasStyle();
  gROOT->SetStyle("ATLAS");
  gROOT->ForceStyle();
}

void PlotUtil::animateTH2D(TH2D *h2, TString xname, TString yname,
			    TString zname, TString sname) {
  TCanvas *can = new TCanvas("can","can",800,800);
  can->cd();
  can->Clear();
  gPad->SetRightMargin(0.15);
  h2->Draw("colz");
  h2->GetXaxis()->SetTitle(xname);
  h2->GetYaxis()->SetTitle(yname);
  h2->GetZaxis()->SetTitle(zname);
  can->Print(Form("%s.gif+5", sname.Data()));
  delete can;
}

void PlotUtil::finishAnimation(TString sname) {
  TCanvas *can = new TCanvas("can","can",800,800);
  can->cd();
  TLatex text; text.SetNDC(); text.SetTextColor(1);
  text.DrawLatex(0.4, 0.45, "END");
  can->Print(Form("%s.gif++", sname.Data()));
  delete can;
}

void PlotUtil::plotTH1F(TH1F *h, TString xname, TString yname, TString sname, 
			double x1, double x2, double y1, double y2, bool log) {
  TCanvas *can = new TCanvas("can","can",800,800);
  can->cd();
  can->Clear();
  h->Draw();
  h->GetXaxis()->SetNoExponent(false);
  if (log) gPad->SetLogy();
  h->GetXaxis()->SetTitle(xname);
  if (x1 != 0 || x2 != 0) h->GetXaxis()->SetRangeUser(x1,x2);
  h->GetYaxis()->SetTitle(yname);
  if (y1 != 0 || y2 != 0) h->GetYaxis()->SetRangeUser(y1,y2);
  can->Print(Form("%s.eps", sname.Data()));
  can->Print(Form("%s.gif+5", sname.Data()));
  if (log) gPad->SetLogy(false);
  delete can;
}

void PlotUtil::plotTH1F(TH1F *h, TString xname, TString yname, TString sname, 
			double x1, double x2, double y1, double y2) {
  PlotUtil::plotTH1F(h, xname, yname, sname, x1, x2, y1, y2, false);
}

void PlotUtil::plotTH1F(TH1F *h, TString xname, TString yname, TString sname,
			bool log) {
  plotTH1F(h, xname, yname, sname, 0, 0, 0, 0, log);
}

void PlotUtil::plotTH1F(TH1F *h, TString xname, TString yname, TString sname) {
  plotTH1F(h, xname, yname, sname, false);
}

void PlotUtil::plotTwoTH1Fs(TH1F *h1, TH1F *h2, TString xname, TString yname, 
			    TString sname, bool normalize) {
  TCanvas *can = new TCanvas("can","can",800,800);
  can->cd();
  can->Clear();
  if (normalize) {
    h1->Scale(1.0/h1->GetSumOfWeights());
    h2->Scale(1.0/h2->GetSumOfWeights());
  }
  h1->Draw();
  h1->GetXaxis()->SetTitle(xname);
  h1->GetYaxis()->SetTitle(yname);
  h2->Draw("SAME");
  can->Print(Form("%s.eps", sname.Data()));
  can->Print(Form("%s.gif+5", sname.Data()));
  delete can;
}

void PlotUtil::plotTH2D(TH2D *h2, TString xname, TString yname, TString zname,
			TString sname, double x1=0, double x2=0, double y1=0,
			double y2=0, double z1=0, double z2=0) {
  TCanvas *can = new TCanvas("can","can",800,800);
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
  //can->Print(Form("%s.eps", sname.Data()));
  can->Print(Form("%s.root", sname.Data()));
  can->Print(Form("%s.gif+5", sname.Data()));
  delete can;
}


void PlotUtil::plotTH2D(TH2D *h2, TString xname, TString yname, TString zname,
			TString sname) {
  plotTH2D(h2, xname, yname, zname, sname, 0, 0, 0, 0, 0, 0);
}

void PlotUtil::plotTGraph(TGraph *g, TString xname, TString yname, 
			  TString sname) {
  TCanvas *can = new TCanvas("can","can",800,800);
  can->cd();
  can->Clear();
  g->GetXaxis()->SetTitle(xname);
  g->GetYaxis()->SetTitle(yname);
  g->Draw("ALP");
  can->Print(Form("%s.eps", sname.Data()));
  delete can;
}

void PlotUtil::plotTGraphErrFit(TGraphErrors *g, TF1 *fit, TString xname,
				TString yname, TString sname) {
  TCanvas *can = new TCanvas("can","can",800,800);
  can->cd();
  can->Clear();
  g->GetXaxis()->SetTitle(xname);
  g->GetYaxis()->SetTitle(yname);
  g->Draw("AP");
  fit->Draw("SAME");
  can->Print(Form("%s.eps", sname.Data()));
  delete can;
}
