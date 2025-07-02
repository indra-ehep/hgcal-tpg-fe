/**********************************************************************
 Created on : 30/06/2025
 Purpose    : plots the bkg rate
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/

int plotPerformance()
{
  void SetCanvasStyle(TCanvas *canvas);
  //TPaletteAxis * SetPalette(TH1* hist1D);
  void Set2DHistStyle(TH2D* hist2D);
  
  //const char* infile = "stage2SemiEmulator_ntuples_16_0.root" ;
  const char* infile = "/Data/root_files/stage2_emulation_results/Result_iter41/vbfHInv_200PU/stage2SemiEmulator_ntuples_16_merged.root" ;
  //const char* infile = "/Data/root_files/stage2_emulation_results/Result_iter41/vbfHInv_200PU/stage2SemiEmulator_ntuples_30_merged.root" ;
  //const char* infile = "/Data/root_files/stage2_emulation_results/Result_iter41/vbfHInv_200PU/stage2SemiEmulator_ntuples_45_merged.root" ;
  TFile *fin = TFile::Open(infile);
  // TH1D *hbkg_uncorr = (TH1D *)fin->Get("hBkgRateSingle_Uncorr_MaxPt");
  // TH1D *hbkg1D = (TH1D *)fin->Get("hBkgRateSingle1D_MaxPt");
  // TH1D *hbkg2D = (TH1D *)fin->Get("hBkgRateSingle2D_MaxPt");
  TH1D *hbkg_uncorr = (TH1D *)fin->Get("hBkgRateSingle_Uncorr");
  TH1D *hbkg1D = (TH1D *)fin->Get("hBkgRateSingle1D");
  TH1D *hbkg2D = (TH1D *)fin->Get("hBkgRateSingle2D");

  int rebin = 8;
  hbkg_uncorr->Rebin(rebin);
  hbkg1D->Rebin(rebin);
  hbkg2D->Rebin(rebin);
  
  TH1D *hSelEvents = (TH1D *)fin->Get("hSelEvents");
  
  TGraphErrors *gr_uncorr = new TGraphErrors(hbkg_uncorr->GetNbinsX());
  TGraphErrors *gr1D = new TGraphErrors(hbkg1D->GetNbinsX());
  TGraphErrors *gr2D = new TGraphErrors(hbkg2D->GetNbinsX());
  gr_uncorr->SetTitle("");
  gr1D->SetTitle("");
  gr2D->SetTitle("");
  double err;
  
  double tot = hSelEvents->GetBinContent(1);
  double tot_err = sqrt(tot)/tot;
  
  std::cout << "Tot: " << tot << ", tot_err: " << tot_err << std::endl;
  
  // for(int ibin=1;ibin<hbkg_uncorr->GetNbinsX();ibin++){
  //   float pt = hbkg_uncorr->GetXaxis()->GetBinCenter(ibin);
  //   double integral_uncorr = hbkg_uncorr->IntegralAndError(ibin,hbkg_uncorr->GetNbinsX(),err);
  //   double intg_err = err/integral_uncorr;
  //   double yval_uncorr = integral_uncorr * 40.e3 * 2760 / 3564 / tot ;
  //   double yval_err_uncorr = yval_uncorr*sqrt(intg_err*intg_err + tot_err*tot_err);
  //   gr_uncorr->SetPoint(ibin,pt,yval_uncorr);
  //   gr_uncorr->SetPointError(ibin, 0, yval_err_uncorr);
  //   //std::cout << "ibin: " << ibin << ", pt: " << pt << ", integral: " << integral << ", yval: " << yval << " +/- " << 100.*yval_err/yval << " %"<< std::endl;
  // }
  
  // for(int ibin=1;ibin<hbkg1D->GetNbinsX();ibin++){
  //   float pt = hbkg1D->GetXaxis()->GetBinCenter(ibin);
  //   double integral1D = hbkg1D->IntegralAndError(ibin,hbkg1D->GetNbinsX(),err);
  //   double intg_err = err/integral1D;
  //   double yval1D = integral1D * 40.e3 * 2760 / 3564 / tot ;
  //   double yval_err1D = yval1D*sqrt(intg_err*intg_err + tot_err*tot_err);
  //   gr1D->SetPoint(ibin,pt,yval1D);
  //   gr1D->SetPointError(ibin, 0, yval_err1D);
  //   //std::cout << "ibin: " << ibin << ", pt: " << pt << ", integral: " << integral << ", yval: " << yval << " +/- " << 100.*yval_err/yval << " %"<< std::endl;
  // }
  
  // for(int ibin=1;ibin<hbkg2D->GetNbinsX();ibin++){
  //   float pt = hbkg2D->GetXaxis()->GetBinCenter(ibin);
  //   double integral2D = hbkg2D->IntegralAndError(ibin,hbkg2D->GetNbinsX(),err);
  //   double intg_err = err/integral2D;
  //   double yval2D = integral2D * 40.e3 * 2760 / 3564 / tot ;
  //   double yval_err2D = yval2D*sqrt(intg_err*intg_err + tot_err*tot_err);
  //   gr2D->SetPoint(ibin,pt,yval2D);
  //   gr2D->SetPointError(ibin, 0, yval_err2D);
  //   //std::cout << "ibin: " << ibin << ", pt: " << pt << ", integral: " << integral << ", yval: " << yval << " +/- " << 100.*yval_err/yval << " %"<< std::endl;
  // }

  for(int ibin=1;ibin<hbkg_uncorr->GetNbinsX();ibin++){
    float pt = hbkg_uncorr->GetXaxis()->GetBinCenter(ibin);
    double integral_uncorr = hbkg_uncorr->GetBinContent(ibin);
    double intg_err = sqrt(integral_uncorr)/integral_uncorr;
    double yval_uncorr = integral_uncorr * 40.e3 * 2760 / 3564 / tot / float(rebin);
    double yval_err_uncorr = yval_uncorr*sqrt(intg_err*intg_err + tot_err*tot_err);
    gr_uncorr->SetPoint(ibin,pt,yval_uncorr);
    gr_uncorr->SetPointError(ibin, 0, yval_err_uncorr);
    //std::cout << "ibin: " << ibin << ", pt: " << pt << ", integral: " << integral << ", yval: " << yval << " +/- " << 100.*yval_err/yval << " %"<< std::endl;
  }

  for(int ibin=1;ibin<hbkg1D->GetNbinsX();ibin++){
    float pt = hbkg1D->GetXaxis()->GetBinCenter(ibin);
    double integral1D = hbkg1D->GetBinContent(ibin);
    double intg_err = sqrt(integral1D)/integral1D;
    double yval1D = integral1D * 40.e3 * 2760 / 3564 / tot / float(rebin);
    double yval_err1D = yval1D*sqrt(intg_err*intg_err + tot_err*tot_err);
    gr1D->SetPoint(ibin,pt,yval1D);
    gr1D->SetPointError(ibin, 0, yval_err1D);
    //std::cout << "ibin: " << ibin << ", pt: " << pt << ", integral: " << integral << ", yval: " << yval << " +/- " << 100.*yval_err/yval << " %"<< std::endl;
  }
  
  for(int ibin=1;ibin<hbkg2D->GetNbinsX();ibin++){
    float pt = hbkg2D->GetXaxis()->GetBinCenter(ibin);
    double integral2D = hbkg2D->GetBinContent(ibin);
    double intg_err = sqrt(integral2D)/integral2D;
    double yval2D = integral2D * 40.e3 * 2760 / 3564 / tot / float(rebin);
    double yval_err2D = yval2D*sqrt(intg_err*intg_err + tot_err*tot_err);
    gr2D->SetPoint(ibin,pt,yval2D);
    gr2D->SetPointError(ibin, 0, yval_err2D);
    //std::cout << "ibin: " << ibin << ", pt: " << pt << ", integral: " << integral << ", yval: " << yval << " +/- " << 100.*yval_err/yval << " %"<< std::endl;
  }

  
  gr_uncorr->SetFillColor(kBlack);
  gr1D->SetFillColor(kBlue);
  gr2D->SetFillColor(kRed);

  gr_uncorr->SetLineColor(kBlack);
  gr1D->SetLineColor(kBlue);
  gr2D->SetLineColor(kRed);

  gr_uncorr->SetLineWidth(3);
  gr1D->SetLineWidth(3);
  gr2D->SetLineWidth(3);

  // TCanvas *c1 = new TCanvas("c1","c1");
  // hbkg->Draw();


  TLatex *texl = new TLatex(3.369606,58503.36,"CMS");
  texl->SetTextSize(0.035);
  TLatex *texp = new TLatex(26.9,59533.27,"Preliminary");
  texp->SetTextSize(0.025);
  texp->SetTextFont(52);
  auto leg0 = new TLegend(0.19,0.25,0.58,0.42);
  leg0->SetTextSize(0.028);
  leg0->SetHeader(Form("PU 200 : a = 0.016"));
  leg0->AddEntry(gr_uncorr,"jet without correction","lp");
  leg0->AddEntry(gr1D,"jet with 1D correctionn","lp");
  leg0->AddEntry(gr2D,"jet with 2D correctionn","lp");
  leg0->SetShadowColor(kWhite);
  
  gr_uncorr->SetMinimum(1);
  gr_uncorr->SetMaximum(5.e4);
  gr_uncorr->GetYaxis()->SetTitle("Single jet rate [kHz]");
  gr_uncorr->GetXaxis()->SetTitle("Threshold [GeV]");
  gr_uncorr->GetXaxis()->SetRangeUser(0,200);
  auto c1 = new TCanvas("c1","c1",900,900);
  c1->SetLogy();
  c1->SetGridx();
  c1->SetGridy();
  SetCanvasStyle(c1);
  gr_uncorr->Draw("a3c");
  gr1D->Draw("3c same");
  gr2D->Draw("3c same");
  
  leg0->Draw();
  texl->Draw("same");
  texp->Draw("same");
  // c1->SaveAs(Form("BC_noTcTp12_Module_%d.png",bcmodid));

  return true;
}

void Set2DHistStyle(TH2D* hist2D){

  int rebin = 2;
  hist2D->RebinX(rebin);
  hist2D->RebinY(rebin);
  
  //hist2D->GetXaxis()->SetRange(1,50);
  hist2D->GetXaxis()->SetNdivisions(509);
  hist2D->GetXaxis()->SetLabelFont(42);
  hist2D->GetXaxis()->SetTitleSize(0.03);
  hist2D->GetXaxis()->SetTitleOffset(1.1);
  hist2D->GetXaxis()->SetTitleFont(42);
  //hist2D->GetYaxis()->SetRange(1,200);
  hist2D->GetYaxis()->SetNdivisions(509);
  hist2D->GetYaxis()->SetLabelFont(42);
  hist2D->GetYaxis()->SetTitleSize(0.03);
  hist2D->GetYaxis()->SetTitleFont(42);
  hist2D->GetZaxis()->SetLabelFont(42);
  hist2D->GetZaxis()->SetTitleOffset(1);
  hist2D->GetZaxis()->SetTitleFont(42);

}

void SetCanvasStyle(TCanvas *canvas){
  canvas->Range(-3858.024,-4870.967,23858.02,23387.1);
  canvas->SetFillColor(0);
  canvas->SetBorderMode(0);
  canvas->SetBorderSize(2);
  canvas->SetTickx(1);
  canvas->SetTicky(1);
  canvas->SetLeftMargin(0.1391982);
  canvas->SetRightMargin(0.1391982);
  canvas->SetTopMargin(0.119863);
  canvas->SetBottomMargin(0.1723744);
  canvas->SetFrameBorderMode(0);
  canvas->SetFrameBorderMode(0);

}

// TPaletteAxis * SetPalette(TH1* hist1D){
//   TPaletteAxis *palette = new TPaletteAxis(20138.58,0,21385.8,20000,hist1D);
//    palette->SetNdivisions(510);
//    palette->SetAxisColor(1);
//    palette->SetLabelColor(1);
//    palette->SetLabelFont(42);
//    palette->SetLabelOffset(0.005);
//    palette->SetLabelSize(0.035);
//    palette->SetMaxDigits(0);
//    palette->SetTickLength(0.03);
//    palette->SetTitleOffset(1);
//    palette->SetTitleSize(0.035);
//    palette->SetTitleColor(1);
//    palette->SetTitleFont(42);
//    palette->SetTitle("");

//    Int_t ci;      // for color index setting
//    TColor *color; // for color definition with alpha
//    ci = TColor::GetColor("#ff0000");
//    palette->SetFillColor(ci);
//    palette->SetFillStyle(1001);

//    return palette;
// }
