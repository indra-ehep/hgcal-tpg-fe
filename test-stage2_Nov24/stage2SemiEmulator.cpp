/**********************************************************************
 Created on : 24/04/2025
 Purpose    : Stage2 semi-emulator test
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <TROOT.h>
#include <TChain.h>
#include <TProfile.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TMath.h>
#include <TH2.h>
#include "TParticlePDG.h"
#include "TDatabasePDG.h"

#include "Stage2.hh"

#include "TPGTCFloats.hh"
#include "TPGLSBScales.hh"

typedef struct{
  std::string name;
  int index;
}JetPart;

typedef struct{
  uint32_t tcid, layer;
  float z,eta;
}TCOrder;

bool comp(TCOrder x, TCOrder y) {
  float diff = x.z - y.z ;
  bool condn = ((x.layer<y.layer) or ((x.layer==y.layer) and (abs(diff)>1.e-5)) or ((x.layer==y.layer) and (abs(diff)<1.e-5) and (abs(x.eta)<abs(y.eta))));
  return condn ;
}

int main(int argc, char** argv)
{
  // TPGLSBScales::TPGStage2ClusterLSB lsbScales;
  // lsbScales.print();  
  // return true;
  
  bool doPrint = 0;
  std::cout << "========= Run as : ./stage2HtoTauTauEnergyCorrelation.exe $input_file $index $nofevents ==========" << std::endl;
  
  if(argc < 2){
    std::cerr << argv[0] << ": no input file has been specified specified" << std::endl;
    return false;
  }
  std::string inputfile;
  std::istringstream issInfile(argv[1]);
  issInfile >> inputfile;
  std::string index = "0";
  if(argc >= 3){
    std::istringstream issnIndex(argv[2]);
    issnIndex >> index;
  }
  uint64_t nofEvents(0);
  if(argc >= 4){
    std::istringstream issnEvents(argv[3]);
    issnEvents >> nofEvents;
  }
  std::unique_ptr<TFile> fin( TFile::Open(Form(inputfile.c_str())) );
  //TFile *fin = TFile::Open(Form(inputfile.c_str()));
  std::cout<<"Input filename : " << fin->GetName() << std::endl;
  //TTree *tr  = (TTree*)fin->Get("l1tHGCalTriggerNtuplizer/HGCalTriggerNtuple");
  std::unique_ptr<TTree> tr((TTree*)fin->Get("l1tHGCalTriggerNtuplizer/HGCalTriggerNtuple"));  
  tr->SetBranchStatus("*",0);
  Int_t event ;
  tr->SetBranchStatus("event",1);
  tr->SetBranchAddress("event"	, &event);

  Int_t gen_n = 0 ;
  tr->SetBranchStatus("gen_n",1);
  tr->SetBranchAddress("gen_n" , &gen_n);
  
  std::vector<float>  *gen_pt = 0 ;
  tr->SetBranchStatus("gen_pt",1);
  tr->SetBranchAddress("gen_pt" , &gen_pt);
  
  std::vector<float>  *gen_eta = 0 ;
  tr->SetBranchStatus("gen_eta",1);
  tr->SetBranchAddress("gen_eta" , &gen_eta);

  std::vector<float>  *gen_phi = 0 ;
  tr->SetBranchStatus("gen_phi",1);
  tr->SetBranchAddress("gen_phi" , &gen_phi);
  
  std::vector<float>  *gen_pdgid = 0 ;
  tr->SetBranchStatus("gen_pdgid",1);
  tr->SetBranchAddress("gen_pdgid" , &gen_pdgid);
  
  std::vector<float>  *gen_energy = 0 ;
  tr->SetBranchStatus("gen_energy",1);
  tr->SetBranchAddress("gen_energy" , &gen_energy);
  
  std::vector<float>  *gen_charge = 0 ;
  tr->SetBranchStatus("gen_charge",1);
  tr->SetBranchAddress("gen_charge" , &gen_charge);
  
  std::vector<float>  *gen_status = 0 ;
  tr->SetBranchStatus("gen_status",1);
  tr->SetBranchAddress("gen_status" , &gen_status);

  gInterpreter->GenerateDictionary("std::vector<std::vector<int> >", "vector");
  std::vector<std::vector<int>>  *gen_daughters = 0 ;
  tr->SetBranchStatus("gen_daughters",1);
  tr->SetBranchAddress("gen_daughters" , &gen_daughters);
  
  std::vector<float>  *genpart_pt = 0 ;
  tr->SetBranchStatus("genpart_pt",1);
  tr->SetBranchAddress("genpart_pt" , &genpart_pt);
  
  std::vector<float>  *genpart_eta = 0 ;
  tr->SetBranchStatus("genpart_eta",1);
  tr->SetBranchAddress("genpart_eta" , &genpart_eta);

  std::vector<float>  *genpart_phi = 0 ;
  tr->SetBranchStatus("genpart_phi",1);
  tr->SetBranchAddress("genpart_phi" , &genpart_phi);
  
  std::vector<float>  *genpart_pid = 0 ;
  tr->SetBranchStatus("genpart_pid",1);
  tr->SetBranchAddress("genpart_pid" , &genpart_pid);

  std::vector<float>  *genpart_mother = 0 ;
  tr->SetBranchStatus("genpart_mother",1);
  tr->SetBranchAddress("genpart_mother" , &genpart_mother);

  std::vector<float>  *genpart_energy = 0 ;
  tr->SetBranchStatus("genpart_energy",1);
  tr->SetBranchAddress("genpart_energy" , &genpart_energy);

  std::vector<float>  *genpart_gen = 0 ;
  tr->SetBranchStatus("genpart_gen",1);
  tr->SetBranchAddress("genpart_gen" , &genpart_gen);
  
  std::vector<float>  *genpart_fromBeamPipe = 0 ;
  tr->SetBranchStatus("genpart_fromBeamPipe",1);
  tr->SetBranchAddress("genpart_fromBeamPipe" , &genpart_fromBeamPipe);

  std::vector<float>  *genpart_exeta = 0 ;
  tr->SetBranchStatus("genpart_exeta",1);
  tr->SetBranchAddress("genpart_exeta" , &genpart_exeta);

  std::vector<float>  *genpart_exphi = 0 ;
  tr->SetBranchStatus("genpart_exphi",1);
  tr->SetBranchAddress("genpart_exphi" , &genpart_exphi);

  Int_t genjet_n = 0 ;
  tr->SetBranchStatus("genjet_n",1);
  tr->SetBranchAddress("genjet_n" , &genjet_n);

  std::vector<float>  *genjet_pt = 0 ;
  tr->SetBranchStatus("genjet_pt",1);
  tr->SetBranchAddress("genjet_pt" , &genjet_pt);

  std::vector<float>  *genjet_eta = 0 ;
  tr->SetBranchStatus("genjet_eta",1);
  tr->SetBranchAddress("genjet_eta" , &genjet_eta);
  
  std::vector<float>  *genjet_phi = 0 ;
  tr->SetBranchStatus("genjet_phi",1);
  tr->SetBranchAddress("genjet_phi" , &genjet_phi);

  std::vector<float>  *genjet_energy = 0 ;
  tr->SetBranchStatus("genjet_energy",1);
  tr->SetBranchAddress("genjet_energy" , &genjet_energy);

  std::vector<float>  *tc_pt = 0 ;
  tr->SetBranchStatus("tc_pt",1);
  tr->SetBranchAddress("tc_pt" , &tc_pt);
  
  std::vector<float>  *tc_eta = 0 ;
  tr->SetBranchStatus("tc_eta",1);
  tr->SetBranchAddress("tc_eta" , &tc_eta);

  std::vector<float>  *tc_phi = 0 ;
  tr->SetBranchStatus("tc_phi",1);
  tr->SetBranchAddress("tc_phi" , &tc_phi);

  std::vector<float>  *tc_layer = 0 ;
  tr->SetBranchStatus("tc_layer",1);
  tr->SetBranchAddress("tc_layer" , &tc_layer);
  
  std::vector<float>  *tc_x = 0 ;
  tr->SetBranchStatus("tc_x",1);
  tr->SetBranchAddress("tc_x" , &tc_x);
  
  std::vector<float>  *tc_y = 0 ;
  tr->SetBranchStatus("tc_y",1);
  tr->SetBranchAddress("tc_y" , &tc_y);
  
  std::vector<float>  *tc_z = 0 ;
  tr->SetBranchStatus("tc_z",1);
  tr->SetBranchAddress("tc_z" , &tc_z);
  
  std::vector<float>  *tc_energy = 0 ;
  tr->SetBranchStatus("tc_energy",1);
  tr->SetBranchAddress("tc_energy" , &tc_energy);
  
  std::vector<float>  *tc_data = 0 ;
  tr->SetBranchStatus("tc_data",1);
  tr->SetBranchAddress("tc_data" , &tc_data);
  
  std::vector<float>  *tc_uncompressedCharge = 0 ;
  tr->SetBranchStatus("tc_uncompressedCharge",1);
  tr->SetBranchAddress("tc_uncompressedCharge" , &tc_uncompressedCharge);

  std::cout << "Total number of Events: " << tr->GetEntries() << std::endl;
  // std::cout << "Sizeof: " << sizeof(TPGTCBits) << std::endl;
  // std::cout << "Sizeof: " << sizeof(uint32_t) << std::endl;

  // TPGTCBits tcbit;
  // tcbit.setEnergy(262142);
  // tcbit.setROverZ(4094);
  // tcbit.setPhi(4094);
  // tcbit.setLayer(62);
  // tcbit.setU(32);
  // tcbit.setUbar(62);
  // tcbit.print();
  
  TH1D *hEt = new TH1D("hEt","hEt",100,0,1000);
  TH1D *hPhi = new TH1D("hPhi","hPhi",2*TMath::TwoPi(),-1.0*TMath::TwoPi(),TMath::TwoPi());
  TH1D *hPhiDeg = new TH1D("hPhiDeg","hPhiDeg",2*TMath::RadToDeg()*TMath::TwoPi(),-1.0*TMath::RadToDeg()*TMath::TwoPi(),TMath::RadToDeg()*TMath::TwoPi());
  TH1D *hPtReso = new TH1D("hPtReso","Energy resolution",200,-100.,100.0);
  TH1D *hTrigEff = new TH1D("hTrigEff","Trigger Efficiency",200, 0.,200.);
  
  uint64_t totalEntries = tr->GetEntries();
  if(nofEvents==0 or nofEvents>totalEntries) nofEvents = totalEntries;
  
  std::cout << "Total Entries : " << totalEntries << std::endl;
  std::cout << "Loop for: " << nofEvents << " entries"  << std::endl;
  
  TH2D *tcxy[6],*tcxyOverZ[6],*clusxyOverZ[6],*clusxy[6],*deltaGenclusSeg[6];
  TH2D *tcxyAll = new TH2D("TCXYAll","x-y distribution of TCs",1000,-500,500,1000,-500,500);
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) {
    tcxy[isect] = new TH2D(Form("TCXY_%u",isect),Form("x-y distribution of TCs for sector %u",isect),1000,-500,500,1000,-500,500);
    tcxyOverZ[isect] = new TH2D(Form("TCXYOverZ_%u",isect),Form("x/z and y/z distribution of TCs for sector %u",isect),1000,-0.6,0.6,1000,-0.6,0.6);
    clusxyOverZ[isect] = new TH2D(Form("CLUSXYOverZ_%u",isect),Form("x/z and y/z distribution of clusters for sector %u",isect),1000,-0.6,0.6,1000,-0.6,0.6);
    clusxy[isect] = new TH2D(Form("CLUSXY_%u",isect),Form("x-y distribution of clusters for sector %u",isect),1000,-0.6,0.6,1000,-0.6,0.6);
    deltaGenclusSeg[isect] = new TH2D(Form("deltaGenclus_%u",isect),Form("deltaGenclus_%u",isect),100,-0.25,0.25,100,-0.25,0.25);
  }
  TH2D *deltaGenclus = new TH2D("deltaGenclusAll","deltaGenclusAll",100,-0.25,0.25,100,-0.25,0.25);
  TH2D *deltaTCclus = new TH2D("deltaTcclusAll","deltaTCclusAll",100,-0.25,0.25,100,-0.25,0.25);
  TH2D *deltaGentc = new TH2D("deltaGentc","deltaGentc",100,-0.25,0.25,100,-0.25,0.25);
  TH1D *hClusE = new TH1D("hClusE","hClusE",100,0.0,100.0);
  TH2D *hGenClusE = new TH2D("hGenClusE","hGenClusE",200,0.0,200.0,200,0.0,200.0);
  TH2D *hGenClusE_1 = new TH2D("hGenClusE_1","hGenClusE_1",200,0.0,200.0,200,0.0,200.0);
  TH2D *hGenClusE_2 = new TH2D("hGenClusE_2","hGenClusE_2",200,0.0,200.0,200,0.0,200.0);
  TH2D *hGenClusE_3 = new TH2D("hGenClusE_3","hGenClusE_3",200,0.0,200.0,200,0.0,200.0);
  TH2D *hGenClusE_4 = new TH2D("hGenClusE_4","hGenClusE_4",200,0.0,200.0,200,0.0,200.0);
  TH2D *hGenClusE_5 = new TH2D("hGenClusE_5","hGenClusE_5",200,0.0,200.0,200,0.0,200.0);
  
  TProfile *hPtCorrGenjetvsTC = new TProfile("hPtCorrGenjetvsTC","hPtCorr Genjet-vs-TC",200,0.,200.,0.,200.);
  TProfile *hPtCorrGenjetvsClus = new TProfile("hPtCorrGenjetvsClus","hPtCorr Genjet-vs-Cluster",200,0.,200.,0.,200.);
  TProfile *hPtCorrGenjetvsClus_1 = new TProfile("hPtCorrGenjetvsClus_1","hPtCorr Genjet-vs-Cluster",200,0.,200.,0.,200.);
  TProfile *hPtCorrGenjetvsClus_2 = new TProfile("hPtCorrGenjetvsClus_2","hPtCorr Genjet-vs-Cluster",200,0.,200.,0.,200.);
  TProfile *hPtCorrGenjetvsClus_3 = new TProfile("hPtCorrGenjetvsClus_3","hPtCorr Genjet-vs-Cluster",200,0.,200.,0.,200.);
  TProfile *hPtCorrGenjetvsClus_4 = new TProfile("hPtCorrGenjetvsClus_4","hPtCorr Genjet-vs-Cluster",200,0.,200.,0.,200.);
  TProfile *hPtCorrGenjetvsClus_5 = new TProfile("hPtCorrGenjetvsClus_5","hPtCorr Genjet-vs-Cluster",200,0.,200.,0.,200.);
  
  TProfile *hPtCorrGenjetvsTC_pion = new TProfile("hPtCorrGenjetvsTC_pion","hPtCorr Genjet-vs-TC (pion) with #DeltaR<0.07",200,0.,200.,0.,200.);
  TProfile *hPtCorrGenjetvsClus_pion = new TProfile("hPtCorrGenjetvsClus_pion","hPtCorr Genjet-vs-Cluster (pion) with #DeltaR<0.07",200,0.,200.,0.,200.);
  TProfile *hPtCorrGenjetvsTC_pion_0p2 = new TProfile("hPtCorrGenjetvsTC_pion_0p2","hPtCorr Genjet-vs-TC (pion) with #DeltaR<0.2",200,0.,200.,0.,200.);
  TProfile *hPtCorrGenjetvsClus_pion_0p2 = new TProfile("hPtCorrGenjetvsClus_pion_0p2","hPtCorr Genjet-vs-Cluster (pion) with #DeltaR<0.2",200,0.,200.,0.,200.);
  TProfile *hPtCorrGenjetvsTC_pion_0p4 = new TProfile("hPtCorrGenjetvsTC_pion_0p4","hPtCorr Genjet-vs-TC (pion) with #DeltaR<0.4",200,0.,200.,0.,200.);
  TProfile *hPtCorrGenjetvsClus_pion_0p4 = new TProfile("hPtCorrGenjetvsClus_pion_0p4","hPtCorr Genjet-vs-Cluster (pion) with #DeltaR<0.4",200,0.,200.,0.,200.);
  
  hPtCorrGenjetvsTC_pion->GetXaxis()->SetTitle("#tau-jet p_{T} (#tau_{p_{T}}-MET) [GeV]");
  hPtCorrGenjetvsTC_pion->GetYaxis()->SetTitle("Total TC p_{T} [GeV]");
  hPtCorrGenjetvsClus_pion->GetXaxis()->SetTitle("#tau-jet p_{T} (#tau_{p_{T}}-MET) [GeV]");
  hPtCorrGenjetvsClus_pion->GetYaxis()->SetTitle("Total Cluster p_{T} [GeV]");
  
  hPtCorrGenjetvsTC_pion_0p2->GetXaxis()->SetTitle("#tau-jet p_{T} (#tau_{p_{T}}-MET) [GeV]");
  hPtCorrGenjetvsTC_pion_0p2->GetYaxis()->SetTitle("Total TC p_{T} [GeV]");
  hPtCorrGenjetvsClus_pion_0p2->GetXaxis()->SetTitle("#tau-jet p_{T} (#tau_{p_{T}}-MET) [GeV]");
  hPtCorrGenjetvsClus_pion_0p2->GetYaxis()->SetTitle("Total Cluster p_{T} [GeV]");
  
  hPtCorrGenjetvsTC_pion_0p4->GetXaxis()->SetTitle("#tau-jet p_{T} (#tau_{p_{T}}-MET) [GeV]");
  hPtCorrGenjetvsTC_pion_0p4->GetYaxis()->SetTitle("Total TC p_{T} [GeV]");
  hPtCorrGenjetvsClus_pion_0p4->GetXaxis()->SetTitle("#tau-jet p_{T} (#tau_{p_{T}}-MET) [GeV]");
  hPtCorrGenjetvsClus_pion_0p4->GetYaxis()->SetTitle("Total Cluster p_{T} [GeV]");
  
  TProfile *hPtCorrGenjetvsTC_electron = new TProfile("hPtCorrGenjetvsTC_electron","hPtCorr Genjet-vs-TC (electron) with #DeltaR<0.07",200,0.,200.,0.,200.);
  TProfile *hPtCorrGenjetvsClus_electron = new TProfile("hPtCorrGenjetvsClus_electron","hPtCorr Genjet-vs-Cluster (electron) with #DeltaR<0.07",200,0.,200.,0.,200.);
  hPtCorrGenjetvsTC_electron->GetXaxis()->SetTitle("#tau-jet p_{T} (#tau_{p_{T}}-MET) [GeV]");
  hPtCorrGenjetvsTC_electron->GetYaxis()->SetTitle("Total TC p_{T} [GeV]");
  hPtCorrGenjetvsClus_electron->GetXaxis()->SetTitle("#tau-jet p_{T} (#tau_{p_{T}}-MET) [GeV]");
  hPtCorrGenjetvsClus_electron->GetYaxis()->SetTitle("Total Cluster p_{T} [GeV]");
  
  TH1D *hTCLayerEWt = new TH1D("hTCLayerEWt","Energy weighted layer",50,-0.5,49.5);
  TH1D *hTCLayerEWt_pion = new TH1D("hTCLayerEWt_pion","Energy weighted layer (pion)",50,-0.5,49.5);
  TH1D *hTCLayerEWt_electron = new TH1D("hTCLayerEWt_electron","Energy weighted layer (electron)",50,-0.5,49.5);

  
  TProfile *hPtEta_TC_pion = new TProfile("hPtEta_TC_pion","hPtEta_TC_pion",200,0.,5.,0.,200.);
  TH2D *hECorrc_pion = new TH2D("hECorrc_pion","hECorrc_pion",50,0.0,50.0,1000,0.0,5.0);
  TH2D *hECorrc_electron = new TH2D("hECorrc_electron","hECorrc_electron",50,0.0,50.0,1000,0.0,5.0);

  TH2D *hECorrc_pion_HT = new TH2D("hECorrc_pion_HT","hECorrc_pion_HT",50,0.0,50.0,1000,0.0,100.0);
  
  TH2D *hTCPhiCorr = new TH2D("hTCPhiCorr","hTCPhiCorr",750,-375.,375., 750,-375.,375.);

  TH1D *hTCLocalPhiBits[6];
  TH1D *hClusLocalPhiBits[6], *hClusLocalEtaBits[6], *hClusLocalPhi[6],  *hClusGlobalPhi[6], *hClusGlobalEta[6];
  TH2D *hTCRoZ2Eta[6], *hTCRoZ2CalcEta[6];
  TH1D *hTCRozBits[6];
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) {
    hTCLocalPhiBits[isect] = new TH1D(Form("hTCLocalPhiBits_%u",isect),Form("Local #phi bits for sector %u",isect), 1000,0,10000);
    hClusLocalPhiBits[isect] = new TH1D(Form("hClusLocalPhiBits_%u",isect),Form("Local #phi bits for sector %u",isect), 1000,-1000,1000);
    hClusLocalPhi[isect] = new TH1D(Form("hClusLocalPhi_%u",isect),Form("Local #phi for sector %u",isect), 200*TMath::TwoPi(),-1.0*TMath::TwoPi(),TMath::TwoPi());
    hClusGlobalPhi[isect] = new TH1D(Form("hClusGlobalPhi_%u",isect),Form("Global #phi for sector %u",isect), 200*TMath::TwoPi(),-1.0*TMath::TwoPi(),TMath::TwoPi());
    
    hClusLocalEtaBits[isect] = new TH1D(Form("hClusLocalEtaBits_%u",isect),Form("Local #eta bits for sector %u",isect), 5000, 0, 5000);
    hClusGlobalEta[isect] = new TH1D(Form("hClusGlobalEta_%u",isect),Form("Global #eta for sector %u",isect), 200*TMath::TwoPi(),-2.0*TMath::TwoPi(),2.0*TMath::TwoPi());
    
    hTCRoZ2Eta[isect] = new TH2D(Form("hTCRoZ2Eta_%u",isect),Form("TC RoZ-vs-#eta for sector %u",isect),600, 0.0, 0.6, 1000, 0.0,1000.0);
    hTCRoZ2CalcEta[isect] = new TH2D(Form("hTCRoZ2CalcEta_%u",isect),Form("TC-Calculated RoZ-vs-#eta for sector %u",isect),600, 0.0, 0.6, 1000, 0.0,1000.0);

    hTCRozBits[isect] = new TH1D(Form("hTCRozBits_%u",isect),Form("Local RoZ bits for sector %u",isect), 1000,0,10000);
  }

  
  const auto default_precision{std::cout.precision()};
  
  // std::string board_config = "../hgcal-tpg-fe-data/local-input/stage2/firmware-data/vbf_Captures/CaptureStage2_250404_1118/Stage2Configuration.yaml" ;
  // TPGStage2Configuration::Stage2Board sb;
  // sb.readConfigYaml(board_config.c_str());
  // sb.print();
  
  TPGStage2Configuration::ClusPropLUT cplut;
  cplut.readMuEtaLUT("input/stage2/configuration/mean_eta_LUT.csv");
  cplut.readSigmaEtaLUT("input/stage2/configuration/sigma_eta_LUT.csv");

  TPGStage2Emulation::Stage2 s2Clustering;
  s2Clustering.setClusPropLUT(&cplut);
  //s2Clustering.setConfiguration(&sb);
  
  //TPGTriggerCellFloats tcf0,tcf1;
  TPGTCFloats tcf0,tcf1;
  for (Long64_t ievent = 0 ; ievent < nofEvents ; ievent++ ) {
    
    tr->GetEntry(ievent) ;
    
    if(doPrint) std::cout<<"Event : "<< ievent <<", nof gen "<< gen_n << ", nof_gen: " << genpart_pt->size()  <<", TCs : "<< tc_pt->size() << std::endl;
    if(ievent%1==0) std::cout<<"Event : "<< ievent <<", nof TCs : "<< tc_pt->size() << std::endl;
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::vector<int> taudlist,taugdlist;
    for(int igen=0; igen<gen_n; igen++ ){
      TParticlePDG *partPDG = TDatabasePDG::Instance()->GetParticle(gen_pdgid->at(igen));
      std::string gennom = (!partPDG)?"unknown":partPDG->GetName() ;
      std::string daughterlist = "(";
      for(int const& idx: gen_daughters->at(igen)) {
  	daughterlist += std::to_string(idx);
  	daughterlist += ",";
  	if(gennom.find("       pi+")!=std::string::npos) taudlist.push_back(idx);
  	if(taudlist.size()>0 and std::find(taudlist.begin(),taudlist.end(),igen)!=taudlist.end()) taugdlist.push_back(idx);
      }
      daughterlist += ")";
      
      if(doPrint
	 // and
  	 // (gennom.find("h0")!=std::string::npos
  	 // or
  	 // gennom.find("tau")!=std::string::npos
  	 // or
  	 // (taudlist.size()>0 and std::find(taudlist.begin(),taudlist.end(),igen)!=taudlist.end())
  	 // or
  	 // (taugdlist.size()>0 and std::find(taugdlist.begin(),taugdlist.end(),igen)!=taugdlist.end()))
  	 ){
  	std::cout << "gen-ievent: " << std::setprecision(default_precision) << std::setw(4) <<ievent << ", igen: " << std::setw(4) << igen
  		  <<", pid: " << std::setw(5) << gen_pdgid->at(igen) << ", status: " << std::setw(5) << gen_status->at(igen)
  		  <<", Name: " << std::setw(10) << gennom.data() //((!partPDG)?"unknown":partPDG->GetName())
  		  <<" gen:(pt,eta,phi,energy) : (" << std::fixed << std::setprecision(2) << std::setw(10) << gen_pt->at(igen)
  		  << ", " << std::setw(10) << gen_eta->at(igen) << ", " << std::setw(10) << (TMath::RadToDeg()*gen_phi->at(igen)) << ", " << std::setw(10) << gen_energy->at(igen) << ") "
  		  <<" daughters: " << (gen_daughters->at(igen)).size()
  		  << ", " << daughterlist
		  << ", taudlist.size(): " << taudlist.size() 
  		  << std::defaultfloat
  		  << std::endl;
      }//print condition
    }//gen loop
    
    std::vector<JetPart> partlist;
    for(int ipart=0; ipart<genpart_pt->size(); ipart++ ){
      TParticlePDG *partPDG = TDatabasePDG::Instance()->GetParticle(genpart_pid->at(ipart));
      // if(
      // 	 (taudlist.size()>0 )//and std::find(taudlist.begin(),taudlist.end(),(genpart_gen->at(ipart)-1))!=taudlist.end())
      // 	 or
      // 	 (taugdlist.size()>0)//and std::find(taugdlist.begin(),taugdlist.end(),(genpart_gen->at(ipart)-1))!=taugdlist.end())
      // 	 ){
	JetPart p;
	p.name = (!partPDG)?"unknown":partPDG->GetName();
	p.index = ipart;
	partlist.push_back(p);
	if(doPrint)
	  std::cout << "genpart-ievent: " << std::setprecision(default_precision) << std::setw(4) <<ievent << ", ipart: " << std::setw(4) << ipart
		    <<", pid: " << std::setw(5) << genpart_pid->at(ipart) << ", mother: " << std::setw(4) << genpart_mother->at(ipart)
	    //<<", status: " << std::setw(5) << gen_status->at(ipart)
		    <<", gen: " << std::setw(4) << (genpart_gen->at(ipart)-1) //<< ", fromBeamPipe: " << std::setw(4) << genpart_fromBeamPipe->at(ipart)
		    <<", Name: " << std::setw(10) << ((!partPDG)?"unknown":partPDG->GetName())
		    <<" part:(pt,eta,phi,energy) : (" << std::fixed << std::setprecision(2) << std::setw(8) << genpart_pt->at(ipart)
		    << ", " << std::setw(8) << genpart_eta->at(ipart) << ", " //<< std::setw(5) << genpart_exeta->at(ipart)
		    << ", " << std::setw(8) << (TMath::RadToDeg()*genpart_phi->at(ipart)) //<< ", " << std::setw(5) << (TMath::RadToDeg()*genpart_exphi->at(ipart))
		    << ", " << std::setw(8) << genpart_energy->at(ipart) << ") "
		    << std::defaultfloat
		    << std::endl;
      // }
    }//genpart loop

    
    std::vector<JetPart> jetlist;
    for(int ijet=0; ijet<genjet_n; ijet++ ){
      JetPart jet;
      // jet.name = "";
      // jet.index = ijet ;
      // jetlist.push_back(jet);
      
      float minDeltaR = 1.0; int minDeltaRindex = -1;
      for(int ipart=0; ipart<partlist.size(); ipart++){
	int refpart = partlist.at(ipart).index;
	double deltaR = TMath::Sqrt((genpart_eta->at(refpart)-genjet_eta->at(ijet))*(genpart_eta->at(refpart)-genjet_eta->at(ijet)) + (genpart_phi->at(refpart)-genjet_phi->at(ijet))*(genpart_phi->at(refpart)-genjet_phi->at(ijet)));
	if(deltaR<0.4 and deltaR<minDeltaR){
	  minDeltaR = deltaR;
	  minDeltaRindex = ipart;
	}
      }
      for(int ipart=0; ipart<partlist.size(); ipart++){
	JetPart p = partlist.at(ipart);
	if(ipart==minDeltaRindex){
	  JetPart jet;
	  jet.name = p.name;
	  jet.index = ijet ;
	  jetlist.push_back(jet);
	  if(doPrint)
	    std::cout << "ijet-ievent: " << std::setprecision(default_precision) << std::setw(4) <<ievent << ", ijet: " << std::setw(4) << ijet
		      <<", Name: " << std::setw(10) << p.name << ", deltaR : " << std::setprecision(3) << std::setw(8) << minDeltaR
		      <<" jet:(pt,eta,phi,energy) : (" << std::fixed << std::setprecision(2) << std::setw(8) << genjet_pt->at(ijet)
		      << ", " << std::setw(8) << genjet_eta->at(ijet) << ", " //<< std::setw(5) << genjet_exeta->at(ijet)
		      << ", " << std::setw(8) << (TMath::RadToDeg()*genjet_phi->at(ijet)) //<< ", " << std::setw(5) << (TMath::RadToDeg()*genjet_exphi->at(ijet))
		      << ", " << std::setw(8) << genjet_energy->at(ijet) << ") "
		      << std::defaultfloat
		      << std::endl;
	}
      }//part condition
    }//jet loop
    /////////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////////
    float tot_tc_pt = 0.0, tot_tc_e = 0.0;
    //std::vector<TPGTriggerCellWord> vTcw[6];
    std::vector<TPGTCBits> vTcw[6];
    for(unsigned itc=0;itc<tc_pt->size();itc++){
      double z(fabs(tc_z->at(itc)));
      float phi_deg = TMath::RadToDeg() * tc_phi->at(itc) ;
      uint16_t sec0(7),sec1(7);
      
      //The following segmentation is applied for trigger cells
      if(phi_deg>=0. and phi_deg<=60.) {sec0 = 2; sec1 = 0;}
      else if(phi_deg>60. and phi_deg<=120.) {sec0 = sec1 = 0;}
      else if(phi_deg>120. and phi_deg<=180.) {sec0 = 0; sec1 = 1;}
      else if(phi_deg>=-180. and phi_deg<=-120.) {sec0 = sec1 = 1;}
      else if(phi_deg>-120. and phi_deg<=-60.) {sec0 = 1; sec1 = 2;}
      else {sec0 = sec1 = 2;}
      
      if(tc_z->at(itc)<0.){
	switch(sec0){
	case 0:
	  sec0 += 3;
	  break;
	case 1:
	  sec0 += 4;
	  break;
	case 2:
	  sec0 += 2;
	  break;
	default:
	  ;
	}
	switch(sec1){
	case 0:
	  sec1 += 3;
	  break;
	case 1:
	  sec1 += 4;
	  break;
	case 2:
	  sec1 += 2;
	  break;
	default:
	  ;
	}	
      }
      
      if(doPrint){
	std::cout << "itc-ievent: " << std::fixed << std::setprecision(2) << std::setw(4) << ievent
		  << ", itc: " << std::setw(5) << itc
		  << ", layer: " << std::setw(2) << int(tc_layer->at(itc))
		  <<", (x,y,z): (" << std::setw(7) << tc_x->at(itc) << ", " << std::setw(7) << tc_y->at(itc) << ", " << std::setw(7) << tc_z->at(itc) << ")"
		  <<", (pt,eta,phi,energy): (" << std::fixed << std::setprecision(2) << std::setw(8) << tc_pt->at(itc)
		  << ", " << std::setw(8) << tc_eta->at(itc) 
		  << ", " << std::setw(8) << (TMath::RadToDeg()*tc_phi->at(itc)) 
		  << ", " << std::setw(8) << tc_energy->at(itc)*1.e6 << ") "
		  << std::defaultfloat
		  << std::endl;
      }
      tcxyAll->Fill(tc_x->at(itc),tc_y->at(itc));
      tcxy[sec0]->Fill(tc_x->at(itc),tc_y->at(itc));      
      if(sec0!=sec1) tcxy[sec1]->Fill(tc_x->at(itc),tc_y->at(itc));	
      
      for (uint32_t isect = 0 ; isect < 3 ; isect++ ){
	uint32_t addisect = 0;
	if(tc_z->at(itc)>0.0) addisect = 3;
	tcf0.setZero();
	tcf0.setROverZPhiF(tc_x->at(itc)/z,tc_y->at(itc)/z,isect+addisect);
	if(tcf0.getXOverZF()>=0.0){
	  float scale = 1.0;
	  if(tc_layer->at(itc)==27 or tc_layer->at(itc)==29) scale = 1.5;
	  tcf0.setEnergyGeV(scale * tc_pt->at(itc));
	  tcf0.setLayer(tc_layer->at(itc));
	  //if(doPrint) tcf0.print();
	  vTcw[isect+addisect].push_back(tcf0);

	  hTCLocalPhiBits[(isect+addisect)]->Fill(tcf0.getPhi());
	  hTCRoZ2Eta[(isect+addisect)]->Fill(tcf0.getROverZF(), tc_eta->at(itc)*720/acos(-1));
	  hTCRoZ2CalcEta[(isect+addisect)]->Fill(tcf0.getROverZF(), asinh(1/tcf0.getROverZF())*720/acos(-1));

	  hTCRozBits[(isect+addisect)]->Fill(tcf0.getROverZ());
	}
	if((isect+addisect)==0 and tcf0.getXOverZF()>=0.0)
	  hTCPhiCorr->Fill((TMath::RadToDeg()*tc_phi->at(itc)), (TMath::RadToDeg() * tcf0.getPhiF()));
	
	// if((isect+addisect)==0)
	//   hTCLocalPhiBits[(isect+addisect)]->Fill(tcf0.getPhi());
      }
      
      tot_tc_pt += tc_pt->at(itc);
      tot_tc_e += tc_energy->at(itc);
      hPhi->Fill(tc_phi->at(itc));
      hPhiDeg->Fill(phi_deg);
    }//end of TC loop

    
    //if(doPrint)
    //std::cout<<"tot_tc_pt : "<< tot_tc_pt << std::endl;
    for (uint32_t isect = 0 ; isect < 6 ; isect++ ){
      //std::cout << "ievent : " << ievent << ", isect: " << isect << ", size : " << vTcw[isect].size() << std::endl;
      for(TPGTCFloats const& tcf : vTcw[isect]) tcxyOverZ[isect]->Fill(tcf.getXOverZF(),tcf.getYOverZF());
        //for(TPGTriggerCellFloats const& tcf : vTcw[isect]) tcxyOverZ[isect]->Fill(tcf.getXOverZF(),tcf.getYOverZF());
    }

    
    ///////////////////=========== Emulation ============= ///////////////////////
    //std::vector<TPGClusterData> vCld[6];
    std::vector<TPGCluster> vCld[6];    
    for (uint32_t isect = 0 ; isect < 6 ; isect++ ) {
      s2Clustering.run(vTcw[isect],vCld[isect]);
      if(doPrint) std::cout << isect << ", Size of Tcs: " << vTcw[isect].size() << ", Size of Clusters: " << vCld[isect].size() << std::endl;
    }
    ///////////////////=========== Emulation ============= ///////////////////////
    
    for (uint32_t isect = 0 ; isect < 6 ; isect++ ){
      //for(TPGCluster const& clf : vCld[isect]){
      int iclus = 0;
      for(TPGCluster const& clf : vCld[isect]){
	clusxyOverZ[isect]->Fill(clf.getLocalXOverZF(),clf.getLocalYOverZF());
	clusxy[isect]->Fill(clf.getGlobalXOverZF(isect),clf.getGlobalYOverZF(isect));
	hClusLocalPhi[isect]->Fill(clf.getLocalPhiRad());
	hClusGlobalPhi[isect]->Fill(clf.getGlobalPhiRad(isect));
	hClusLocalPhiBits[isect]->Fill(clf.getLocalPhi());
	hClusLocalEtaBits[isect]->Fill(clf.getEta());
	hClusGlobalEta[isect]->Fill(clf.getGlobalEtaRad(isect));
	if(doPrint){
	  std::cout << "itc-ievent: " << std::fixed << std::setprecision(2) << std::setw(4) << ievent
		    << ", sector: " << std::setw(5) << isect
		    << ", iclus: " << std::setw(5) << iclus++
		    <<", (x,y,z): (" << std::setw(7) << (clf.getLocalXOverZF() * clf.getZCm()) << ", " << std::setw(7) << (clf.getLocalYOverZF() * clf.getZCm()) << ", " << std::setw(7) << clf.getZCm() << ")"
		    <<", (pt,eta,phi): (" << std::fixed << std::setprecision(2) << std::setw(8) << clf.getEnergyGeV()
		    << ", " << std::setw(8) << clf.getGlobalEtaRad(isect)
		    << ", " << std::setw(8) << (TMath::RadToDeg()* clf.getGlobalPhiRad(isect))
	            //<< ", " << std::setw(8) << tc_energy->at(itc)*1.e6
		    << ") "
		    << std::defaultfloat
		    << std::endl;
	}
	
      }
    }
    
    for(int ipjet=0; ipjet < jetlist.size() ; ipjet++ ){
      int ijet = jetlist.at(ipjet).index ;
      //if(fabs(genjet_eta->at(ijet))>2.82 or fabs(genjet_eta->at(ijet))<1.72) continue;
      double tot_clus_pt_deltaR = 0;
      double tot_clus_pt_deltaR_0p4 = 0;
      double tot_clus_pt_deltaR_0p2 = 0;
      int minsect = (genjet_eta->at(ijet) < 0) ? 0 : 3;
      int maxsect = (genjet_eta->at(ijet) < 0) ? 3 : 6;
      for (uint32_t isect = minsect ; isect < maxsect ; isect++ ){
	for(TPGCluster const& clf : vCld[isect]){
	  hClusE->Fill(clf.getEnergyGeV());
	  double deltaR = TMath::Sqrt((clf.getGlobalEtaRad(isect)-genjet_eta->at(ijet))*(clf.getGlobalEtaRad(isect)-genjet_eta->at(ijet))
				      +
				      (clf.getGlobalPhiRad(isect)-genjet_phi->at(ijet))*(clf.getGlobalPhiRad(isect)-genjet_phi->at(ijet)));
	  if(deltaR<0.07 ){
	  //if(fabs(clf.getGlobalEtaRad(isect)-genjet_eta->at(ijet))<0.05 and fabs(clf.getGlobalPhiRad(isect) - genjet_phi->at(ijet))<0.05){
	    double emfrac = clf.getCeeFractionF();	    
	    hGenClusE_1->Fill(genjet_pt->at(ijet), (emfrac+2-2*emfrac)*clf.getEnergyGeV());
	    hGenClusE_2->Fill(genjet_pt->at(ijet), (emfrac+3-3*emfrac)*clf.getEnergyGeV());
	    hGenClusE_3->Fill(genjet_pt->at(ijet), (emfrac+4-4*emfrac)*clf.getEnergyGeV());
	    hGenClusE_4->Fill(genjet_pt->at(ijet), (emfrac+5-5*emfrac)*clf.getEnergyGeV());
	    hGenClusE_5->Fill(genjet_pt->at(ijet), (emfrac+6-6*emfrac)*clf.getEnergyGeV());
	    hPtCorrGenjetvsClus_1->Fill(genjet_pt->at(ijet), (emfrac+2-2*emfrac)*clf.getEnergyGeV());
	    hPtCorrGenjetvsClus_2->Fill(genjet_pt->at(ijet), (emfrac+3-3*emfrac)*clf.getEnergyGeV());
	    hPtCorrGenjetvsClus_3->Fill(genjet_pt->at(ijet), (emfrac+4-4*emfrac)*clf.getEnergyGeV());
	    hPtCorrGenjetvsClus_4->Fill(genjet_pt->at(ijet), (emfrac+5-5*emfrac)*clf.getEnergyGeV());
	    hPtCorrGenjetvsClus_5->Fill(genjet_pt->at(ijet), (emfrac+6-6*emfrac)*clf.getEnergyGeV());
	    tot_clus_pt_deltaR += clf.getEnergyGeV();
	    //if(jetlist.at(ipjet).name.find("pi")!=std::string::npos) hPtCorrGenjetvsClus_pion->Fill(genjet_pt->at(ijet), clf.getEnergyGeV());
	    //if(jetlist.at(ipjet).name.find("e")!=std::string::npos) hPtCorrGenjetvsClus_electron->Fill(genjet_pt->at(ijet), clf.getEnergyGeV());
	  }
	  if(deltaR<0.4) tot_clus_pt_deltaR_0p4 += clf.getEnergyGeV() ;
	  if(deltaR<0.2) tot_clus_pt_deltaR_0p2 += clf.getEnergyGeV() ;	  
	  if(clf.getEnergyGeV()>10.0) {
	    hGenClusE->Fill(genjet_pt->at(ijet), clf.getEnergyGeV());
	    hPtReso->Fill( (clf.getEnergyGeV() - genjet_pt->at(ijet)) );
	    hTrigEff->Fill( genjet_pt->at(ijet) );
	    hPtCorrGenjetvsClus->Fill(genjet_pt->at(ijet), clf.getEnergyGeV());
	    deltaGenclus->Fill(clf.getGlobalEtaRad(isect)-genjet_eta->at(ijet), clf.getGlobalPhiRad(isect) - genjet_phi->at(ijet));
	    deltaGenclusSeg[isect]->Fill(clf.getGlobalEtaRad(isect)-genjet_eta->at(ijet), clf.getGlobalPhiRad(isect) - genjet_phi->at(ijet));
	  }
	}	
      }//isect loop
      if(jetlist.at(ipjet).name.find("pi")!=std::string::npos) {
	hPtCorrGenjetvsClus_pion->Fill(genjet_pt->at(ijet), tot_clus_pt_deltaR);
	hPtCorrGenjetvsClus_pion_0p2->Fill(genjet_pt->at(ijet), tot_clus_pt_deltaR_0p2);
	hPtCorrGenjetvsClus_pion_0p4->Fill(genjet_pt->at(ijet), tot_clus_pt_deltaR_0p4);
      }
      if(jetlist.at(ipjet).name.find("e")!=std::string::npos) hPtCorrGenjetvsClus_electron->Fill(genjet_pt->at(ijet), tot_clus_pt_deltaR);
      
      double tot_tc_pt_deltaR = 0;
      double tot_tc_pt_deltaR_0p4 = 0;
      double tot_tc_pt_deltaR_0p2 = 0;
      for(unsigned itc=0;itc<tc_pt->size();itc++){
	deltaGentc->Fill(tc_eta->at(itc) - genjet_eta->at(ijet), tc_phi->at(itc) - genjet_phi->at(ijet));
	double deltaR = TMath::Sqrt((tc_eta->at(itc)-genjet_eta->at(ijet))*(tc_eta->at(itc)-genjet_eta->at(ijet))
				    +
				    (tc_phi->at(itc)-genjet_phi->at(ijet))*(tc_phi->at(itc)-genjet_phi->at(ijet)));
	//double theta = 2*TMath::ATan(TMath::Exp(-1.*tc_eta->at(itc)));
	double correction = 1000*tc_pt->at(itc)/(tc_data->at(itc)/std::cosh(tc_eta->at(itc)));
	//std::cout <<"pt : " << tc_pt->at(itc) << ", correction : " << correction << std::endl;
	if(deltaR<0.07 ){ //0.3 works for no pileup case
	  tot_tc_pt_deltaR += tc_pt->at(itc) ;
	}
	if(deltaR<0.4) tot_tc_pt_deltaR_0p4 += tc_pt->at(itc) ;
	if(deltaR<0.2) tot_tc_pt_deltaR_0p2 += tc_pt->at(itc) ;
	hTCLayerEWt->Fill(tc_layer->at(itc),tc_pt->at(itc)/tot_tc_pt);
	if(jetlist.at(ipjet).name.find("pi")!=std::string::npos) {
	  hTCLayerEWt_pion->Fill(tc_layer->at(itc),tc_pt->at(itc)/tot_tc_pt);
	  hPtEta_TC_pion->Fill(tc_eta->at(itc),tc_pt->at(itc));
	  hECorrc_pion->Fill(tc_layer->at(itc),correction);
	  hECorrc_pion_HT->Fill(tc_layer->at(itc),correction);
	}
	if(jetlist.at(ipjet).name.find("e")!=std::string::npos) {
	  hTCLayerEWt_electron->Fill(tc_layer->at(itc),tc_pt->at(itc)/tot_tc_pt);
	  hECorrc_electron->Fill(tc_layer->at(itc),correction);
	}
      }//tc loop
      hPtCorrGenjetvsTC->Fill(genjet_pt->at(ijet),tot_tc_pt_deltaR);
      if(jetlist.at(ipjet).name.find("pi")!=std::string::npos) {
	hPtCorrGenjetvsTC_pion->Fill(genjet_pt->at(ijet), tot_tc_pt_deltaR);
	hPtCorrGenjetvsTC_pion_0p2->Fill(genjet_pt->at(ijet), tot_tc_pt_deltaR_0p2);
	hPtCorrGenjetvsTC_pion_0p4->Fill(genjet_pt->at(ijet), tot_tc_pt_deltaR_0p4);
      }
      if(jetlist.at(ipjet).name.find("e")!=std::string::npos) hPtCorrGenjetvsTC_electron->Fill(genjet_pt->at(ijet), tot_tc_pt_deltaR);
      
    }//jet from decay of pions from tau
    
    hEt->Fill(tot_tc_pt);
    
    for(uint16_t i=0;i<6;i++) {
      vCld[i].clear();
      vTcw[i].clear();
    }
    
    taudlist.clear();
    taugdlist.clear();
    partlist.clear();
    jetlist.clear();

    
    gen_pt->clear();
    gen_eta->clear();
    gen_phi->clear();
    gen_pdgid->clear();
    gen_energy->clear();
    gen_charge->clear();
    gen_status->clear();
    for(uint32_t i=0;i<gen_daughters->size();i++) (gen_daughters->at(i)).clear();
    gen_daughters->clear();
    genpart_pt->clear();
    genpart_eta->clear();
    genpart_phi->clear();
    genpart_pid->clear();
    genpart_mother->clear();
    genpart_energy->clear();
    genpart_gen->clear();
    genpart_fromBeamPipe->clear();
    genpart_exeta->clear();
    genpart_exphi->clear();
    genjet_pt->clear();
    genjet_eta->clear();
    genjet_phi->clear();
    genjet_energy->clear();
    tc_pt->clear();
    tc_eta->clear();
    tc_phi->clear();
    tc_layer->clear();
    tc_x->clear();
    tc_y->clear();
    tc_z->clear();
    tc_energy->clear();
    tc_data->clear();
    tc_uncompressedCharge->clear();
    
  }//event loop
  
  std::string outname = "stage2SemiEmulator_" + index;
  // hEt->SetTitle(outname.c_str());
  // hEt->GetXaxis()->SetTitle("total p_{T} trigger cells (in GeV)");
  // TCanvas *c1 = new TCanvas("c1",outname.c_str());
  // hEt->Draw();
  // c1->Update();
  
  //hTrigEff->Scale(1.0/float(nofEvents));
  //hTrigEff->Scale(200.0/hTrigEff->GetEntries());
  float eventsPerPt = 50;
  hTrigEff->Scale(1.0/float(eventsPerPt));
  TFile *fout = new TFile(Form("%s.root",outname.c_str()),"recreate");
  hEt->Write();
  hPtReso->Write();
  hTrigEff->Write();
  hPhi->Write();
  hPhiDeg->Write();
  hTCPhiCorr->Write();
  tcxyAll->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) tcxy[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) tcxyOverZ[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) clusxyOverZ[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) clusxy[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) hClusLocalPhi[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) hClusGlobalPhi[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) hClusLocalPhiBits[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) hClusLocalEtaBits[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) hTCLocalPhiBits[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) hClusGlobalEta[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) hTCRoZ2Eta[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) hTCRoZ2CalcEta[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) hTCRozBits[isect]->Write();
  
  deltaGenclus->Write();
  deltaGentc->Write();
  hClusE->Write();
  hGenClusE->Write();
  hGenClusE_1->Write();
  hGenClusE_2->Write();
  hGenClusE_3->Write();
  hGenClusE_4->Write();
  hGenClusE_5->Write();
  hPtCorrGenjetvsClus_1->Write();
  hPtCorrGenjetvsClus_2->Write();
  hPtCorrGenjetvsClus_3->Write();
  hPtCorrGenjetvsClus_4->Write();
  hPtCorrGenjetvsClus_5->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) deltaGenclusSeg[isect]->Write();
  hPtCorrGenjetvsTC->Write();
  hPtCorrGenjetvsClus->Write();
  hPtCorrGenjetvsTC_pion->Write();
  hPtCorrGenjetvsTC_pion_0p2->Write();
  hPtCorrGenjetvsTC_pion_0p4->Write();
  hPtCorrGenjetvsClus_pion->Write();
  hPtCorrGenjetvsClus_pion_0p2->Write();
  hPtCorrGenjetvsClus_pion_0p4->Write();
  hPtCorrGenjetvsTC_electron->Write();
  hPtCorrGenjetvsClus_electron->Write();
  hTCLayerEWt->Write();
  hTCLayerEWt_pion->Write();
  hTCLayerEWt_electron->Write();
  hPtEta_TC_pion->Write();
  hECorrc_pion->Write();
  hECorrc_electron->Write();
  hECorrc_pion_HT->Write();
  fout->Close();
  delete fout;

  // fin->Close();
  // delete fin;

  return true;
}
