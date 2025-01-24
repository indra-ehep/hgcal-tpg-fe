/**********************************************************************
 Created on : 06/11/2024
 Purpose    : Read the hgcal-tpg info from ntuples
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include <TROOT.h>
#include <TChain.h>
//#include <TEntryList.h>
#include <TProfile.h>
#include <TFile.h>
#include <TCanvas.h>

// #include "TH1D.h"
// #include "TH2D.h"
// #include "TMath.h"
// #include "TSystem.h"




int main(int argc, char** argv)
{
  
  std::cout << "========= Run as : ./readNtuple.exe $input_file $nofevents ==========" << std::endl;
  
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

  std::vector<float>  *genpart_pt = 0 ;
  tr->SetBranchStatus("genpart_pt",1);
  tr->SetBranchAddress("genpart_pt" , &genpart_pt);
  
  std::vector<float>  *genpart_eta = 0 ;
  tr->SetBranchStatus("genpart_eta",1);
  tr->SetBranchAddress("genpart_eta" , &genpart_eta);

  std::vector<float>  *genpart_phi = 0 ;
  tr->SetBranchStatus("genpart_phi",1);
  tr->SetBranchAddress("genpart_phi" , &genpart_phi);

  std::vector<float>  *tc_pt = 0 ;
  // tr->SetBranchStatus("tc_mipPt",1);
  // tr->SetBranchAddress("tc_mipPt" , &tc_pt);
  tr->SetBranchStatus("tc_pt",1);
  tr->SetBranchAddress("tc_pt" , &tc_pt);
  // tr->SetBranchStatus("cl3d_pt",1);
  // tr->SetBranchAddress("cl3d_pt" , &tc_pt);
  
  std::vector<float>  *tc_eta = 0 ;
  tr->SetBranchStatus("tc_eta",1);
  tr->SetBranchAddress("tc_eta" , &tc_eta);
  // tr->SetBranchStatus("cl3d_eta",1);
  // tr->SetBranchAddress("cl3d_eta" , &tc_eta);

  std::vector<float>  *tc_phi = 0 ;
  tr->SetBranchStatus("tc_phi",1);
  tr->SetBranchAddress("tc_phi" , &tc_phi);
  // tr->SetBranchStatus("cl3d_phi",1);
  // tr->SetBranchAddress("cl3d_phi" , &tc_phi);
  
  TProfile *hEtRatio = new TProfile("hEtRatio","hEtRatio",100,0,1000);

  uint64_t totalEntries = tr->GetEntries();
  if(nofEvents==0) nofEvents = totalEntries;
  
  std::cout << "Total Entries : " << totalEntries << std::endl;
  std::cout << "Loop for: " << nofEvents << " entries"  << std::endl;

  for (Long64_t ievent = 0 ; ievent < nofEvents ; ievent++ ) {    
    
    tr->GetEntry(ievent) ;
    std::cout<<"Event : "<< ievent << std::endl;
    std::cout<<"nof gen parts : "<< genpart_pt->size() << std::endl;
    float tot_gen_pt = 0.0;
    for(unsigned igen=0;igen<genpart_pt->size();igen++) tot_gen_pt += genpart_pt->at(igen);
    std::cout<<"tot_gen_pt : "<< tot_gen_pt << std::endl;
    
    std::cout<<"nof TCs : "<< tc_pt->size() << std::endl;
    float tot_tc_pt = 0.0;
    for(unsigned itc=0;itc<tc_pt->size();itc++){      
      tot_tc_pt += tc_pt->at(itc);
    }
    std::cout<<"tot_tc_pt : "<< tot_tc_pt << std::endl;
    
    hEtRatio->Fill(tot_gen_pt, tot_tc_pt);
    
    genpart_pt->clear();
    tc_pt->clear();
    
  }//event loop

  std::string outname = "PU0_noDeltaR";
  hEtRatio->SetTitle(outname.c_str());
  hEtRatio->GetXaxis()->SetTitle("total p_{T} of Generated particle (in GeV)");
  hEtRatio->GetYaxis()->SetTitle("total p_{T} of Trigger Cell cluster (in GeV)");
  TCanvas *c1 = new TCanvas("c1",outname.c_str());
  hEtRatio->Draw();
  c1->Update();
  
  TFile *fout = new TFile(Form("%s.root",outname.c_str()),"recreate");
  hEtRatio->Write();
  fout->Close();
  delete fout;

  fin->Close();
  delete fin;

  return true;
}
