/**********************************************************************
 Created on : 11/05/2025
 Purpose    : Calculate the jet resolutions
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

bool GetHistoBin(TH2F *h, float eta, float et, int& binX, int& binY)
{
  eta = abs(eta);
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
  
  std::unique_ptr<TFile> fin( TFile::Open(Form(inputfile.c_str())) );
  //TFile *fin = TFile::Open(Form(inputfile.c_str()));
  std::cout<<"Input: filename : " << fin->GetName() << std::endl;
  std::cout << "Input: outfile index : " << index << std::endl;
  std::cout << "Input: nofEvents : " << nofEvents << std::endl;
  std::cout << "Input: sidelength : " << sidelength << std::endl;
  std::cout << "Input: MC Sample : " << sampleType << std::endl;
  std::cout << "pt_clusThresh for L1 : " << pt_clusThresh << std::endl;
  std::cout << "pt_clusThresh_etaeff for Trigger eta eff : " << pt_clusThresh_etaeff << std::endl;
  
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
  
  // nJetEtaBins = 52;
  // Float_t jetEtaBin[53] = {-2.500, -2.322, 
  // 			   -2.172, -2.043, -1.930, -1.830, -1.740, 
  // 			   -1.653, -1.566, -1.479, -1.392, -1.305, 
  // 			   -1.218, -1.131, -1.044, -0.957, -0.870, 
  // 			   -0.783, -0.696, -0.609, -0.522, -0.435, 
  // 			   -0.348, -0.261, -0.174, -0.087, 
  // 			   0.0, 0.087, 0.174, 0.261, 0.348, 
  // 			   0.435, 0.522, 0.609, 0.696, 0.783, 
  // 			   0.870, 0.957, 1.044, 1.131, 1.218, 
  // 			   1.305, 1.392, 1.479, 1.566, 1.653, 
  // 			   1.740, 1.830, 1.930, 2.043, 2.172, 
  // 			   2.322, 2.5};

  // nETBins = 42;
  // Float_t ETBin[43] = {15., 16., 18., 20., 22., 25., 30., 35., 40., 45., 50., 55., 60., 65., 70., 75., 80., 85., 90., 95., 100.,  
  //                      110., 120., 130., 140., 150., 160., 170., 180., 190., 200.,
  // 	               220., 240., 260., 280., 300.,
  // 	               330., 360., 390., 420.,
  //                      440., 480., 520.}; 
  
  // const int nJetEtaBins = 31;
  // Float_t jetEtaBin[32] = { -3.212,
  //                           -3.034, -2.856, -2.678, -2.500, -2.322, 
  // 			    -2.172, -2.043, -1.930, -1.830, -1.740, 
  // 			    -1.653, -1.566, -1.479, -1.392, -1.305, 
  // 			    1.305, 1.392, 1.479, 1.566, 1.653, 
  // 			    1.740, 1.830, 1.930, 2.043, 2.172, 
  // 			    2.322, 2.5, 2.678, 2.856, 3.034,
  // 			    3.212};
  // const int nJetPtBins = 43;
  // Float_t jetPtBin[44] = {0, 15., 16., 18., 20., 22., 25., 30., 35., 40., 45., 50., 55., 60., 65., 70., 75., 80., 85., 90., 95., 100.,  
  //                      110., 120., 130., 140., 150., 160., 170., 180., 190., 200.,
  // 	               220., 240., 260., 280., 300.,
  // 	               330., 360., 390., 420.,
  //                      440., 480., 520.};
  
  //Eta : outermost(Sci) = 1.321, innermost(Si) =  3.152
  //use delta-Eta = 0.0873 (or mutltiple) and make range for |\eta|
  float deltaEta = 0.0873;
  const int nJetEtaBins = 21;
  Float_t jetEtaBin[22];
  jetEtaBin[0] = 1.321;
  for(int ieta=1;ieta<=nJetEtaBins;ieta++)
    jetEtaBin[ieta] = jetEtaBin[0] + ieta*deltaEta;
  //jetEtaBin[21] would be 3.1543;
  
  const int nJetPtBins = 43;
  Float_t jetPtBin[44] = {0, 15., 16., 18., 20., 22., 25., 30., 35., 40., 45., 50., 55., 60., 65., 70., 75., 80., 85., 90., 95., 100.,  
                       110., 120., 130., 140., 150., 160., 170., 180., 190., 200.,
	               220., 240., 260., 280., 300.,
	               330., 360., 390., 420.,
                       440., 480., 520.};
  
  int nPtResoBins = 400;
  
  //////////////// 2D pt,eta histos ////////////////////////////
  TH1F ***hGenClusPtReso, ***hGenTCPtReso, ***hTCClusPtReso;  
  hGenClusPtReso = new TH1F**[nJetEtaBins];
  hGenTCPtReso = new TH1F**[nJetEtaBins];
  hTCClusPtReso = new TH1F**[nJetEtaBins];
  for(int ieta=0;ieta<nJetEtaBins;ieta++){
    hGenClusPtReso[ieta] = new TH1F*[nJetPtBins];
    hGenTCPtReso[ieta] = new TH1F*[nJetPtBins];
    hTCClusPtReso[ieta] = new TH1F*[nJetPtBins];
    for(int ipt=0;ipt<nJetPtBins;ipt++){
      hGenClusPtReso[ieta][ipt] = new TH1F(Form("hGenClusPtReso_%d_%d",ieta,ipt),
					   Form("hGenClusPtReso #eta:(%4.3f-%4.3f) p_{T}:(%2.0f-%2.0f) GeV",jetEtaBin[ieta],jetEtaBin[ieta+1],jetPtBin[ipt],jetPtBin[ipt+1]),
					   nPtResoBins,-400.,400.);
      hGenTCPtReso[ieta][ipt] = new TH1F(Form("hGenTCPtReso_%d_%d",ieta,ipt),
					   Form("hGenTCPtReso #eta:(%4.3f-%4.3f) p_{T}:(%2.0f-%2.0f) GeV",jetEtaBin[ieta],jetEtaBin[ieta+1],jetPtBin[ipt],jetPtBin[ipt+1]),
					   nPtResoBins,-400.,400.);
      hTCClusPtReso[ieta][ipt] = new TH1F(Form("hTCClusPtReso_%d_%d",ieta,ipt),
					  Form("hTCClusPtReso #eta:(%4.3f-%4.3f) p_{T}:(%2.0f-%2.0f) GeV",jetEtaBin[ieta],jetEtaBin[ieta+1],jetPtBin[ipt],jetPtBin[ipt+1]),
					  nPtResoBins,-400.,400.);
    }// jet et
  }//jet eta
  //////////////////////////////////////////////////////////

  //////////////// 2D pt,eta TC ELoss ////////////////////////////
  TH1F ***hTCElossCE_E,***hTCElossCE_H;
  hTCElossCE_E = new TH1F**[nJetEtaBins];
  hTCElossCE_H = new TH1F**[nJetEtaBins];
  for(int ieta=0;ieta<nJetEtaBins;ieta++){
    hTCElossCE_E[ieta] = new TH1F*[nJetPtBins];
    hTCElossCE_H[ieta] = new TH1F*[nJetPtBins];
    for(int ipt=0;ipt<nJetPtBins;ipt++){
      hTCElossCE_E[ieta][ipt] = new TH1F(Form("hTCElossCE_E_%d_%d",ieta,ipt),
					   Form("hTCElossCE_E #eta:(%4.3f-%4.3f) p_{T}:(%2.0f-%2.0f) GeV",jetEtaBin[ieta],jetEtaBin[ieta+1],jetPtBin[ipt],jetPtBin[ipt+1]),
					   nPtResoBins,-400.,400.);
      hTCElossCE_H[ieta][ipt] = new TH1F(Form("hTCElossCE_H_%d_%d",ieta,ipt),
					  Form("hTCElossCE_H #eta:(%4.3f-%4.3f) p_{T}:(%2.0f-%2.0f) GeV",jetEtaBin[ieta],jetEtaBin[ieta+1],jetPtBin[ipt],jetPtBin[ipt+1]),
					  nPtResoBins,-400.,400.);
    }// jet et
  }//jet eta
  //////////////////////////////////////////////////////////
  
  // //////////////// 3D pt,eta TC ELoss ////////////////////////////
  // TH1F ****hTCElossCE_E_3D,****hTCElossCE_H_3D;
  // hTCElossCE_E_3D = new TH1F***[7];
  // hTCElossCE_H_3D = new TH1F***[7];
  // for(int idet=0;idet<7;idet++){
  //   hTCElossCE_E_3D[idet] = new TH1F**[nJetEtaBins];
  //   hTCElossCE_H_3D[idet] = new TH1F**[nJetEtaBins];
  //   for(int ieta=0;ieta<nJetEtaBins;ieta++){
  //     hTCElossCE_E_3D[idet][ieta] = new TH1F*[nJetPtBins];
  //     hTCElossCE_H_3D[idet][ieta] = new TH1F*[nJetPtBins];
  //     for(int ipt=0;ipt<nJetPtBins;ipt++){
  // 	hTCElossCE_E_3D[idet][ieta][ipt] = new TH1F(Form("hTCElossCE_E_3D_%d_%d_%d",idet,ieta,ipt),
  // 						 Form("hTCElossCE_E_3D det:%d, #eta:(%4.3f-%4.3f) p_{T}:(%2.0f-%2.0f) GeV",idet,jetEtaBin[ieta],jetEtaBin[ieta+1],jetPtBin[ipt],jetPtBin[ipt+1]),
  // 						 nPtResoBins,-400.,400.);
  // 	hTCElossCE_H_3D[idet][ieta][ipt] = new TH1F(Form("hTCElossCE_H_3D_%d_%d_%d",idet,ieta,ipt),
  // 						    Form("hTCElossCE_H_3D det:%d, #eta:(%4.3f-%4.3f) p_{T}:(%2.0f-%2.0f) GeV",idet,jetEtaBin[ieta],jetEtaBin[ieta+1],jetPtBin[ipt],jetPtBin[ipt+1]),
  // 						    nPtResoBins,-400.,400.);
  //     }// jet et
  //   }//jet eta
  // }//idet loop
  // //////////////////////////////////////////////////////////

  //////////////// 2D pt,eta TC ELoss ////////////////////////////
  TH1F **hTCElossCEEID, **hTCElossCEHID ;
  hTCElossCEEID = new TH1F*[nJetEtaBins];
  hTCElossCEHID = new TH1F*[nJetEtaBins];
  for(int ieta=0;ieta<nJetEtaBins;ieta++){
    hTCElossCEEID[ieta] = new TH1F(Form("hTCElossCEEID_%d",ieta),
				   Form("hTCElossCEEID #eta:(%4.3f-%4.3f)",jetEtaBin[ieta],jetEtaBin[ieta+1]),
				   nPtResoBins,-400.,400.);
    hTCElossCEHID[ieta] = new TH1F(Form("hTCElossCEHID_%d",ieta),
				   Form("hTCElossCEHID #eta:(%4.3f-%4.3f)",jetEtaBin[ieta],jetEtaBin[ieta+1]),
				   nPtResoBins,-400.,400.);
  }//jet eta
  //////////////////////////////////////////////////////////

  //////////////// 1D pt //////////////////////////////////
  TH1F **hGenClusPtReso1D, **hGenTCPtReso1D, **hTCClusPtReso1D;
  hGenClusPtReso1D = new TH1F*[nJetPtBins];
  hGenTCPtReso1D = new TH1F*[nJetPtBins];
  hTCClusPtReso1D = new TH1F*[nJetPtBins];
  for(int ipt=0;ipt<nJetPtBins;ipt++){
    hGenClusPtReso1D[ipt] = new TH1F(Form("hGenClusPtReso1D_%d",ipt),
				     Form("hGenClusPtReso1D p_{T}:(%2.0f-%2.0f) GeV",jetPtBin[ipt],jetPtBin[ipt+1]),
				     nPtResoBins,-400.,400.);
    hGenTCPtReso1D[ipt] = new TH1F(Form("hGenTCPtReso1D_%d",ipt),
				     Form("hGenTCPtReso1D p_{T}:(%2.0f-%2.0f) GeV",jetPtBin[ipt],jetPtBin[ipt+1]),
				     nPtResoBins,-400.,400.);
    hTCClusPtReso1D[ipt] = new TH1F(Form("hTCClusPtReso1D_%d",ipt),
				    Form("hTCClusPtReso1D p_{T}:(%2.0f-%2.0f) GeV",jetPtBin[ipt],jetPtBin[ipt+1]),
				    nPtResoBins,-400.,400.);
  }// jet pt
  ////////////////////////////////////////////////////////
  
  //////////////// 1D pt //////////////////////////////////
  TH1F **hGenTCPtResovsTCPt1D;
  hGenTCPtResovsTCPt1D = new TH1F*[100];
  for(int ipt=0;ipt<100;ipt++){
    hGenTCPtResovsTCPt1D[ipt] = new TH1F(Form("hGenTCPtResovsTCPt1D_%d",ipt),
					 Form("hGenTCPtResovsTCPt1D p_{T}:(%2.0f-%2.0f) GeV",float(ipt),float(ipt+1)),
					 nPtResoBins,-400.,400.);
  }
  //////////////// 1D pt //////////////////////////////////

  //////////////// 1D pt log x//////////////////////////////////
  double xm1 = -1;
  int nlog_tcptbins = 0;
  std::vector<double> logbins;
  int logmax = 100000;
  for(int itcpt=1 ; itcpt < logmax ; itcpt++){
    double x = log(itcpt);
    if(x-xm1>=0.1){
      std::cout << "nlog_tcptbins: " << nlog_tcptbins << ", itcpt: " << itcpt << ", x : " << x << std::endl;
      xm1 = x;
      logbins.push_back(float(itcpt));
      nlog_tcptbins++;
      
    }
  }
  
  TH1F **hGenTCPtResovsTCPt1DLog;
  hGenTCPtResovsTCPt1DLog = new TH1F*[int(logbins.size()-1)];
  for(int ipt=0;ipt<int(logbins.size()-1);ipt++){
    hGenTCPtResovsTCPt1DLog[ipt] = new TH1F(Form("hGenTCPtResovsTCPt1DLog_%d",ipt),
					    Form("hGenTCPtResovsTCPt1DLog p_{T}:(%2.0f-%2.0f) MeV",logbins.at(ipt),logbins.at(ipt+1)),
					    nPtResoBins,-400.,400.);
  }
  //////////////// 1D pt //////////////////////////////////

  //////////////// 2D pt //////////////////////////////////
  const int nTCPtBins = 100;
  TH1F ***hGenTCPtResovsTCPt2D;
  hGenTCPtResovsTCPt2D = new TH1F**[7];
  for(int idet=0;idet<7;idet++){
    hGenTCPtResovsTCPt2D[idet] = new TH1F*[nTCPtBins];
    for(int ipt=0;ipt<nTCPtBins;ipt++){
      hGenTCPtResovsTCPt2D[idet][ipt] = new TH1F(Form("hGenTCPtResovsTCPt2D_%d_%d",idet,ipt),
						 Form("hGenTCPtResovsTCPt2D idet: %d, p_{T}:(%2.0f-%2.0f) GeV",idet,float(ipt),float(ipt+1)),
						 nPtResoBins,-400.,400.);
    }
  }
  //////////////// 1D pt //////////////////////////////////
  
  // /////////////////// 1D pt CEH only correction /////////////////
  // TH1F **hHadGenClusPtReso1D, **hHadGenTCPtReso1D, **hHadTCClusPtReso1D;
  // hHadGenClusPtReso1D = new TH1F*[nJetPtBins];
  // hHadGenTCPtReso1D = new TH1F*[nJetPtBins];
  // hHadTCClusPtReso1D = new TH1F*[nJetPtBins];
  // for(int ipt=0;ipt<nJetPtBins;ipt++){
  //   hHadGenClusPtReso1D[ipt] = new TH1F(Form("hHadGenClusPtReso1D_%d",ipt),
  // 				     Form("hHadGenClusPtReso1D p_{T}:(%2.0f-%2.0f) GeV",jetPtBin[ipt],jetPtBin[ipt+1]),
  // 				     nPtResoBins,-400.,400.);
  //   hHadGenTCPtReso1D[ipt] = new TH1F(Form("hHadGenTCPtReso1D_%d",ipt),
  // 				     Form("hHadGenTCPtReso1D p_{T}:(%2.0f-%2.0f) GeV",jetPtBin[ipt],jetPtBin[ipt+1]),
  // 				     nPtResoBins,-400.,400.);
  //   hHadTCClusPtReso1D[ipt] = new TH1F(Form("hHadTCClusPtReso1D_%d",ipt),
  // 				    Form("hHadTCClusPtReso1D p_{T}:(%2.0f-%2.0f) GeV",jetPtBin[ipt],jetPtBin[ipt+1]),
  // 				    nPtResoBins,-400.,400.);
  // }// jet pt
  // ////////////////////////////////////////////////////////

  // /////////////////// 2D CEH only correction /////////////////
  // TH1F ***hHadGenClusPtReso2D;
  // hHadGenClusPtReso2D = new TH1F**[nJetEtaBins];
  // for(int ieta=0;ieta<nJetEtaBins;ieta++){
  //   hHadGenClusPtReso2D[ieta] = new TH1F*[nJetPtBins];
  //   for(int ipt=0;ipt<nJetPtBins;ipt++){
  //     hHadGenClusPtReso2D[ieta][ipt] = new TH1F(Form("hHadGenClusPtReso2D_%d_%d",ieta,ipt),
  // 					   Form("hHadGenClusPtReso2D #eta:(%4.3f-%4.3f) p_{T}:(%2.0f-%2.0f) GeV",jetEtaBin[ieta],jetEtaBin[ieta+1],jetPtBin[ipt],jetPtBin[ipt+1]),
  // 					   nPtResoBins,-400.,400.);
  //   }// jet et
  // }//jet eta
  // ////////////////////////////////////////////////////////

  // /////////////////// 3D CEH only correction /////////////////
  // TH1F ****hHadGenClusPtReso3D;
  // hHadGenClusPtReso3D = new TH1F***[7];
  // for(int idet=0;idet<7;idet++){
  //   hHadGenClusPtReso3D[idet] = new TH1F**[nJetEtaBins];
  //   for(int ieta=0;ieta<nJetEtaBins;ieta++){
  //     hHadGenClusPtReso3D[idet][ieta] = new TH1F*[nJetPtBins];
  //     for(int ipt=0;ipt<nJetPtBins;ipt++){
  // 	hHadGenClusPtReso3D[idet][ieta][ipt] = new TH1F(Form("hHadGenClusPtReso3D_%d_%d_%d",idet,ieta,ipt),
  // 						 Form("hHadGenClusPtReso3D det:%d, #eta:(%4.3f-%4.3f) p_{T}:(%2.0f-%2.0f) GeV",idet,jetEtaBin[ieta],jetEtaBin[ieta+1],jetPtBin[ipt],jetPtBin[ipt+1]),
  // 						 nPtResoBins,-400.,400.);
  //     }// jet et
  //   }//jet eta
  // }//idet loop
  // ////////////////////////////////////////////////////////

  //////////////// 3D pt,eta histos ////////////////////////////
  TH1F ****hGenClusPtReso3D; 
  hGenClusPtReso3D = new TH1F***[7];
  for(int idet=0;idet<7;idet++){
    hGenClusPtReso3D[idet] = new TH1F**[nJetEtaBins];
    for(int ieta=0;ieta<nJetEtaBins;ieta++){
      hGenClusPtReso3D[idet][ieta] = new TH1F*[nJetPtBins];
      for(int ipt=0;ipt<nJetPtBins;ipt++){
	hGenClusPtReso3D[idet][ieta][ipt] = new TH1F(Form("hGenClusPtReso3D_%d_%d_%d",idet,ieta,ipt),
						 Form("hGenClusPtReso3D det:%d, #eta:(%4.3f-%4.3f) p_{T}:(%2.0f-%2.0f) GeV",idet,jetEtaBin[ieta],jetEtaBin[ieta+1],jetPtBin[ipt],jetPtBin[ipt+1]),
						 nPtResoBins,-400.,400.);
      }// jet et
    }//jet eta
  }//idet loop
  //////////////////////////////////////////////////////////

  // ///////////Energy correction for Cl3d cluster //////////
  // TH1F **hGenClusPtReso1Dcl3d;
  // hGenClusPtReso1Dcl3d = new TH1F*[nJetPtBins];
  // for(int ipt=0;ipt<nJetPtBins;ipt++){
  //   hGenClusPtReso1Dcl3d[ipt] = new TH1F(Form("hGenClusPtReso1Dcl3d_%d",ipt),
  // 				     Form("hGenClusPtReso1Dcl3d p_{T}:(%2.0f-%2.0f) GeV",jetPtBin[ipt],jetPtBin[ipt+1]),
  // 				     nPtResoBins,-400.,400.);
  // }// jet pt
  // ////////////////////////////////////////////////////////
  
  TH2F *hJetEtaPtBin = new TH2F("hJetEtaPtBin","hJetEtaPtBin", nJetEtaBins, jetEtaBin, nJetPtBins, jetPtBin);
  TH2F *hJetEtaPtBinDet = new TH2F("hJetEtaPtBinDet","hJetEtaPtBinDEt", nJetEtaBins, jetEtaBin, nJetPtBins, jetPtBin);
  
  TH2F *hJetEtaTCPtBin = new TH2F("hJetEtaTCPtBin","hJetEtaTCPtBin", 21, 1.6, 3.0, 100, 0.,100.);
  const Double_t *plogbins = logbins.data();
  TH2F *hJetEtaTCPtBinLog = new TH2F("hJetEtaTCPtBinLog","hJetEtaTCPtBinLog", 21, 1.6, 3.0, int(logbins.size()-1), plogbins);
  
  TH1D *hMaxLayerE = new TH1D("hMaxLayerE","Layer with maximum energy",50, -0.5, 49.5);
  TH1D *hMaxDetE = new TH1D("hMaxDetE","dettype with maximum energy",10, -0.5, 9.5);

  ////////////// Check Eloss in CEE /////////////////////////
  TH2D *h2TCvsGenPt = new TH2D("h2TCvsGenPt","h2TCvsGenPt", 100, 0., 100., 200, 0., 400.);
  TProfile *pTCvsGenPt = new TProfile("pTCvsGenPt","pTCvsGenPt", 100, 0., 100., 0., 400.);
  TH2D *h2TCvsGenPtDet[7];
  TProfile *pTCvsGenPtDet[7];
  for(int idet=0;idet<7;idet++){
    h2TCvsGenPtDet[idet] = new TH2D(Form("h2TCvsGenPtDet_%d",idet),Form("h2TCvsGenPtDet_%d",idet), 100, 0., 100., 200, 0., 400.);
    pTCvsGenPtDet[idet] = new TProfile(Form("pTCvsGenPtDet_%d",idet),Form("pTCvsGenPtDet_%d",idet), 100, 0., 100., 0., 400.);
  }
  ///////////////////////////////////////////////////////////
  TH2D *h2TCPtvsGenPtByTCPtsum = new TH2D("h2TCPtvsGenPtByTCPtsum","h2TCPtvsGenPtByTCPtsum", 6000, 0., 60000., 200, -4., 4.); //TCpt in MeV
  TProfile *pTCPtvsGenPtByTCPtsum = new TProfile("pTCPtvsGenPtByTCPtsum","pTCPtvsGenPtByTCPtsum", 6000, 0., 60000., -4., 4.);
  TProfile *pTCPtvsGenPtByTCPtsumLogx = new TProfile("pTCPtvsGenPtByTCPtsumLogx","pTCPtvsGenPtByTCPtsumLogx", int(logbins.size()-1), plogbins, -4., 4.);
  TProfile *pTCPtvsGenPtByTCPtsumDet[7];
  for(int idet=0;idet<7;idet++){
    pTCPtvsGenPtByTCPtsumDet[idet] = new TProfile(Form("pTCPtvsGenPtByTCPtsumDet_%d",idet),Form("pTCPtvsGenPtByTCPtsumDet_%d",idet), 6000, 0., 60000., -4., 4.);
  }
  //////////////////////////////////////////////////////////
  TH1D *tcRoz = new TH1D("tcRoz",Form("Roz distribution of TCs w.r.t gen"),1000,0.0,1.0);
  double tcRoZCut = 0.5;
  ///////////////////////////////////////////////////////////
  TH1D *hGenvsJetPtDiff = new TH1D("hGenvsJetPtDiff",Form("(p_{T}^{gen}-p_{T}{genJet}) GeV"),200,-100.0,100.0);
  TProfile *pGenvsJetPtDiff = new TProfile("pGenvsJetPtDiff","pGenvsJetPtDiff", 100, 0., 500., -10., 10.);
  TH1D *hGenvsJetDR = new TH1D("hGenvsJetDR",Form("#DeltaR"),4000,-0.1,0.3);
  
  ///////////////////////////////////////////////////////////
  TF1* f2 = new TF1(Form("func"),"[0]*sqrt((x-[1])*(x-[1])+[2]*(x-[1]))*pow(([3]-(x-[1])),2)-1",0,40);
  f2->SetParameters(1.75139e-05,-0.191719,290.217,45.1571);

  //////////////////////////////////////////////////////////
  uint64_t totalEntries = tr->GetEntries();
  if(nofEvents==0 or nofEvents>totalEntries) nofEvents = totalEntries;
  
  std::cout << "Total Entries : " << totalEntries << std::endl;
  std::cout << "Loop for: " << nofEvents << " entries"  << std::endl;
  
  
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
    std::vector<JetPart> genlist;
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
      
      JetPart p;
      p.name = (!partPDG)?"unknown":partPDG->GetName();
      p.index = igen;
      genlist.push_back(p);
      
      if(doPrint and igen<=8
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
	// if(doPrint)
	//   std::cout << "genpart-ievent: " << std::setprecision(default_precision) << std::setw(4) <<ievent << ", ipart: " << std::setw(4) << ipart
	// 	    <<", pid: " << std::setw(5) << genpart_pid->at(ipart) << ", mother: " << std::setw(4) << genpart_mother->at(ipart)
	//     //<<", status: " << std::setw(5) << gen_status->at(ipart)
	// 	    <<", gen: " << std::setw(4) << (genpart_gen->at(ipart)-1) //<< ", fromBeamPipe: " << std::setw(4) << genpart_fromBeamPipe->at(ipart)
	// 	    <<", Name: " << std::setw(10) << ((!partPDG)?"unknown":partPDG->GetName())
	// 	    <<" part:(pt,eta,phi,energy) : (" << std::fixed << std::setprecision(2) << std::setw(8) << genpart_pt->at(ipart)
	// 	    << ", " << std::setw(8) << genpart_eta->at(ipart) << ", " //<< std::setw(5) << genpart_exeta->at(ipart)
	// 	    << ", " << std::setw(8) << (TMath::RadToDeg()*genpart_phi->at(ipart)) //<< ", " << std::setw(5) << (TMath::RadToDeg()*genpart_exphi->at(ipart))
	// 	    << ", " << std::setw(8) << genpart_energy->at(ipart) << ") "
	// 	    << std::defaultfloat
	// 	    << std::endl;
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
      int minDeltaRindex = -1;
      float minDeltaR = 1.0; 
      float minDR[3] = {1., 1., 1.};      
      // for(int ipart=0; ipart<partlist.size(); ipart++){
      // 	int refpart = partlist.at(ipart).index;
      // double deltaR = TMath::Sqrt((genpart_eta->at(refpart)-genjet_eta->at(ijet))*(genpart_eta->at(refpart)-genjet_eta->at(ijet)) + (genpart_phi->at(refpart)-genjet_phi->at(ijet))*(genpart_phi->at(refpart)-genjet_phi->at(ijet)));
      for(int ipart=0; ipart<genlist.size(); ipart++){
	int refpart = genlist.at(ipart).index;
	double deltaR = TMath::Sqrt((gen_eta->at(refpart)-genjet_eta->at(ijet))*(gen_eta->at(refpart)-genjet_eta->at(ijet)) + (gen_phi->at(refpart)-genjet_phi->at(ijet))*(gen_phi->at(refpart)-genjet_phi->at(ijet)));
	double deltaR_gen5 = TMath::Sqrt( (gen_eta->at(5)-genjet_eta->at(ijet))*(gen_eta->at(5)-genjet_eta->at(ijet)) + (gen_phi->at(5)-genjet_phi->at(ijet))*(gen_phi->at(5)-genjet_phi->at(ijet)) );
	double deltaR_gen6 = TMath::Sqrt( (gen_eta->at(6)-genjet_eta->at(ijet))*(gen_eta->at(6)-genjet_eta->at(ijet)) + (gen_phi->at(6)-genjet_phi->at(ijet))*(gen_phi->at(6)-genjet_phi->at(ijet)) );
	double deltaR_gen7 = TMath::Sqrt( (gen_eta->at(7)-genjet_eta->at(ijet))*(gen_eta->at(7)-genjet_eta->at(ijet)) + (gen_phi->at(7)-genjet_phi->at(ijet))*(gen_phi->at(7)-genjet_phi->at(ijet)) );
	if(deltaR<0.4 and deltaR<minDeltaR and (deltaR_gen5<0.1 or deltaR_gen6<0.1 or deltaR_gen7<0.1)){
	  //if(deltaR<0.4 and deltaR<minDeltaR){
	  minDeltaR = deltaR;
	  minDeltaRindex = ipart;
	  minDR[0] = deltaR_gen5;
	  minDR[1] = deltaR_gen6;
	  minDR[2] = deltaR_gen7;
	}
      }
      //for(int ipart=0; ipart<partlist.size(); ipart++){
      for(int ipart=0; ipart<genlist.size(); ipart++){
	//JetPart p = partlist.at(ipart);
	JetPart p = genlist.at(ipart);
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
	  int mindr = TMath::LocMin(3,minDR);
	  int minId = mindr + 5; //index of non-Higgs particles of the hardest subprocess in VBF sample
	  hGenvsJetPtDiff->Fill(gen_pt->at(minId)-genjet_pt->at(ijet));	  
	  pGenvsJetPtDiff->Fill( gen_pt->at(minId),  gen_pt->at(minId)/genjet_pt->at(ijet) );
	  hGenvsJetDR->Fill( minDR[mindr]  );
	  if(doPrint)
	    std::cout << "ijet-ievent: " << std::setprecision(default_precision) << std::setw(4) <<ievent << ", ijet: " << std::setw(4) << ijet
		      <<", Name: " << std::setw(10) << p.name << ", deltaR : " << std::setprecision(3) << std::setw(5) << minDeltaR
		      << ", deltaR5 : " << std::setprecision(3) << std::setw(5) << minDR[0]
		      << ", deltaR6 : " << std::setprecision(3) << std::setw(5) << minDR[1]
		      << ", deltaR7 : " << std::setprecision(3) << std::setw(5) << minDR[2]
		      <<" jet:(pt,eta,phi,energy) : (" << std::fixed << std::setprecision(2) << std::setw(8) << genjet_pt->at(ijet)
		      << ", " << std::setw(8) << genjet_eta->at(ijet) << ", " //<< std::setw(5) << genjet_exeta->at(ijet)
		      << ", " << std::setw(8) << (TMath::RadToDeg()*genjet_phi->at(ijet)) //<< ", " << std::setw(5) << (TMath::RadToDeg()*genjet_exphi->at(ijet))
		      << ", " << std::setw(8) << genjet_energy->at(ijet) << ") "
		      <<" jet:(roz,xoz,yoz) : (" << std::fixed << std::setprecision(2) << std::setw(5) << roz
		      <<", " << std::fixed << std::setprecision(2) << std::setw(5) << xoz
		      <<", " << std::fixed << std::setprecision(2) << std::setw(5) << yoz
		      << ") "
		      << std::defaultfloat
		      << std::endl;
	}//minDelta condition
      }//part loop
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
    double totTCpt_posEta_iter1 = 0.,totTCpt_negEta_iter1 = 0.;
    double totTCpt_posEta_CEE_iter1 = 0.,totTCpt_negEta_CEE_iter1 = 0.;
    for(unsigned itc=0;itc<tc_pt->size();itc++){
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
      tcRoz->Fill(minRoz, tc_pt->at(itc));
      if(fabs(clstEta)>1.6 and fabs(clstEta)<3.0 and minRoz<tcRoZCut){ //only add for the plateau region	
	if(tc_eta->at(itc)<0){
	  totTCpt_negEta_iter1 += tc_pt->at(itc);
	  if(tc_layer->at(itc)<=26) totTCpt_negEta_CEE_iter1 += tc_pt->at(itc);
	}else{
	  totTCpt_posEta_iter1 += tc_pt->at(itc);
	  if(tc_layer->at(itc)<=26) totTCpt_posEta_CEE_iter1 += tc_pt->at(itc);
	}
      }
    }    
    /////////////////////////////////////////////////////////////////////////////////////////
    float tot_tc_pt = 0.0, tot_tc_e = 0.0;
    double totTCpt_posEta = 0.,totTCpt_negEta = 0.;
    double totTCpt_posEta_CEE = 0.,totTCpt_negEta_CEE = 0.;
    double totTCpt_posEta_layer1 = 0.,totTCpt_negEta_layer1 = 0.;
    uint32_t nofPosEtaTCs = 0, nofNegEtaTCs = 0, nofTCsTrigL2 = 0, nofTCsTrigL9 = 0;
    double tcXoZposEta = 0.,tcYoZposEta = 0., tcXoZnegEta = 0.,tcYoZnegEta = 0.;
    double maxroz_posEta = -1.0, maxroz_negEta = -1.0;
    double tcPhi_posEta = 0.,tcPhi_negEta = 0.;
    double tcEta_posEta = 0.,tcEta_negEta = 0.;
    double maxTCPt_posEta = -1.0, maxTCPt_negEta = -1.0;
    int maxTCPt_posEta_layer = -1, maxTCPt_negEta_layer = -1;
    int maxTCPt_posEta_detType = -1, maxTCPt_negEta_detType = -1;
    double maxTCPt_posEta_genPt = -1, maxTCPt_negEta_genPt = -1;
    double maxTCPt_posEta_genEta = -1, maxTCPt_negEta_genEta = -1;
    int detType = -1;
    int subdet = -1, wafertype = -1;
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
      
      double tcpt = (tc_pt->at(itc)<40.)?tc_pt->at(itc):40;
      double reso = f2->Eval(tcpt);
      double ptcorr = 1.;
      if(tc_layer->at(itc)<=26) {
	if(reso>0.)
	  ptcorr = 1./(1. - reso);
	else if(reso<0.)
	  ptcorr = 1 - abs(reso);
	else
	  ptcorr = 1. ;
      }
      
      // if(doPrint){
      // 	std::cout << "itc-ievent: " << std::fixed << std::setprecision(2) << std::setw(4) << ievent
      // 		  << ", itc: " << std::setw(5) << itc
      // 		  << ", layer: " << std::setw(2) << int(tc_layer->at(itc))
      // 		  <<", (x,y,z): (" << std::setw(7) << tc_x->at(itc) << ", " << std::setw(7) << tc_y->at(itc) << ", " << std::setw(7) << tc_z->at(itc) << ")"
      // 		  <<", (pt,eta,phi,energy): (" << std::fixed << std::setprecision(5) << std::setw(8) << tc_pt->at(itc)
      // 		  << ", " << std::setw(8) << tc_eta->at(itc) 
      // 		  << ", " << std::setw(8) << (TMath::RadToDeg()*tc_phi->at(itc)) 
      // 		  << ", " << std::setw(8) << tc_energy->at(itc)*1.e3 << ") "
      // 		  <<", (reso, ptcorr,new_pt) : ( " << std::fixed << std::setprecision(5) << std::setw(8) << reso
      // 		  << ", " << std::setw(8) << ptcorr
      // 		  << ", " << std::setw(8) << tc_pt->at(itc)*ptcorr << ") "
      // 		  << std::defaultfloat
      // 		  << std::endl;
      // }
      
      for (uint32_t isect = 0 ; isect < 3 ; isect++ ){
	uint32_t addisect = 0;
	if(tc_z->at(itc)>0.0) addisect = 3;
	tcf0.setZero();
	tcf0.setROverZPhiF(tc_x->at(itc)/z,tc_y->at(itc)/z,isect+addisect);
	if(tcf0.getXOverZF()>=0.0){
	  float scale = 1.;//ptcorr;
	  //if(tc_layer->at(itc)==27 or tc_layer->at(itc)==29) scale = 1.5;
	  tcf0.setEnergyGeV(scale * tc_pt->at(itc));
	  tcf0.setLayer(tc_layer->at(itc));
	  //if(doPrint) tcf0.print();
	  vTcw[isect+addisect].push_back(tcf0);
	}
      }
      // bool hgcalEta = (fabs(tc_eta->at(itc))>1.321 and fabs(tc_eta->at(itc))<3.152) ? true : false;
      //bool hgcalInnerEta = (fabs(tc_eta->at(itc))>1.6 and fabs(tc_eta->at(itc))<3.0) ? true : false;
      bool hgcalInnerEta = (fabs(clstEta)>1.6 and fabs(clstEta)<3.0) ? true : false;
      double tcptMeV = tc_pt->at(itc) * 1.e3;
      int binEta = 1, binPt = 1, binPtLog = 1;
      GetHistoBin(hJetEtaTCPtBin, tc_eta->at(itc), tc_pt->at(itc), binEta, binPt);
      GetHistoBin(hJetEtaTCPtBinLog, tc_eta->at(itc), tcptMeV, binEta, binPtLog);

      if(tc_eta->at(itc)<0){
	//if(hgcalInnerEta){
	totTCpt_negEta += tc_pt->at(itc);
	if(tc_layer->at(itc)<=26) totTCpt_negEta_CEE += tc_pt->at(itc);
	//}
	tcXoZnegEta += tc_pt->at(itc)*tc_x->at(itc)/tc_z->at(itc) ;
	tcYoZnegEta += tc_pt->at(itc)*tc_y->at(itc)/tc_z->at(itc) ;
	tcPhi_negEta += tc_pt->at(itc)*tc_phi->at(itc) ;
	tcEta_negEta += tc_pt->at(itc)*tc_eta->at(itc) ;
	nofNegEtaTCs++;
	double droz = sqrt( (tc_x->at(itc)/tc_z->at(itc)-xoz_negEta)*(tc_x->at(itc)/tc_z->at(itc)-xoz_negEta) + (tc_y->at(itc)/tc_z->at(itc)-yoz_negEta)*(tc_y->at(itc)/tc_z->at(itc)-yoz_negEta) );
	if(droz>maxroz_negEta) maxroz_negEta = droz;
	if(tc_pt->at(itc)>maxTCPt_negEta){
	  //if(tc_uncompressedCharge->at(itc)>maxTCPt_negEta){
	  //maxTCPt_negEta = tc_uncompressedCharge->at(itc);//tc_pt->at(itc);
	  maxTCPt_negEta = tc_pt->at(itc);
	  maxTCPt_negEta_layer = tc_layer->at(itc);
	  maxTCPt_negEta_detType = detType;
	  maxTCPt_negEta_genPt = clstPt;
	  maxTCPt_negEta_genEta = clstEta;
	}
	//if(hgcalInnerEta and minRoz<tcRoZCut and clstPt>(pt_clusThresh*0.5)){
	if(hgcalInnerEta and minRoz<tcRoZCut){
	  h2TCPtvsGenPtByTCPtsum->Fill(tcptMeV, (clstPt/totTCpt_negEta_iter1));
	  pTCPtvsGenPtByTCPtsum->Fill(tcptMeV, (clstPt/totTCpt_negEta_iter1));
	  pTCPtvsGenPtByTCPtsumLogx->Fill(tcptMeV, (clstPt/totTCpt_negEta_iter1));
	  pTCPtvsGenPtByTCPtsumDet[detType-1]->Fill(tcptMeV, (clstPt/totTCpt_negEta_iter1));
	  hGenTCPtResovsTCPt1D[binPt-1]->Fill( (clstPt - totTCpt_negEta_iter1) );
	  hGenTCPtResovsTCPt1DLog[binPtLog-1]->Fill( (clstPt - totTCpt_negEta_iter1) );
     	  hGenTCPtResovsTCPt2D[detType-1][binPt-1]->Fill( (clstPt - totTCpt_negEta_iter1) );
	}
      }else{
	//if(hgcalInnerEta){
	totTCpt_posEta += tc_pt->at(itc);
	if(tc_layer->at(itc)<=26) totTCpt_posEta_CEE += tc_pt->at(itc);
	//}
	tcXoZposEta += tc_pt->at(itc)*tc_x->at(itc)/tc_z->at(itc) ;
	tcYoZposEta += tc_pt->at(itc)*tc_y->at(itc)/tc_z->at(itc) ;
	tcPhi_posEta += tc_pt->at(itc)*tc_phi->at(itc) ;
	tcEta_posEta += tc_pt->at(itc)*tc_eta->at(itc) ;
	nofPosEtaTCs++;
	double droz = sqrt( (tc_x->at(itc)/tc_z->at(itc)-xoz_posEta)*(tc_x->at(itc)/tc_z->at(itc)-xoz_posEta) + (tc_y->at(itc)/tc_z->at(itc)-yoz_posEta)*(tc_y->at(itc)/tc_z->at(itc)-yoz_posEta) );
	if(droz>maxroz_posEta) maxroz_posEta = droz;
	if(tc_pt->at(itc)>maxTCPt_posEta){
	  //if(tc_uncompressedCharge->at(itc)>maxTCPt_posEta){
	  //maxTCPt_posEta = tc_uncompressedCharge->at(itc);//tc_pt->at(itc);
	  maxTCPt_posEta = tc_pt->at(itc);
	  maxTCPt_posEta_layer = tc_layer->at(itc);
	  maxTCPt_posEta_detType = detType;
	  maxTCPt_posEta_genPt = clstPt;
	  maxTCPt_posEta_genEta = clstEta;
	}
	//if(hgcalInnerEta and minRoz<tcRoZCut and clstPt>(pt_clusThresh*0.5)){
	if(hgcalInnerEta and minRoz<tcRoZCut){
	  h2TCPtvsGenPtByTCPtsum->Fill(tcptMeV, (clstPt/totTCpt_posEta_iter1));
	  pTCPtvsGenPtByTCPtsum->Fill(tcptMeV, (clstPt/totTCpt_posEta_iter1));
	  pTCPtvsGenPtByTCPtsumLogx->Fill(tcptMeV, (clstPt/totTCpt_posEta_iter1));
	  pTCPtvsGenPtByTCPtsumDet[detType-1]->Fill(tcptMeV, (clstPt/totTCpt_posEta_iter1));
	  hGenTCPtResovsTCPt1D[binPt-1]->Fill( (clstPt - totTCpt_posEta_iter1) );
	  hGenTCPtResovsTCPt1DLog[binPtLog-1]->Fill( (clstPt - totTCpt_posEta_iter1) );
     	  hGenTCPtResovsTCPt2D[detType-1][binPt-1]->Fill( (clstPt - totTCpt_posEta_iter1) );
	}
      }
      
      tot_tc_pt += tc_pt->at(itc);
      tot_tc_e += tc_energy->at(itc);
      if(tc_layer->at(itc)==2) nofTCsTrigL2++;
      if(tc_layer->at(itc)==9) nofTCsTrigL9++;
    }//end of TC loop
    //rearrgdtcs.clear();
    
    if(fabs(maxTCPt_posEta_genEta)>1.6 and fabs(maxTCPt_posEta_genEta)<3.0){
      hMaxLayerE->Fill(maxTCPt_posEta_layer,totTCpt_posEta);
      hMaxDetE->Fill(maxTCPt_posEta_detType,totTCpt_posEta);
      if(maxTCPt_posEta_layer<=26){
	h2TCvsGenPt->Fill(maxTCPt_posEta, maxTCPt_posEta_genPt);
	pTCvsGenPt->Fill(maxTCPt_posEta, maxTCPt_posEta_genPt);
      }
      h2TCvsGenPtDet[maxTCPt_posEta_detType-1]->Fill(maxTCPt_posEta, maxTCPt_posEta_genPt);
      pTCvsGenPtDet[maxTCPt_posEta_detType-1]->Fill(maxTCPt_posEta, maxTCPt_posEta_genPt);
    }
    
    if(fabs(maxTCPt_negEta_genEta)>1.6 and fabs(maxTCPt_negEta_genEta)<3.0){
      hMaxLayerE->Fill(maxTCPt_negEta_layer,totTCpt_negEta);
      hMaxDetE->Fill(maxTCPt_negEta_detType, totTCpt_negEta);
      if(maxTCPt_negEta_layer<=26){
	h2TCvsGenPt->Fill(maxTCPt_negEta, maxTCPt_negEta_genPt);
	pTCvsGenPt->Fill(maxTCPt_negEta, maxTCPt_negEta_genPt);
      }
      h2TCvsGenPtDet[maxTCPt_negEta_detType-1]->Fill(maxTCPt_negEta, maxTCPt_negEta_genPt);
      pTCvsGenPtDet[maxTCPt_negEta_detType-1]->Fill(maxTCPt_negEta, maxTCPt_negEta_genPt);
    }
    
    //if(doPrint)
    //std::cout<<"tot_tc_pt : "<< tot_tc_pt << std::endl;
    for (uint32_t isect = 0 ; isect < 6 ; isect++ ){
      //std::cout << "ievent : " << ievent << ", isect: " << isect << ", size : " << vTcw[isect].size() << std::endl;
    }
    
    // ///////////////////=========== Emulation ============= ///////////////////////
    // //std::vector<TPGClusterData> vCld[6];
    std::vector<TPGCluster> vCld[6];    
    // for (uint32_t isect = 0 ; isect < 6 ; isect++ ) {
    //   s2Clustering.run(vTcw[isect],vCld[isect]);
    //   if(doPrint) std::cout << isect << ", Size of Tcs: " << vTcw[isect].size() << ", Size of Clusters: " << vCld[isect].size() << std::endl;
    // }
    // ///////////////////=========== Emulation ============= ///////////////////////

    // int nofClus1GeV = 0;
    // int nofClus3GeV = 0;
    // int nofClus5GeV = 0;
    // int nofClus10GeV = 0;
    // for (uint32_t isect = 0 ; isect < 6 ; isect++ ){
    //   //for(TPGCluster const& clf : vCld[isect]){
    //   if(isect==0 or isect==3){
    // 	nofClus1GeV = 0;
    // 	nofClus3GeV = 0;
    // 	nofClus5GeV = 0;
    // 	nofClus10GeV = 0;
    //   }
    //   int iclus = 0;
    //   for(TPGCluster const& clf : vCld[isect]){
    // 	double tcPtSum = (clf.getGlobalEtaRad(isect) < 0) ? totTCpt_negEta : totTCpt_posEta ;
    // 	if(doPrint){
    // 	  std::cout << "iclus-ievent: " << std::fixed << std::setprecision(2) << std::setw(4) << ievent
    // 		    << ", sector: " << std::setw(5) << isect
    // 		    << ", iclus: " << std::setw(5) << iclus++
    // 		    <<", (x,y,z): (" << std::setw(7) << (clf.getGlobalXOverZF(isect) * clf.getZCm()) << ", " << std::setw(7) << (clf.getGlobalYOverZF(isect) * clf.getZCm()) << ", " << std::setw(7) << clf.getGlobalZCm(isect) << ")"
    // 		    <<", (xoz,yoz,roz): (" << std::setw(5) << clf.getGlobalXOverZF(isect) << ", " << std::setw(5) << clf.getGlobalYOverZF(isect) << ", " << std::setw(5) << clf.getGlobalRhoOverZF(isect) << ")"
    // 		    <<", (pt,eta,phi): (" << std::fixed << std::setprecision(2) << std::setw(8) << clf.getEnergyGeV()
    // 		    << ", " << std::setw(8) << clf.getGlobalEtaRad(isect)
    // 		    << ", " << std::setw(8) << (TMath::RadToDeg()* clf.getGlobalPhiRad(isect))
    // 	            //<< ", " << std::setw(8) << tc_energy->at(itc)*1.e6
    // 		    << ") "
    // 		    // <<", (tcPtSum,tcPtSum/lsb,energy,energy*lsb): (" << std::setw(8) << tcPtSum
    // 		    // << ", " << std::setw(8) << uint32_t(tcPtSum/lsbScales.LSB_E_TC()+0.5)
    // 		    // << ", " << std::setw(8) << clf.getEnergy()
    // 		    // << ", " << std::setw(8) << (clf.getEnergy()*lsbScales.LSB_E_TC())
    // 		    // << ") "
    // 		    << std::defaultfloat
    // 		    << std::endl;
    // 	}

    // 	if(clf.getEnergyGeV()>1.0) nofClus1GeV++;
    // 	if(clf.getEnergyGeV()>3.0) nofClus3GeV++;	
    // 	if(clf.getEnergyGeV()>5.0) nofClus5GeV++;
    // 	if(clf.getEnergyGeV()>10.0) nofClus10GeV++;

    //   }//cluster loop
    // }//sector loop
    
    // for(int ipjet=0; ipjet < jetlist.size() ; ipjet++ ){
    //   int ijet = jetlist.at(ipjet).index ;
    //   //hEt->Fill(genjet_pt->at(ijet));
    //   int minsect = (genjet_eta->at(ijet) < 0) ? 0 : 3;
    //   int maxsect = (genjet_eta->at(ijet) < 0) ? 3 : 6;
    //   double tcPtSum = (genjet_eta->at(ijet) < 0) ? totTCpt_negEta : totTCpt_posEta ;
    //   double tcPtSum_CEE = (genjet_eta->at(ijet) < 0) ? totTCpt_negEta_CEE : totTCpt_posEta_CEE ;
    //   int tcDetType = (genjet_eta->at(ijet) < 0) ? maxTCPt_negEta_detType : maxTCPt_posEta_detType ;
    //   double tcPt = (genjet_eta->at(ijet) < 0) ? maxTCPt_negEta : maxTCPt_posEta ;
    //   double avgXoZ = (genjet_eta->at(ijet) < 0) ? tcXoZnegEta/tcPtSum : tcXoZposEta/tcPtSum ;
    //   double avgYoZ = (genjet_eta->at(ijet) < 0) ? tcYoZnegEta/tcPtSum : tcYoZposEta/tcPtSum ;
    //   double avgPhi = (genjet_eta->at(ijet) < 0) ? tcPhi_negEta/tcPtSum : tcPhi_posEta/tcPtSum ;
    //   double avgEta = (genjet_eta->at(ijet) < 0) ? tcEta_negEta/tcPtSum : tcEta_posEta/tcPtSum ;
    //   double gjroz = 1/sinh(genjet_eta->at(ijet)) ; //tan(2*atan(exp(-1.0*genjet_eta->at(ijet))));
    //   double gjxoz = gjroz*cos(genjet_phi->at(ijet));
    //   double gjyoz = gjroz*sin(genjet_phi->at(ijet));
    //   double avgRoZ = sqrt((avgXoZ-gjxoz)*(avgXoZ-gjxoz) + (avgYoZ-gjyoz)*(avgYoZ-gjyoz));
    //   bool hasFound = false, hasFoundPt = false;
    //   int binPt, binEta;
    //   double cluspt = genjet_pt->at(ijet);
    //   int nofClus = 0;
    //   //hTrigGen->Fill(genjet_pt->at(ijet));
    //   double minRoz = 1., closestClusE= 100., closestClusEDiff= 100., totClusE = 0.;
    //   for (uint32_t isect = minsect ; isect < maxsect ; isect++ ){
    // 	for(TPGCluster const& clf : vCld[isect]){	  
    // 	  if(clf.getEnergyGeV()<3.0) continue;
    // 	  double dClusGenRoz = sqrt( (clf.getGlobalXOverZF(isect) - gjxoz)*(clf.getGlobalXOverZF(isect) - gjxoz) + (clf.getGlobalYOverZF(isect) - gjyoz)*(clf.getGlobalYOverZF(isect) - gjyoz) );
    // 	  if(dClusGenRoz<minRoz){
    // 	    minRoz = dClusGenRoz;
    // 	    closestClusE = clf.getEnergyGeV();
    // 	    closestClusEDiff = clf.getEnergyGeV() - genjet_pt->at(ijet);
    // 	  }//minRoZ condition
    // 	  totClusE += clf.getEnergyGeV() ; 
    // 	}//cluster loop
    //   }//sector loop
    //   double minRoz_cl3d = 1., closestClusE_cl3d= 100.;
    //   for(int icl3=0;icl3<cl3d_n;icl3++){	  
    // 	if(cl3d_pt->at(icl3)<3.0) continue;
    // 	double clusXoZ = cos(cl3d_phi->at(icl3))/sinh(cl3d_eta->at(icl3)) ; 
    // 	double clusYoZ = sin(cl3d_phi->at(icl3))/sinh(cl3d_eta->at(icl3)); 
    // 	double dClusGenRoz = sqrt( (clusXoZ - gjxoz)*(clusXoZ - gjxoz) + (clusYoZ - gjyoz)*(clusYoZ - gjyoz) );
    // 	if(dClusGenRoz<minRoz_cl3d){
    // 	  minRoz_cl3d = dClusGenRoz;
    // 	  closestClusE_cl3d = cl3d_pt->at(icl3);
    // 	}//minRoZ condition
    //   }//cluster loop
      
    //   bool hasClosestFound = (minRoz<1.)?true:false;
    //   bool hasClosestFound_cl3d = (minRoz_cl3d<1.)?true:false;
    //   // bool hasClosestFoundPt = (hasClosestFound and closestClusE>pt_clusThresh)?true:false;
    //   // bool hasClosestFoundPtTot = (hasClosestFound and totClusE>pt_clusThresh)?true:false;
    //   //Eta : outermost(Sci) = 1.321, innermost(Si) =  3.152
    //   bool hgcalEta = (fabs(genjet_eta->at(ijet))>1.321 and fabs(genjet_eta->at(ijet))<3.152) ? true : false;
    //   bool hgcalInnerEta = (fabs(genjet_eta->at(ijet))>1.6 and fabs(genjet_eta->at(ijet))<3.0) ? true : false;
    //   // bool hgcalOuterEta = (fabs(genjet_eta->at(ijet))>1.6 and fabs(genjet_eta->at(ijet))<3.05) ? true : false;
      
    //   hJetEtaPtBin->Fill(genjet_eta->at(ijet), genjet_pt->at(ijet));
    //   GetHistoBin(hJetEtaPtBin,genjet_eta->at(ijet), genjet_pt->at(ijet), binEta, binPt);
    //   if(hgcalEta and hasClosestFound) {
    // 	hJetEtaPtBinDet->Fill( abs(genjet_eta->at(ijet)), genjet_pt->at(ijet));
	
    // 	hGenClusPtReso[binEta-1][binPt-1]->Fill( (genjet_pt->at(ijet) - closestClusE) );
    // 	hGenTCPtReso[binEta-1][binPt-1]->Fill( (genjet_pt->at(ijet) - tcPtSum_CEE) );
    // 	//hTCClusPtReso[binEta-1][binPt-1]->Fill( (tcPtSum - closestClusE) );
    // 	hTCClusPtReso[binEta-1][binPt-1]->Fill( (genjet_pt->at(ijet) - tcPtSum) );
	
    // 	hGenClusPtReso3D[tcDetType-1][binEta-1][binPt-1]->Fill( (genjet_pt->at(ijet) - closestClusE) );
	
    // 	if(tcDetType<=3){
    // 	  hTCElossCEEID[binEta-1]->Fill( tcPtSum_CEE );
    // 	  hTCElossCE_E[binEta-1][binPt-1]->Fill( tcPtSum_CEE );
    // 	  // hTCElossCE_E_3D[tcDetType-1][binEta-1][binPt-1]->Fill( tcPtSum_CEE );
    // 	}else{
    // 	  hTCElossCEHID[binEta-1]->Fill( (tcPtSum-tcPtSum_CEE) );
    // 	  hTCElossCE_H[binEta-1][binPt-1]->Fill( (tcPtSum-tcPtSum_CEE) );
    // 	  // hTCElossCE_H_3D[tcDetType-1][binEta-1][binPt-1]->Fill( (tcPtSum-tcPtSum_CEE) );

    // 	  //hHadGenClusPtReso2D[binEta-1][binPt-1]->Fill( ((genjet_pt->at(ijet)-tcPtSum_CEE) - closestClusE) );
    // 	  // hHadGenClusPtReso3D[tcDetType-1][binEta-1][binPt-1]->Fill( ((genjet_pt->at(ijet)-tcPtSum_CEE) - closestClusE) );
    // 	}
	
    // 	if(hgcalInnerEta){
    // 	  hGenClusPtReso1D[binPt-1]->Fill( (genjet_pt->at(ijet) - closestClusE) );
    // 	  hGenTCPtReso1D[binPt-1]->Fill( (genjet_pt->at(ijet) - tcPtSum_CEE) );
    // 	  //hTCClusPtReso1D[binPt-1]->Fill( (tcPtSum - closestClusE) );
    // 	  hTCClusPtReso1D[binPt-1]->Fill( (genjet_pt->at(ijet) - tcPtSum) );	  
	  
    // 	  GetHistoBin(hJetEtaTCPtBin,genjet_eta->at(ijet), tcPt, binEta, binPt);
    // 	  hGenTCPtResovsTCPt1D[binPt-1]->Fill( (genjet_pt->at(ijet) - tcPtSum) );
    // 	  hGenTCPtResovsTCPt2D[tcDetType-1][binPt-1]->Fill( (genjet_pt->at(ijet) - tcPtSum) );
	  
    // 	  // if(tcDetType>=4)  hHadGenClusPtReso1D[binPt-1]->Fill( ((genjet_pt->at(ijet)-tcPtSum_CEE) - closestClusE) );
    // 	  // hHadGenTCPtReso1D[binPt-1]->Fill( ((genjet_pt->at(ijet)-tcPtSum_CEE) - closestClusE) );
    // 	  // hHadTCClusPtReso1D[binPt-1]->Fill( ((tcPtSum-tcPtSum_CEE) - closestClusE) );
    // 	  // hHadGenClusPtReso1D[binPt-1]->Fill((genjet_pt->at(ijet)-tcPtSum_CEE));
    // 	  // hHadGenTCPtReso1D[binPt-1]->Fill( tcPtSum - tcPtSum_CEE );
    // 	  // hHadTCClusPtReso1D[binPt-1]->Fill( tcPtSum_CEE );
    // 	}
    //   }
    //   // if(hgcalInnerEta and hasClosestFound_cl3d) {
    //   // 	hGenClusPtReso1Dcl3d[binPt-1]->Fill( (genjet_pt->at(ijet) - closestClusE_cl3d) );
    //   // }
      
    // }//jet loop 
    

    
    for(uint16_t i=0;i<6;i++) {
      vCld[i].clear();
      vTcw[i].clear();
    }
    
    taudlist.clear();
    taugdlist.clear();
    genlist.clear();
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
    tc_subdet->clear();
    tc_wafertype->clear();
    tc_uncompressedCharge->clear();
    cl3d_pt->clear();
    cl3d_phi->clear();
    cl3d_eta->clear();
  }//event loop
  
  std::string outname = "CalcResolution_" + outputfile_extn + "_" + index;

  TH1D *tcRoz_cumul = (TH1D *)tcRoz->GetCumulative(kTRUE,"_cumul");
  tcRoz_cumul->Scale(1./tcRoz_cumul->GetBinContent(tcRoz_cumul->GetMaximumBin()));

  TFile *fout = new TFile(Form("%s.root",outname.c_str()),"recreate");
  hGenvsJetPtDiff->Write();
  pGenvsJetPtDiff->Write();
  hGenvsJetDR->Write();
  hJetEtaPtBin->Write();
  hJetEtaPtBinDet->Write();
  h2TCvsGenPt->Write();
  pTCvsGenPt->Write();
  h2TCPtvsGenPtByTCPtsum->Write();
  pTCPtvsGenPtByTCPtsum->Write();
  pTCPtvsGenPtByTCPtsumLogx->Write();
  tcRoz->Write();
  tcRoz_cumul->Write();
  //=============================================
  TDirectory *dir = fout->mkdir("Reso_1D");
  dir->cd();
  for(int ipt=0;ipt<nJetPtBins;ipt++) hGenClusPtReso1D[ipt]->Write();
  for(int ipt=0;ipt<nJetPtBins;ipt++) hGenTCPtReso1D[ipt]->Write();
  for(int ipt=0;ipt<nJetPtBins;ipt++) hTCClusPtReso1D[ipt]->Write();
  // for(int ipt=0;ipt<nJetPtBins;ipt++) hHadGenClusPtReso1D[ipt]->Write();
  // for(int ipt=0;ipt<nJetPtBins;ipt++) hHadGenTCPtReso1D[ipt]->Write();
  // for(int ipt=0;ipt<nJetPtBins;ipt++) hHadTCClusPtReso1D[ipt]->Write();
  // for(int ipt=0;ipt<nJetPtBins;ipt++) hGenClusPtReso1Dcl3d[ipt]->Write();
  for(int ieta=0;ieta<nJetEtaBins;ieta++) hTCElossCEEID[ieta]->Write();
  for(int ieta=0;ieta<nJetEtaBins;ieta++) hTCElossCEHID[ieta]->Write();
  for(int idet=0;idet<7;idet++) h2TCvsGenPtDet[idet]->Write();
  for(int idet=0;idet<7;idet++) pTCvsGenPtDet[idet]->Write();
  for(int ipt=0;ipt<nTCPtBins;ipt++) hGenTCPtResovsTCPt1D[ipt]->Write();
  for(int ipt=0;ipt<int(logbins.size()-1);ipt++) hGenTCPtResovsTCPt1DLog[ipt]->Write();
  for(int idet=0;idet<7;idet++) pTCPtvsGenPtByTCPtsumDet[idet]->Write();
  fout->cd();
  //=============================================
  dir = fout->mkdir("Reso_2D");
  dir->cd();
  for(int ieta=0;ieta<nJetEtaBins;ieta++){
    for(int ipt=0;ipt<nJetPtBins;ipt++){
      hGenClusPtReso[ieta][ipt]->Write();
    }// jet et
  }//jet eta
  
  for(int ieta=0;ieta<nJetEtaBins;ieta++){
    for(int ipt=0;ipt<nJetPtBins;ipt++){
      hTCClusPtReso[ieta][ipt]->Write();
    }// jet et
  }//jet eta
  
  for(int ieta=0;ieta<nJetEtaBins;ieta++){
    for(int ipt=0;ipt<nJetPtBins;ipt++){
      hGenTCPtReso[ieta][ipt]->Write();
    }// jet et
  }//jet eta
  
  for(int ieta=0;ieta<nJetEtaBins;ieta++){
    for(int ipt=0;ipt<nJetPtBins;ipt++){
      hTCElossCE_E[ieta][ipt]->Write();
    }// jet et
  }//jet eta
  
  for(int ieta=0;ieta<nJetEtaBins;ieta++){
    for(int ipt=0;ipt<nJetPtBins;ipt++){
      hTCElossCE_H[ieta][ipt]->Write();
    }// jet et
  }//jet eta

  for(int idet=0;idet<7;idet++)
  for(int ipt=0;ipt<nTCPtBins;ipt++)
    hGenTCPtResovsTCPt2D[idet][ipt]->Write();
  // for(int ieta=0;ieta<nJetEtaBins;ieta++){
  //   for(int ipt=0;ipt<nJetPtBins;ipt++){
  //     hHadGenClusPtReso2D[ieta][ipt]->Write();
  //   }// jet et
  // }//jet eta
  
  fout->cd();
  //=============================================
  dir = fout->mkdir("Reso_3D");
  dir->cd();
  // for(int idet=0;idet<7;idet++){
  //   for(int ieta=0;ieta<nJetEtaBins;ieta++){
  //     for(int ipt=0;ipt<nJetPtBins;ipt++){
  // 	hHadGenClusPtReso3D[idet][ieta][ipt]->Write();
  //     }// jet et
  //   }//jet eta
  // }//idet

  for(int idet=0;idet<7;idet++){
    for(int ieta=0;ieta<nJetEtaBins;ieta++){
      for(int ipt=0;ipt<nJetPtBins;ipt++){
	hGenClusPtReso3D[idet][ieta][ipt]->Write();
      }// jet et
    }//jet eta
  }//idet
  
  // for(int idet=0;idet<7;idet++){
  //   for(int ieta=0;ieta<nJetEtaBins;ieta++){
  //     for(int ipt=0;ipt<nJetPtBins;ipt++){
  // 	hGenTCPtReso3D[idet][ieta][ipt]->Write();
  //     }// jet et
  //   }//jet eta
  // }//idet
  
  fout->cd();
  // //=============================================
  // dir = fout->mkdir("CEE_3D");
  // dir->cd();
  // for(int idet=0;idet<7;idet++){
  //   for(int ieta=0;ieta<nJetEtaBins;ieta++){
  //     for(int ipt=0;ipt<nJetPtBins;ipt++){
  // 	hTCElossCE_E_3D[idet][ieta][ipt]->Write();
  //     }// jet et
  //   }//jet eta
  // }//idet
  
  // fout->cd();
  // dir = fout->mkdir("CEH_3D");
  // dir->cd();
  // for(int idet=0;idet<7;idet++){
  //   for(int ieta=0;ieta<nJetEtaBins;ieta++){
  //     for(int ipt=0;ipt<nJetPtBins;ipt++){
  // 	hTCElossCE_H_3D[idet][ieta][ipt]->Write();
  //     }// jet et
  //   }//jet eta
  // }//idet  
  // fout->cd();
  // //=============================================
  hMaxLayerE->Write();
  hMaxDetE->Write();
  
  fout->Close();
  delete fout;

  logbins.clear();
  // fin->Close();
  // delete fin;

  return true;
}
