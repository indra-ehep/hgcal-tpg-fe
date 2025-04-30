/**********************************************************************
 Created on : 29/04/2025
 Purpose    : Plots 2D histograms with circle on top of it
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/

int plotDxozDyoz(int index = 5)
{
  std::string infile = "stage2SemiEmulator_flatpt_10K_GenTCClus_Thresh.root";
  //std::string infile = "stage2SemiEmulator_electron-flatpt_10K_GenTCClus_Thresh.root";
  TFile *fin = TFile::Open(infile.c_str());
  TH2D *h2GenClus = (TH2D *)fin->Get(Form("deltaGenTCXYoZ_%d",index));
  int rebin = 8;
  h2GenClus->RebinX(rebin);
  h2GenClus->RebinY(rebin);

  h2GenClus->GetXaxis()->SetTitle("#Deltax/z (Cluster-Gen)");
  h2GenClus->GetYaxis()->SetTitle("#Deltay/z (Cluster-Gen)");
  h2GenClus->GetXaxis()->SetTitleOffset(1.2);
  h2GenClus->GetYaxis()->SetTitleOffset(1.4);
  h2GenClus->GetXaxis()->SetRangeUser(-0.03,0.03);
  h2GenClus->GetYaxis()->SetRangeUser(-0.03,0.03);

  float radius1 =  0.0139;
  TEllipse *f1 = new TEllipse(0.0, 0.0, radius1, radius1);
  f1->SetLineColor(kRed);
  f1->SetLineWidth(3);
  f1->SetFillStyle(0);

  TEllipse *f2 = new TEllipse(-radius1/2., 0.0, radius1, radius1);
  f2->SetLineColor(kBlue);
  f2->SetLineWidth(3);
  f2->SetFillStyle(0);

  gStyle->SetOptStat(1111111);
  TCanvas *c1 = new TCanvas ("c1","c1",900,900);
  c1->SetTickx();
  c1->SetTicky();
  h2GenClus->Draw("colz");
  f1->Draw("sames");
  f2->Draw("sames");
  c1->Update();
  c1->SaveAs(Form("0430_Pion-Flat-Thresh_DeltaXoZDeltaYoZ_TC-genJet_Sect-%d_TwoCircles.pdf",index));
  
  return true;
}
