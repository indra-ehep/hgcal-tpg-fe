/**********************************************************************
 Created on : 06/11/2024
 Purpose    : Read ntuple and fill the input data for stage2 processing
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

#include "TPGTriggerCellFloats.hh"


int main(int argc, char** argv)
{
  
  bool doPrint = 1;
  std::cout << "========= Run as : ./fillInputData.exe $input_file $nofevents ==========" << std::endl;
  
  if(argc < 2){
    std::cerr << argv[0] << ": no input file has been specified specified" << std::endl;
    return false;
  }
  std::string inputfile;
  std::istringstream issInfile(argv[1]);
  issInfile >> inputfile;
  uint64_t nofEvents(0);
  if(argc >= 3){
    std::istringstream issnEvents(argv[2]);
    issnEvents >> nofEvents;
  }
  
  TFile *fin = TFile::Open(Form(inputfile.c_str()));
  std::cout<<"Input filename : " << fin->GetName() << std::endl;
  TTree *tr  = (TTree*)fin->Get("l1tHGCalTriggerNtuplizer/HGCalTriggerNtuple");  
  tr->SetBranchStatus("*",0);
  Int_t event ;
  tr->SetBranchStatus("event",1);
  tr->SetBranchAddress("event"	, &event);
  
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
  
  TH1D *hEt = new TH1D("hEt","hEt",100,0,1000);
  TH1D *hPhi = new TH1D("hPhi","hPhi",2*TMath::TwoPi(),-1.0*TMath::TwoPi(),TMath::TwoPi());
  TH1D *hPhiDeg = new TH1D("hPhiDeg","hPhiDeg",2*TMath::RadToDeg()*TMath::TwoPi(),-1.0*TMath::RadToDeg()*TMath::TwoPi(),TMath::RadToDeg()*TMath::TwoPi());
  
  uint64_t totalEntries = tr->GetEntries();
  if(nofEvents==0) nofEvents = totalEntries;
  
  std::cout << "Total Entries : " << totalEntries << std::endl;
  std::cout << "Loop for: " << nofEvents << " entries"  << std::endl;
  
  TPGTriggerCellFloats tcf0,tcf1;
  for (Long64_t ievent = 0 ; ievent < nofEvents ; ievent++ ) {    
    
    tr->GetEntry(ievent) ;

    if(doPrint) std::cout<<"Event : "<< ievent <<", nof TCs : "<< tc_pt->size() << std::endl;
    
    float tot_tc_pt = 0.0;
    std::vector<TPGTriggerCellWord> vTcw[6];    
    for(unsigned itc=0;itc<tc_pt->size();itc++){
      
      double z(fabs(tc_z->at(itc)));
      float phi_deg = TMath::RadToDeg() * tc_phi->at(itc) ;
      uint16_t sec0(7),sec1(7);

      //The following segmentation is applicable for towers
      // if((phi_deg>-15. and phi_deg<=0.) or (phi_deg>=0. and phi_deg<=5.)) {sec0 = 2; sec1 = 0;}
      // else if(phi_deg>5. and phi_deg<=105.) {sec0 = sec1 = 0;}
      // else if(phi_deg>105. and phi_deg<=125.) {sec0 = 0; sec1 = 1;}
      // else if((phi_deg>125. and phi_deg<=180.) or (phi_deg>=-180. and phi_deg<=-135.)) {sec0 = sec1 = 1;}
      // else if(phi_deg>-135. and phi_deg<=-115.) {sec0 = 1; sec1 = 2;}
      // else {sec0 = sec1 = 2;}

      //The following segmentation is applied for trigger cells
      if(phi_deg>=0. and phi_deg<=60.) {sec0 = 2; sec1 = 0;}
      else if(phi_deg>60. and phi_deg<=120.) {sec0 = sec1 = 0;}
      else if(phi_deg>120. and phi_deg<=180.) {sec0 = 0; sec1 = 1;}
      else if(phi_deg>=-180. and phi_deg<=-120.) {sec0 = sec1 = 1;}
      else if(phi_deg>-120. and phi_deg<=-60.) {sec0 = 1; sec1 = 2;}
      else {sec0 = sec1 = 2;}
      
      if(tc_z->at(itc)<0.){sec0 += 3; sec1 += 3;}
      
      if(doPrint) std::cout << "ievent: " << ievent << ", eventId: " << event << ", itc: " << itc <<", z: " << tc_z->at(itc) 
			    << ", phi_deg: " << phi_deg << ", sec0: " <<  sec0 << ", sec1: " <<  sec1 << std::endl;
      
      tcf0.setZero();
      tcf0.setEnergyGeV(tc_pt->at(itc));
      tcf0.setLayer(tc_layer->at(itc));
      tcf0.setXYOverZF(tc_x->at(itc)/z,tc_y->at(itc)/z,sec0);
      if(tcf0.getYOverZF()>=0.0) vTcw[sec0].push_back(tcf0);
      if(sec0!=sec1){
	tcf1.setZero();
	tcf1.setEnergyGeV(tc_pt->at(itc));
	tcf1.setLayer(tc_layer->at(itc));
	tcf1.setXYOverZF(tc_x->at(itc)/z,tc_y->at(itc)/z,sec1);
	if(tcf1.getYOverZF()>=0.0) vTcw[sec1].push_back(tcf1);
      }
      
      tot_tc_pt += tc_pt->at(itc);
      hPhi->Fill(tc_phi->at(itc));
      hPhiDeg->Fill(phi_deg);
    }//end of TC loop    
    if(doPrint) std::cout<<"tot_tc_pt : "<< tot_tc_pt << std::endl;
    
    hEt->Fill(tot_tc_pt);
    
    tc_pt->clear();
    tc_eta->clear();
    tc_phi->clear();
    tc_layer->clear();
    tc_x->clear();
    tc_y->clear();
    tc_z->clear();
    
  }//event loop
  
  std::string outname = "testhisto";
  // hEt->SetTitle(outname.c_str());
  // hEt->GetXaxis()->SetTitle("total p_{T} trigger cells (in GeV)");
  // TCanvas *c1 = new TCanvas("c1",outname.c_str());
  // hEt->Draw();
  // c1->Update();
  
  TFile *fout = new TFile(Form("%s.root",outname.c_str()),"recreate");
  hEt->Write();
  hPhi->Write();
  hPhiDeg->Write();
  fout->Close();
  delete fout;

  fin->Close();
  delete fin;

  return true;
}
