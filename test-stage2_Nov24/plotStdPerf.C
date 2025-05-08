/**********************************************************************
 Created on : 05/05/2025
 Purpose    : Plots the standard performance distributions resolution, 
              efficiency and others
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/

int gindex = 1;
int plotStdPerf(int index = 5)
{
  
  //////////////////// input firectory ////////////////////////
  std::string inpath1 = "/Data/root_files/stage2_emulation_results/Result_iter3/SinglePi_Ideal_PU0/";
  std::string inpath2 = "/Data/root_files/stage2_emulation_results/Result_iter3/SingleEle_Ideal_PU0/";
  /////////////////////////////////////////////////////////
  
  //////////////////// function declaration ///////////////
  void PlotResolution(TFile *fin, const char* histName, const char* plotTitle, const char* xaxisTitle, TCanvas*& cPtReso);
  void PlotRozGenTC(TFile *fin, const char* histName, const char* plotTitle, const char* xaxisTitle, TCanvas*& cPtReso);
  void PlotRozGenTCCumul(TFile *fin1, TFile *fin2, const char* histName, const char* plotTitle, const char* xaxisTitle, TCanvas*& cPtReso);
  void PlotRozClusGen(TFile *fin1, TFile *fin2, TFile *fin3, const char* histName, const char* plotTitle, const char* xaxisTitle, TCanvas*& c1, TCanvas*& c2);
  void PlotTrigEff(TFile *fin[], bool ishist, const char* histName, const char* plotTitle, const char* xaxisTitle, TCanvas*& cPtReso);
  void PlotDR(TFile *fin1, const char* h2histName, const char* profName, const char* plotTitle, const char* xaxisTitle, TCanvas*& cPtReso);
  void PlotNofClus(TFile *fin1, TFile *fin2, TFile *fin3, const char* histName, const char* plotTitle, const char* xaxisTitle, TCanvas*& c1);
  ////////////////////////////////////////////////////////
  
  //////////////////// global ROOT settings //////////////
  gStyle->SetOptFit(0010);
  gStyle->SetOptStat(1);
  gStyle->SetOptFit(0);
  gStyle->SetOptStat(0);
  ////////////////////////////////////////////////////////
  
  // /////////////////////Cluster size: TC level //////////////
  // std::string infile1 = inpath1 + "/" + "stage2SemiEmulator_pt100GeV_10K_16_0.root";
  // std::string infile2 = inpath2 + "/" + "stage2SemiEmulator_pt100GeV_10K_16_0.root";
  // TFile *fin1 = TFile::Open(infile1.c_str());
  // TFile *fin2 = TFile::Open(infile2.c_str());
  // TCanvas *rozGenTCPi, *rozGenTCEle, *rozGenTCCumul;
  // PlotRozGenTC(fin1,"tcRoz","E_{T} weighted #Delta(R/Z) of #pi^{+}#pi^{-} (TC-genJet)", "#Delta(R/Z)",rozGenTCPi);
  // PlotRozGenTC(fin2,"tcRoz","E_{T} weighted #Delta(R/Z) of #it{e}^{+}#it{e}^{-} (TC-genJet)", "#Delta(R/Z)",rozGenTCEle);
  // PlotRozGenTCCumul(fin1,fin2,"tcRoz_cumul","E_{T} weighted #Delta(R/Z) of #pi^{+}#pi^{-} and #it{e}^{+}#it{e}^{-} (TC-genJet)", "#Delta(R/Z)",rozGenTCEle);
  // ///////////////////////////////////////////////
  
  // /////////////////////Cluster size: cluster level //////////////
  // inpath1 = "/Data/root_files/stage2_emulation_results/Result_iter14/SingleEle_Ideal_PU0/";
  // inpath2 = "/Data/root_files/stage2_emulation_results/Result_iter14/doubleElectron_PU200/";
  
  // // std::string infile1 = inpath1 + "/" + "stage2SemiEmulator_flatpt_10K_16_0.root";
  // // std::string infile2 = inpath1 + "/" + "stage2SemiEmulator_flatpt_10K_30_0.root";
  // // std::string infile3 = inpath1 + "/" + "stage2SemiEmulator_flatpt_10K_45_0.root";
  // std::string infile1 = inpath2 + "/" + "stage2SemiEmulator_ntuples_16_merged.root";
  // std::string infile2 = inpath2 + "/" + "stage2SemiEmulator_ntuples_30_merged.root";
  // std::string infile3 = inpath2 + "/" + "stage2SemiEmulator_ntuples_45_merged.root";
  // TFile *fin1 = TFile::Open(infile1.c_str());
  // TFile *fin2 = TFile::Open(infile2.c_str());
  // TFile *fin3 = TFile::Open(infile3.c_str());
  // TCanvas *rozGenClus, *rozGenClusCumul, *rozGenClusClst, *rozGenClusClstCumul;
  // //PlotRozClusGen(fin1,fin2,fin3,"hclusRoz","E_{T} weighted #Delta(R/Z) of Ideal #it{e}^{+}#it{e}^{-} (Clus-genJet) for cluster p_{T}>3 GeV", "#Delta(R/Z)",rozGenClus, rozGenClusCumul);
  // //PlotRozClusGen(fin1,fin2,fin3,"hclusRozClst","#Delta(R/Z) for the closest clusters to genJet of Ideal #it{e}^{+}#it{e}^{-}", "#Delta(R/Z)",rozGenClusClst,rozGenClusClstCumul);
  // //PlotRozClusGen(fin1,fin2,fin3,"hclusRoz","E_{T} weighted #Delta(R/Z) of  #it{e}^{+}#it{e}^{-} PU200 (Clus-genJet) for cluster p_{T}>3 GeV", "#Delta(R/Z)",rozGenClus, rozGenClusCumul);
  // PlotRozClusGen(fin1,fin2,fin3,"hclusRozClst","#Delta(R/Z) for the closest clusters to genJet of #it{e}^{+}#it{e}^{-} PU200", "#Delta(R/Z)",rozGenClusClst,rozGenClusClstCumul);
  // ///////////////////////////////////////////////
  
  /////////////////////Resolution plots //////////////
  // std::string infile1 = inpath1 + "/" + "stage2SemiEmulator_pt100GeV_10K_45_0.root";
  // std::string infile2 = inpath2 + "/" + "stage2SemiEmulator_pt100GeV_10K_45_0.root";
  // TFile *fin1 = TFile::Open(infile1.c_str());
  // TFile *fin2 = TFile::Open(infile2.c_str());
  // TCanvas *cPtResoPi,*cPtResoPiGenTC,*cPtResoPiTCclus;
  // TCanvas *cPtResoEle,*cPtResoEleGenTC,*cPtResoEleTCclus;
  // PlotResolution(fin1,"hPtReso","E_{T} resolution of #pi^{+}#pi^{-}", "(clus-genJet) [GeV]",cPtResoPi);
  // PlotResolution(fin1,"hPtResoGenTC","E_{T} resolution of #pi^{+}#pi^{-}", "(TC-genJet) [GeV]",cPtResoPiGenTC);
  // PlotResolution(fin1,"hPtResoTCClus","E_{T} resolution of #pi^{+}#pi^{-}", "(clus-TC) [GeV]",cPtResoPiTCclus);  
  // PlotResolution(fin2,"hPtReso","E_{T} resolution of #it{e}^{+}#it{e}^{-}", "(clus-genJet) [GeV]",cPtResoEle);
  // PlotResolution(fin2,"hPtResoGenTC","E_{T} resolution of #it{e}^{+}#it{e}^{-}", "(TC-genJet) [GeV]",cPtResoPiGenTC);
  // PlotResolution(fin2,"hPtResoTCClus","E_{T} resolution of #it{e}^{+}#it{e}^{-}", "(clus-TC) [GeV]",cPtResoPiTCclus);
  //////////////////////////////////////////////////////
  
  // ///////////////////Efficiency plots ////////////////////
  // std::string aval[7] = {"16", "30", "45", "60", "75", "90", "105"};
  // TFile *fin1[7],*fin2[7];
  // for(int idroz=0;idroz<7;idroz++){
  //   std::cout << "idroz : " << idroz << ", aval: "<< aval[idroz]<< std::endl;
  //   std::string infile1 = inpath1 + "/" + "stage2SemiEmulator_flatpt_10K_" + aval[idroz] + "_0.root";
  //   std::string infile2 = inpath2 + "/" + "stage2SemiEmulator_flatpt_10K_" + aval[idroz] + "_0.root";
  //   fin1[idroz] = TFile::Open(infile1.c_str());
  //   fin2[idroz] = TFile::Open(infile2.c_str());
  // }
  // TCanvas *cTrigEffPiIdeal;
  // //PlotTrigEff(fin1, "effTrigGen", "Trigger Efficiencies", "p_{T} (GeV}", cTrigEffPiIdeal);
  // PlotTrigEff(fin1, "hTrigEff", "Trigger Efficiencies of #pi^{+}#pi^{-}", "p_{T} (GeV}", cTrigEffPiIdeal);
  // PlotTrigEff(fin2, "hTrigEff", "Trigger Efficiencies of #it{e}^{+}#it{e}^{-}", "p_{T} (GeV}", cTrigEffPiIdeal);
  // ////////////////////////////////////////////////////////
  
  // ///////////////////Efficiency plots PU200 ////////////////////
  // // inpath1 = "/Data/root_files/stage2_emulation_results/Result_iter6/SinglePi_realistic_PU200/";
  // // inpath2 = "/Data/root_files/stage2_emulation_results/Result_iter6/SingleEle_realistic_PU200/";
  // //inpath1 = "/Data/root_files/stage2_emulation_results/Result_iter11/singlePion_PU0/";
  // //inpath1 = "/Data/root_files/stage2_emulation_results/Result_iter11/vbfHInv_200PU/";
  // //inpath2 = "/Data/root_files/stage2_emulation_results/Result_iter10/doubleElectron_PU200/";
  // //inpath2 = "/Data/root_files/stage2_emulation_results/Result_iter11/singlePion_PU200/";
  // //inpath2 = "/Data/root_files/stage2_emulation_results/Result_iter12/vbfHInv_200PU/";
  // inpath1 = "/Data/root_files/stage2_emulation_results/Result_iter14/doubleElectron_PU200/";
  // inpath2 = "/Data/root_files/stage2_emulation_results/Result_iter15/doubleElectron_PU200/";
  
  // std::string aval[7] = {"16", "30", "45", "60", "75", "90", "105"};
  // TFile *fin1[7],*fin2[7];
  // for(int idroz=0;idroz<3;idroz++){
  //   std::cout << "idroz : " << idroz << ", aval: "<< aval[idroz]<< std::endl;
  //   std::string infile1 = inpath1 + "/" + "stage2SemiEmulator_ntuples_" + aval[idroz] + "_merged.root";
  //   std::string infile2 = inpath2 + "/" + "stage2SemiEmulator_ntuples_" + aval[idroz] + "_merged.root";
  //   fin1[idroz] = TFile::Open(infile1.c_str());
  //   fin2[idroz] = TFile::Open(infile2.c_str());
  // }
  // TCanvas *cTrigEffPiIdeal,*cTrigEffEleIdeal;
  
  // //PlotTrigEff(fin1, 0, "effTrigGen", "Trigger Efficiencies of #pi^{+}#pi^{-} PU0 (100 GeV)", "p_{T} (GeV}", cTrigEffPiIdeal);
  // //PlotTrigEff(fin2, 0, "effTrigGen", "Trigger Efficiencies of #pi^{+}#pi^{-} PU200 (100 GeV)", "p_{T} (GeV}", cTrigEffPiIdeal);
  // // PlotTrigEff(fin1, 0, "effTrigGenPU", "Trigger Efficiencies of VBFtoInv PU200 (100 GeV)", "p_{T} (GeV}", cTrigEffPiIdeal);
  // // PlotTrigEff(fin2, 0, "effTrigGenPU", "Trigger Efficiencies of VBFtoInv PU200 (150 GeV)", "p_{T} (GeV}", cTrigEffPiIdeal);
  // //PlotTrigEff(fin2, 0, "effTrigGen", "Trigger Efficiencies of #it{e}^{+}#it{e}^{-} PU0", "p_{T} (GeV}", cTrigEffEleIdeal);
  // //PlotTrigEff(fin1, 1, "hTrigEff", "Trigger Efficiencies of #pi^{+}#pi^{-}", "p_{T} (GeV}", cTrigEffPiIdeal);
  // //PlotTrigEff(fin2, 1, "hTrigEff", "Trigger Efficiencies of #it{e}^{+}#it{e}^{-}", "p_{T} (GeV}", cTrigEffEleIdeal);
  // //PlotTrigEff(fin1, 0, "effTrigGen", "Trigger Efficiencies of #it{e}^{+}#it{e}^{-} PU200 (#Delta(R/Z)<0.02)", "p_{T} (GeV}", cTrigEffPiIdeal);
  // PlotTrigEff(fin2, 0, "effTrigGen", "Trigger Efficiencies of #it{e}^{+}#it{e}^{-} PU200 (#Delta(R/Z)<0.05)", "p_{T} (GeV}", cTrigEffPiIdeal);
  // ////////////////////////////////////////////////////////
  
  // /////////////////////Jet cone DeltaR //////////////
  // inpath1 = "/Data/root_files/stage2_emulation_results/Result_iter7/SingleEle_Ideal_PU0";
  // inpath2 = "/Data/root_files/stage2_emulation_results/Result_iter7/SinglePi_Ideal_PU0";
  // std::string infile1 = inpath1 + "/" + "stage2SemiEmulator_flatpt_10K_16_0.root";
  // std::string infile2 = inpath2 + "/" + "stage2SemiEmulator_flatpt_10K_16_0.root";
  // // inpath1 = "/Data/root_files/stage2_emulation_results/Result_iter10/doublePhoton_PU0";
  // // inpath2 = "/Data/root_files/stage2_emulation_results/Result_iter7/SinglePi_Ideal_PU0";
  // // std::string infile1 = inpath1 + "/" + "stage2SemiEmulator_ntuples_16_merged.root";
  // // std::string infile2 = inpath2 + "/" + "stage2SemiEmulator_flatpt_10K_16_0.root";
  // TFile *fin1 = TFile::Open(infile1.c_str());
  // TFile *fin2 = TFile::Open(infile2.c_str());
  // TCanvas *rozGenTCPi;
  // PlotDR(fin1,"hDeltaRGCPt","hDeltaRGCPtProf","#DeltaR (cluster-genJet) distribution of #it{e}^{+}#it{e}^{-} jets", "genJet p_{T} (GeV)",rozGenTCPi);
  // PlotDR(fin2,"hDeltaRGCPt","hDeltaRGCPtProf","#DeltaR (cluster-genJet) distribution of #pi^{+}#pi^{-} jets", "genJet p_{T} (GeV)",rozGenTCPi);
  // ///////////////////////////////////////////////

  /////////////////////Nof Clusters //////////////
  inpath1 = "/Data/root_files/stage2_emulation_results/Result_iter14/SingleEle_Ideal_PU0/";
  inpath2 = "/Data/root_files/stage2_emulation_results/Result_iter14/doubleElectron_PU200/";
  
  // std::string infile1 = inpath1 + "/" + "stage2SemiEmulator_flatpt_10K_16_0.root";
  // std::string infile2 = inpath1 + "/" + "stage2SemiEmulator_flatpt_10K_30_0.root";
  // std::string infile3 = inpath1 + "/" + "stage2SemiEmulator_flatpt_10K_45_0.root";
  std::string infile1 = inpath2 + "/" + "stage2SemiEmulator_ntuples_16_merged.root";
  std::string infile2 = inpath2 + "/" + "stage2SemiEmulator_ntuples_30_merged.root";
  std::string infile3 = inpath2 + "/" + "stage2SemiEmulator_ntuples_45_merged.root";
  TFile *fin1 = TFile::Open(infile1.c_str());
  TFile *fin2 = TFile::Open(infile2.c_str());
  TFile *fin3 = TFile::Open(infile3.c_str());
  TCanvas *cNofClus;
  PlotNofClus(fin1,fin2,fin3,"hNClus1GeV","Nof clusters #it{e}^{+}#it{e}^{-} PU200 for p_{T}>1", "Number of clusters",cNofClus);
  //PlotNofClus(fin1,fin2,fin3,"hNClus3GeV","Nof clusters #it{e}^{+}#it{e}^{-} PU200 for p_{T}>3", "Number of clusters",cNofClus);
  ///////////////////////////////////////////////

  
  return true;
}

Color_t GetColor(int iset){
  
  
  int maxindex = 12;
  iset=iset%maxindex;
  
  Color_t color;
  switch(iset){

  case 0:  
    color = kRed;
    break;

  case 1:  
    color = kMagenta;
    break;

  case 2:  
    color = kBlue;
    break;

  case 3:  
    color =  kGray+2;
    break;

  case 4:  
    color = kAzure;
    break;

  case 5:  
    color = kRed;
    break;

  case 6:  
    color =  kYellow+1;
    break;

  case 7:  
    color =  kSpring;
    break;

  case 8:  
    color =  kViolet;
    break;

  case 9:  
    color =  kCyan;
    break;

  case 10:  
    color =  kOrange;
    break;
    
  case 11:  
    color =  kTeal;
    break;
    
  case 12:  
    color =  kGreen+1;    
    break;
    
  };
  
  return color;
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

void PlotResolution(TFile *fin, const char* histName, const char* plotTitle, const char* xaxisTitle, TCanvas*& c1){
  
  TH1D *h1PtReso = (TH1D *)fin->Get(histName);
  h1PtReso->SetTitle(plotTitle);
  h1PtReso->GetXaxis()->SetTitle(xaxisTitle);
  h1PtReso->GetYaxis()->SetTitle("Entries");
  h1PtReso->GetXaxis()->SetTitleOffset(1.2);
  h1PtReso->GetYaxis()->SetTitleOffset(1.6);

  auto f1 = new TF1(Form("func%d",gindex),"gaus",-100,100);
  f1->SetNpx(1000);
  c1 = new TCanvas(Form("c%d",gindex),Form("c%d",gindex),900,900); gindex++;
  SetCanvasStyle(c1);
  h1PtReso->Fit(f1,"Q");
  h1PtReso->Draw();
  c1->Update();
  
}

void PlotRozGenTC(TFile *fin, const char* histName, const char* plotTitle, const char* xaxisTitle, TCanvas*& c1){
  
  TH1D *h1DRoz = (TH1D *)fin->Get(histName);
  h1DRoz->SetTitle(plotTitle);
  h1DRoz->GetXaxis()->SetTitle(xaxisTitle);
  h1DRoz->GetYaxis()->SetTitle("Entries");
  h1DRoz->GetXaxis()->SetTitleOffset(1.2);
  h1DRoz->GetYaxis()->SetTitleOffset(1.6);
  h1DRoz->GetXaxis()->SetRangeUser(0,0.1);
  
  c1 = new TCanvas(Form("c%d",gindex),Form("c%d",gindex),900,900); gindex++;
  SetCanvasStyle(c1);
  c1->SetLogy();
  h1DRoz->Draw("hist");
  c1->Update();
  
}

void PlotRozGenTCCumul(TFile *fin1, TFile *fin2, const char* histName, const char* plotTitle, const char* xaxisTitle, TCanvas*& c1){
  
  TH1D *h1DRozCumul1 = (TH1D *)fin1->Get(histName);
  TH1D *h1DRozCumul2 = (TH1D *)fin2->Get(histName);
  h1DRozCumul1->SetTitle(plotTitle);
  h1DRozCumul1->GetXaxis()->SetTitle(xaxisTitle);
  h1DRozCumul1->GetYaxis()->SetTitle("Fraction of total energy");
  h1DRozCumul1->GetXaxis()->SetTitleOffset(1.2);
  h1DRozCumul1->GetYaxis()->SetTitleOffset(1.6);
  h1DRozCumul1->GetXaxis()->SetRangeUser(0,0.1);

  h1DRozCumul1->SetLineColor(kRed);
  h1DRozCumul1->SetLineWidth(2.0);
  
  h1DRozCumul2->SetLineColor(kBlue);
  h1DRozCumul2->SetLineWidth(2.0);

  auto legend = new TLegend(0.31,0.75,0.85,0.86);
  legend->SetTextSize(0.028);
  legend->SetHeader("Energy fraction as a function of #Delta(R/Z)");
  legend->AddEntry(h1DRozCumul1,"#pi^{+}#pi^{-}","lp");
  legend->AddEntry(h1DRozCumul2,"#it{e}^{+}#it{e}^{-}","lp");
  legend->SetShadowColor(kWhite);

  c1 = new TCanvas(Form("c%d",gindex),Form("c%d",gindex),900,900); gindex++;
  SetCanvasStyle(c1);
  h1DRozCumul1->Draw("hist");
  h1DRozCumul2->Draw("hist same");
  legend->Draw();
  c1->Update();
  
}

void PlotRozClusGen(TFile *fin1, TFile *fin2, TFile *fin3, const char* histName, const char* plotTitle, const char* xaxisTitle, TCanvas*& c1, TCanvas*& c2){
  
  TH1D *h1DRoz1 = (TH1D *)fin1->Get(histName);
  TH1D *h1DRoz2 = (TH1D *)fin2->Get(histName);
  TH1D *h1DRoz3 = (TH1D *)fin3->Get(histName);
  
  TH1D *hclusRoz1_cumul = (TH1D *)h1DRoz1->GetCumulative(kTRUE,"_cumul");
  hclusRoz1_cumul->Scale(1./hclusRoz1_cumul->GetBinContent(hclusRoz1_cumul->GetMaximumBin()));
  TH1D *hclusRoz2_cumul = (TH1D *)h1DRoz2->GetCumulative(kTRUE,"_cumul");
  hclusRoz2_cumul->Scale(1./hclusRoz2_cumul->GetBinContent(hclusRoz2_cumul->GetMaximumBin()));
  TH1D *hclusRoz3_cumul = (TH1D *)h1DRoz3->GetCumulative(kTRUE,"_cumul");
  hclusRoz3_cumul->Scale(1./hclusRoz3_cumul->GetBinContent(hclusRoz3_cumul->GetMaximumBin()));
  
  h1DRoz1->SetTitle(plotTitle);
  h1DRoz1->GetXaxis()->SetTitle(xaxisTitle);
  h1DRoz1->GetYaxis()->SetTitle("Entries");
  h1DRoz1->GetXaxis()->SetTitleOffset(1.2);
  h1DRoz1->GetYaxis()->SetTitleOffset(1.6);
  h1DRoz1->GetXaxis()->SetRangeUser(0,0.1);

  hclusRoz1_cumul->SetTitle(plotTitle);
  hclusRoz1_cumul->GetXaxis()->SetTitle(xaxisTitle);
  hclusRoz1_cumul->GetYaxis()->SetTitle("Entries");
  hclusRoz1_cumul->GetXaxis()->SetTitleOffset(1.2);
  hclusRoz1_cumul->GetYaxis()->SetTitleOffset(1.6);
  hclusRoz1_cumul->GetXaxis()->SetRangeUser(0,0.1);

  h1DRoz1->SetLineColor(kRed);
  h1DRoz2->SetLineColor(kBlue);
  h1DRoz3->SetLineColor(kBlack);
  h1DRoz1->SetLineWidth(2.0);
  h1DRoz2->SetLineWidth(2.0);
  h1DRoz3->SetLineWidth(2.0);

  hclusRoz1_cumul->SetLineColor(kRed);
  hclusRoz2_cumul->SetLineColor(kBlue);
  hclusRoz3_cumul->SetLineColor(kBlack);
  hclusRoz1_cumul->SetLineWidth(2.0);
  hclusRoz2_cumul->SetLineWidth(2.0);
  hclusRoz3_cumul->SetLineWidth(2.0);

  auto legend = new TLegend(0.31,0.75,0.85,0.86);
  legend->SetTextSize(0.028);
  legend->SetHeader("#Delta(R/Z) plot");
  legend->AddEntry(h1DRoz1,"a = 0.016","lp");
  legend->AddEntry(h1DRoz2,"a = 0.03","lp");
  legend->AddEntry(h1DRoz3,"a = 0.045","lp");
  legend->SetShadowColor(kWhite);

  auto legend1 = new TLegend(0.31,0.75,0.85,0.86);
  legend1->SetTextSize(0.028);
  legend1->SetHeader("Cumulative #Delta(R/Z) plot");
  legend1->AddEntry(hclusRoz1_cumul,"a = 0.016","lp");
  legend1->AddEntry(hclusRoz2_cumul,"a = 0.03","lp");
  legend1->AddEntry(hclusRoz3_cumul,"a = 0.045","lp");
  legend1->SetShadowColor(kWhite);

  
  c1 = new TCanvas(Form("c%d",gindex),Form("c%d",gindex),900,900); gindex++;
  SetCanvasStyle(c1);
  c1->SetLogy();
  h1DRoz1->Draw("hist");
  h1DRoz2->Draw("hist same");
  h1DRoz3->Draw("hist same");
  legend->Draw();
  c1->Update();
  
  c2 = new TCanvas(Form("c%d",gindex),Form("c%d",gindex),900,900); gindex++;
  SetCanvasStyle(c2);
  hclusRoz1_cumul->Draw("hist");
  hclusRoz2_cumul->Draw("hist same");
  hclusRoz3_cumul->Draw("hist same");
  legend1->Draw();
  c2->Update();
  
}

void PlotTrigEff(TFile *fin[], bool ishist, const char* histName, const char* plotTitle, const char* xaxisTitle, TCanvas*& c1){

  std::string avalf[7] = {"0.016", "0.03", "0.045", "60", "75", "90", "105"};

  //TH1D *hTrigEff[7];
  TEfficiency *hTrigEff[7];
  TH1D *hTrigEffReCalc[7];
  TH1D *hTrigEffPass[7];
  TH1D *hTrigEffTotal[7];
  
  int maxid = 3;
  int rebin = 5;
  for(int idroz=0;idroz<maxid;idroz++){

    //hTrigEff[idroz] = (TH1D *)fin[idroz]->Get(histName) ;
    hTrigEff[idroz] = (TEfficiency *)fin[idroz]->Get(histName) ;
    
    hTrigEff[idroz]->SetLineColor(GetColor(idroz));
    hTrigEff[idroz]->SetLineWidth(2.0);
    // if(ishist){
    //   hTrigEff[idroz]->Rebin(rebin);
    //   hTrigEff[idroz]->Scale(1/(3.*rebin));
    // }
    
    hTrigEffPass[idroz] = (TH1D *) hTrigEff[idroz]->GetCopyPassedHisto();
    hTrigEffTotal[idroz] = (TH1D *) hTrigEff[idroz]->GetCopyTotalHisto();
    hTrigEffReCalc[idroz] = (TH1D *)hTrigEffPass[idroz]->Clone(Form("h1d_eff_%d",idroz));
    hTrigEffReCalc[idroz]->SetTitle(plotTitle);
    hTrigEffReCalc[idroz]->Divide(hTrigEffTotal[idroz]);
    hTrigEffReCalc[idroz]->SetLineColor(GetColor(idroz));
    hTrigEffReCalc[idroz]->SetLineWidth(2.0);
    hTrigEffReCalc[idroz]->GetXaxis()->SetRangeUser(0,100.);
    hTrigEffReCalc[idroz]->GetXaxis()->SetTitle(xaxisTitle);
    hTrigEffReCalc[idroz]->GetYaxis()->SetTitle("Efficiency");
  }
  
  hTrigEff[0]->SetTitle(plotTitle);
  
  
  // if(ishist){
  //   hTrigEff[0]->GetXaxis()->SetTitle(xaxisTitle);
  //   hTrigEff[0]->GetYaxis()->SetTitle("Efficiency");
  //   hTrigEff[0]->GetXaxis()->SetTitleOffset(1.2);
  //   hTrigEff[0]->GetYaxis()->SetTitleOffset(1.6);
  //   // //hTrigEff[0]->GetXaxis()->SetRangeUser(0,40.);
  // }
  
  auto legend = new TLegend(0.47,0.21,0.83,0.45);
  legend->SetTextSize(0.028);
  legend->SetHeader("Triangle side length");
  for(int idroz=0;idroz<maxid;idroz++){
    //legend->AddEntry(hTrigEff[idroz],Form("a = %s",avalf[idroz].c_str()),"lp");
    legend->AddEntry(hTrigEffReCalc[idroz],Form("a = %s",avalf[idroz].c_str()),"lp");
  }
  legend->SetShadowColor(kWhite);

  TF1 *func[7];
  //func[0] = new TF1(Form("func%d",gindex), "0.5 * (1.0 + TMath::Erf((x - [0]) / [1] / TMath::Sqrt2()))", 0., 200.);
  for(int idroz=0;idroz<maxid;idroz++){
    func[idroz] = new TF1(Form("func%d",gindex), "[2] * ([3] + TMath::Erf((x - [0]) / [1] / TMath::Sqrt2()))", 0., hTrigEffReCalc[idroz]->GetXaxis()->GetXmax());
    //func[idroz] = new TF1(Form("func%d",gindex++), "TMath::ATan((x - [0]) / [1]) / TMath::Pi() + 0.5", 0., 400.); // CDF
    //func[idroz] = new TF1(Form("func%d",gindex), "[1]*TMath::TanH(x - [0])", 0., 100.);
    func[idroz]->SetParNames("mean", "sigma","par3","par4");
    func[idroz]->SetParameters(30., 1.5, 1, 1);
    func[idroz]->SetNpx(1000);
    func[idroz]->SetLineColor(GetColor(idroz));
  }
  
  c1 = new TCanvas(Form("c%d",gindex),Form("c%d",gindex),900,900); gindex++;
  SetCanvasStyle(c1);
  // for(int idroz=0;idroz<maxid;idroz++)
  //   hTrigEff[idroz]->Fit(func[idroz]);
  // for(int idroz=0;idroz<maxid;idroz++){
  //   if(idroz==0)
  //     hTrigEff[idroz]->Draw();
  //   else
  //     hTrigEff[idroz]->Draw("same");
  // }
  for(int idroz=0;idroz<maxid;idroz++)
    hTrigEffReCalc[idroz]->Fit(func[idroz]);
  for(int idroz=0;idroz<maxid;idroz++){
    if(idroz==0)
      hTrigEffReCalc[idroz]->Draw();
    else
      hTrigEffReCalc[idroz]->Draw("same");
  }
  legend->Draw();
  c1->Update();

}


void PlotDR(TFile *fin, const char* histName, const char* profName, const char* plotTitle, const char* xaxisTitle, TCanvas*& c1){
  
  TH2D *hist2DR = (TH2D *)fin->Get(histName);
  TProfile *profDR = (TProfile *)fin->Get(profName);
  hist2DR->SetTitle(plotTitle);
  hist2DR->GetXaxis()->SetTitle(xaxisTitle);
  hist2DR->GetYaxis()->SetTitle("#DeltaR");
  hist2DR->GetXaxis()->SetTitleOffset(1.2);
  hist2DR->GetYaxis()->SetTitleOffset(1.6);

  profDR->SetLineColor(kRed);
  profDR->SetLineWidth(3);
  
  //auto f1 = new TF1(Form("func%d",gindex),"[0] + [1]*TMath::Log(x/[2]-10)",20,200);
  auto f1 = new TF1(Form("func%d",gindex++),"[0]/(x^[1]-[2])",20,200);
  auto f2 = new TF1(Form("func%d",gindex++),"0.02+[0]/(x^[1]-[2])",10,200);
  f1->SetNpx(1000);
  f2->SetNpx(1000);
  //f1->SetParameters(0.0406474, -0.00568423, 0.0001);
  //f1->SetParameters(0.00864408,0.116996,1);
  f1->SetParameters(-0.00428022,-0.297381,0.6113);
  f2->SetParameters(-0.00428022,-0.297381,0.6113);

  f2->SetLineColor(kBlack);
  
  c1 = new TCanvas(Form("c%d",gindex),Form("c%d",gindex),900,900); gindex++;
  SetCanvasStyle(c1);
  profDR->Fit(f1);
  f2->SetParameters(f2->GetParameters());
  hist2DR->Draw();
  profDR->Draw("sames");
  f2->Draw("same");
  c1->Update();
  
}

void PlotNofClus(TFile *fin1, TFile *fin2, TFile *fin3, const char* histName, const char* plotTitle, const char* xaxisTitle, TCanvas*& c1){
  
  TH1D *h1NClus1 = (TH1D *)fin1->Get(histName);
  TH1D *h1NClus2 = (TH1D *)fin2->Get(histName);
  TH1D *h1NClus3 = (TH1D *)fin3->Get(histName);

  double maxbin1 = h1NClus1->GetBinContent(h1NClus1->GetMaximumBin());
  double maxbin2 = h1NClus2->GetBinContent(h1NClus2->GetMaximumBin());
  double maxbin3 = h1NClus3->GetBinContent(h1NClus3->GetMaximumBin());

  double maxbin = (maxbin1>maxbin2)?maxbin1:maxbin2;
  maxbin = (maxbin>maxbin3)?maxbin:maxbin3;
  
  h1NClus1->SetTitle(plotTitle);
  h1NClus1->GetXaxis()->SetTitle(xaxisTitle);
  h1NClus1->GetYaxis()->SetTitle("Entries");
  h1NClus1->GetXaxis()->SetTitleOffset(1.2);
  h1NClus1->GetYaxis()->SetTitleOffset(1.6);
  //h1NClus1->GetXaxis()->SetRangeUser(0,0.1);
  h1NClus1->GetYaxis()->SetRangeUser(0,1.2*maxbin);
  
  h1NClus1->SetLineColor(kRed);
  h1NClus2->SetLineColor(kBlue);
  h1NClus3->SetLineColor(kBlack);
  h1NClus1->SetLineWidth(2.0);
  h1NClus2->SetLineWidth(2.0);
  h1NClus3->SetLineWidth(2.0);

  auto legend = new TLegend(0.31,0.75,0.85,0.86);
  legend->SetTextSize(0.028);
  legend->SetHeader("Number of clusters");
  legend->AddEntry(h1NClus1,"a = 0.016","lp");
  legend->AddEntry(h1NClus2,"a = 0.03","lp");
  legend->AddEntry(h1NClus3,"a = 0.045","lp");
  legend->SetShadowColor(kWhite);
  
  c1 = new TCanvas(Form("c%d",gindex),Form("c%d",gindex),900,900); gindex++;
  SetCanvasStyle(c1);
  //c1->SetLogy();
  h1NClus1->Draw("hist");
  h1NClus2->Draw("hist same");
  h1NClus3->Draw("hist same");
  legend->Draw();
  c1->Update();
  
  
}
