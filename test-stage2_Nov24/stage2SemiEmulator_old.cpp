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
#include <TF1.h>
#include <TGraph.h>

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
  bool condn = ((x.layer<y.layer) or ((x.layer==y.layer) and (fabs(diff)>1.e-5)) or ((x.layer==y.layer) and (fabs(diff)<1.e-5) and (fabs(x.eta)<fabs(y.eta))));
  return condn ;
}

bool GetHistoBin(TH2F *h, float eta, float et, int& binX, int& binY)
{
  eta = fabs(eta);
  double minX = h->GetXaxis()->GetBinCenter(1);
  double maxX = h->GetXaxis()->GetBinCenter(h->GetNbinsX());
  double minY = h->GetYaxis()->GetBinCenter(1);
  double maxY = h->GetYaxis()->GetBinCenter(h->GetNbinsY());

  binX = -1;
  if(eta <= minX)
    binX = 1;
  else if(eta >= maxX)
    binX = h->GetXaxis()->FindBin(maxX);
  else
    binX = h->GetXaxis()->FindBin(eta);

  binY = -1;
  if(et <= minY)
    binY = 1;
  else if(et >= maxY)
    binY = h->GetYaxis()->FindBin(maxY);
  else
    binY = h->GetYaxis()->FindBin(et);

  return true;
}

