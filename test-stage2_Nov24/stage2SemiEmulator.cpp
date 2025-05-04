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
#include "TEfficiency.h"

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
  TPGLSBScales::TPGStage2ClusterLSB lsbScales;
  lsbScales.print();  
  // return true;
  
  bool doPrint = 0;
  float pt_clusThresh = 10.; //GeV
  float pt_TCThresh = 10.; //GeV
  std::cout << "========= Run as : ./stage2SemiEmulator.exe $input_file $index $nofevents ==========" << std::endl;
  
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
  
  float sidelength = 0.016;
  if(argc >= 5){
    std::istringstream issSidelength(argv[4]);
    issSidelength >> sidelength;
  }
  
  std::string outputfile_extn = "";
  if(argc >= 6){
    std::istringstream issOutputfileExtn(argv[5]);
    issOutputfileExtn >> outputfile_extn;
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

  float vtx_z;
  tr->SetBranchStatus("vtx_z",1);
  tr->SetBranchAddress("vtx_z" , &vtx_z);
  
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
  
  TH1D *hEt = new TH1D("hEt","hEt",100,0,200);
  TH1D *hPhi = new TH1D("hPhi","hPhi",2*TMath::TwoPi(),-1.0*TMath::TwoPi(),TMath::TwoPi());
  TH1D *hPhiDeg = new TH1D("hPhiDeg","hPhiDeg",2*TMath::RadToDeg()*TMath::TwoPi(),-1.0*TMath::RadToDeg()*TMath::TwoPi(),TMath::RadToDeg()*TMath::TwoPi());
  TH1D *hPtReso = new TH1D("hPtReso","Energy resolution",200,-100.,100.0);
  TH1D *hPtResoGenTC = new TH1D("hPtResoGenTC","Energy resolution (TC-genJet)",200,-100.,100.0);
  TH1D *hPtResoTCClus = new TH1D("hPtResoTCClus","Energy resolution (Clus-TC)",200,-100.,100.0);
  TH1D *hTrigGen = new TH1D("hTrigGen","Trigger Gen",200, 0.,200.);
  TH1D *hTrigSelGen = new TH1D("hTrigSelGen","Selected Trigger Gen",200, 0.,200.);
  TH1D *hTrigClus = new TH1D("hTrigClus","Trigger Clus",200, 0.,200.);
  TH1D *hLayerE = new TH1D("hLayerE","Layer energy distribution",50, -0.5, 49.5);
  //TH1D *hgenRoZ = new TH1D("hgenRoZ","genJet RoZ",200, 0.,200.);
  TH1D *hDeltaRGenClus = new TH1D("hDeltaRGenClus","hDeltaRGenClus",500,0,1.0);
  // TH1D *hDeltaRGenTC = new TH1D("hDeltaRGenTC","hDeltaRGenTC",200,0,1.0);
  // TH1D *hDeltaRTCClus = new TH1D("hDeltaRTCClus","hDeltaRTCClus",200,0,1.0);
  TH2D *hDeltaRGCPt = new TH2D("hDeltaRGCPt","hDeltaRGCPt",100,0,200.0, 200,0,0.5);
  TH2D *hDeltaRGCEta = new TH2D("hDeltaRGCEta","hDeltaRGCEta",100,1.5,3.1, 200,0,0.5);
  TProfile *hDeltaRGCPtProf = new TProfile("hDeltaRGCPtProf","#DeltaR vs genJet-p_{T}",100,0.,200.,0.,1.0);
  TProfile *hDeltaRGCEtaProf = new TProfile("hDeltaRGCEtaProf","#DeltaR vs genJet-#eta",100,1.5,3.1,0.,1.0);
  
  uint64_t totalEntries = tr->GetEntries();
  if(nofEvents==0 or nofEvents>totalEntries) nofEvents = totalEntries;
  
  std::cout << "Total Entries : " << totalEntries << std::endl;
  std::cout << "Loop for: " << nofEvents << " entries"  << std::endl;

  TH2D *tcxy[6],*tcxyOverZ[6],*clusxyOverZ[6],*clusxy[6],*deltaGenclusSeg[6],*deltaTCclusXYoZSeg[6],*deltaGenclusXYoZSeg[6],*deltaGenTCXYoZSeg[6],*genJetXYoZ[6],*tcAvgXYoZ[6],*clusXYoZ[6];
  TH2D *deltaGenClusDRoZRDPhioZ[6], *deltaGenTCDRoZRDPhioZ[6], *deltaTCClusDRoZRDPhioZ[6], *deltaGenClusRotatedDRoZRDPhioZ[6];
  TH2D *deltaGenClusDRoZVz[6];
  TH2D *tcxyAll = new TH2D("TCXYAll","x-y distribution of TCs",1000,-500,500,1000,-500,500);
  TH2D *tcDxyozsize = new TH2D("tcDxyozsize","Dx/z-Dyz distribution of TCs w.r.t gen",2000,-10.0,10.0,2000,-10.0,10.0);
  TH1D *tcMaxroz = new TH1D("tcMaxroz",Form("Maximum Maxroz distribution of TCs w.r.t gen (p_{T}>%2.0f)",pt_TCThresh),100,0.0,1.0);
  TH1D *tcRoz = new TH1D("tcRoz",Form("Roz distribution of TCs w.r.t gen"),1000,0.0,1.0);

  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) {
    tcxy[isect] = new TH2D(Form("TCXY_%u",isect),Form("x-y distribution of TCs for sector %u",isect),1000,-500,500,1000,-500,500);
    tcxyOverZ[isect] = new TH2D(Form("TCXYOverZ_%u",isect),Form("x/z and y/z distribution of TCs for sector %u",isect),1000,-0.6,0.6,1000,-0.6,0.6);
    clusxyOverZ[isect] = new TH2D(Form("CLUSXYOverZ_%u",isect),Form("x/z and y/z distribution of clusters for sector %u",isect),1000,-0.6,0.6,1000,-0.6,0.6);
    clusxy[isect] = new TH2D(Form("CLUSXY_%u",isect),Form("x-y distribution of clusters for sector %u",isect),1000,-0.6,0.6,1000,-0.6,0.6);
    deltaGenclusSeg[isect] = new TH2D(Form("deltaGenclus_%u",isect),Form("deltaGenclus_%u",isect),100,-0.25,0.25,100,-0.25,0.25);
    deltaTCclusXYoZSeg[isect] = new TH2D(Form("deltaTCclusXYoZ_%u",isect),Form("deltaTCclusXYoZ_%u",isect),1000,-0.05,0.05,1000,-0.05,0.05);
    deltaGenclusXYoZSeg[isect] = new TH2D(Form("deltaGenclusXYoZ_%u",isect),Form("deltaGenclusXYoZ_%u",isect),1000,-0.05,0.05,1000,-0.05,0.05);
    deltaGenTCXYoZSeg[isect] = new TH2D(Form("deltaGenTCXYoZ_%u",isect),Form("deltaGenTCXYoZ_%u",isect),1000,-0.05,0.05,1000,-0.05,0.05);
    
    deltaGenClusDRoZRDPhioZ[isect] = new TH2D(Form("deltaGenClusDRoZRDPhioZ_%u",isect),Form("deltaGenClusDRoZRDPhioZ_%u",isect),500,-0.025,0.025,500,-0.025,0.025);
    deltaGenTCDRoZRDPhioZ[isect] = new TH2D(Form("deltaGenTCDRoZRDPhioZ_%u",isect),Form("deltaGenTCDRoZRDPhioZ_%u",isect),500,-0.025,0.025,500,-0.025,0.025);
    deltaTCClusDRoZRDPhioZ[isect] = new TH2D(Form("deltaTCClusDRoZRDPhioZ_%u",isect),Form("deltaTCClusDRoZRDPhioZ_%u",isect),500,-0.025,0.025,500,-0.025,0.025);
    deltaGenClusRotatedDRoZRDPhioZ[isect] = new TH2D(Form("deltaGenClusRotatedDRoZRDPhioZ_%u",isect),Form("deltaGenClusRotatedDRoZRDPhioZ_%u",isect),500,-0.055,0.055,500,-0.055,0.055);

    deltaGenClusDRoZVz[isect] = new TH2D(Form("deltaGenClusDRoZVz_%u",isect),Form("deltaGenClusDRoZVz_%u",isect),600,-30.,30.,500,-0.025,0.025);
    
    genJetXYoZ[isect] = new TH2D(Form("genJetXYoZ_%u",isect),Form("genJetXYoZ_%u",isect),100,-1.25,1.25,100,-1.25,1.25);
    tcAvgXYoZ[isect] = new TH2D(Form("tcAvgXYoZ_%u",isect),Form("tcAvgXYoZ_%u",isect),100,-1.25,1.25,100,-1.25,1.25);
    clusXYoZ[isect] = new TH2D(Form("clusXYoZ_%u",isect),Form("clusXYoZ_%u",isect),100,-1.25,1.25,100,-1.25,1.25);
  }
  TH2D *deltaGenclus = new TH2D("deltaGenclusAll","deltaGenclusAll",100,-0.25,0.25,100,-0.25,0.25);
  TH2D *deltaTCclus = new TH2D("deltaTcclusAll","deltaTCclusAll",1000,-0.05,0.05,1000,-0.05,0.05);
  TH2D *deltaGentc = new TH2D("deltaGentc","deltaGentc",1000,-0.05,0.05,1000,-0.05,0.05);
  TH1D *hClusE = new TH1D("hClusE","hClusE",100,0.0,100.0);
  TH2D *hGenClusE = new TH2D("hGenClusE","hGenClusE",200,0.0,200.0,200,0.0,200.0);
  TH2D *hGenClusE_1 = new TH2D("hGenClusE_1","hGenClusE_1",200,0.0,200.0,200,0.0,200.0);
  TH2D *hGenClusE_2 = new TH2D("hGenClusE_2","hGenClusE_2",200,0.0,200.0,200,0.0,200.0);
  TH2D *hGenClusE_3 = new TH2D("hGenClusE_3","hGenClusE_3",200,0.0,200.0,200,0.0,200.0);
  TH2D *hGenClusE_4 = new TH2D("hGenClusE_4","hGenClusE_4",200,0.0,200.0,200,0.0,200.0);
  TH2D *hGenClusE_5 = new TH2D("hGenClusE_5","hGenClusE_5",200,0.0,200.0,200,0.0,200.0);
  TH2D *hGenTCE = new TH2D("hGenTCE","hGenTCE",200,0.0,200.0,200,0.0,200.0);
  TH2D *hTCClusE = new TH2D("hTCClusE","hTCClusE",200,0.0,200.0,200,0.0,200.0);
  
  TProfile *hPtCorrGenjetvsClus = new TProfile("hPtCorrGenjetvsClus","hPtCorr Genjet-vs-Cluster",200,0.,200.,0.,200.);
  TProfile *hPtCorrGenjetvsTC = new TProfile("hPtCorrGenjetvsTC","hPtCorr Genjet-vs-TC",200,0.,200.,0.,200.);
  TProfile *hPtCorrTCvsClus = new TProfile("hPtCorrTCvsClus","hPtCorr TC-vs-Cluster",200,0.,200.,0.,200.);
  TProfile *hPtCorrGenjetvsClus_1 = new TProfile("hPtCorrGenjetvsClus_1","hPtCorr Genjet-vs-Cluster",200,0.,200.,0.,200.);
  TProfile *hPtCorrGenjetvsClus_2 = new TProfile("hPtCorrGenjetvsClus_2","hPtCorr Genjet-vs-Cluster",200,0.,200.,0.,200.);
  TProfile *hPtCorrGenjetvsClus_3 = new TProfile("hPtCorrGenjetvsClus_3","hPtCorr Genjet-vs-Cluster",200,0.,200.,0.,200.);
  TProfile *hPtCorrGenjetvsClus_4 = new TProfile("hPtCorrGenjetvsClus_4","hPtCorr Genjet-vs-Cluster",200,0.,200.,0.,200.);
  TProfile *hPtCorrGenjetvsClus_5 = new TProfile("hPtCorrGenjetvsClus_5","hPtCorr Genjet-vs-Cluster",200,0.,200.,0.,200.);
  
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

  TPGStage2Emulation::Stage2::_rOverZ = sidelength * sqrt(3.0) ;
  TPGStage2Emulation::Stage2 s2Clustering;
  s2Clustering.setClusPropLUT(&cplut);
  //s2Clustering.setConfiguration(&sb);
  s2Clustering.setkpower(0);
  std::cout << " Stage2::_rOverZ : " << s2Clustering.getROverZ() << std::endl;
  
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

    double xoz_posEta = -10., yoz_posEta = -10.;
    double xoz_negEta = -10., yoz_negEta = -10.;
    double pt_posEta =  -1., pt_negEta = -1.;
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

	  double roz = 1/sinh(genjet_eta->at(ijet)) ; //tan(2*atan(exp(-1.0*genjet_eta->at(ijet))));
	  double xoz = roz*cos(genjet_phi->at(ijet));
	  double yoz = roz*sin(genjet_phi->at(ijet));
	  if(genjet_eta->at(ijet)>0){
	    xoz_posEta = xoz; yoz_posEta = yoz;
	    pt_posEta = genjet_pt->at(ijet);
	  }else{
	    xoz_negEta = xoz; yoz_negEta = yoz;
	    pt_negEta = genjet_pt->at(ijet);
	  }
	  
	  if(doPrint)
	    std::cout << "ijet-ievent: " << std::setprecision(default_precision) << std::setw(4) <<ievent << ", ijet: " << std::setw(4) << ijet
		      <<", Name: " << std::setw(10) << p.name << ", deltaR : " << std::setprecision(3) << std::setw(8) << minDeltaR
		      <<" jet:(pt,eta,phi,energy) : (" << std::fixed << std::setprecision(2) << std::setw(8) << genjet_pt->at(ijet)
		      << ", " << std::setw(8) << genjet_eta->at(ijet) << ", " //<< std::setw(5) << genjet_exeta->at(ijet)
		      << ", " << std::setw(8) << (TMath::RadToDeg()*genjet_phi->at(ijet)) //<< ", " << std::setw(5) << (TMath::RadToDeg()*genjet_exphi->at(ijet))
		      << ", " << std::setw(8) << genjet_energy->at(ijet) << ") "
		      <<" jet:(roz,xoz,yoz) : (" << std::fixed << std::setprecision(2) << std::setw(8) << roz
		      <<", " << std::fixed << std::setprecision(2) << std::setw(8) << xoz
		      <<", " << std::fixed << std::setprecision(2) << std::setw(8) << yoz
		      << ") "
		      << std::defaultfloat
		      << std::endl;
	}
      }//part condition
    }//jet loop
    /////////////////////////////////////////////////////////////////////////////////////////

    // std::vector<TCOrder> rearrgdtcs;
    // for(unsigned itc=0;itc<tc_pt->size();itc++){
    //   TCOrder tco;
    //   tco.tcid = itc;
    //   tco.layer = uint32_t(tc_layer->at(itc));
    //   tco.z = tc_z->at(itc);
    //   tco.eta = tc_eta->at(itc);
    //   rearrgdtcs.push_back(tco);
    // }
    // std::vector<TCOrder>::iterator itstart = rearrgdtcs.begin();
    // std::vector<TCOrder>::iterator itstop = rearrgdtcs.end();
    // std::sort(itstart,itstop,comp);
    
    /////////////////////////////////////////////////////////////////////////////////////////
    float tot_tc_pt = 0.0, tot_tc_e = 0.0;
    double totTCpt_posEta = 0.,totTCpt_negEta = 0.;
    uint32_t nofPosEtaTCs = 0, nofNegEtaTCs = 0;
    double tcXoZposEta = 0.,tcYoZposEta = 0., tcXoZnegEta = 0.,tcYoZnegEta = 0.;
    double maxroz_posEta = -1.0, maxroz_negEta = -1.0;
    double tcPhi_posEta = 0.,tcPhi_negEta = 0.;
    //std::vector<TPGTriggerCellWord> vTcw[6];
    std::vector<TPGTCBits> vTcw[6];
    // for(unsigned itco=0;itco<tc_pt->size();itco++){
    //   unsigned itc = rearrgdtcs.at(itco).tcid ; 
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
		  <<", (pt,eta,phi,energy): (" << std::fixed << std::setprecision(2) << std::setw(8) << tc_pt->at(itc)*1.e3
		  << ", " << std::setw(8) << tc_eta->at(itc) 
		  << ", " << std::setw(8) << (TMath::RadToDeg()*tc_phi->at(itc)) 
		  << ", " << std::setw(8) << tc_energy->at(itc)*1.e3 << ") "
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
	  //if(tc_layer->at(itc)==27 or tc_layer->at(itc)==29) scale = 1.5;
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
      if(tc_eta->at(itc)<0){
	totTCpt_negEta += tc_pt->at(itc);
	tcXoZnegEta += tc_pt->at(itc)*tc_x->at(itc)/tc_z->at(itc) ;
	tcYoZnegEta += tc_pt->at(itc)*tc_y->at(itc)/tc_z->at(itc) ;
	tcPhi_negEta += tc_pt->at(itc)*tc_phi->at(itc) ;
	nofNegEtaTCs++;
	tcDxyozsize->Fill( (tc_x->at(itc)/tc_z->at(itc)-xoz_negEta), (tc_y->at(itc)/tc_z->at(itc)-yoz_negEta) );
	double droz = sqrt( (tc_x->at(itc)/tc_z->at(itc)-xoz_negEta)*(tc_x->at(itc)/tc_z->at(itc)-xoz_negEta) + (tc_y->at(itc)/tc_z->at(itc)-yoz_negEta)*(tc_y->at(itc)/tc_z->at(itc)-yoz_negEta) );
	if(droz>maxroz_negEta) maxroz_negEta = droz;
	tcRoz->Fill(droz,tc_pt->at(itc));
      }else{
	totTCpt_posEta += tc_pt->at(itc);
	tcXoZposEta += tc_pt->at(itc)*tc_x->at(itc)/tc_z->at(itc) ;
	tcYoZposEta += tc_pt->at(itc)*tc_y->at(itc)/tc_z->at(itc) ;
	tcPhi_posEta += tc_pt->at(itc)*tc_phi->at(itc) ;
	nofPosEtaTCs++;
	tcDxyozsize->Fill( (tc_x->at(itc)/tc_z->at(itc)-xoz_posEta), (tc_y->at(itc)/tc_z->at(itc)-yoz_posEta) );
	double droz = sqrt( (tc_x->at(itc)/tc_z->at(itc)-xoz_posEta)*(tc_x->at(itc)/tc_z->at(itc)-xoz_posEta) + (tc_y->at(itc)/tc_z->at(itc)-yoz_posEta)*(tc_y->at(itc)/tc_z->at(itc)-yoz_posEta) );
	if(droz>maxroz_posEta) maxroz_posEta = droz;
	tcRoz->Fill(droz,tc_pt->at(itc));
      }
      
      tot_tc_pt += tc_pt->at(itc);
      tot_tc_e += tc_energy->at(itc);
      hPhi->Fill(tc_phi->at(itc));
      hPhiDeg->Fill(phi_deg);
      hLayerE->Fill(tc_layer->at(itc), tc_pt->at(itc));
    }//end of TC loop
    //rearrgdtcs.clear();
    if(pt_negEta>pt_TCThresh) tcMaxroz->Fill(maxroz_negEta,totTCpt_negEta);
    if(pt_posEta>pt_TCThresh) tcMaxroz->Fill(maxroz_posEta,totTCpt_posEta);
    
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
	double tcPtSum = (clf.getGlobalEtaRad(isect) < 0) ? totTCpt_negEta : totTCpt_posEta ;
	if(doPrint){
	  std::cout << "iclus-ievent: " << std::fixed << std::setprecision(2) << std::setw(4) << ievent
		    << ", sector: " << std::setw(5) << isect
		    << ", iclus: " << std::setw(5) << iclus++
		    <<", (x,y,z): (" << std::setw(7) << (clf.getGlobalXOverZF(isect) * clf.getZCm()) << ", " << std::setw(7) << (clf.getGlobalYOverZF(isect) * clf.getZCm()) << ", " << std::setw(7) << clf.getGlobalZCm(isect) << ")"
		    <<", (xoz,yoz,roz): (" << std::setw(5) << clf.getGlobalXOverZF(isect) << ", " << std::setw(5) << clf.getGlobalYOverZF(isect) << ", " << std::setw(5) << clf.getGlobalRhoOverZF(isect) << ")"
		    <<", (pt,eta,phi): (" << std::fixed << std::setprecision(2) << std::setw(8) << clf.getEnergyGeV()
		    << ", " << std::setw(8) << clf.getGlobalEtaRad(isect)
		    << ", " << std::setw(8) << (TMath::RadToDeg()* clf.getGlobalPhiRad(isect))
	            //<< ", " << std::setw(8) << tc_energy->at(itc)*1.e6
		    << ") "
		    <<", (tcPtSum,tcPtSum/lsb,energy,energy*lsb): (" << std::setw(8) << tcPtSum
		    << ", " << std::setw(8) << uint32_t(tcPtSum/lsbScales.LSB_E_TC()+0.5)
		    << ", " << std::setw(8) << clf.getEnergy()
		    << ", " << std::setw(8) << (clf.getEnergy()*lsbScales.LSB_E_TC())
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
      double tcPtSum = (genjet_eta->at(ijet) < 0) ? totTCpt_negEta : totTCpt_posEta ;
      double avgXoZ = (genjet_eta->at(ijet) < 0) ? tcXoZnegEta/tcPtSum : tcXoZposEta/tcPtSum ;
      double avgYoZ = (genjet_eta->at(ijet) < 0) ? tcYoZnegEta/tcPtSum : tcYoZposEta/tcPtSum ;
      double avgPhi = (genjet_eta->at(ijet) < 0) ? tcPhi_negEta/tcPtSum : tcPhi_posEta/tcPtSum ;
      double gjroz = 1/sinh(genjet_eta->at(ijet)) ; //tan(2*atan(exp(-1.0*genjet_eta->at(ijet))));
      double gjxoz = gjroz*cos(genjet_phi->at(ijet));
      double gjyoz = gjroz*sin(genjet_phi->at(ijet));
      double avgRoZ = sqrt((avgXoZ-gjxoz)*(avgXoZ-gjxoz) + (avgYoZ-gjyoz)*(avgYoZ-gjyoz));
      
      hTrigGen->Fill(genjet_pt->at(ijet));
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
	  if(clf.getEnergyGeV()>pt_clusThresh) {
	    //===============
	    hGenClusE->Fill(genjet_pt->at(ijet), clf.getEnergyGeV());
	    hGenTCE->Fill(genjet_pt->at(ijet), tcPtSum);
	    hTCClusE->Fill(tcPtSum, clf.getEnergyGeV());
	    //===============
	    hPtReso->Fill( (clf.getEnergyGeV() - genjet_pt->at(ijet)) );
	    hPtResoGenTC->Fill( (tcPtSum - genjet_pt->at(ijet)) );
	    hPtResoTCClus->Fill( (clf.getEnergyGeV() - tcPtSum) );
	    //===============
	    hPtCorrGenjetvsClus->Fill(genjet_pt->at(ijet), clf.getEnergyGeV());
	    hPtCorrGenjetvsTC->Fill(genjet_pt->at(ijet), tcPtSum);
	    hPtCorrTCvsClus->Fill(tcPtSum, clf.getEnergyGeV());
	    //===============
	    deltaGenclus->Fill(clf.getGlobalEtaRad(isect)-genjet_eta->at(ijet), clf.getGlobalPhiRad(isect) - genjet_phi->at(ijet));
	    deltaGenclusSeg[isect]->Fill(clf.getGlobalEtaRad(isect)-genjet_eta->at(ijet), clf.getGlobalPhiRad(isect) - genjet_phi->at(ijet));
	    deltaGenclusXYoZSeg[isect]->Fill( (clf.getGlobalXOverZF(isect) - gjxoz), (clf.getGlobalYOverZF(isect) - gjyoz) );
	    deltaGenTCXYoZSeg[isect]->Fill( (avgXoZ - gjxoz), (avgYoZ - gjyoz) );
	    deltaTCclusXYoZSeg[isect]->Fill( (clf.getGlobalXOverZF(isect) - avgXoZ), (clf.getGlobalYOverZF(isect) - avgYoZ) );
	    tcAvgXYoZ[isect]->Fill( avgXoZ, avgYoZ );
	    clusXYoZ[isect]->Fill( clf.getGlobalXOverZF(isect), clf.getGlobalYOverZF(isect) );
	    genJetXYoZ[isect]->Fill( (cos(genjet_phi->at(ijet))/sinh(genjet_phi->at(ijet))), (sin(genjet_phi->at(ijet))/sinh(genjet_phi->at(ijet))) ); 
	    //===============
	    deltaGenClusDRoZRDPhioZ[isect]->Fill( gjroz*(clf.getGlobalPhiRad(isect)-genjet_phi->at(ijet)), (clf.getGlobalRhoOverZF(isect) - gjroz)  );
	    deltaGenClusRotatedDRoZRDPhioZ[isect]->Fill( gjroz*(clf.getGlobalPhiRad(isect)-genjet_phi->at(ijet)), (clf.getGlobalRhoOverZF(isect)*clf.getGlobalPhiRad(isect) - gjroz*genjet_phi->at(ijet)) );
	    deltaGenTCDRoZRDPhioZ[isect]->Fill( gjroz*(avgPhi-genjet_phi->at(ijet)), (avgRoZ - gjroz)  );
	    deltaTCClusDRoZRDPhioZ[isect]->Fill( avgRoZ*(clf.getGlobalPhiRad(isect)-avgPhi), (clf.getGlobalRhoOverZF(isect) - avgRoZ)  );
	    //===============	    
	    deltaGenClusDRoZVz[isect]->Fill( vtx_z, (clf.getGlobalRhoOverZF(isect) - gjroz)  );
	    //===============
	    hDeltaRGenClus->Fill(deltaR); 
	    hDeltaRGCPt->Fill(genjet_pt->at(ijet), deltaR); 
	    hDeltaRGCEta->Fill(abs(genjet_eta->at(ijet)), deltaR); 
	    hDeltaRGCPtProf->Fill(genjet_pt->at(ijet), deltaR); 
	    hDeltaRGCEtaProf->Fill(abs(genjet_eta->at(ijet)), deltaR); 
	    //===============
	    hTrigSelGen->Fill(genjet_pt->at(ijet));
	    hTrigClus->Fill(clf.getEnergyGeV());
	  }
	}	
      }//isect loop
      
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
  
  std::string outname = "stage2SemiEmulator_" + outputfile_extn + "_" + index;
  
  TH1D *hTrigEff = (TH1D *) hTrigSelGen->Clone("hTrigEff");
  hTrigEff->Divide(hTrigGen);
  TH1D *hTrigEffClusE = (TH1D *) hTrigClus->Clone("hTrigEffClusE");
  hTrigEffClusE->Divide(hTrigGen);
  
  TH1D *tcMaxroz_cumul = (TH1D *)tcMaxroz->GetCumulative(kTRUE,"_cumul");
  tcMaxroz_cumul->Scale(1./tcMaxroz_cumul->GetBinContent(tcMaxroz_cumul->GetMaximumBin()));
  TH1D *tcRoz_cumul = (TH1D *)tcRoz->GetCumulative(kTRUE,"_cumul");
  tcRoz_cumul->Scale(1./tcRoz_cumul->GetBinContent(tcRoz_cumul->GetMaximumBin()));
  
  TFile *fout = new TFile(Form("%s.root",outname.c_str()),"recreate");
  hEt->Write();
  hPtReso->Write();
  hPtResoGenTC->Write();
  hPtResoTCClus->Write();
  hTrigGen->Write();
  hTrigSelGen->Write();
  hTrigClus->Write();
  hTrigEff->Write();
  hTrigEffClusE->Write();
  hLayerE->Write();
  hPhi->Write();
  hPhiDeg->Write();
  hDeltaRGenClus->Write();
  hDeltaRGCPt->Write();
  hDeltaRGCEta->Write();
  hDeltaRGCPtProf->Write();
  hDeltaRGCEtaProf->Write();
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
  hGenTCE->Write();
  hTCClusE->Write();
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
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) deltaGenclusXYoZSeg[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) deltaGenTCXYoZSeg[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) deltaTCclusXYoZSeg[isect]->Write();
  hPtCorrGenjetvsClus->Write();
  hPtCorrGenjetvsTC->Write();
  hPtCorrTCvsClus->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) genJetXYoZ[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) tcAvgXYoZ[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) clusXYoZ[isect]->Write();
  tcDxyozsize->Write();
  tcMaxroz->Write();
  tcMaxroz_cumul->Write();
  tcRoz->Write();
  tcRoz_cumul->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) deltaGenClusDRoZRDPhioZ[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) deltaGenClusRotatedDRoZRDPhioZ[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) deltaGenTCDRoZRDPhioZ[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) deltaTCClusDRoZRDPhioZ[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) deltaGenClusDRoZVz[isect]->Write();
  fout->Close();
  delete fout;

  // fin->Close();
  // delete fin;

  return true;
}
