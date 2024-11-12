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
  
  std::vector<float>  *genpart_gen = 0 ;
  tr->SetBranchStatus("genpart_gen",1);
  tr->SetBranchAddress("genpart_gen" , &genpart_gen);
  
  std::vector<float>  *genpart_fromBeamPipe = 0 ;
  tr->SetBranchStatus("genpart_fromBeamPipe",1);
  tr->SetBranchAddress("genpart_fromBeamPipe" , &genpart_fromBeamPipe);
  
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

    if(doPrint) std::cout<<"Event : "<< ievent <<", nof genpart : "<< genpart_pt->size() << std::endl;
    if(ievent%100==0) std::cout<<"Event : "<< ievent <<", nof genpart : "<< genpart_pt->size() << std::endl;

    for(int ipart=0; ipart<genpart_pt->size(); ipart++ ){
      TParticlePDG *partPDG = TDatabasePDG::Instance()->GetParticle(genpart_pid->at(ipart));
      std::cout << "ievent: " << std::setprecision(default_precision) << std::setw(4) <<ievent << ", ipart: " << std::setw(4) << ipart
		<<", pid: " << std::setw(5) << genpart_pid->at(ipart) << ", mother: " << std::setw(4) << genpart_mother->at(ipart)
		<<", gen: " << std::setw(4) << genpart_gen->at(ipart) << ", fromBeamPipe: " << std::setw(4) << genpart_fromBeamPipe->at(ipart)
		<<", Name: " << std::setw(10) << ((!partPDG)?"unknown":partPDG->GetName())
		<<" (pt,eta,phi,mass) : (" << std::fixed << std::setprecision(2) << std::setw(3) << genpart_pt->at(ipart)
		<< std::defaultfloat
		<< std::endl;
      //   printf("\t LHE : %03d, PDG : %5d (%7s), (Pt, Eta, Phi, Mass) = (%5.2f, %5.2f, %5.2f, %5.2f)\n", 
      //   	 imc, LHEPart_pdgId_[imc], partPDG->GetName(), LHEPart_pt_[imc],
      //   	 LHEPart_eta_[imc] , LHEPart_phi_[imc], LHEPart_mass_[imc]);
      
    }
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
