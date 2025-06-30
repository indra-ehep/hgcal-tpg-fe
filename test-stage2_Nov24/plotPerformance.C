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
  const char* infile = "/home/hep/idas/stage2_emulation_results/Result_iter41/vbfHInv_200PU/stage2SemiEmulator_ntuples_16_merged.root" ;
  TFile *fin = TFile::Open(infile);
  TH1D *hbkg = (TH1D *)fin->Get("hBkgRateSingle_Uncorr_MaxPt");
  
  TGraphErrors *gr = new TGraphErrors(hbkg->GetNbinsX());
  gr->SetTitle("");
  double err;
  double tot = hbkg->IntegralAndError(1,hbkg->GetNbinsX(),err);
  double tot_err = err/tot;
  for(int ibin=1;ibin<hbkg->GetNbinsX();ibin++){
    float pt = hbkg->GetXaxis()->GetBinCenter(ibin);
    double integral = hbkg->IntegralAndError(ibin,hbkg->GetNbinsX(),err);
    double intg_err = err/integral;
    double yval = integral * 40.e3 * 2760 / 3564 / hbkg->GetEntries() ;
    double yval_err = yval*sqrt(intg_err*intg_err + tot_err*tot_err);
    gr->SetPoint(ibin,pt,yval);
    gr->SetPointError(ibin, 0, yval_err);
    //std::cout << "ibin: " << ibin << ", pt: " << pt << ", integral: " << integral << ", yval: " << yval << " +/- " << 100.*yval_err/yval << " %"<< std::endl;
  }
  
  // TCanvas *c1 = new TCanvas("c1","c1");
  // hbkg->Draw();

  gr->SetFillColor(kBlack);
  gr->SetFillStyle(3005);
  TCanvas *c2 = new TCanvas("c2","c2");
  gr->Draw("a4");

  // TLatex *texl = new TLatex(61.73,20258.06,"CMS");
  // texl->SetTextSize(0.035);
  // TLatex *texp = new TLatex(2191.359,20258.06,"Preliminary");
  // texp->SetTextSize(0.025);
  // texp->SetTextFont(52);
  // auto leg0 = new TLegend(0.17,0.80,0.52,0.85);
  // leg0->SetTextSize(0.028);
  // leg0->SetHeader(Form("Module %d with BestChoice",bcmodid));
  // leg0->SetShadowColor(kWhite);
  // h2BC->SetTitle("");
  // auto c1 = new TCanvas("c1","c1",900,900);
  // SetCanvasStyle(c1);
  // Set2DHistStyle(h2BC);
  // TPaletteAxis *pal0 =  SetPalette(((TH1*)h2BC));
  // h2BC->GetListOfFunctions()->Add(pal0,"br");
  // h2BC->Draw("COLZ");
  // leg0->Draw();
  // texl->Draw("same");
  // texp->Draw("same");
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
