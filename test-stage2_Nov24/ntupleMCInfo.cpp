/**********************************************************************
 Created on : 12/11/2024
 Purpose    : Read the MC info of generated particles and jets
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
#include <iostream>
#include <iomanip>
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

int main(int argc, char** argv)
{
  
  bool doPrint = 1;
  std::cout << "========= Run as : ./ntupleMCInfo.exe $input_file $nofevents ==========" << std::endl;
  
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

  std::vector<float>  *genjet_pt = 0 ;
  tr->SetBranchStatus("genjet_pt",1);
  tr->SetBranchAddress("genjet_pt" , &genjet_pt);

  std::vector<float>  *genjet_eta = 0 ;
  tr->SetBranchStatus("genjet_eta",1);
  tr->SetBranchAddress("genjet_eta" , &genjet_eta);
  
  std::vector<float>  *genjet_phi = 0 ;
  tr->SetBranchStatus("genjet_phi",1);
  tr->SetBranchAddress("genjet_phi" , &genjet_phi);
  
  uint64_t totalEntries = tr->GetEntries();
  if(nofEvents==0) nofEvents = totalEntries;
  
  std::cout << "Total Entries : " << totalEntries << std::endl;
  std::cout << "Loop for: " << nofEvents << " entries"  << std::endl;
  
  const auto default_precision{std::cout.precision()};
  for (Long64_t ievent = 0 ; ievent < nofEvents ; ievent++ ) {    
    
    tr->GetEntry(ievent) ;
    
    if(doPrint) std::cout<<"Event : "<< ievent <<", nof gen : "<< gen_n << ", nof genpart : "<< genpart_pt->size() << std::endl;

    
    for(int igen=0; igen<gen_n; igen++ ){
      TParticlePDG *partPDG = TDatabasePDG::Instance()->GetParticle(gen_pdgid->at(igen));
      std::string daughterlist = "(";
      for(int const& idx: gen_daughters->at(igen)) { daughterlist += std::to_string(idx); daughterlist += ",";}
      daughterlist += ")";
      std::cout << "ievent: " << std::setprecision(default_precision) << std::setw(4) <<ievent << ", igen: " << std::setw(4) << igen
    		<<", pid: " << std::setw(5) << gen_pdgid->at(igen) << ", status: " << std::setw(5) << gen_status->at(igen)
    		<<", Name: " << std::setw(10) << ((!partPDG)?"unknown":partPDG->GetName())
    		<<" (pt,eta,phi,energy) : (" << std::fixed << std::setprecision(2) << std::setw(10) << gen_pt->at(igen)
		<< ", " << std::setw(10) << gen_eta->at(igen) << ", " << std::setw(10) << (TMath::RadToDeg()*gen_phi->at(igen)) << ", " << std::setw(10) << gen_energy->at(igen) << ") "
		<<" daughters: " << (gen_daughters->at(igen)).size()
		<< ", " << daughterlist 
    		<< std::defaultfloat
    		<< std::endl;
    }
    
    for(int ipart=0; ipart<genpart_pt->size(); ipart++ ){
      TParticlePDG *partPDG = TDatabasePDG::Instance()->GetParticle(genpart_pid->at(ipart));
      std::cout << "ievent: " << std::setprecision(default_precision) << std::setw(4) <<ievent << ", ipart: " << std::setw(4) << ipart
    		<<", pid: " << std::setw(5) << genpart_pid->at(ipart) << ", mother: " << std::setw(4) << genpart_mother->at(ipart)
    		<<", gen: " << std::setw(4) << (genpart_gen->at(ipart)-1) << ", fromBeamPipe: " << std::setw(4) << genpart_fromBeamPipe->at(ipart)
    		<<", Name: " << std::setw(10) << ((!partPDG)?"unknown":partPDG->GetName())
    		<<" (pt,eta,exeta,phi,exphi,energy) : (" << std::fixed << std::setprecision(2) << std::setw(3) << genpart_pt->at(ipart)
		<< ", " << std::setw(5) << genpart_eta->at(ipart) << ", " << std::setw(5) << genpart_exeta->at(ipart)
		<< ", " << std::setw(5) << (TMath::RadToDeg()*genpart_phi->at(ipart)) << ", " << std::setw(5) << (TMath::RadToDeg()*genpart_exphi->at(ipart))
		<< ", " << std::setw(5) << genpart_energy->at(ipart) << ") "
    		<< std::defaultfloat
    		<< std::endl;
    }
    
    gen_pt->clear();
    gen_eta->clear();
    gen_phi->clear();
    gen_pdgid->clear();
    gen_energy->clear();
    gen_charge->clear();
    gen_status->clear();
    genpart_pt->clear();
    genpart_eta->clear();
    genpart_phi->clear();
    genpart_pid->clear();
    genpart_mother->clear();
    genpart_gen->clear();
    genpart_fromBeamPipe->clear();
    genjet_pt->clear();
    genjet_eta->clear();
    genjet_phi->clear();
    
  }//event loop

  fin->Close();
  delete fin;

  return true;
}