int main(int argc, char** argv)
{
  TPGLSBScales::TPGStage2ClusterLSB lsbScales;
  lsbScales.print();  
  // return true;
  
  bool doPrint = 0;
  float pt_clusThresh = 30.; //GeV electron 30 GeV //pion = 100, VBF = 150 GeV
  float pt_clusThresh_etaeff = 50.; //GeV electron 50 GeV //pion = 150, VBF = 200 GeV
  
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

  std::string sampleType = "";
  if(argc >= 7){
    std::istringstream issSampleType(argv[6]);
    issSampleType >> sampleType;
  }
  
  pt_clusThresh = 30.; //GeV electron 30 GeV //pion = 100, VBF = 150 GeV
  pt_clusThresh_etaeff = 50.; //GeV electron 50 GeV //pion = 150, VBF = 200 GeV
  if(sampleType.find("vbfHInv")!=std::string::npos){
    pt_clusThresh = 150.; 
    pt_clusThresh_etaeff = 200.; 
  }else if(sampleType.find("singlePion")!=std::string::npos){
    pt_clusThresh = 100.; 
    pt_clusThresh_etaeff = 150.; 
  }
  
  float etaMin = 1.321;
  float etaMax = 3.152;

  float innerEtaMin = 1.6;
  float innerEtaMax = 3.0;
  
  std::unique_ptr<TFile> fin( TFile::Open(Form(inputfile.c_str())) );
  //TFile *fin = TFile::Open(Form(inputfile.c_str()));
  std::cout<<"Input: filename : " << fin->GetName() << std::endl;
  std::cout << "Input: outfile index : " << index << std::endl;
  std::cout << "Input: nofEvents : " << nofEvents << std::endl;
  std::cout << "Input: sidelength : " << sidelength << std::endl;
  std::cout << "Input: MC Sample : " << sampleType << std::endl;
  std::cout << "pt_clusThresh for L1 : " << pt_clusThresh << std::endl;
  std::cout << "pt_clusThresh_etaeff for Trigger eta eff : " << pt_clusThresh_etaeff << std::endl;
  int  resoIndex = int(1000*sidelength);
  std::cout << "resoIndex : " << resoIndex << std::endl;
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

  std::vector<float>  *tc_subdet = 0 ;
  tr->SetBranchStatus("tc_subdet",1);
  tr->SetBranchAddress("tc_subdet" , &tc_subdet);
  
  std::vector<float>  *tc_wafertype = 0 ;
  tr->SetBranchStatus("tc_wafertype",1);
  tr->SetBranchAddress("tc_wafertype" , &tc_wafertype);

  std::vector<float>  *tc_uncompressedCharge = 0 ;
  tr->SetBranchStatus("tc_uncompressedCharge",1);
  tr->SetBranchAddress("tc_uncompressedCharge" , &tc_uncompressedCharge);

  Int_t cl3d_n = 0 ;
  tr->SetBranchStatus("cl3d_n",1);
  tr->SetBranchAddress("cl3d_n" , &cl3d_n);

  std::vector<float>  *cl3d_pt = 0 ;
  tr->SetBranchStatus("cl3d_pt",1);
  tr->SetBranchAddress("cl3d_pt" , &cl3d_pt);
  
  std::vector<float>  *cl3d_eta = 0 ;
  tr->SetBranchStatus("cl3d_eta",1);
  tr->SetBranchAddress("cl3d_eta" , &cl3d_eta);
  
  std::vector<float>  *cl3d_phi = 0 ;
  tr->SetBranchStatus("cl3d_phi",1);
  tr->SetBranchAddress("cl3d_phi" , &cl3d_phi);

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
  
  const double effPtBin[29] = {0, 10., 20., 30., 40., 50., 60., 70., 80., 90., 100., 110., 120., 130., 140., 150., 160., 170., 180., 190., 200., 210., 220., 230., 240., 250., 300., 400., 600.};
  
  TF1* f2 = new TF1(Form("func"),"0.01+[0]/(x^[1]-[2])",10,1000);
  f2->SetParameters(-0.00428022,-0.297381,0.6113);

  ////////////////Cluster correction ///////////////////////////
  TF1* fClusCorr = new TF1("fClusCorr","[0]+[1]/(x+[2])",7,500);
  fClusCorr->SetParameters(1.22466, 22.8516, 0.401581); //PU0 VBF

  TF1* fcl3d = new TF1("fcl3d","[0]+[1]/(x+[2])",7,500);
  fcl3d->SetParameters(1.17576, 19.7599, 0.908577); //PU0 VBF
  //////////////////////////////////////////////////////////

  // TGraph *gRzTC = new TGraph();
  // gRzTC->SetName("gRzTC");
  // gRzTC->SetTitle("RZ distribution of TCs");
  
  // TGraph *gRzTCsd1 = new TGraph();
  // gRzTCsd1->SetName("gRzTCsd1");
  // gRzTCsd1->SetTitle("RZ distribution of TCs of subdet 1");
  
  // TGraph *gRzTCsd2 = new TGraph();
  // gRzTCsd2->SetName("gRzTCsd2");
  // gRzTCsd2->SetTitle("RZ distribution of TCs of subdet 2");
  
  // TGraph *gRzTCsd10 = new TGraph();
  // gRzTCsd10->SetName("gRzTCsd10");
  // gRzTCsd10->SetTitle("RZ distribution of TCs of subdet 10");

  // TGraph *gRzTCsd1_100 = new TGraph();
  // gRzTCsd1_100->SetName("gRzTCsd1_100");
  // gRzTCsd1_100->SetTitle("RZ distribution of TCs of subdet 1 with 100#mum wafer");

  // TGraph *gRzTCsd1_200 = new TGraph();
  // gRzTCsd1_200->SetName("gRzTCsd1_200");
  // gRzTCsd1_200->SetTitle("RZ distribution of TCs of subdet 1 with 200#mum wafer");

  // TGraph *gRzTCsd1_300 = new TGraph();
  // gRzTCsd1_300->SetName("gRzTCsd1_300");
  // gRzTCsd1_300->SetTitle("RZ distribution of TCs of subdet 1 with 300#mum wafer");

  // TGraph *gRzTCsd2_100 = new TGraph();
  // gRzTCsd2_100->SetName("gRzTCsd2_100");
  // gRzTCsd2_100->SetTitle("RZ distribution of TCs of subdet 2 with 100#mum wafer");

  // TGraph *gRzTCsd2_200 = new TGraph();
  // gRzTCsd2_200->SetName("gRzTCsd2_200");
  // gRzTCsd2_200->SetTitle("RZ distribution of TCs of subdet 2 with 200#mum wafer");

  // TGraph *gRzTCsd2_300 = new TGraph();
  // gRzTCsd2_300->SetName("gRzTCsd2_300");
  // gRzTCsd2_300->SetTitle("RZ distribution of TCs of subdet 2 with 300#mum wafer");
  
  TH1D *hEt = new TH1D("hEt","hEt",1000,0,1000.);
  TH1D *hPhi = new TH1D("hPhi","hPhi",2*TMath::TwoPi(),-1.0*TMath::TwoPi(),TMath::TwoPi());
  TH1D *hPhiDeg = new TH1D("hPhiDeg","hPhiDeg",2*TMath::RadToDeg()*TMath::TwoPi(),-1.0*TMath::RadToDeg()*TMath::TwoPi(),TMath::RadToDeg()*TMath::TwoPi());
  TH1D *hPtReso = new TH1D("hPtReso","Energy resolution",200,-100.,100.0);
  TH1D *hPtResoInnerEta = new TH1D("hPtResoInnerEta",Form("Energy resolution for inner eta (%3.2f-%3.2f)",innerEtaMin,innerEtaMax),200,-100.,100.0);
  TH1D *hPtResoGenTC = new TH1D("hPtResoGenTC","Energy resolution (TC-genJet)",200,-100.,100.0);
  TH1D *hPtResoTCClus = new TH1D("hPtResoTCClus","Energy resolution (Clus-TC)",200,-100.,100.0);
  
  TH1D *hTrigGen = new TH1D("hTrigGen","Trigger Gen",600, 0.,600.);
  TH1D *hTrigGenPt = new TH1D("hTrigGenPt","Trigger Gen Pt",600, 0.,600.);
  TH1D *hTrigGenEta = new TH1D("hTrigGenEta","Trigger Gen Eta",300, 1., 4.);
  TH1D *hTrigGenEtaInner = new TH1D("hTrigGenEtaInner","Trigger Gen Eta",300, 1., 4.);
  TH1D *hTrigGenEtaOuter = new TH1D("hTrigGenEtaOuter","Trigger Gen Eta",300, 1., 4.);
  TH1D *hTrigGenClst = new TH1D("hTrigGenClst","Trigger eff w.r.t closest",600, 0.,600.);
  TH1D *hTrigGenClstPt = new TH1D("hTrigGenClstPt","Trigger eff w.r.t closest and pt cut",600, 0.,600.);
  TH1D *hTrigGenClstPtTot = new TH1D("hTrigGenClstPtTot","Trigger eff w.r.t closest and total pt cut",600, 0.,600.);
  TH1D *hTrigGenClstEta = new TH1D("hTrigGenClstEta","Trigger eff w.r.t closest and pt plateau cut",300, 1., 4.);
  TH1D *hTrigGenClstEtaInner = new TH1D("hTrigGenClstEtaInner","Trigger eff w.r.t closest and pt plateau cut",300, 1., 4.);
  TH1D *hTrigGenClstEtaOuter = new TH1D("hTrigGenClstEtaOuter","Trigger eff w.r.t closest and pt plateau cut",300, 1., 4.);
  TH1D *hTrigSelGen = new TH1D("hTrigSelGen","Selected Trigger Gen",600, 0.,600.);
  TH1D *hTrigClus = new TH1D("hTrigClus","Trigger Clus",600, 0.,600.);
  
  TEfficiency* effTrigGen = new TEfficiency("effTrigGen","Trigger efficiency;p_{T}(GeV);#epsilon",600,0,600.);
  TEfficiency* effTrigGenClst = new TEfficiency("effTrigGenClst","Trigger efficiency;p_{T}(GeV);#epsilon",600,0,600.);
  TEfficiency* effTrigGenClstPt = new TEfficiency("effTrigGenClstPt","Trigger efficiency;p_{T}(GeV);#epsilon",600,0,600.);
  TEfficiency* effTrigGenClstPtInner = new TEfficiency("effTrigGenClstPtInner","Trigger efficiency;p_{T}(GeV);#epsilon",600,0,600.);
  TEfficiency* effTrigGenClstPtOuter = new TEfficiency("effTrigGenClstPtOuter","Trigger efficiency;p_{T}(GeV);#epsilon",600,0,600.);
  TEfficiency* effTrigGenClstPtTot = new TEfficiency("effTrigGenClstPtTot","Trigger efficiency;p_{T}(GeV);#epsilon",600,0,600.);
  TEfficiency* effTrigGenClstEta = new TEfficiency("effTrigGenClstEta","Trigger efficiency;#eta;#epsilon",300, 1., 4.);
  TEfficiency* effTrigGenClstEtaInner = new TEfficiency("effTrigGenClstEtaInner","Trigger efficiency;#eta;#epsilon",300, 1., 4.);
  TEfficiency* effTrigGenClstEtaOuter = new TEfficiency("effTrigGenClstEtaOuter","Trigger efficiency;#eta;#epsilon",300, 1., 4.);
  
  TEfficiency* effTrigClus = new TEfficiency("effTrigClus","Trigger efficiency;p_{T}(GeV);#epsilon",600,0,600.);
  TEfficiency* effTrigGenPU = new TEfficiency("effTrigGenPU","Trigger efficiency;p_{T}(GeV);#epsilon",28,effPtBin);
  // TEfficiency* effTrigClusPU = new TEfficiency("effTrigClusPU","Trigger efficiency;p_{T}(GeV);#epsilon",300,0,600.);
  // TEfficiency* effTrigGenPUEtOnly = new TEfficiency("effTrigGenPUEtOnly","Trigger efficiency;p_{T}(GeV);#epsilon",300,0,600.);
  // TEfficiency* effTrigClusPUEtOnly = new TEfficiency("effTrigClusPUEtOnly","Trigger efficiency;p_{T}(GeV);#epsilon",300,0,600.);
  TH1D *hLayerE = new TH1D("hLayerE","Layer energy distribution",50, -0.5, 49.5);
  
  //TH1D *hgenRoZ = new TH1D("hgenRoZ","genJet RoZ",200, 0.,200.);
  // TH1D *hDeltaRGenClus = new TH1D("hDeltaRGenClus","hDeltaRGenClus",500,0,0.2);
  // TH1D *hDeltaRGenTC = new TH1D("hDeltaRGenTC","hDeltaRGenTC",200,0,1.0);
  // TH1D *hDeltaRTCClus = new TH1D("hDeltaRTCClus","hDeltaRTCClus",200,0,1.0);
  // TH2D *hDeltaRGCPt = new TH2D("hDeltaRGCPt","hDeltaRGCPt",100,0,200.0, 100,0,0.05);
  // TH2D *hDeltaRGCEta = new TH2D("hDeltaRGCEta","hDeltaRGCEta",100,1.2,3.3, 100,0,0.05);
  // TH2D *hDeltaRGCPhi = new TH2D("hDeltaRGCPhi","hDeltaRGCPhi",100,-4.,4., 100,0,0.05);
  // TH2D *hDeltaRGCPtEta = new TH2D("hDeltaRGCPtEtai","hDeltaRGCPtEta",100,0.,200., 100, 1.2,3.3);
  // TProfile *hDeltaRGCPtProf = new TProfile("hDeltaRGCPtProf","#DeltaR vs genJet-p_{T}",100,0.,200.,0.,0.2);
  // TProfile *hDeltaRGCEtaProf = new TProfile("hDeltaRGCEtaProf","#DeltaR vs genJet-#eta",100,1.2,3.3,0.,0.2);
  // TProfile *hDeltaRGCPhiProf = new TProfile("hDeltaRGCPhiProf","#DeltaR vs genJet-#phi",100,-4.,4.,0.,0.2);
  
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
  TH1D *hclusRoz = new TH1D("hclusRoz",Form("Roz distribution of Clusters w.r.t gen"),1000,0.0,1.0);
  
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
    // deltaGenTCDRoZRDPhioZ[isect] = new TH2D(Form("deltaGenTCDRoZRDPhioZ_%u",isect),Form("deltaGenTCDRoZRDPhioZ_%u",isect),500,-0.025,0.025,500,-0.025,0.025);
    // deltaTCClusDRoZRDPhioZ[isect] = new TH2D(Form("deltaTCClusDRoZRDPhioZ_%u",isect),Form("deltaTCClusDRoZRDPhioZ_%u",isect),500,-0.025,0.025,500,-0.025,0.025);
    deltaGenClusRotatedDRoZRDPhioZ[isect] = new TH2D(Form("deltaGenClusRotatedDRoZRDPhioZ_%u",isect),Form("deltaGenClusRotatedDRoZRDPhioZ_%u",isect),500,-0.055,0.055,500,-0.055,0.055);

    deltaGenClusDRoZVz[isect] = new TH2D(Form("deltaGenClusDRoZVz_%u",isect),Form("deltaGenClusDRoZVz_%u",isect),600,-30.,30.,500,-0.025,0.025);
    
    genJetXYoZ[isect] = new TH2D(Form("genJetXYoZ_%u",isect),Form("genJetXYoZ_%u",isect),100,-1.25,1.25,100,-1.25,1.25);
    //tcAvgXYoZ[isect] = new TH2D(Form("tcAvgXYoZ_%u",isect),Form("tcAvgXYoZ_%u",isect),100,-1.25,1.25,100,-1.25,1.25);
    clusXYoZ[isect] = new TH2D(Form("clusXYoZ_%u",isect),Form("clusXYoZ_%u",isect),100,-1.25,1.25,100,-1.25,1.25);
  }
  TH2D *deltaGenclus = new TH2D("deltaGenclusAll","deltaGenclusAll",200,-0.25,0.25,200,-0.25,0.25);
  //TH2D *deltaTCclus = new TH2D("deltaTcclusAll","deltaTCclusAll",200,-0.25,0.25,200,-0.25,0.25);
  //TH2D *deltaGenTC = new TH2D("deltaGenTCAll","deltaGenTCAll",200,-0.25,0.25,200,-0.25,0.25);
  
  TH1D *hClusE = new TH1D("hClusE","hClusE",1000,0.0,1000.0);
  TH2D *hGenClusE = new TH2D("hGenClusE","hGenClusE",200,0.0,200.0,200,0.0,200.0);
  // TH2D *hGenClusE_1 = new TH2D("hGenClusE_1","hGenClusE_1",200,0.0,200.0,200,0.0,200.0);
  // TH2D *hGenClusE_2 = new TH2D("hGenClusE_2","hGenClusE_2",200,0.0,200.0,200,0.0,200.0);
  // TH2D *hGenClusE_3 = new TH2D("hGenClusE_3","hGenClusE_3",200,0.0,200.0,200,0.0,200.0);
  // TH2D *hGenClusE_4 = new TH2D("hGenClusE_4","hGenClusE_4",200,0.0,200.0,200,0.0,200.0);
  // TH2D *hGenClusE_5 = new TH2D("hGenClusE_5","hGenClusE_5",200,0.0,200.0,200,0.0,200.0);
  TH1D *hGCEDiffClst = new TH1D("hGCEDiffClst","hGCEDiffClst",100,-100.0,100.0);
  TH1D *hGTotCEDiff = new TH1D("hGTotCEDiff","hGTotCEDiff",100,-100.0,100.0);
  TH1D *hclusRozClst = new TH1D("hclusRozClst",Form("Roz distribution of the closest clusters w.r.t gen"),1000,0.0,1.0);
  
  TH2D *hGenTCE = new TH2D("hGenTCE","hGenTCE",200,0.0,200.0,200,0.0,200.0);
  TH2D *hTCClusE = new TH2D("hTCClusE","hTCClusE",200,0.0,200.0,200,0.0,200.0);
  TH1D *hNTCTrigL2 = new TH1D("hNTCTrigL2","nof TCs in HGCAL layer 2",501,-0.5,500.5);
  TH1D *hNTCTrigL9 = new TH1D("hNTCTrigL9","nof TCs in HGCAL layer 9",500,0,5000);
  TH1D *hNClusSel = new TH1D("hNClusSel","nof cluster per endcap",501,-0.5,500.5);
  TH1D *hNClus1GeV = new TH1D("hNClus1GeV","nof cluster per endcap pt>1 GeV",501,-0.5,500.5);
  TH1D *hNClus3GeV = new TH1D("hNClus3GeV","nof cluster per endcap pt>3 GeV",501,-0.5,500.5);
  TH1D *hNClus5GeV = new TH1D("hNClus5GeV","nof cluster per endcap pt>5 GeV",501,-0.5,500.5);
  TH1D *hNClus10GeV = new TH1D("hNClus10GeV","nof cluster per endcap pt>10 GeV",501,-0.5,500.5);
  TH2D *hNClusPt = new TH2D("hNClusPt","hNClusPt",100,0.0,200.0,20,-0.5,19.5);
  TH2D *hNClusEta = new TH2D("hNClusEta","hNClusEta",100,1.2,3.3,20,-0.5,19.5);
  
  TProfile *hPtCorrGenjetvsClus = new TProfile("hPtCorrGenjetvsClus","hPtCorr Genjet-vs-Cluster",200,0.,200.,0.,200.);
  TProfile *hPtCorrGenjetvsTC = new TProfile("hPtCorrGenjetvsTC","hPtCorr Genjet-vs-TC",200,0.,200.,0.,200.);
  TProfile *hPtCorrTCvsClus = new TProfile("hPtCorrTCvsClus","hPtCorr TC-vs-Cluster",200,0.,200.,0.,200.);
  // TProfile *hPtCorrGenjetvsClus_1 = new TProfile("hPtCorrGenjetvsClus_1","hPtCorr Genjet-vs-Cluster",200,0.,200.,0.,200.);
  // TProfile *hPtCorrGenjetvsClus_2 = new TProfile("hPtCorrGenjetvsClus_2","hPtCorr Genjet-vs-Cluster",200,0.,200.,0.,200.);
  // TProfile *hPtCorrGenjetvsClus_3 = new TProfile("hPtCorrGenjetvsClus_3","hPtCorr Genjet-vs-Cluster",200,0.,200.,0.,200.);
  // TProfile *hPtCorrGenjetvsClus_4 = new TProfile("hPtCorrGenjetvsClus_4","hPtCorr Genjet-vs-Cluster",200,0.,200.,0.,200.);
  // TProfile *hPtCorrGenjetvsClus_5 = new TProfile("hPtCorrGenjetvsClus_5","hPtCorr Genjet-vs-Cluster",200,0.,200.,0.,200.);
  
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

  const int nJetEtaBins = 21;
  const int nJetPtBins = 43;
  TH1F *hGenClusPtReso1D[nJetPtBins], *hGenTCPtReso1D[nJetPtBins], *hTCClusPtReso1D[nJetPtBins];
  //std::unique_ptr<TFile> finReso(TFile::Open(Form("/home/hep/idas/stage2_emulation_results/Reso_iter2/singlePion_PU0/CalcResolution_ntuples_%d_merged.root",resoIndex)));
  //std::unique_ptr<TFile> finReso(TFile::Open(Form("/home/hep/idas/stage2_emulation_results/Reso_iter10/singlePion_PU0_Ideal/CalcResolution_ntuples_%d_merged.root",resoIndex)));
  //std::unique_ptr<TFile> finReso(TFile::Open(Form("/home/hep/idas/stage2_emulation_results/Reso_iter12/singlePion_PU0_Ideal/CalcResolution_ntuples_%d_merged.root",resoIndex)));
  //std::unique_ptr<TFile> finReso(TFile::Open(Form("/home/hep/idas/stage2_emulation_results/Reso_iter13/singlePion_PU0_Ideal/CalcResolution_ntuples_%d_merged.root",resoIndex)));
  std::unique_ptr<TFile> finReso(TFile::Open(Form("/home/hep/idas/stage2_emulation_results/Reso_iter14/singlePion_PU0_Ideal/CalcResolution_ntuples_%d_merged.root",resoIndex)));
  std::cout<<"Resolution filename : " << finReso->GetName() << std::endl;
  for(int ipt=0;ipt<nJetPtBins;ipt++){
    // hGenClusPtReso1D[ipt] = (TH1F *) finReso->Get(Form("hGenClusPtReso1D_%d",ipt));
    // hGenTCPtReso1D[ipt] = (TH1F *) finReso->Get(Form("hGenTCPtReso1D_%d",ipt));
    // hTCClusPtReso1D[ipt] = (TH1F *) finReso->Get(Form("hTCClusPtReso1D_%d",ipt));
    hGenClusPtReso1D[ipt] = (TH1F *) finReso->Get(Form("Reso_1D/hGenClusPtReso1D_%d",ipt));
    hGenTCPtReso1D[ipt] = (TH1F *) finReso->Get(Form("Reso_1D/hGenTCPtReso1D_%d",ipt));
    hTCClusPtReso1D[ipt] = (TH1F *) finReso->Get(Form("Reso_1D/hTCClusPtReso1D_%d",ipt));
  }
  
  TH1F *hGenClusPtReso[nJetEtaBins][nJetPtBins], *hTCClusPtReso[nJetEtaBins][nJetPtBins];
  for(int ieta=0;ieta<nJetEtaBins;ieta++)
    for(int ipt=0;ipt<nJetPtBins;ipt++){
      hGenClusPtReso[ieta][ipt] = (TH1F *) finReso->Get(Form("Reso_2D/hGenClusPtReso_%d_%d",ieta,ipt));
      hTCClusPtReso[ieta][ipt] = (TH1F *) finReso->Get(Form("Reso_2D/hTCClusPtReso_%d_%d",ieta,ipt));
    }
  //hGenClusPtReso[ieta][ipt] = (TH1F *) finReso->Get(Form("hGenClusPtReso_%d_%d",ieta,ipt));

  const int nofdetType = 7;
  TH1F *hGenClusPtReso3D[nofdetType][nJetEtaBins][nJetPtBins];
  for(int idet=0;idet<nofdetType;idet++)
    for(int ieta=0;ieta<nJetEtaBins;ieta++)
      for(int ipt=0;ipt<nJetPtBins;ipt++){
        hGenClusPtReso3D[idet][ieta][ipt] = (TH1F *) finReso->Get(Form("Reso_3D/hGenClusPtReso3D_%d_%d_%d",idet,ieta,ipt));
	hGenClusPtReso3D[idet][ieta][ipt]->Rebin(4);
      }
  
  TH2F *hJetEtaPtBin = (TH2F *) finReso->Get("hJetEtaPtBin"); 
  
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
    double totTCpt_posEta_layer1 = 0.,totTCpt_negEta_layer1 = 0.;
    uint32_t nofPosEtaTCs = 0, nofNegEtaTCs = 0, nofTCsTrigL2 = 0, nofTCsTrigL9 = 0;
    double tcXoZposEta = 0.,tcYoZposEta = 0., tcXoZnegEta = 0.,tcYoZnegEta = 0.;
    double maxroz_posEta = -1.0, maxroz_negEta = -1.0;
    double tcPhi_posEta = 0.,tcPhi_negEta = 0.;
    double tcEta_posEta = 0.,tcEta_negEta = 0.;
    double tcRxy = 0.0;
    int detType = -1;
    int subdet = -1, wafertype = -1;
    int binPt, binEta;
    double ptCorr = -1;
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
      detType = -1;
      subdet = tc_subdet->at(itc);
      wafertype = tc_wafertype->at(itc);
      if(subdet==1 and wafertype==0) detType = 1;
      else if(subdet==1 and wafertype==1) detType = 2;
      else if(subdet==1 and wafertype==2) detType = 3;
      else if(subdet==2 and wafertype==0) detType = 4;
      else if(subdet==2 and wafertype==1) detType = 5;
      else if(subdet==2 and wafertype==2) detType = 6;
      else if(subdet==10) detType = 7;

      double minRoz = 1., clstPt = 0, clstEta = 0;
      for(int ipjet=0; ipjet < jetlist.size() ; ipjet++ ){
	int ijet = jetlist.at(ipjet).index ;
	double gjroz = 1/sinh(genjet_eta->at(ijet)) ; 
	double gjxoz = gjroz*cos(genjet_phi->at(ijet));
	double gjyoz = gjroz*sin(genjet_phi->at(ijet));
	double dClusGenRoz = sqrt( (tc_x->at(itc)/tc_z->at(itc) - gjxoz)*(tc_x->at(itc)/tc_z->at(itc) - gjxoz) + (tc_y->at(itc)/tc_z->at(itc) - gjyoz)*(tc_y->at(itc)/tc_z->at(itc) - gjyoz) );
	if(dClusGenRoz<minRoz){
	    minRoz = dClusGenRoz;
	    clstPt = genjet_pt->at(ijet);
	    clstEta = genjet_eta->at(ijet);
	}
      }
      GetHistoBin(hJetEtaPtBin, clstEta, clstPt, binEta, binPt);
      
      // if( tc_layer->at(itc)>26 )
      // 	ptCorr = 1. + hHadGenClusPtReso3D[detType-1][binEta-1][binPt-1]->GetMean()/clstPt;
      // else
      // 	ptCorr = 1.;
      
      // if(hGenClusPtReso3D[detType-1][binEta-1][binPt-1]->GetEntries()>0)
      // 	ptCorr = 1. + hGenClusPtReso3D[detType-1][binEta-1][binPt-1]->GetMean()/clstPt;
      // else
      // 	ptCorr = 1.;
      
      ptCorr = 1. + hGenClusPtReso[binEta-1][binPt-1]->GetMean()/clstPt + hTCClusPtReso[binEta-1][binPt-1]->GetMean()/clstPt;
      
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
	  float scale = ptCorr;
	  //float scale = 1.0;
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
	tcEta_negEta += tc_pt->at(itc)*tc_eta->at(itc) ;
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
	tcEta_posEta += tc_pt->at(itc)*tc_eta->at(itc) ;
	nofPosEtaTCs++;
	tcDxyozsize->Fill( (tc_x->at(itc)/tc_z->at(itc)-xoz_posEta), (tc_y->at(itc)/tc_z->at(itc)-yoz_posEta) );
	double droz = sqrt( (tc_x->at(itc)/tc_z->at(itc)-xoz_posEta)*(tc_x->at(itc)/tc_z->at(itc)-xoz_posEta) + (tc_y->at(itc)/tc_z->at(itc)-yoz_posEta)*(tc_y->at(itc)/tc_z->at(itc)-yoz_posEta) );
	if(droz>maxroz_posEta) maxroz_posEta = droz;
	tcRoz->Fill(droz,tc_pt->at(itc));
      }
      
      tot_tc_pt += tc_pt->at(itc);
      tot_tc_e += tc_energy->at(itc);
      
      tcRxy = sqrt( (tc_x->at(itc)*tc_x->at(itc)) + (tc_y->at(itc)*tc_y->at(itc)) );
      // gRzTC->AddPoint( fabs(tc_z->at(itc)), tcRxy);
      // if(tc_subdet->at(itc)==1) {
      // 	gRzTCsd1->AddPoint( fabs(tc_z->at(itc)), tcRxy);
      // 	if(tc_wafertype->at(itc)==0) gRzTCsd1_100->AddPoint( fabs(tc_z->at(itc)), tcRxy);
      // 	if(tc_wafertype->at(itc)==1) gRzTCsd1_200->AddPoint( fabs(tc_z->at(itc)), tcRxy);
      // 	if(tc_wafertype->at(itc)==2) gRzTCsd1_300->AddPoint( fabs(tc_z->at(itc)), tcRxy);
      // }
      // if(tc_subdet->at(itc)==2) {
      // 	gRzTCsd2->AddPoint( fabs(tc_z->at(itc)), tcRxy);
      // 	if(tc_wafertype->at(itc)==0) gRzTCsd2_100->AddPoint( fabs(tc_z->at(itc)), tcRxy);
      // 	if(tc_wafertype->at(itc)==1) gRzTCsd2_200->AddPoint( fabs(tc_z->at(itc)), tcRxy);
      // 	if(tc_wafertype->at(itc)==2) gRzTCsd2_300->AddPoint( fabs(tc_z->at(itc)), tcRxy);
      // }
      // if(tc_subdet->at(itc)==10) gRzTCsd10->AddPoint( fabs(tc_z->at(itc)), tcRxy);
      hPhi->Fill(tc_phi->at(itc));
      hPhiDeg->Fill(phi_deg);
      hLayerE->Fill(tc_layer->at(itc), tc_pt->at(itc));
      if(tc_layer->at(itc)==2) nofTCsTrigL2++;
      if(tc_layer->at(itc)==9) nofTCsTrigL9++;
    }//end of TC loop
    //rearrgdtcs.clear();
    if(pt_negEta>pt_TCThresh) tcMaxroz->Fill(maxroz_negEta,totTCpt_negEta);
    if(pt_posEta>pt_TCThresh) tcMaxroz->Fill(maxroz_posEta,totTCpt_posEta);
    hNTCTrigL9->Fill(0.5*nofTCsTrigL9); //0.5 for per HGCAL arm
    hNTCTrigL2->Fill(0.5*nofTCsTrigL2); //0.5 for per HGCAL arm
    
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

    int nofClus1GeV = 0;
    int nofClus3GeV = 0;
    int nofClus5GeV = 0;
    int nofClus10GeV = 0;
    for (uint32_t isect = 0 ; isect < 6 ; isect++ ){
      //for(TPGCluster const& clf : vCld[isect]){
      if(isect==0 or isect==3){
	nofClus1GeV = 0;
	nofClus3GeV = 0;
	nofClus5GeV = 0;
	nofClus10GeV = 0;
      }
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
		    // <<", (tcPtSum,tcPtSum/lsb,energy,energy*lsb): (" << std::setw(8) << tcPtSum
		    // << ", " << std::setw(8) << uint32_t(tcPtSum/lsbScales.LSB_E_TC()+0.5)
		    // << ", " << std::setw(8) << clf.getEnergy()
		    // << ", " << std::setw(8) << (clf.getEnergy()*lsbScales.LSB_E_TC())
		    // << ") "
		    << std::defaultfloat
		    << std::endl;
	}
	
	if(clf.getEnergyGeV()>1.0) nofClus1GeV++;
	if(clf.getEnergyGeV()>3.0) nofClus3GeV++;	
	if(clf.getEnergyGeV()>5.0) nofClus5GeV++;
	if(clf.getEnergyGeV()>10.0) nofClus10GeV++;

      }//cluster loop
      if(isect==2 or isect==5){
	hNClus1GeV->Fill(nofClus1GeV);
	hNClus3GeV->Fill(nofClus3GeV);
	hNClus5GeV->Fill(nofClus5GeV);
	hNClus10GeV->Fill(nofClus10GeV);
      }
    }//sector loop

    for(int ipjet=0; ipjet < jetlist.size() ; ipjet++ ){
      int ijet = jetlist.at(ipjet).index ;
      hEt->Fill(genjet_pt->at(ijet));
      int minsect = (genjet_eta->at(ijet) < 0) ? 0 : 3;
      int maxsect = (genjet_eta->at(ijet) < 0) ? 3 : 6;
      double tcPtSum = (genjet_eta->at(ijet) < 0) ? totTCpt_negEta : totTCpt_posEta ;
      double avgXoZ = (genjet_eta->at(ijet) < 0) ? tcXoZnegEta/tcPtSum : tcXoZposEta/tcPtSum ;
      double avgYoZ = (genjet_eta->at(ijet) < 0) ? tcYoZnegEta/tcPtSum : tcYoZposEta/tcPtSum ;
      double avgPhi = (genjet_eta->at(ijet) < 0) ? tcPhi_negEta/tcPtSum : tcPhi_posEta/tcPtSum ;
      double avgEta = (genjet_eta->at(ijet) < 0) ? tcEta_negEta/tcPtSum : tcEta_posEta/tcPtSum ;
      double gjroz = 1/sinh(genjet_eta->at(ijet)) ; //tan(2*atan(exp(-1.0*genjet_eta->at(ijet))));
      double gjxoz = gjroz*cos(genjet_phi->at(ijet));
      double gjyoz = gjroz*sin(genjet_phi->at(ijet));
      double avgRoZ = sqrt((avgXoZ-gjxoz)*(avgXoZ-gjxoz) + (avgYoZ-gjyoz)*(avgYoZ-gjyoz));
      bool hasFound = false, hasFoundPt = false;
      int binPt, binEta;
      double cluspt = genjet_pt->at(ijet);
      int nofClus = 0;
      hTrigGen->Fill(genjet_pt->at(ijet));
      double minRoz = 1., closestClusE= 100., closestClusEDiff= 100., totClusE = 0.;
      double clstdeltaR = 0., clstEta = 0., clstPhi = 0.;
      double clstXoZ = 0., clstYoZ = 0., clstRoZ = 0., clstZcm = 0.;
      int clstisect = -1;
      for (uint32_t isect = minsect ; isect < maxsect ; isect++ ){
	for(TPGCluster const& clf : vCld[isect]){	  
	  if(clf.getEnergyGeV()<3.0) continue;
	  hClusE->Fill(clf.getEnergyGeV());
	  double deltaR = TMath::Sqrt((clf.getGlobalEtaRad(isect)-genjet_eta->at(ijet))*(clf.getGlobalEtaRad(isect)-genjet_eta->at(ijet))
				      +
				      (clf.getGlobalPhiRad(isect)-genjet_phi->at(ijet))*(clf.getGlobalPhiRad(isect)-genjet_phi->at(ijet)));

	  double dClusGenRoz = sqrt( (clf.getGlobalXOverZF(isect) - gjxoz)*(clf.getGlobalXOverZF(isect) - gjxoz) + (clf.getGlobalYOverZF(isect) - gjyoz)*(clf.getGlobalYOverZF(isect) - gjyoz) );
	  if(dClusGenRoz<minRoz){
	    minRoz = dClusGenRoz;
	    closestClusE = clf.getEnergyGeV();
	    closestClusEDiff = clf.getEnergyGeV() - genjet_pt->at(ijet);
	    clstdeltaR = deltaR;
	    clstisect = isect;
	    clstEta = clf.getGlobalEtaRad(isect);
	    clstPhi = clf.getGlobalPhiRad(isect);
	    clstXoZ = clf.getGlobalXOverZF(isect);
	    clstYoZ = clf.getGlobalYOverZF(isect);
	    clstRoZ = clf.getGlobalRhoOverZF(isect);
	    clstZcm = clf.getGlobalZCm(isect);
	  }//minRoZ condition
	  totClusE += clf.getEnergyGeV() ;
	  nofClus++;
	}//cluster loop
      }//sector loop      
      // GetHistoBin(hJetEtaPtBin, genjet_eta->at(ijet), genjet_pt->at(ijet), binEta, binPt);
      // //if(genjet_pt->at(ijet)>=200.0) binPt = 31;
      // //float PtCorrRefGen = fabs(hGenClusPtReso1D[binPt-1]->GetMean());
      // //float PtCorrRefGen = fabs(hGenClusPtReso[binEta-1][binPt-1]->GetMean());
      // float PtCorrRefGen = fabs(hGenClusPtReso[binEta-1][binPt-1]->GetXaxis()->GetBinCenter(hGenClusPtReso[binEta-1][binPt-1]->GetMaximumBin()));
      // float PtCorrRefTC = fabs(hTCClusPtReso1D[binPt-1]->GetMean());
      // closestClusE += PtCorrRefGen ; //PtCorrRefGen;
      
      bool hasClosestFound = (minRoz<1.)?true:false;
      bool hasClosestFoundPt = (hasClosestFound and closestClusE>pt_clusThresh)?true:false;
      bool hasClosestFoundPtTot = (hasClosestFound and totClusE>(pt_clusThresh))?true:false;
      bool hgcalInnerEta = (fabs(genjet_eta->at(ijet))>innerEtaMin and fabs(genjet_eta->at(ijet))<innerEtaMax) ? true : false;
      bool hgcalOuterEta = (fabs(genjet_eta->at(ijet))>etaMin and fabs(genjet_eta->at(ijet))<etaMax) ? true : false;
      
      
      if(hgcalInnerEta) {
	hTrigGenPt->Fill(genjet_pt->at(ijet));
	hGTotCEDiff->Fill((totClusE-genjet_pt->at(ijet)));
	hGCEDiffClst->Fill(closestClusEDiff);
	hclusRozClst->Fill(minRoz, closestClusE );
	
	if(hasClosestFound) hTrigGenClst->Fill(genjet_pt->at(ijet));
	if(hasClosestFoundPt) hTrigGenClstPt->Fill(genjet_pt->at(ijet));
	  
       
	if(hasClosestFoundPtTot) hTrigGenClstPtTot->Fill(genjet_pt->at(ijet));
	effTrigGenClst->Fill(hasClosestFound,genjet_pt->at(ijet));
	effTrigGenClstPt->Fill(hasClosestFoundPt,genjet_pt->at(ijet));
	effTrigGenPU->Fill(hasClosestFoundPt,genjet_pt->at(ijet));
	effTrigGenClstPtTot->Fill(hasClosestFoundPtTot,genjet_pt->at(ijet));
	//////=============================================================
	if(hasClosestFound){
	  //===============
	  hGenClusE->Fill(genjet_pt->at(ijet), closestClusE);
	  hGenTCE->Fill(genjet_pt->at(ijet), tcPtSum);
	  hTCClusE->Fill(tcPtSum, closestClusE);
	  //===============
	  hPtReso->Fill( (closestClusE - genjet_pt->at(ijet)) );
	  if(hgcalInnerEta) hPtResoInnerEta->Fill( (closestClusE - genjet_pt->at(ijet)) );
	  hPtResoGenTC->Fill( (tcPtSum - genjet_pt->at(ijet)) );
	  hPtResoTCClus->Fill( (closestClusE - tcPtSum) );
	  //===============
	  hPtCorrGenjetvsClus->Fill(genjet_pt->at(ijet), closestClusE);
	  hPtCorrGenjetvsTC->Fill(genjet_pt->at(ijet), tcPtSum);
	  hPtCorrTCvsClus->Fill(tcPtSum, closestClusE);
	  //===============
	  deltaGenclus->Fill(clstEta-genjet_eta->at(ijet), clstPhi-genjet_phi->at(ijet));
	  deltaGenclusSeg[clstisect]->Fill(clstEta-genjet_eta->at(ijet), clstPhi-genjet_phi->at(ijet));
	  deltaGenclusXYoZSeg[clstisect]->Fill( (clstXoZ - gjxoz), (clstYoZ - gjyoz) );
	  clusXYoZ[clstisect]->Fill( clstXoZ, clstYoZ );
	  genJetXYoZ[clstisect]->Fill( gjxoz, gjyoz); 
	  // //===============
	  deltaGenClusDRoZRDPhioZ[clstisect]->Fill( gjroz*(clstPhi-genjet_phi->at(ijet)), minRoz  );
	  deltaGenClusRotatedDRoZRDPhioZ[clstisect]->Fill( gjroz*(clstPhi-genjet_phi->at(ijet)), minRoz*sin(genjet_phi->at(ijet)) );
	  //===============	    
	  deltaGenClusDRoZVz[clstisect]->Fill( vtx_z, (clstRoZ - gjroz)  );
	  //===============
	  // hDeltaRGenClus->Fill(clstdeltaR); 
	  // hDeltaRGCPt->Fill(genjet_pt->at(ijet), clstdeltaR); 
	  // hDeltaRGCEta->Fill(fabs(genjet_eta->at(ijet)), clstdeltaR); 
	  // hDeltaRGCPtProf->Fill(genjet_pt->at(ijet), clstdeltaR); 
	  // hDeltaRGCEtaProf->Fill(fabs(genjet_eta->at(ijet)), clstdeltaR);
	  // hDeltaRGCPhi->Fill(genjet_phi->at(ijet), clstdeltaR); 
	  // hDeltaRGCPhiProf->Fill(genjet_phi->at(ijet), clstdeltaR);
	  // hDeltaRGCPtEta->Fill(genjet_pt->at(ijet), fabs(genjet_eta->at(ijet)), clstdeltaR);
	  //===============
	  //===============
	  hasFound = true;
	  hasFoundPt = (totClusE>pt_clusThresh)?true:false;
	  cluspt = closestClusE;
	}
	//////=============================================================
      }
      if(genjet_pt->at(ijet)>pt_clusThresh_etaeff){
	hTrigGenEta->Fill(fabs(genjet_eta->at(ijet)));
	if(hasClosestFoundPt) hTrigGenClstEta->Fill(fabs(genjet_eta->at(ijet)));
	effTrigGenClstEta->Fill(hasClosestFoundPt,fabs(genjet_eta->at(ijet)));
      }
      if(hgcalOuterEta){
	effTrigGenClstPtOuter->Fill(hasClosestFoundPt,genjet_pt->at(ijet));
	effTrigGenClstPtInner->Fill((hgcalInnerEta and hasClosestFoundPt),genjet_pt->at(ijet));
      }
      if(genjet_pt->at(ijet)>60.0){
	hTrigGenEtaOuter->Fill(fabs(genjet_eta->at(ijet)));
	if(hasClosestFoundPt) hTrigGenClstEtaOuter->Fill(fabs(genjet_eta->at(ijet)));
	effTrigGenClstEtaOuter->Fill(hasClosestFoundPt,fabs(genjet_eta->at(ijet)));
	if(hgcalInnerEta){
	  hTrigGenEtaInner->Fill(fabs(genjet_eta->at(ijet)));
	  if(hasClosestFoundPt) hTrigGenClstEtaInner->Fill(fabs(genjet_eta->at(ijet)));
	  effTrigGenClstEtaInner->Fill(hasClosestFoundPt,fabs(genjet_eta->at(ijet)));
	}
      }//pt cut for eta
      
      hNClusPt->Fill(genjet_pt->at(ijet), nofClus);
      hNClusEta->Fill(fabs(genjet_eta->at(ijet)), nofClus);
      
      hNClusSel->Fill(nofClus);
      if(hasFound) hTrigSelGen->Fill(genjet_pt->at(ijet));
      if(hasFound) hTrigClus->Fill(cluspt);
      
      effTrigGen->Fill(hasFound,genjet_pt->at(ijet));
      effTrigClus->Fill(hasFound,cluspt);
      
      //effTrigGenPU->Fill(hasFound,genjet_pt->at(ijet));
      //effTrigClusPU->Fill(hasFound,cluspt);
      
      // effTrigGenPUEtOnly->Fill(hasFoundPt,genjet_pt->at(ijet));
      // effTrigClusPUEtOnly->Fill(hasFoundPt,cluspt);
      
    }//jet from decay of pions from tau
    

    
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
    tc_subdet->clear();
    tc_wafertype->clear();
    cl3d_pt->clear();
    cl3d_phi->clear();
    cl3d_eta->clear();

  }//event loop
  
  std::string outname = "stage2SemiEmulator_" + outputfile_extn + "_" + index;

  effTrigGen->SetStatisticOption(TEfficiency::kBJeffrey);
  effTrigClus->SetStatisticOption(TEfficiency::kBJeffrey);
  
  TH1D *hTrigEff = (TH1D *) hTrigSelGen->Clone("hTrigEff");
  hTrigEff->Divide(hTrigGen);

  TH1D *hTrigEffClusE = (TH1D *) hTrigClus->Clone("hTrigEffClusE");
  hTrigEffClusE->Divide(hTrigGen);
  
  TH1D *tcMaxroz_cumul = (TH1D *)tcMaxroz->GetCumulative(kTRUE,"_cumul");
  tcMaxroz_cumul->Scale(1./tcMaxroz_cumul->GetBinContent(tcMaxroz_cumul->GetMaximumBin()));
  TH1D *tcRoz_cumul = (TH1D *)tcRoz->GetCumulative(kTRUE,"_cumul");
  tcRoz_cumul->Scale(1./tcRoz_cumul->GetBinContent(tcRoz_cumul->GetMaximumBin()));

  TH1D *hclusRoz_cumul = (TH1D *)hclusRoz->GetCumulative(kTRUE,"_cumul");
  hclusRoz_cumul->Scale(1./hclusRoz_cumul->GetBinContent(hclusRoz_cumul->GetMaximumBin()));

  TFile *fout = new TFile(Form("%s.root",outname.c_str()),"recreate");
  hEt->Write();
  hClusE->Write();
  hLayerE->Write();
  hPhi->Write();
  hPhiDeg->Write();
  //============
  hPtReso->Write();
  hPtResoGenTC->Write();
  hPtResoTCClus->Write();
  hPtResoInnerEta->Write();
  //============
  hTrigGen->Write();
  hTrigGenPt->Write();
  hTrigGenEta->Write();
  hTrigGenEtaOuter->Write();
  hTrigGenEtaInner->Write();
  //============
  hTrigGenClst->Write();
  hTrigGenClstPt->Write();
  hTrigGenClstPtTot->Write();
  hTrigGenClstEta->Write();
  hTrigGenClstEtaOuter->Write();
  hTrigGenClstEtaInner->Write();
  hTrigSelGen->Write();
  hTrigClus->Write();
  //============
  hTrigEff->Write();
  hTrigEffClusE->Write();
  //============
  effTrigGen->Write();
  effTrigGenClst->Write();
  effTrigGenClstPt->Write();
  effTrigGenClstPtTot->Write();
  effTrigGenClstEta->Write();
  effTrigGenClstEtaOuter->Write();
  effTrigGenClstEtaInner->Write();
  effTrigGenClstPtInner->Write();
  effTrigGenClstPtOuter->Write();
  //============
  effTrigClus->Write();
  effTrigGenPU->Write();
  // effTrigClusPU->Write();
  // effTrigGenPUEtOnly->Write();
  // effTrigClusPUEtOnly->Write();
  
  hclusRoz->Write();
  hclusRoz_cumul->Write();
  hclusRozClst->Write();
  hGCEDiffClst->Write();
  hGTotCEDiff->Write();
  
  hNTCTrigL2->Write();
  hNTCTrigL9->Write();
  hNClusSel->Write();
  hNClus1GeV->Write();
  hNClus3GeV->Write();
  hNClus5GeV->Write();
  hNClus10GeV->Write();
  hNClusPt->Write();
  hNClusEta->Write();
  // hDeltaRGenClus->Write();
  // hDeltaRGCPt->Write();
  // hDeltaRGCEta->Write();
  // hDeltaRGCPhi->Write();
  // hDeltaRGCPtProf->Write();
  // hDeltaRGCEtaProf->Write();
  // hDeltaRGCPhiProf->Write();
  // hDeltaRGCPtEta->Write();
  
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
  // deltaGenTC->Write();
  // deltaTCclus->Write();
  hGenClusE->Write();
  hGenTCE->Write();
  hTCClusE->Write();
  // hGenClusE_1->Write();
  // hGenClusE_2->Write();
  // hGenClusE_3->Write();
  // hGenClusE_4->Write();
  // hGenClusE_5->Write();
  // hPtCorrGenjetvsClus_1->Write();
  // hPtCorrGenjetvsClus_2->Write();
  // hPtCorrGenjetvsClus_3->Write();
  // hPtCorrGenjetvsClus_4->Write();
  // hPtCorrGenjetvsClus_5->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) deltaGenclusSeg[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) deltaGenclusXYoZSeg[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) deltaGenTCXYoZSeg[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) deltaTCclusXYoZSeg[isect]->Write();
  hPtCorrGenjetvsClus->Write();
  hPtCorrGenjetvsTC->Write();
  hPtCorrTCvsClus->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) genJetXYoZ[isect]->Write();
  //for (uint32_t isect = 0 ; isect < 6 ; isect++ ) tcAvgXYoZ[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) clusXYoZ[isect]->Write();
  tcDxyozsize->Write();
  tcMaxroz->Write();
  tcMaxroz_cumul->Write();
  tcRoz->Write();
  tcRoz_cumul->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) deltaGenClusDRoZRDPhioZ[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) deltaGenClusRotatedDRoZRDPhioZ[isect]->Write();
  // for (uint32_t isect = 0 ; isect < 6 ; isect++ ) deltaGenTCDRoZRDPhioZ[isect]->Write();
  // for (uint32_t isect = 0 ; isect < 6 ; isect++ ) deltaTCClusDRoZRDPhioZ[isect]->Write();
  for (uint32_t isect = 0 ; isect < 6 ; isect++ ) deltaGenClusDRoZVz[isect]->Write();
  // gRzTC->Write();
  // gRzTCsd1->Write();
  // gRzTCsd2->Write();
  // gRzTCsd10->Write();
  // gRzTCsd1_100->Write();
  // gRzTCsd1_200->Write();
  // gRzTCsd1_300->Write();
  // gRzTCsd2_100->Write();
  // gRzTCsd2_200->Write();
  // gRzTCsd2_300->Write();
  fout->Close();
  delete fout;
  
  // fin->Close();
  // delete fin;

  return true;
}
