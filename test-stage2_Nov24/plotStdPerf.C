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
  void PlotCompareTrigEff(TFile *fin[], const char* histName1, const char* histName2, const char* plotTitle, const char* xaxisTitle, TCanvas*& cPtReso);
  void PlotCompareTrigEffTwo(TFile *fin1[], TFile *fin2[], const char* histName1, const char* histName2, const char* plotTitle, const char* xaxisTitle, TCanvas*& cPtReso);
  void PlotDR(TFile *fin1, const char* h2histName, const char* profName, const char* plotTitle, const char* xaxisTitle, TCanvas*& cPtReso);
  void PlotNofClus(TFile *fin1, TFile *fin2, TFile *fin3, const char* histName, const char* plotTitle, const char* xaxisTitle, TCanvas*& c1);
  ////////////////////////////////////////////////////////
  
  // //////////////////// global ROOT settings //////////////
  //gStyle->SetOptFit(0010);
  // gStyle->SetOptStat(1);
  // gStyle->SetOptFit(0);
  // gStyle->SetOptStat(0);
  // ////////////////////////////////////////////////////////
  
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
  
  ///////////////////Efficiency plots PU200 ////////////////////
  // inpath1 = "/Data/root_files/stage2_emulation_results/Result_iter6/SinglePi_realistic_PU200/";
  // inpath2 = "/Data/root_files/stage2_emulation_results/Result_iter6/SingleEle_realistic_PU200/";
  //inpath1 = "/Data/root_files/stage2_emulation_results/Result_iter11/singlePion_PU0/";
  //inpath1 = "/Data/root_files/stage2_emulation_results/Result_iter11/vbfHInv_200PU/";
  //inpath2 = "/Data/root_files/stage2_emulation_results/Result_iter10/doubleElectron_PU200/";
  //inpath2 = "/Data/root_files/stage2_emulation_results/Result_iter11/singlePion_PU200/";
  //inpath2 = "/Data/root_files/stage2_emulation_results/Result_iter12/vbfHInv_200PU/";
  // inpath1 = "/Data/root_files/stage2_emulation_results/Result_iter20/doublePhoton_PU0/";
  // inpath2 = "/Data/root_files/stage2_emulation_results/Result_iter20/doubleElectron_PU200/";
  //inpath1 = "/Data/root_files/stage2_emulation_results/Result_iter19/singlePion_PU0/";
  //inpath2 = "/Data/root_files/stage2_emulation_results/Result_iter19/singlePion_PU0/";
  //inpath1 = "/Data/root_files/stage2_emulation_results/Result_iter17/singlePion_PU200/";
  //inpath1 = "/Data/root_files/stage2_emulation_results/Result_iter26/vbfHInv_200PU/";
  //inpath1 = "/Data/root_files/stage2_emulation_results/Result_iter24/singlePion_PU200/";
  // inpath1 = "/Data/root_files/stage2_emulation_results/Result_iter17/singlePion_PU0/";
  // inpath2 = "/Data/root_files/stage2_emulation_results/Result_iter26/vbfHInv_200PU/";
  // inpath1 = "/Data/root_files/stage2_emulation_results/Result_iter27/singlePion_PU0/";
  // inpath2 = "/Data/root_files/stage2_emulation_results/Result_iter33/singlePion_PU0/";
  
  // std::string aval[7] = {"16", "30", "45", "60", "75", "90", "105"};
  // //std::string aval[7] = {"11", "16", "22", "45", "60", "75", "90"};
  // TFile *fin1[7],*fin2[7];
  // for(int idroz=0;idroz<1;idroz++){
  //   std::cout << "idroz : " << idroz << ", aval: "<< aval[idroz]<< std::endl;
  //   std::string infile1 = inpath1 + "/" + "stage2SemiEmulator_ntuples_" + aval[idroz] + "_merged.root";
  //   std::string infile2 = inpath2 + "/" + "stage2SemiEmulator_ntuples_" + aval[idroz] + "_merged.root";
  //   fin1[idroz] = TFile::Open(infile1.c_str());
  //   fin2[idroz] = TFile::Open(infile2.c_str());
  // }
  // TCanvas *cTrigEffPiIdeal,*cTrigEffEleIdeal;
  
  //PlotTrigEff(fin1, 0, "effTrigGen", "Trigger Efficiencies of #pi^{+}#pi^{-} PU0 (100 GeV)", "p_{T} (GeV}", cTrigEffPiIdeal);
  //PlotTrigEff(fin2, 0, "effTrigGen", "Trigger Efficiencies of #pi^{+}#pi^{-} PU200 (100 GeV)", "p_{T} (GeV}", cTrigEffPiIdeal);
  // PlotTrigEff(fin1, 0, "effTrigGenPU", "Trigger Efficiencies of VBFtoInv PU200 (100 GeV)", "p_{T} (GeV}", cTrigEffPiIdeal);
  //PlotTrigEff(fin1, 0, "effTrigGenClstPt", "Trigger Efficiencies of VBFtoInv PU200 (150 GeV)", "p_{T} (GeV}", cTrigEffPiIdeal);
  //PlotTrigEff(fin2, 0, "effTrigGen", "Trigger Efficiencies of #it{e}^{+}#it{e}^{-} PU0", "p_{T} (GeV}", cTrigEffEleIdeal);
  //PlotTrigEff(fin1, 1, "hTrigEff", "Trigger Efficiencies of #pi^{+}#pi^{-}", "p_{T} (GeV}", cTrigEffPiIdeal);
  //PlotTrigEff(fin2, 1, "hTrigEff", "Trigger Efficiencies of #it{e}^{+}#it{e}^{-}", "p_{T} (GeV}", cTrigEffEleIdeal);
  //PlotTrigEff(fin1, 0, "effTrigGen", "Trigger Efficiencies of #it{e}^{+}#it{e}^{-} PU200 (#Delta(R/Z)<0.02)", "p_{T} (GeV}", cTrigEffPiIdeal);
  //PlotTrigEff(fin2, 0, "effTrigGenClstPt", "Trigger Efficiencies of #it{e}/#gamma for PU0", "p_{T} (GeV)", cTrigEffPiIdeal);
  //PlotTrigEff(fin2, 0, "effTrigGenClstPt", "Trigger Efficiencies of #it{e}/#gamma for PU200", "p_{T} (GeV)", cTrigEffPiIdeal);
  //PlotTrigEff(fin1, 0, "effTrigGenClstEta", "Trigger Efficiencies of #it{e}/#gamma for PU0", "|#eta|", cTrigEffPiIdeal);
  //PlotTrigEff(fin2, 0, "effTrigGenClstEta", "Trigger Efficiencies of #it{e}/#gamma for PU200", "|#eta|", cTrigEffPiIdeal);
  //PlotTrigEff(fin1, 0, "effTrigGenClstEta", "Trigger Efficiencies of #pi^{+}#pi^{-} for PU0", "|#eta|", cTrigEffPiIdeal);
  //PlotTrigEff(fin1, 0, "effTrigGenClstPt", "Trigger Efficiencies of #pi^{+}#pi^{-} for PU0", "p_{T} (GeV)", cTrigEffPiIdeal);
  // PlotCompareTrigEffTwo(fin1, fin2, "effTrigGenClstPt", "effTrigGenClstPt", "Trigger Efficiencies of #pi^{+}#pi^{-} for PU0", "p_{T} (GeV)", cTrigEffPiIdeal);
  //PlotTrigEff(fin1, 0, "effTrigGenClstPt", "Trigger Efficiencies of #pi^{+}#pi^{-} for PU200", "p_{T} (GeV)", cTrigEffPiIdeal);
  //PlotTrigEff(fin1, 0, "effTrigGenClstPt", "Trigger Efficiencies of VBF for PU200", "p_{T} (GeV)", cTrigEffPiIdeal);
  //PlotTrigEff(fin1, "effTrigGenClstPt", "effTrigGenClstPtTot", "Trigger Efficiencies of #pi^{+}#pi^{-} for PU0", "p_{T} (GeV)", cTrigEffPiIdeal);
  ////////////////////////////////////////////////////////
  
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

  // /////////////////////Nof Clusters //////////////
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
  // TCanvas *cNofClus;
  // PlotNofClus(fin1,fin2,fin3,"hNClus1GeV","Nof clusters #it{e}^{+}#it{e}^{-} PU200 for p_{T}>1", "Number of clusters",cNofClus);
  // //PlotNofClus(fin1,fin2,fin3,"hNClus3GeV","Nof clusters #it{e}^{+}#it{e}^{-} PU200 for p_{T}>3", "Number of clusters",cNofClus);
  // ///////////////////////////////////////////////
  
  ////////////////////// Calibration plots //////////////////////////////////////
  void PlotPtReso1D(TFile *fin, const char* histName, const char* plotTitle, const char* yaxisTitle, TCanvas*& cPtReso);
  void PlotTCPtReso1D(TFile *fin, const char* histName, const char* plotTitle, const char* yaxisTitle, TCanvas*& cPtReso);
  //inpath1 = "/Data/root_files/stage2_emulation_results/Reso_iter16/singlePion_PU0_Ideal";
  //inpath1 = "/Data/root_files/stage2_emulation_results/Reso_iter16/doublePhoton_PU0";
  //inpath1 = "/Data/root_files/stage2_emulation_results/Reso_iter18/doublePhoton_PU0";
  inpath1 = "/Data/root_files/stage2_emulation_results/Reso_iter18/doublePhoton_PU0";
  std::string infile1 = inpath1 + "/" + "CalcResolution_ntuples_16_merged.root";
  TCanvas *cPtReso;
  TFile *fin = TFile::Open(infile1.c_str());
  //PlotPtReso1D(fin,"Reso_1D/hGenClusPtReso1D","(Genjet-Cluster) vs Genjet","p_{T}^{Genjet}-p_{T}^{Cluster}  (GeV)",cPtReso);
  //PlotPtReso1D(fin,"Reso_1D/hGenClusPtReso1D","GenClus p_{T} resolution vs Genjet","#frac{p_{T}^{Genjet}-p_{T}^{Cluster}}{p_{T}^{Genjet}} ",cPtReso);
  //PlotPtReso1D(fin,"Reso_1D/hGenTCPtReso1D","Genjet-TC vs Genjet","(p_{T}^{Genjet}-p_{T}^{TC}) (GeV)",cPtReso);
  //PlotPtReso1D(fin,"Reso_1D/hGenTCPtReso1D","GenTC p_{T} resolution vs Genjet","#frac{p_{T}^{Genjet}-p_{T}^{TC}}{p_{T}^{Genjet}} ",cPtReso);
  //PlotPtReso1D(fin,"Reso_1D/hTCClusPtReso1D","TC-Clus vs Genjet","(p_{T}^{TC}-p_{T}^{Clus}) (GeV)",cPtReso);
  //PlotPtReso1D(fin,"Reso_1D/hTCClusPtReso1D","TCClus p_{T} resolution vs Genjet","#frac{p_{T}^{TC}-p_{T}^{Clus}}{p_{T}^{Genjet}} ",cPtReso);
  
  PlotTCPtReso1D(fin,"Reso_1D/hGenTCPtResovsTCPt1D","GenTC p_{T} resolution vs p_{T}^{maxTC} (e/#gamma)","#frac{p_{T}^{Genjet}-p_{T}^{TCPtsum}}{p_{T}^{maxTC}} ",cPtReso);
  
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
  //std::string avalf[7] = {"0.0113", "0.016", "0.0226", "0.045", "60", "75", "90"};

  //TH1D *hTrigEff[7];
  TEfficiency *hTrigEff[7];
  TH1D *hTrigEffReCalc[7];
  TH1D *hTrigEffPass[7];
  TH1D *hTrigEffTotal[7];
  
  int maxid = 1;
  int rebin = 2;
  for(int idroz=0;idroz<maxid;idroz++){

    //hTrigEff[idroz] = (TH1D *)fin[idroz]->Get(histName) ;
    hTrigEff[idroz] = (TEfficiency *)fin[idroz]->Get(histName) ;

    //cout <<"idroz : " << idroz << ", pointer " << hTrigEff[idroz] << std::endl;

    
    hTrigEff[idroz]->SetLineColor(GetColor(idroz));
    hTrigEff[idroz]->SetLineWidth(2.0);
    // if(ishist){
    //   hTrigEff[idroz]->Rebin(rebin);
    //   hTrigEff[idroz]->Scale(1/(3.*rebin));
    // }
    
    hTrigEffPass[idroz] = (TH1D *) hTrigEff[idroz]->GetCopyPassedHisto();
    hTrigEffTotal[idroz] = (TH1D *) hTrigEff[idroz]->GetCopyTotalHisto();
    hTrigEffPass[idroz]->Rebin(rebin);
    hTrigEffTotal[idroz]->Rebin(rebin);
    hTrigEffReCalc[idroz] = (TH1D *)hTrigEffPass[idroz]->Clone(Form("h1d_eff_%d",idroz));
    hTrigEffReCalc[idroz]->SetTitle(plotTitle);
    hTrigEffReCalc[idroz]->Divide(hTrigEffTotal[idroz]);
    hTrigEffReCalc[idroz]->SetLineColor(GetColor(idroz));
    hTrigEffReCalc[idroz]->SetLineWidth(2.0);
    hTrigEffReCalc[idroz]->GetXaxis()->SetRangeUser(0,200.);
    //hTrigEffReCalc[idroz]->GetXaxis()->SetRangeUser(1.2,3.5);
    hTrigEffReCalc[idroz]->GetYaxis()->SetRangeUser(0,1.2);
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
    func[idroz] = new TF1(Form("func%d",gindex), "[3] * ([2] + TMath::Erf((x - [0]) / [1] / TMath::Sqrt2()))", 0., hTrigEffReCalc[idroz]->GetXaxis()->GetXmax());
    //func[idroz] = new TF1(Form("func%d",gindex++), "TMath::ATan((x - [0]) / [1]) / TMath::Pi() + 0.5", 0., 400.); // CDF
    //func[idroz] = new TF1(Form("func%d",gindex), "[1]*TMath::TanH(x - [0])", 0., 100.);
    func[idroz]->SetParNames("mean", "sigma","par3", "par4");
    func[idroz]->SetParameters(150., 100.5, 0.5, 0.9);
    // func[idroz]->SetParNames("mean", "sigma");
    // func[idroz]->SetParameters(150., 10.5);
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
      hTrigEffReCalc[idroz]->Draw("");
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

void PlotCompareTrigEff(TFile *fin[], const char* histName1, const char* histName2, const char* plotTitle, const char* xaxisTitle, TCanvas*& c1){

  std::string avalf[7] = {"0.016", "0.03", "0.045", "60", "75", "90", "105"};
  //std::string avalf[7] = {"0.0113", "0.016", "0.0226", "0.045", "60", "75", "90"};
  
  TEfficiency *hTrigEff1[7];
  TEfficiency *hTrigEff2[7];
  TH1D *hTrigEffReCalc1[7];
  TH1D *hTrigEffReCalc2[7];
  TH1D *hTrigEffPass1[7];
  TH1D *hTrigEffPass2[7];
  TH1D *hTrigEffTotal1[7];
  TH1D *hTrigEffTotal2[7];
  
  int maxid = 1;
  int rebin = 2;
  float xmin = 0, xmax = 200.;
  for(int idroz=0;idroz<maxid;idroz++){

    hTrigEff1[idroz] = (TEfficiency *)fin[idroz]->Get(histName1) ;
    hTrigEff1[idroz]->SetLineColor(GetColor(idroz));
    hTrigEff1[idroz]->SetLineWidth(2.0);

    hTrigEff2[idroz] = (TEfficiency *)fin[idroz]->Get(histName2) ;
    hTrigEff2[idroz]->SetLineColor(GetColor(idroz));
    hTrigEff2[idroz]->SetLineWidth(2.0);

    hTrigEffPass1[idroz] = (TH1D *) hTrigEff1[idroz]->GetCopyPassedHisto();
    hTrigEffTotal1[idroz] = (TH1D *) hTrigEff1[idroz]->GetCopyTotalHisto();
    hTrigEffPass1[idroz]->Rebin(rebin);
    hTrigEffTotal1[idroz]->Rebin(rebin);
    hTrigEffReCalc1[idroz] = (TH1D *)hTrigEffPass1[idroz]->Clone(Form("h1d_eff_%d_1",idroz));
    hTrigEffReCalc1[idroz]->SetTitle(plotTitle);
    hTrigEffReCalc1[idroz]->Divide(hTrigEffTotal1[idroz]);
    hTrigEffReCalc1[idroz]->SetLineColor(GetColor(idroz));
    hTrigEffReCalc1[idroz]->SetLineWidth(2.0);
    hTrigEffReCalc1[idroz]->GetXaxis()->SetRangeUser(xmin, xmax);
    hTrigEffReCalc1[idroz]->GetYaxis()->SetRangeUser(0,1.2);
    hTrigEffReCalc1[idroz]->GetXaxis()->SetTitle(xaxisTitle);
    hTrigEffReCalc1[idroz]->GetYaxis()->SetTitle("Efficiency");

    hTrigEffPass2[idroz] = (TH1D *) hTrigEff2[idroz]->GetCopyPassedHisto();
    hTrigEffTotal2[idroz] = (TH1D *) hTrigEff2[idroz]->GetCopyTotalHisto();
    hTrigEffPass2[idroz]->Rebin(rebin);
    hTrigEffTotal2[idroz]->Rebin(rebin);
    hTrigEffReCalc2[idroz] = (TH1D *)hTrigEffPass2[idroz]->Clone(Form("h1d_eff_%d_2",idroz));
    hTrigEffReCalc2[idroz]->SetTitle(plotTitle);
    hTrigEffReCalc2[idroz]->Divide(hTrigEffTotal2[idroz]);
    hTrigEffReCalc2[idroz]->SetLineColor(GetColor(idroz+2));
    hTrigEffReCalc2[idroz]->SetLineWidth(2.0);
    hTrigEffReCalc2[idroz]->GetXaxis()->SetRangeUser(xmin, xmax);
    hTrigEffReCalc2[idroz]->GetYaxis()->SetRangeUser(0,1.2);
    hTrigEffReCalc2[idroz]->GetXaxis()->SetTitle(xaxisTitle);
    hTrigEffReCalc2[idroz]->GetYaxis()->SetTitle("Efficiency");

  }
  
  hTrigEff1[0]->SetTitle(plotTitle);
  
  auto legend = new TLegend(0.47,0.21,0.83,0.45);
  legend->SetTextSize(0.028);
  legend->SetHeader("Triangle side length");
  for(int idroz=0;idroz<maxid;idroz++){
    //legend->AddEntry(hTrigEff[idroz],Form("a = %s",avalf[idroz].c_str()),"lp");
    legend->AddEntry(hTrigEffReCalc1[idroz],Form("a = %s: closest cluster energy",avalf[idroz].c_str()),"lp");
    legend->AddEntry(hTrigEffReCalc2[idroz],Form("a = %s: Total cluster energy",avalf[idroz].c_str()),"lp");
  }
  legend->SetShadowColor(kWhite);

  TF1 *func1[7], *func2[7];
  //func[0] = new TF1(Form("func%d",gindex), "0.5 * (1.0 + TMath::Erf((x - [0]) / [1] / TMath::Sqrt2()))", 0., 200.);
  for(int idroz=0;idroz<maxid;idroz++){
    func1[idroz] = new TF1(Form("func1%d",gindex), "[3] * ([2] + TMath::Erf((x - [0]) / [1] / TMath::Sqrt2()))", 0., hTrigEffReCalc1[idroz]->GetXaxis()->GetXmax());
    //func[idroz] = new TF1(Form("func%d",gindex++), "TMath::ATan((x - [0]) / [1]) / TMath::Pi() + 0.5", 0., 400.); // CDF
    //func[idroz] = new TF1(Form("func%d",gindex), "[1]*TMath::TanH(x - [0])", 0., 100.);
    func1[idroz]->SetParNames("mean", "sigma","par3", "par4");
    func1[idroz]->SetParameters(150., 100.5, 0.5, 0.9);
    // func[idroz]->SetParNames("mean", "sigma");
    // func[idroz]->SetParameters(150., 10.5);
    func1[idroz]->SetNpx(1000);
    func1[idroz]->SetLineColor(GetColor(idroz));

    func2[idroz] = new TF1(Form("func2%d",gindex), "[3] * ([2] + TMath::Erf((x - [0]) / [1] / TMath::Sqrt2()))", 0., hTrigEffReCalc1[idroz]->GetXaxis()->GetXmax());
    //func[idroz] = new TF1(Form("func%d",gindex++), "TMath::ATan((x - [0]) / [1]) / TMath::Pi() + 0.5", 0., 400.); // CDF
    //func[idroz] = new TF1(Form("func%d",gindex), "[1]*TMath::TanH(x - [0])", 0., 100.);
    func2[idroz]->SetParNames("mean", "sigma","par3", "par4");
    func2[idroz]->SetParameters(150., 100.5, 0.5, 0.9);
    // func[idroz]->SetParNames("mean", "sigma");
    // func[idroz]->SetParameters(150., 10.5);
    func2[idroz]->SetNpx(1000);
    func2[idroz]->SetLineColor(GetColor(idroz+2));

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
  for(int idroz=0;idroz<maxid;idroz++){
    hTrigEffReCalc1[idroz]->Fit(func1[idroz],"+");
    hTrigEffReCalc2[idroz]->Fit(func2[idroz],"+");
  }
  for(int idroz=0;idroz<maxid;idroz++){
    if(idroz==0){
      hTrigEffReCalc1[idroz]->Draw("");
      hTrigEffReCalc2[idroz]->Draw("same");
    }else{
      hTrigEffReCalc1[idroz]->Draw("same");
      hTrigEffReCalc2[idroz]->Draw("same");
    }
  }
  legend->Draw();
  c1->Update();

}

void PlotCompareTrigEffTwo(TFile *fin1[], TFile *fin2[], const char* histName1, const char* histName2, const char* plotTitle, const char* xaxisTitle, TCanvas*& c1){

  std::string avalf[7] = {"0.016", "0.03", "0.045", "60", "75", "90", "105"};
  //std::string avalf[7] = {"0.0113", "0.016", "0.0226", "0.045", "60", "75", "90"};
  
  TEfficiency *hTrigEff1[7];
  TEfficiency *hTrigEff2[7];
  TH1D *hTrigEffReCalc1[7];
  TH1D *hTrigEffReCalc2[7];
  TH1D *hTrigEffPass1[7];
  TH1D *hTrigEffPass2[7];
  TH1D *hTrigEffTotal1[7];
  TH1D *hTrigEffTotal2[7];
  
  int maxid = 1;
  int rebin = 2;
  float xmin = 0, xmax = 200.;
  for(int idroz=0;idroz<maxid;idroz++){

    hTrigEff1[idroz] = (TEfficiency *)fin1[idroz]->Get(histName1) ;
    hTrigEff1[idroz]->SetLineColor(GetColor(idroz));
    hTrigEff1[idroz]->SetLineWidth(2.0);

    hTrigEff2[idroz] = (TEfficiency *)fin2[idroz]->Get(histName2) ;
    hTrigEff2[idroz]->SetLineColor(GetColor(idroz));
    hTrigEff2[idroz]->SetLineWidth(2.0);

    hTrigEffPass1[idroz] = (TH1D *) hTrigEff1[idroz]->GetCopyPassedHisto();
    hTrigEffTotal1[idroz] = (TH1D *) hTrigEff1[idroz]->GetCopyTotalHisto();
    hTrigEffPass1[idroz]->Rebin(rebin);
    hTrigEffTotal1[idroz]->Rebin(rebin);
    hTrigEffReCalc1[idroz] = (TH1D *)hTrigEffPass1[idroz]->Clone(Form("h1d_eff_%d_1",idroz));
    hTrigEffReCalc1[idroz]->SetTitle(plotTitle);
    hTrigEffReCalc1[idroz]->Divide(hTrigEffTotal1[idroz]);
    hTrigEffReCalc1[idroz]->SetLineColor(GetColor(idroz));
    hTrigEffReCalc1[idroz]->SetLineWidth(2.0);
    hTrigEffReCalc1[idroz]->GetXaxis()->SetRangeUser(xmin, xmax);
    hTrigEffReCalc1[idroz]->GetYaxis()->SetRangeUser(0,1.2);
    hTrigEffReCalc1[idroz]->GetXaxis()->SetTitle(xaxisTitle);
    hTrigEffReCalc1[idroz]->GetYaxis()->SetTitle("Efficiency");

    hTrigEffPass2[idroz] = (TH1D *) hTrigEff2[idroz]->GetCopyPassedHisto();
    hTrigEffTotal2[idroz] = (TH1D *) hTrigEff2[idroz]->GetCopyTotalHisto();
    hTrigEffPass2[idroz]->Rebin(rebin);
    hTrigEffTotal2[idroz]->Rebin(rebin);
    hTrigEffReCalc2[idroz] = (TH1D *)hTrigEffPass2[idroz]->Clone(Form("h1d_eff_%d_2",idroz));
    hTrigEffReCalc2[idroz]->SetTitle(plotTitle);
    hTrigEffReCalc2[idroz]->Divide(hTrigEffTotal2[idroz]);
    hTrigEffReCalc2[idroz]->SetLineColor(GetColor(idroz+2));
    hTrigEffReCalc2[idroz]->SetLineWidth(2.0);
    hTrigEffReCalc2[idroz]->GetXaxis()->SetRangeUser(xmin, xmax);
    hTrigEffReCalc2[idroz]->GetYaxis()->SetRangeUser(0,1.2);
    hTrigEffReCalc2[idroz]->GetXaxis()->SetTitle(xaxisTitle);
    hTrigEffReCalc2[idroz]->GetYaxis()->SetTitle("Efficiency");

  }
  
  hTrigEff1[0]->SetTitle(plotTitle);
  
  auto legend = new TLegend(0.47,0.21,0.83,0.45);
  legend->SetTextSize(0.028);
  legend->SetHeader("Triangle side length");
  for(int idroz=0;idroz<maxid;idroz++){
    // legend->AddEntry(hTrigEffReCalc1[idroz],Form("a = %s: closest cluster energy",avalf[idroz].c_str()),"lp");
    // legend->AddEntry(hTrigEffReCalc2[idroz],Form("a = %s: Total cluster energy",avalf[idroz].c_str()),"lp");
    legend->AddEntry(hTrigEffReCalc1[idroz],Form("a = %s: Cluster energy correction",avalf[idroz].c_str()),"lp");
    legend->AddEntry(hTrigEffReCalc2[idroz],Form("a = %s: TC energy correction",avalf[idroz].c_str()),"lp");
  }
  legend->SetShadowColor(kWhite);

  TF1 *func1[7], *func2[7];
  //func[0] = new TF1(Form("func%d",gindex), "0.5 * (1.0 + TMath::Erf((x - [0]) / [1] / TMath::Sqrt2()))", 0., 200.);
  for(int idroz=0;idroz<maxid;idroz++){
    func1[idroz] = new TF1(Form("func1%d",gindex), "[3] * ([2] + TMath::Erf((x - [0]) / [1] / TMath::Sqrt2()))", 0., hTrigEffReCalc1[idroz]->GetXaxis()->GetXmax());
    //func[idroz] = new TF1(Form("func%d",gindex++), "TMath::ATan((x - [0]) / [1]) / TMath::Pi() + 0.5", 0., 400.); // CDF
    //func[idroz] = new TF1(Form("func%d",gindex), "[1]*TMath::TanH(x - [0])", 0., 100.);
    func1[idroz]->SetParNames("mean", "sigma","par3", "par4");
    func1[idroz]->SetParameters(150., 100.5, 0.5, 0.9);
    // func[idroz]->SetParNames("mean", "sigma");
    // func[idroz]->SetParameters(150., 10.5);
    func1[idroz]->SetNpx(1000);
    func1[idroz]->SetLineColor(GetColor(idroz));

    func2[idroz] = new TF1(Form("func2%d",gindex), "[3] * ([2] + TMath::Erf((x - [0]) / [1] / TMath::Sqrt2()))", 0., hTrigEffReCalc1[idroz]->GetXaxis()->GetXmax());
    //func[idroz] = new TF1(Form("func%d",gindex++), "TMath::ATan((x - [0]) / [1]) / TMath::Pi() + 0.5", 0., 400.); // CDF
    //func[idroz] = new TF1(Form("func%d",gindex), "[1]*TMath::TanH(x - [0])", 0., 100.);
    func2[idroz]->SetParNames("mean", "sigma","par3", "par4");
    func2[idroz]->SetParameters(150., 100.5, 0.5, 0.9);
    // func[idroz]->SetParNames("mean", "sigma");
    // func[idroz]->SetParameters(150., 10.5);
    func2[idroz]->SetNpx(1000);
    func2[idroz]->SetLineColor(GetColor(idroz+2));

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
  for(int idroz=0;idroz<maxid;idroz++){
    hTrigEffReCalc1[idroz]->Fit(func1[idroz],"+");
    hTrigEffReCalc2[idroz]->Fit(func2[idroz],"+");
  }
  for(int idroz=0;idroz<maxid;idroz++){
    if(idroz==0){
      hTrigEffReCalc1[idroz]->Draw("");
      hTrigEffReCalc2[idroz]->Draw("same");
    }else{
      hTrigEffReCalc1[idroz]->Draw("same");
      hTrigEffReCalc2[idroz]->Draw("same");
    }
  }
  legend->Draw();
  c1->Update();

}

void PlotPtReso1D(TFile *fin, const char* histName, const char* plotTitle, const char* yaxisTitle, TCanvas*& c1)
{
  // //Eta : outermost(Sci) = 1.321, innermost(Si) =  3.152
  // //use delta-Eta = 0.0873 (or mutltiple) and make range for |\eta|
  // float deltaEta = 0.0873;
  // const int nJetEtaBins = 21;
  // Float_t jetEtaBin[22];
  // jetEtaBin[0] = 1.321;
  // for(int ieta=1;ieta<=nJetEtaBins;ieta++)
  //   jetEtaBin[ieta] = jetEtaBin[0] + ieta*deltaEta;
  // //jetEtaBin[21] would be 3.1543;

  float x, xerror, y, yerror;
  TGraphErrors *gr = new TGraphErrors();
  int i=0;

  const int nJetPtBins = 43;
  Float_t jetPtBin[44] = {0, 15., 16., 18., 20., 22., 25., 30., 35., 40., 45., 50., 55., 60., 65., 70., 75., 80., 85., 90., 95., 100.,  
			  110., 120., 130., 140., 150., 160., 170., 180., 190., 200.,
			  220., 240., 260., 280., 300.,
			  330., 360., 390., 420.,
			  440., 480., 520.};
  for(int ipt=0;ipt<nJetPtBins;ipt++){
    TH1F *h1  = (TH1F *)fin->Get(Form("%s_%d",histName,ipt));
    y = h1->GetXaxis()->GetBinCenter(h1->GetMaximumBin());
    double y1 = h1->GetXaxis()->GetBinCenter(h1->GetMaximumBin()+1);
    yerror = 0.5*abs(y1-y);
    y = h1->GetMean();
    //yerror = h1->GetRMS();
    x = 0.5*abs(jetPtBin[ipt+1]+jetPtBin[ipt]);
    xerror = 0.5*abs(jetPtBin[ipt+1]-jetPtBin[ipt]);
    double rerror = sqrt((yerror/y)*(yerror/y)+(xerror/x)*(xerror/x));
    y = y/x;
    yerror = y*rerror;
    if(x<10.) continue;
    std::cout<<"x : "<<x<<", xerror "<<xerror<<", y : "<<y<<", yerror "<<yerror<<std::endl;
    gr->SetPoint(i,x,y);
    gr->SetPointError(i,xerror,yerror);
    i++;
  }
  gr->SetTitle(plotTitle);
  gr->GetXaxis()->SetTitle("p_{T}^{Genjet} (GeV)");
  gr->GetYaxis()->SetTitle(yaxisTitle);
  gr->GetXaxis()->SetTitleOffset(1.2);
  gr->GetYaxis()->SetTitleOffset(1.6);
  gr->SetMinimum(0.0);
  gr->SetMaximum(1.0);

  
  auto f1 = new TF1(Form("func%d",gindex),"[0]*([1]+[2]/(x+[3]))",-100,100);
  f1->SetParameters(0.729,0.199,25.4,80);
  f1->SetNpx(1000);
  gr->Fit(f1);
  
  auto legend = new TLegend(0.31,0.75,0.85,0.86);
  legend->SetTextSize(0.028);
  //legend->SetHeader("");
  legend->AddEntry(f1,"N[a+b/(x+x0)]","lp");
  legend->AddEntry((TObject *)0x0,Form("N: %4.2f #pm %4.2f, a: %4.2f #pm %4.2f",
				       f1->GetParameter(0), f1->GetParError(0),
				       f1->GetParameter(1),f1->GetParError(1)),"");
  legend->AddEntry((TObject *)0x0,Form("b: %4.2f #pm %4.2f, x0: %4.2f #pm %4.2f",
				       f1->GetParameter(2), f1->GetParError(2),
				       f1->GetParameter(3), f1->GetParError(3)),"");
  legend->SetShadowColor(kWhite);

  c1 = new TCanvas(Form("c%d",gindex),Form("c%d",gindex),900,900); gindex++;
  SetCanvasStyle(c1);

  gr->Draw("APE1");
  legend->Draw();
  c1->Update();

}


void PlotTCPtReso1D(TFile *fin, const char* histName, const char* plotTitle, const char* yaxisTitle, TCanvas*& c1)
{

  float x, xerror, y, yerror;
  TGraphErrors *gr = new TGraphErrors();
  int i=0;
  
  const int nTCPtBins = 100;
  TH1F *h1[nTCPtBins];
  TF1 *fh1[nTCPtBins];
  for(int ipt=0;ipt<nTCPtBins;ipt++){
    h1[ipt]  = (TH1F *)fin->Get(Form("%s_%d",histName,ipt));
    if(h1[ipt]->GetEntries()<10) continue;
    fh1[ipt] = new TF1(Form("funch%d",gindex++),"gaus",-15,15);
    double norm = h1[ipt]->GetBinContent(h1[ipt]->GetMaximumBin());
    fh1[ipt]->SetNpx(1000);
    fh1[ipt]->SetParameters(norm,1,2.);
    h1[ipt]->Fit(fh1[ipt],"NQLR");
    i++;
  }
  // int imax = (i<10)?i:10;
  // std::cout << "i: " << i <<", imax :" << imax <<  std::endl;
  
  // c1 = new TCanvas(Form("c%d",gindex),Form("c%d",gindex),900,900); gindex++;
  // c1->Divide(5,2);
  // SetCanvasStyle(c1);
  // for(int ipt=0;ipt<imax;ipt++){
  //   c1->cd(ipt+1);
  //   h1[ipt]->GetXaxis()->SetRangeUser(-15.,15.);
  //   double norm = h1[ipt]->GetBinContent(h1[ipt]->GetMaximumBin());
  //   h1[ipt]->SetMaximum(1.1*norm);
  //   h1[ipt]->GetYaxis()->SetTitle("Entries");
  //   h1[ipt]->GetXaxis()->SetTitle("[p_{T}^{Genjet}-p_{T}^{TCPtsum_{CEE}}] (GeV)");
  //   h1[ipt]->Draw();
  // }
  // int offset = imax;
  // imax = (i<(10+offset))?i:(10+offset);
  // c1 = new TCanvas(Form("c%d",gindex),Form("c%d",gindex),900,900); gindex++;
  // c1->Divide(5,2);
  // SetCanvasStyle(c1);
  // for(int ipt=0+offset;ipt<imax;ipt++){
  //   c1->cd((ipt-offset)+1);
  //   double norm = h1[ipt]->GetBinContent(h1[ipt]->GetMaximumBin());
  //   h1[ipt]->SetMaximum(1.1*norm);
  //   h1[ipt]->GetXaxis()->SetRangeUser(-15.,15.);
  //   h1[ipt]->GetYaxis()->SetTitle("Entries");
  //   h1[ipt]->GetXaxis()->SetTitle("[p_{T}^{Genjet}-p_{T}^{TCPtsum_{CEE}}] (GeV)");
  //   h1[ipt]->Draw();
  // }
  
  int nof1DHists = i;
  i = 0;
  for(int ipt=0;ipt<nof1DHists;ipt++){
    
    y = fh1[ipt]->GetParameter(1);
    //yerror = 0.5*abs(fh1[ipt]->GetParError(1));
    yerror = 0.5*abs(fh1[ipt]->GetParameter(2));
    
    x = 0.5*abs(float(ipt+1)+float(ipt));
    xerror = 0.5*abs(float(ipt+1)-float(ipt));
    
    double rerror = sqrt((yerror/y)*(yerror/y)+(xerror/x)*(xerror/x));
    y = y/x;
    yerror = abs(y*rerror);
    
    std::cout<<"x : "<<x<<", xerror "<<xerror<<", y : "<<y<<", yerror "<<yerror<<std::endl;
    
    gr->SetPoint(i,x,y);
    gr->SetPointError(i,xerror,yerror);
    i++;
  }
  
  gr->SetTitle(plotTitle);
  gr->GetXaxis()->SetTitle("p_{T}^{maxTC} (GeV)");
  gr->GetYaxis()->SetTitle(yaxisTitle);
  gr->GetXaxis()->SetTitleOffset(1.2);
  gr->GetYaxis()->SetTitleOffset(2);
  gr->SetMinimum(-1.0);
  gr->SetMaximum(1.0);

  
  //auto f1 = new TF1(Form("func%d",gindex++),"[0]*([1]-[2]*pow(x-[3],2))",-100,100);
  // auto f1 = new TF1(Form("func%d",gindex++),"[3]*sqrt((x-[0])*(x-[0])+[1]*2*(x-[0]))*pow(([2]-(x-[0])),2)-1",1,17);
  // f1->SetParameters(-0.620,0.0001,30,0.001);
  auto f1 = new TF1(Form("func%d",gindex++),"[0]*sqrt((x-[1])*(x-[1])+[2]*(x-[1]))*pow(([3]-(x-[1])),2)-1",0,40);
  //f1->SetParameters(3.68427e-05,0.056964,2.7018,41.9545);
  f1->SetParameters(1.75139e-05,-0.191719,290.217,45.1571);
  //f1->SetParameters(1.73055e-05,-0.361961,299.341,45.1033);
  //f1->SetParameters(1.09185e-05,0.166938,890.269,43.8177);
  f1->SetNpx(1000);
  //gr->Fit(f1,"R");
  
  auto legend = new TLegend(0.31,0.75,0.85,0.86);
  legend->SetTextSize(0.028);
  //legend->SetHeader("");
  //legend->AddEntry(f1,"N[a+b/(x+x0)]","lp");
  legend->AddEntry(f1,"N#times#sqrt{(x-x0)^{2}+a*(x-x0)}#times(b-(x-x0))^{2}-c","lp");
  legend->AddEntry((TObject *)0x0,Form("N: %4.2e, x0: %4.2f,",f1->GetParameter(0), f1->GetParameter(1)),"");
  legend->AddEntry((TObject *)0x0,Form("a: %4.2f, b: %4.2f, c: 1", f1->GetParameter(2), f1->GetParameter(3)),"");
  legend->SetShadowColor(kWhite);

  gr->GetXaxis()->SetLimits(0,40.);
  c1 = new TCanvas(Form("c%d",gindex),Form("c%d",gindex),900,900); gindex++;
  SetCanvasStyle(c1);
  gr->Draw("APE1 same");
  f1->Draw("same");
  legend->Draw();
  c1->Update();

}
