/**********************************************************************
 Created on : 22/07/2024
 Purpose    : Emulation for Scintillator detector
 Author     : Indranil Das, Research Associate
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
#include <iostream>

#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TProfile.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TFile.h"

#include "TFileHandlerLocal.h"
#include "FileReader.h"

#include "yaml-cpp/yaml.h"
#include <deque>

#include <string>
#include <fstream>
#include <cassert>
#include <bitset>

#include "TPGFEDataformat.hh"
#include "TPGBEDataformat.hh"
#include "TPGFEConfiguration.hh"
#include "TPGFEModuleEmulation.hh"
#include "TPGFEReader3TSep2024.hh"
#include "Stage1IO.hh"

#include "TTree.h"

int main(int argc, char** argv)
{
  //===============================================================================================================================
  //Commandline inputs
  //===============================================================================================================================

  if(argc < 3){
    std::cerr << argv[0] << ": no run and/or trigtime are specified  specified" << std::endl;
    return false;
  }

  uint32_t relayNumber = 180834;
  std::istringstream issRelay(argv[1]);
  issRelay >> relayNumber;
  
  // if(relayNumber>=1727099251){
  // //if(relayNumber>=1727007636){
  //   std::cerr << "Relaynumber should corresponds to those with two silicon layers configuration" << std::endl;
  //   return false;
  // }

  int ref_trigtime = 13;
  std::istringstream issTrigtime(argv[2]);
  issTrigtime >> ref_trigtime;
  
  //===============================================================================================================================
  //Read channel mapping
  //===============================================================================================================================
  TPGFEConfiguration::Configuration cfgs;
  cfgs.setSiChMapFile("cfgmap/WaferCellMapTraces.txt");
  //cfgs.setSciChMapFile("cfgmap/channels_sipmontile_HDtypes.hgcal.txt");
  cfgs.setSciChMapFile("cfgmap/channels_sipmontile_TB2024.txt");
  cfgs.initId();
  cfgs.readSiChMapping();
  cfgs.readSciChMapping();
  cfgs.loadModIdxToNameMapping();
  cfgs.loadMuxMapping();
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Read ECON-T/ROC/(ECON-D setting not needed, just dummy)
  //===============================================================================================================================  
  uint32_t zside = 0, sector = 0, link = 0, det = 1;
  uint32_t econd = 0, econt = 0, selTC4 = 1, module = 0;
  TPGFEConfiguration::TPGFEIdPacking pck;
  
  uint32_t idx = pck.packModId(zside, sector, link, det, econd, selTC4, module); //we assume same ECONT and ECOND number for a given module      
  cfgs.setModulePath(zside, sector, link, det, econd, selTC4, module);
  
  cfgs.setEconTFile("/home/indra/Data/2024-09-15_18-03-52_beamrun_electrons_200_TPGtuning_multfactor2/current_configs_no_timestamps/bravo_train.motherboard.econt_1_new.yaml"); //The _new has correct MUX setting
  //cfgs.setEconTFile("/home/indra/Data/2024-09-15_18-03-52_beamrun_electrons_200_TPGtuning_multfactor2/current_configs_no_timestamps/bravo_train.motherboard.econt_2.yaml");
  cfgs.readEconTConfigYaml();
  
  cfgs.setEconDFile("cfgmap/init_econd.yaml");
  cfgs.readEconDConfigYaml();      
  
  uint32_t iroc = 0;
  uint32_t rocid_0 = pck.packRocId(zside, sector, link, det, econd, selTC4, module, iroc, 0);
  uint32_t rocid_1 = pck.packRocId(zside, sector, link, det, econd, selTC4, module, iroc, 1);
  cfgs.setRocFile("/home/indra/Data/2024-09-15_18-03-52_beamrun_electrons_200_TPGtuning_multfactor2/current_configs_no_timestamps/bravo_train.TB3_A5.hgcroc_A6.yaml"); 
  //cfgs.setRocFile("/home/indra/Data/2024-09-15_18-03-52_beamrun_electrons_200_TPGtuning_multfactor2/current_configs_no_timestamps/bravo_train.TB3_B12.hgcroc_B12_1.yaml");
  cfgs.readRocConfigYaml(rocid_0, rocid_1); //only roc0 is active corresponding to chip 3	
  
  // iroc = 1;
  // rocid_0 = pck.packRocId(zside, sector, link, det, econd, selTC4, module, iroc, 0);
  // rocid_1 = pck.packRocId(zside, sector, link, det, econd, selTC4, module, iroc, 1);
  // cfgs.setRocFile("/home/indra/Data/2024-09-15_18-03-52_beamrun_electrons_200_TPGtuning_multfactor2/current_configs_no_timestamps/bravo_train.TB3_B12.hgcroc_B12_2.yaml");
  // cfgs.readRocConfigYaml(rocid_0, rocid_1); //only roc1 is active corresponding to chip 4

  uint32_t testmodid = pck.packModId(zside, sector, link, det, econt, selTC4, module); //we assume same ECONT and ECOND number for a given module
  //cfgs.setPedThZero();
  //cfgs.setPedZero();
  cfgs.printCfgPedTh(testmodid);
  //===============================================================================================================================  

    //===============================================================================================================================
  //Modify the ECON parameters for special cases
  //===============================================================================================================================
  std::map<uint32_t,TPGFEConfiguration::ConfigEconD>& econDPar =  cfgs.getEconDPara();
  for(const auto& it : econDPar){
    std::cout << "it.first: "<< it.first << std::endl;
    econDPar.at(it.first).print();
    // econDPar.at(it.first).setPassThrough(true);
    // econDPar.at(it.first).setNeRx(6);
  }
  
  // link = 0; econt = 0; uint32_t lp0_bc0 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  // link = 0; econt = 1; uint32_t lp0_bc1 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  // link = 0; econt = 2; uint32_t lp0_bc2 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  // link = 1; econt = 0; uint32_t lp1_stc160 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  // link = 1; econt = 1; uint32_t lp1_stc161 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  // link = 1; econt = 2; uint32_t lp1_stc162 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  // link = 2; econt = 0; uint32_t lp2_stc4a0 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  // link = 2; econt = 1; uint32_t lp2_stc4a1 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  // link = 3; econt = 0; uint32_t lp3_stc4a0 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  // link = 3; econt = 1; uint32_t lp3_stc4a1 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  // link = 4; econt = 0; uint32_t lp4_ext_trigger = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  
  // uint32_t density(0);
  // uint32_t droplsb(1);
  std::map<uint32_t,TPGFEConfiguration::ConfigEconT>& econTPar =  cfgs.getEconTPara();
  for(const auto& it : econTPar){
  //   econTPar[it.first].setDensity(density);
  //   econTPar[it.first].setDropLSB(droplsb);
    
  //   if(it.first==lp0_bc0){              //// starting 1st train
  //     econTPar[it.first].setSelect(2);
  //     econTPar[it.first].setNElinks(3);      
  //   }else if (it.first==lp0_bc1){
  //     econTPar[it.first].setSelect(2);
  //     econTPar[it.first].setNElinks(2);     
  //   }else if (it.first==lp0_bc2){
  //     econTPar[it.first].setSelect(2);
  //     econTPar[it.first].setNElinks(2);
  //   }else if (it.first==lp1_stc160){    //// starting 2nd train
  //     econTPar[it.first].setSelect(1);
  //     econTPar[it.first].setNElinks(2);
  //     econTPar[it.first].setSTCType(1);
  //   }else if (it.first==lp1_stc161){
  //     econTPar[it.first].setSelect(1);
  //     econTPar[it.first].setNElinks(2);
  //     econTPar[it.first].setSTCType(1);
  //   }else if (it.first==lp1_stc162){
  //     econTPar[it.first].setSelect(1);
  //     econTPar[it.first].setNElinks(2);
  //     econTPar[it.first].setSTCType(1);
  //   }else if (it.first==lp2_stc4a0){    //// starting 3rd train
  //     econTPar[it.first].setSelect(1);
  //     econTPar[it.first].setNElinks(4);
  //     econTPar[it.first].setSTCType(3);
  //   }else if (it.first==lp2_stc4a1){
  //     econTPar[it.first].setSelect(1);
  //     econTPar[it.first].setNElinks(2);
  //     econTPar[it.first].setSTCType(3);
  //   }else if (it.first==lp3_stc4a0){    //// starting motherboard
  //     econTPar[it.first].setSelect(1);
  //     econTPar[it.first].setNElinks(4);
  //     econTPar[it.first].setSTCType(3);
  //   }else if (it.first==lp3_stc4a1){
  //     econTPar[it.first].setSelect(1);
  //     econTPar[it.first].setNElinks(3);
  //     econTPar[it.first].setSTCType(3);
  //   }
  //   std::cout << "Modtype " << it.first
  // 	      << ", getOuttype: " << econTPar[it.first].getOutType()
  // 	      << ", select: " << econTPar[it.first].getSelect()
  //     	      << ", stctype: " << econTPar[it.first].getSTCType()
  // 	      << ", typename: " << TPGFEDataformat::tctypeName[econTPar[it.first].getOutType()]
  // 	      << std::endl;
    econTPar[it.first].print();
  }//econt loop
  //===============================================================================================================================

  
  //===============================================================================================================================
  //Read data from tree
  //===============================================================================================================================
  //uint32_t relayNumber = 180834;
  const char *infile_daq = Form("/home/indra/Data/2024-09-15_18-03-52_beamrun_electrons_200_TPGtuning_multfactor2/20240915_%u_OV2_DAQ.root",relayNumber);
  const char *infile_tpg = Form("/home/indra/Data/2024-09-15_18-03-52_beamrun_electrons_200_TPGtuning_multfactor2/20240915_%u_OV2_ECONT.root",relayNumber);
  
  // const char *infile_daq = "/home/indra/Data/2024-09-15_18-03-52_beamrun_electrons_200_TPGtuning_multfactor2/20240915_180524_OV2_DAQ.root";
  // const char *infile_tpg = "/home/indra/Data/2024-09-15_18-03-52_beamrun_electrons_200_TPGtuning_multfactor2/20240915_180624_OV2_ECONT.root";

  //const char *infile_daq = "/home/indra/Data/2024-09-15_18-03-52_beamrun_electrons_200_TPGtuning_multfactor2/20240915_180734_OV2_DAQ.root";
  //const char *infile_tpg = "/home/indra/Data/2024-09-15_18-03-52_beamrun_electrons_200_TPGtuning_multfactor2/20240915_180734_OV2_ECONT.root";

  // const char *infile_daq = "/home/indra/Data/2024-09-15_18-03-52_beamrun_electrons_200_TPGtuning_multfactor2/20240915_180834_OV2_DAQ.root";
  // const char *infile_tpg = "/home/indra/Data/2024-09-15_18-03-52_beamrun_electrons_200_TPGtuning_multfactor2/20240915_180834_OV2_ECONT.root";
  
  // const char *infile_daq = "/home/indra/Data/2024-09-15_18-03-52_beamrun_electrons_200_TPGtuning_multfactor2/20240915_180943_OV2_DAQ.root";
  // const char *infile_tpg = "/home/indra/Data/2024-09-15_18-03-52_beamrun_electrons_200_TPGtuning_multfactor2/20240915_180943_OV2_ECONT.root";
  
  std::unique_ptr<TFile> findaq(TFile::Open(Form("%s",infile_daq)));
  std::unique_ptr<TTree> trdaq((TTree*)findaq->Get("unpacker_data/hgcroc"));
  
  std::unique_ptr<TFile> fintpg(TFile::Open(Form("%s",infile_tpg)));
  std::unique_ptr<TTree> trtpg((TTree*)fintpg->Get("econt"));
  
  void FillHistos(std::unique_ptr<TTree>& daqtr, std::unique_ptr<TTree>& tpgtr);
  void ArrangeDAQData(TPGFEConfiguration::Configuration& cfg, uint32_t modId, int ref_trigtime, std::unique_ptr<TTree>& daqtr,
		      std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>>>& rocarr, uint64_t refevent);
  void ArrangeTPGData(TPGFEConfiguration::Configuration& cfg, uint32_t modId, int ref_trigtime, std::unique_ptr<TTree>& tpgtr,
		      std::map<uint64_t,TPGFEDataformat::TcModuleBxPackets>& tpgarray);
  
  
  //FillHistos(trdaq, trtpg);
  
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>>> hrocarray; //event,rocId
  std::map<uint64_t,TPGFEDataformat::TcModuleBxPackets> tpgarray; //event,rocId

  void BookHistograms(TDirectory*& dir_diff, uint32_t relayNumber);
  void FillCorrHistos(TDirectory*& dir_diff, std::map<uint32_t,TPGFEDataformat::HalfHgcrocData>& hrocdata, TPGFEDataformat::ModuleTcData& mtcdata, TPGFEDataformat::TcRawDataPacket& a, std::vector<TPGFEDataformat::TcRawDataPacket>& b, int nofTcTp3[], uint64_t ievent);
  
  TFile *fout = new TFile(Form("TM-Run-%u_%d.root",relayNumber,ref_trigtime), "recreate");
  TDirectory *dir_diff = fout->mkdir("diff_plots");
  BookHistograms(dir_diff, relayNumber);

  
  // std::map<uint64_t,uint32_t> eventbx;
  // std::vector<uint64_t> eventList;
  //int ref_trigtime = 13;
  ArrangeDAQData(cfgs, testmodid, ref_trigtime, trdaq, hrocarray,16218339345030);
  ArrangeTPGData(cfgs, testmodid, ref_trigtime, trtpg, tpgarray);

  TPGFEModuleEmulation::HGCROCTPGEmulation rocTPGEmul(cfgs);
  TPGFEModuleEmulation::ECONTEmulation econtEmul(cfgs);
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::ModuleTcData>>> modarray; //event,moduleId
  modarray.clear();
  
  uint64_t refEvent = 16218339345030;
  uint32_t refModId = testmodid;
  
  std::cout << "hrocarray.size(): " << hrocarray.size() << std::endl;
  std::cout << "tpgarray.size(): " << tpgarray.size() << std::endl;


  for(const auto& hrocevent : hrocarray){
    // std::cout << "hrocevent.first: " << hrocevent.first  << ", hrocevent.second.size(): " << hrocevent.second.size() << ", hrocevent.second.at(0).first: " << hrocevent.second.at(0).first << ", hrocevent.second.at(0).second.hasTOT(): " << hrocevent.second.at(0).second.hasTOT() << ", hrocevent.second.at(1).first: " << hrocevent.second.at(1).first << ", hrocevent.second.at(1).second.hasTOT(): " << hrocevent.second.at(1).second.hasTOT() << std::endl;
    
    uint32_t moduleId = testmodid;
    uint64_t event =  hrocevent.first;
    std::map<uint32_t,TPGFEDataformat::HalfHgcrocData> rocdata;
    std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
    std::map<uint32_t,TPGFEDataformat::ModuleTcData> moddata;
    std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>> hrocvec = hrocevent.second;
    //hrocvec =  hrocarray.at(event);	  
    for(const auto& data : hrocvec){
      rocdata[data.first] = data.second ;
      //const TPGFEDataformat::HalfHgcrocData& hrocdata = data.second ;
    }
    
    //================================================
    //HGCROC emulation for a given module
    //================================================
    bool isSim = false; //true for CMSSW simulation and false for beam-test analysis
    // if(event==refEvent)
    //   rocTPGEmul.Emulate(isSim, event, moduleId, rocdata, modTcdata, event);
    // else
    rocTPGEmul.Emulate(isSim, event, moduleId, rocdata, modTcdata);
    //================================================

    int nofTcTp3[4];
    bool hasModTCshowed = false;
    refEvent = event;
    modarray[event].push_back(modTcdata);
    if(event==refEvent){
      uint32_t modTcid = modTcdata.first;
      TPGFEDataformat::ModuleTcData& mtcdata = modTcdata.second;
      // if(modTcid==refModId) {
      // //if(modTcid==refModId and !mtcdata.isTcTp1() and !mtcdata.isTcTp2()) {
      // 	for(int i=0;i<70;i++) std::cout << "=";
      // 	std::cout << "Event: " << event ;
      // 	for(int i=0;i<70;i++) std::cout << "=";
      // 	std::cout << std::endl;
      // 	std::cout << "Event: " << event << ", modTcid: " << modTcid << ", slink BxCounter: " << rocdata[modTcid].getSlinkBx() << std::endl;
      // 	mtcdata.print();
      // 	hasModTCshowed = true;
      // }
      for(int istc=0;istc<4;istc++){
	nofTcTp3[istc] = 0;
      }
      for(int itc=0;itc<16;itc++){
	int istc = itc%4;
	if(mtcdata.getTC(itc).isTcTp3()) nofTcTp3[istc]++;
      }
    }
    
    // //DAQ data
    // if(event==refEvent and hasModTCshowed){
    //   for(const auto& data : rocdata) {
    // 	std::cout << "HalfROC id: " << data.first << std::endl;
    // 	data.second.print();
    //   }
    // }
    
    moddata.clear();
    for(const auto& data : modarray.at(event))
      if(data.first==moduleId) moddata[data.first] = data.second ;
    
    //================================================
    //ECONT emulation for a given module
    //================================================
    econtEmul.Emulate(isSim, event, moduleId, moddata);
    TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
    //================================================
    if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
    
    //bool isLowSTC = false;
    // if(tpgarray[event].second.size()>=8){
    //   if(tpgarray[event].second.at(7).energy()>60 and )
    // }
    
    bool hasTCshowed = false;
    // // if(event==refEvent and !TcRawdata.second.isTcTp1() and !TcRawdata.second.isTcTp2()) {
    // if(event==refEvent) {
    //   TcRawdata.second.print();
    //   hasTCshowed = true ;
    // }
    
    if(event==refEvent and hasTCshowed) {
      //TPGFEDataformat::TcModuleBxPackets
      //if(tpgarray[event].second.size()>=8) tpgarray[event].second.at(7).print();
      int ets = 0;
      for(const auto& data : tpgarray[event].second){
	std::cout << "ets: " << ets++ << std::endl;
	data.print();
	//if(data.bx()==TcRawdata.second.bx()) data.print();
      }
    }
    
    FillCorrHistos(dir_diff,rocdata,modTcdata.second, TcRawdata.second, tpgarray[event].second,nofTcTp3, event);
  }//event loop
  
  fout->cd();
  dir_diff->Write();
  fout->Close();
  delete fout;

  return true;
}

void BookHistograms(TDirectory*& dir_diff, uint32_t relayNumber)
{
  TH1D *hEDiff[3];
  for(int istc=0;istc<3;istc++){
    hEDiff[istc] = new TH1D(Form("hEDiff_%d",istc),Form("Run:%u,STC:%d : (E_{emul}-E_{ECONT})",relayNumber,istc), 200, -100, 100);
    hEDiff[istc]->GetXaxis()->SetTitle("E_{emul}-E_{ECONT} (compressed in 4E+3M)");
    hEDiff[istc]->GetYaxis()->SetTitle("Entries");
    hEDiff[istc]->SetDirectory(dir_diff);
  }

  TH2D *hBxCorr[3];
  for(int istc=0;istc<3;istc++){
    hBxCorr[istc] = new TH2D(Form("hBxCorr_%d",istc),Form("Run:%u,STC:%d : Bx Correlation",relayNumber,istc), 16, -0.5, 15.5, 16, -0.5, 15.5);
    hBxCorr[istc]->GetXaxis()->SetTitle("Bx in DAQ");
    hBxCorr[istc]->GetYaxis()->SetTitle("Bx in ECONT");    
    hBxCorr[istc]->SetDirectory(dir_diff);
  }

  TH2D *hEmulCorr[3];
  for(int istc=0;istc<3;istc++){
    hEmulCorr[istc] = new TH2D(Form("hEmulCorr_%d",istc),Form("Run:%u,STC:%d : Data-vs-Emulation",relayNumber,istc), 100, 0, 1e2, 100, 0, 1e2);
    hEmulCorr[istc]->GetXaxis()->SetTitle("E_{emul} (compressed in 4E+3M)");
    hEmulCorr[istc]->GetYaxis()->SetTitle("E_{ECONT} (compressed in 4E+3M)");    
    hEmulCorr[istc]->SetDirectory(dir_diff);
  }

  TH2D *hEmulCorrDecomp[3];
  for(int istc=0;istc<3;istc++){
    hEmulCorrDecomp[istc] = new TH2D(Form("hEmulCorrDecomp_%d",istc),Form("Run:%u,STC:%d : Data-vs-Emulation",relayNumber,istc), 100, 0, 1e4, 100, 0, 1e4);
    hEmulCorrDecomp[istc]->GetXaxis()->SetTitle("E_{emul} (raw decompressed)");
    hEmulCorrDecomp[istc]->GetYaxis()->SetTitle("E_{ECONT} (raw decompressed)");    
    hEmulCorrDecomp[istc]->SetDirectory(dir_diff);
  }

  //TH2D *hRocBxCorr = new TH2D(Form("hRocBxCorr_0"),Form("Run:%u: HalfRoc Bx Correlation",relayNumber), 3565, -0.5, 3564.5, 3565, -0.5, 3564.5);
  TH2D *hRocBxCorr = new TH2D(Form("hRocBxCorr_0"),Form("Run:%u: HalfRoc Bx Correlation",relayNumber), 100, -0.5, 3564.5, 100, -0.5, 3564.5);
  hRocBxCorr->SetDirectory(dir_diff);

  TH2D *hSTCvsTC_pES[3][15][12];
  for(int istc=0;istc<3;istc++){
    for(int ies=0;ies<15;ies++){
      for(int itc=0;itc<12;itc++){
	hSTCvsTC_pES[istc][ies][itc] = new TH2D(Form("hSTCvsTC_pES_%d_%d_%d",istc,ies,itc),Form("Run:%u,STC:%d,ETS:%d,TC:%d : STC-vs-emulTC Correlation",relayNumber,istc,ies,itc), 100, 0, 1e2, 100, 0, 1e2);
	hSTCvsTC_pES[istc][ies][itc]->SetDirectory(dir_diff);
      }//itc
    }//ies
  }//itsc
  

}

void FillCorrHistos(TDirectory*& dir_diff, std::map<uint32_t,TPGFEDataformat::HalfHgcrocData>& hrocdata, TPGFEDataformat::ModuleTcData& mtcdata, TPGFEDataformat::TcRawDataPacket& emul, std::vector<TPGFEDataformat::TcRawDataPacket>& econt, int nofTcTp3[], uint64_t ievent)
{
  TList *list = (TList *)dir_diff->GetList();
  //if(!(emul.isTcTp1()) and !(emul.isTcTp2())){
  for(uint32_t istc=0;istc<3;istc++){
    //if(!(emul.getTc(istc).isTcTp1()) and !(emul.getTc(istc).isTcTp2())){
    if(emul.bx() == econt.at(7).bx()){
      ((TH1D *) list->FindObject(Form("hEDiff_%u",istc)))->Fill( emul.getTc(istc).energy() - econt.at(7).getTc(istc).energy() );
      ((TH2D *) list->FindObject(Form("hEmulCorr_%u",istc)))->Fill( emul.getTc(istc).energy(), econt.at(7).getTc(istc).energy() );
      ((TH2D *) list->FindObject(Form("hEmulCorrDecomp_%u",istc)))->Fill( emul.getTc(istc).decodedE(TPGFEDataformat::STC4A), econt.at(7).getTc(istc).decodedE(TPGFEDataformat::STC4A) );
    }
    //}
    for(int ibx=0;ibx<int(econt.size());ibx++){
      //if(!emul.isTcTp1() and !emul.isTcTp2()){
      double ediff = double(emul.getTc(istc).energy() - econt.at(ibx).getTc(istc).energy());
      if(TMath::Abs(ediff)<=1) {
	((TH1D *) list->FindObject(Form("hBxCorr_%u",istc)))->Fill( emul.bx(), econt.at(ibx).bx() );
	((TH2D *) list->FindObject(Form("hRocBxCorr_0")))->Fill( hrocdata[4352].getSlinkBx(), hrocdata[4353].getSlinkBx() );
      }
      for(int itc=0; itc<12;itc++){
	((TH1D *) list->FindObject(Form("hSTCvsTC_pES_%d_%d_%d",istc,ibx,itc)))->Fill( mtcdata.getTC(itc).getCdata() , econt.at(ibx).getTc(istc).energy() );
	// if(istc==0 and itc==0 and mtcdata.getTC(itc).getCdata()>60 and econt.at(ibx).getTc(istc).energy()<40 and emul.bx() == econt.at(ibx).bx())
	//   std::cout << "Event: " << ievent <<",  mtcdata.getTC(itc).getCdata(): " << mtcdata.getTC(itc).getCdata() << ", econt.at(ibx).getTc(istc).energy(): " << econt.at(ibx).getTc(istc).energy() << std::endl;
      }
      //}//isTcTp
      
      //std::cout << "istc: " << istc << ", Emul: " << emul.getTc(istc).energy() << ", ECONT: " << econt.at(7).getTc(istc).energy() << std::endl;
    }//ibx loop
  }//istc
  //}
  
  
}

void FillHistos(std::unique_ptr<TTree>& trdaq, std::unique_ptr<TTree>& trtpg)
{
  ULong64_t globEventId_DAQ;
  Int_t channel;
  UShort_t adcm, adc, toa, tot, chip, half, eventcounter, orbitcounter, bxcounter;
  bool corruption, totflag;
  UInt_t rawdata, sourceId, contentId, event;
  UShort_t  trigtime;
  
  Long64_t globEventId_TPG;
  UInt_t sourceId_TPG, contentId_TPG, stcenergy;
  UShort_t trigtime_TPG, bx, stcindex, stcenergyraw, largesttc, chip_TPG, bxecon;
  
  trdaq->SetBranchAddress("globalEventId" , &globEventId_DAQ);
  trdaq->SetBranchAddress("channel" , &channel);
  trdaq->SetBranchAddress("adcm" , &adcm);
  trdaq->SetBranchAddress("adc" , &adc);
  trdaq->SetBranchAddress("toa" , &toa);
  trdaq->SetBranchAddress("tot" , &tot);
  trdaq->SetBranchAddress("chip" , &chip);
  trdaq->SetBranchAddress("half" , &half);
  trdaq->SetBranchAddress("eventcounter" , &eventcounter);
  trdaq->SetBranchAddress("orbitcounter" , &orbitcounter);
  trdaq->SetBranchAddress("bxcounter" , &bxcounter);
  trdaq->SetBranchAddress("corruption" , &corruption);
  trdaq->SetBranchAddress("totflag" , &totflag);
  trdaq->SetBranchAddress("rawdata" , &rawdata);
  trdaq->SetBranchAddress("sourceId" , &sourceId);
  trdaq->SetBranchAddress("contentId" , &contentId);
  trdaq->SetBranchAddress("event" , &event);
  trdaq->SetBranchAddress("trigtime" , &trigtime);  
  
  trtpg->SetBranchAddress("globalEventId" , &globEventId_TPG);
  trtpg->SetBranchAddress("chip" , &chip_TPG);
  trtpg->SetBranchAddress("sourceId" , &sourceId_TPG);
  trtpg->SetBranchAddress("contentId" , &contentId_TPG);
  trtpg->SetBranchAddress("trigtime" , &trigtime_TPG);
  trtpg->SetBranchAddress("stcenergy" , &stcenergy);
  trtpg->SetBranchAddress("stcindex" , &stcindex);
  trtpg->SetBranchAddress("stcenergyraw" , &stcenergyraw);
  trtpg->SetBranchAddress("largesttc" , &largesttc);
  trtpg->SetBranchAddress("bx" , &bx);
  trtpg->SetBranchAddress("bxecon" , &bxecon);
  
  std::cout << "Nof entries in DAQ tree : " << trdaq->GetEntries() << std::endl;
  std::cout << "Nof entries in TPG tree : " << trtpg->GetEntries() << std::endl;

  TH1F *hDAQChip = new TH1F("hDAQChip","hDAQChip",33,-0.5,32.5);
  TH1F *hDAQTrigTime = new TH1F("hDAQTrigTime","hDAQTrigTime",33,-0.5,32.5);
  TH1F *hDAQChip2_Ch = new TH1F("hDAQChip2_Ch","hDAQChip2_Ch",100,-2.5,97.5);
  TH1F *hDAQChip3_Ch = new TH1F("hDAQChip3_Ch","hDAQChip3_Ch",100,-2.5,97.5);
  TH1F *hDAQChip4_Ch = new TH1F("hDAQChip4_Ch","hDAQChip4_Ch",100,-2.5,97.5);

  TH1F *hDAQChip2_Half = new TH1F("hDAQChip2_Half","hDAQChip2_Half",3,-0.5,2.5);
  TH1F *hDAQChip3_Half = new TH1F("hDAQChip3_Half","hDAQChip3_Half",3,-0.5,2.5);
  TH1F *hDAQChip4_Half = new TH1F("hDAQChip4_Half","hDAQChip4_Half",3,-0.5,2.5);
  
  TH1F *hTPGChip0_STC = new TH1F("hTPGChip0_STC","hTPGChip0_STC",20,-2.5,17.5);
  TH1F *hTPGChip1_STC = new TH1F("hTPGChip1_STC","hTPGChip1_STC",20,-2.5,17.5);
  TH1F *hTPGChip2_STC = new TH1F("hTPGChip2_STC","hTPGChip2_STC",20,-2.5,17.5);
  TH1F *hTPGChip3_STC = new TH1F("hTPGChip3_STC","hTPGChip3_STC",20,-2.5,17.5);
  
  TH2F *hTPGChip0_STCE = new TH2F("hTPGChip0_STCE","hTPGChip0_STCE",20,-2.5,17.5,100,0,5.e5);
  TH2F *hTPGChip1_STCE = new TH2F("hTPGChip1_STCE","hTPGChip1_STCE",20,-2.5,17.5,100,0,5.e5);
  TH2F *hTPGChip2_STCE = new TH2F("hTPGChip2_STCE","hTPGChip2_STCE",20,-2.5,17.5,100,0,5.e5);
  TH2F *hTPGChip3_STCE = new TH2F("hTPGChip3_STCE","hTPGChip3_STCE",20,-2.5,17.5,100,0,5.e5);

  TH1F *hTPGChip0_STCMax = new TH1F("hTPGChip0_STCMax","hTPGChip0_STCMax",20,-2.5,17.5);
  TH1F *hTPGChip1_STCMax = new TH1F("hTPGChip1_STCMax","hTPGChip1_STCMax",20,-2.5,17.5);
  TH1F *hTPGChip2_STCMax = new TH1F("hTPGChip2_STCMax","hTPGChip2_STCMax",20,-2.5,17.5);
  TH1F *hTPGChip3_STCMax = new TH1F("hTPGChip3_STCMax","hTPGChip3_STCMax",20,-2.5,17.5);

  TH2F *hTPGChip1STC0_trigtimeVsSTCE = new TH2F("hTPGChip1STC0_trigtimeVsSTCE","hTPGChip1STC0_trigtimeVsSTCE",33,-0.5,32.5,1000,0,2.5e4);
  TH2F *hTPGChip1STC1_trigtimeVsSTCE = new TH2F("hTPGChip1STC1_trigtimeVsSTCE","hTPGChip1STC1_trigtimeVsSTCE",33,-0.5,32.5,1000,0,2.5e4);
  TH2F *hTPGChip1STC2_trigtimeVsSTCE = new TH2F("hTPGChip1STC2_trigtimeVsSTCE","hTPGChip1STC2_trigtimeVsSTCE",33,-0.5,32.5,1000,0,2.5e4);

  double totE_chip1[32],totE_chip3[32];
  for(int itm=0;itm<32;itm++){
    totE_chip1[itm] = 0;
    totE_chip3[itm] = 0;
  }
  TH1F *hTPGChip = new TH1F("hTPGChip","hTPGChip",33,-0.5,32.5);
  //trdaq->Scan("*");
  for(Long_t ievent = 0; ievent<trdaq->GetEntries(); ievent++){
  //for(Long_t ievent = 0; ievent<10; ievent++){
    trdaq->GetEvent(ievent);
    // std::cout << "DAQ Event: " << ievent
    // 	      << ", eventId " << globEventId_DAQ
    // 	      << ", channel " << channel
    //   	      << ", adcm " << adcm
    // 	      << ", adc " << adc
    // 	      << ", toa " << toa
    // 	      << ", tot " << tot
    // 	      << ", chip " << chip
    // 	      << ", half " << half
    //   	      << ", ec " << eventcounter
    // 	      << ", oc " << orbitcounter
    // 	      << ", bxc " << bxcounter
    // 	      << ", corr " << corruption
    //   	      << ", istot " << totflag
    // 	      << std::hex
    // 	      << ", raw:  0x" << rawdata
    //   	      << std::dec
    // 	      << ", sId " << sourceId
    //   	      << ", cId " << contentId
    // 	      << ", event " << event
    //   	      << ", trigtime " << trigtime
    // 	      // << ", trigtime[0]: " << trigtime[0]
    //   	      // << ", trigtime[1]: " << trigtime[1]
    //   	      // << ", trigtime[2]: " << trigtime[2]
    //   	      // << ", trigtime[3]: " << trigtime[3]
    //   	      // << ", trigtime[4]: " << trigtime[4]
    // 	      << std::endl;
    hDAQTrigTime->Fill(trigtime);
    hDAQChip->Fill(chip);
    if(chip==2){
      hDAQChip2_Ch->Fill(channel);
      hDAQChip2_Half->Fill(half);
      //if(channel<0)
    } else if(chip==3){
      hDAQChip3_Ch->Fill(channel);
      hDAQChip3_Half->Fill(half);
    } else if(chip==4){
      hDAQChip4_Ch->Fill(channel);
      hDAQChip4_Half->Fill(half);
    }
  }
  
  //trtpg->Scan("*");
  for(Long_t ievent = 0; ievent<trtpg->GetEntries(); ievent++){
    //for(Long_t ievent = 0; ievent<10; ievent++){
    trtpg->GetEvent(ievent);
    //std::cout << "TPG Event: " << ievent << ", eventId " << globEventId_TPG << std::endl;
    hTPGChip->Fill(chip_TPG);
    //if(trigtime_TPG<12 or trigtime_TPG>18) continue;
    if(chip_TPG==0){
      hTPGChip0_STC->Fill(stcindex);
      hTPGChip0_STCMax->Fill(largesttc);
      hTPGChip0_STCE->Fill(stcindex, stcenergy);
    }else if(chip_TPG==1){
      hTPGChip1_STC->Fill(stcindex);
      hTPGChip1_STCMax->Fill(largesttc);
      hTPGChip1_STCE->Fill(stcindex, stcenergy);
      if(stcindex==0){
	hTPGChip1STC0_trigtimeVsSTCE->Fill(trigtime_TPG, stcenergy);
      }else if(stcindex==1){
	hTPGChip1STC1_trigtimeVsSTCE->Fill(trigtime_TPG, stcenergy);
	totE_chip1[trigtime_TPG] += stcenergyraw;
      }else if(stcindex==2){
	hTPGChip1STC2_trigtimeVsSTCE->Fill(trigtime_TPG, stcenergy);
      }

    }else if(chip_TPG==2){
      hTPGChip2_STC->Fill(stcindex);
      hTPGChip2_STCMax->Fill(largesttc);
      hTPGChip2_STCE->Fill(stcindex, stcenergy);
    }else if(chip_TPG==3){
      hTPGChip3_STC->Fill(stcindex);
      hTPGChip3_STCMax->Fill(largesttc);
      hTPGChip3_STCE->Fill(stcindex, stcenergy);
      if(stcindex==1){
	totE_chip3[trigtime_TPG] += stcenergyraw;
      }
    }
  }
  
  for(int itm=0;itm<32;itm++){
    std::cout << "trigtime: " << itm << ", chip1 Etot: " << totE_chip1[itm] << ", chip3 Etot: " << totE_chip3[itm] << std::endl ;
  }

  std::string outfile = "testout.root" ;
  std::unique_ptr<TFile> fout(TFile::Open(Form("%s",outfile.c_str()),"recreate"));
  hDAQChip->Write();
  hDAQTrigTime->Write();
  hDAQChip2_Ch->Write();
  hDAQChip2_Half->Write();
  hDAQChip3_Ch->Write();
  hDAQChip3_Half->Write();
  hDAQChip4_Ch->Write();
  hDAQChip4_Half->Write();
  hTPGChip->Write();
  hTPGChip0_STC->Write();
  hTPGChip1_STC->Write();
  hTPGChip2_STC->Write();
  hTPGChip3_STC->Write();
  hTPGChip0_STCMax->Write();
  hTPGChip1_STCMax->Write();
  hTPGChip2_STCMax->Write();
  hTPGChip3_STCMax->Write();
  hTPGChip0_STCE->Write();
  hTPGChip1_STCE->Write();
  hTPGChip2_STCE->Write();
  hTPGChip3_STCE->Write();
  hTPGChip1STC0_trigtimeVsSTCE->Write();
  hTPGChip1STC1_trigtimeVsSTCE->Write();
  hTPGChip1STC2_trigtimeVsSTCE->Write();
}

void ArrangeDAQData(TPGFEConfiguration::Configuration& configs, uint32_t modId, int ref_trigtime, std::unique_ptr<TTree>& trdaq,
		    std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>>>& rocarr, uint64_t refEvent)
{
  rocarr.clear();
  TPGFEConfiguration::TPGFEIdPacking pck;
  pck.setModId(modId);

  uint32_t zside = pck.getZside();
  uint32_t sector = pck.getSector();
  uint32_t link = pck.getLink();
  uint32_t det = pck.getDetType();
  uint32_t econd = pck.getEconN();
  uint32_t econt = pck.getEconN();
  uint32_t selTC4 = pck.getSelTC4();
  uint32_t module = pck.getModule();
  uint32_t iroc = 0;
  
  // std::map<uint32_t,TPGFEConfiguration::ConfigEconD>& econDPar = configs.getEconDPara();
  // const std::map<std::tuple<uint32_t,uint32_t,uint32_t>,std::string>& modNameMap = configs.getModIdxToName();
  // std::string modName = modNameMap.at(std::make_tuple(pck.getDetType(), pck.getSelTC4(), pck.getModule()));
  // const std::map<std::pair<std::string,std::tuple<uint32_t,uint32_t,uint32_t>>,uint32_t>&seqToRocpin = (pck.getDetType()==0)?configs.getSiSeqToROCpin():configs.getSciSeqToROCpin();
  // std::cout << "modName : " << modName << std::endl;
  
  ULong64_t globEventId_DAQ;
  Int_t channel;
  UShort_t adcm, adc, toa, tot, chip, half, eventcounter, orbitcounter, bxcounter;
  bool corruption, totflag;
  UInt_t rawdata, sourceId, contentId, event;
  UShort_t  trigtime;
  
  trdaq->SetBranchAddress("globalEventId" , &globEventId_DAQ);
  trdaq->SetBranchAddress("channel" , &channel);
  trdaq->SetBranchAddress("adcm" , &adcm);
  trdaq->SetBranchAddress("adc" , &adc);
  trdaq->SetBranchAddress("toa" , &toa);
  trdaq->SetBranchAddress("tot" , &tot);
  trdaq->SetBranchAddress("chip" , &chip);
  trdaq->SetBranchAddress("half" , &half);
  trdaq->SetBranchAddress("eventcounter" , &eventcounter);
  trdaq->SetBranchAddress("orbitcounter" , &orbitcounter);
  trdaq->SetBranchAddress("bxcounter" , &bxcounter);
  trdaq->SetBranchAddress("corruption" , &corruption);
  trdaq->SetBranchAddress("totflag" , &totflag);
  trdaq->SetBranchAddress("rawdata" , &rawdata);
  trdaq->SetBranchAddress("sourceId" , &sourceId);
  trdaq->SetBranchAddress("contentId" , &contentId);
  trdaq->SetBranchAddress("event" , &event);
  trdaq->SetBranchAddress("trigtime" , &trigtime);  
  
  //trdaq->Scan("*");
  UShort_t prevChip = 100, prevHalf = 100, prevtrigtime = 100, prevbxcounter = 100;
  std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocChannelData>> charr;
  int nofcorruption = 0, prevBx = 100;
  ULong64_t prev_globEventId = 0xffffffffffffffff ;
  for(Long_t ievent = 0; ievent<trdaq->GetEntries(); ievent++){
  //for(Long_t ievent = 0; ievent<50000; ievent++){
    trdaq->GetEvent(ievent);
    
    TPGFEDataformat::HalfHgcrocChannelData ch;
    const uint16_t trigflag = (rawdata>>30) & 0x3;
    //const uint16_t trigflag = rawdata & 0x3; //for big/little endian
    // if(trigflag==3){
    //   uint16_t ttot = (rawdata>>10) & 0x3FF;
    //   ch.setTot(ttot,trigflag);
    //   //ch.setTot(uint16_t(tot),uint16_t(trigflag));
    // }else if(trigflag==2){
    //   uint16_t ttot = (rawdata>>10) & 0x3FF;
    //   ch.setTot(ttot,trigflag);
    //   // ch.setTot(uint16_t(tot),uint16_t(trigflag));
    // }else if(trigflag==1){
    //   uint16_t tadc = (rawdata>>10) & 0x3FF;
    //   //ch.setAdc(tadc,trigflag);
    //   ch.setAdc(0,trigflag);
    //   //ch.setAdc(uint16_t(adc),uint16_t(trigflag)); //uint16_t(adcM)
    // }else if(trigflag==0){
    //   uint16_t tadc = (rawdata>>10) & 0x3FF;
    //   ch.setAdc(tadc,trigflag);
    //   // ch.setAdc(uint16_t(adc),uint16_t(trigflag));
    // }else
    //   ch.setZero();
    
    if(totflag){
      ch.setTot(uint16_t(tot),uint16_t(trigflag));
    }else{
      ch.setAdc(uint16_t(adc),uint16_t(trigflag));
    }
    
    if(chip==2 and globEventId_DAQ==refEvent){
      std::cout << "ievent : " << ievent << ", eventcounter: " << eventcounter << ", orbitcounter: " << orbitcounter << ", bxcounter: " << bxcounter << ", globEventId_DAQ: "  << globEventId_DAQ << ", trigtime: " << trigtime
	//<< ", sourceId: " << sourceId <<", contentId: " << contentId
		<< ", chip : " << chip << ", half: " << half << ", channel: " << channel << ", totflag: " << totflag << ", trigflag(from rawdata): " << trigflag  << ", adc: " << adc << ", tot: " << tot << ", rawdata: " << rawdata << std::endl ;
    }
    
    if(chip==2 and channel>=0 and channel<=35) {
      if(globEventId_DAQ==refEvent){
	std::cout << "globEventId_DAQ: "  << globEventId_DAQ << ", ievent : " << ievent << ", half: " << half << ", channel: " << channel << ", data : " ;
	ch.print();
      }
      charr.push_back(std::make_pair(channel,ch));
      if(corruption!=0) nofcorruption++;
    }
    
    if(chip!=prevChip and charr.size()==36){
      if(ref_trigtime==prevtrigtime and nofcorruption==0){
	uint8_t bx = (prevBx==3564) ? 0xF : prevBx & 0x7 ;
	if(prev_globEventId==refEvent)
	  std::cout << "iEvent : " << ievent << ", charr.size: " << charr.size() << ", prevChip : " << prevChip << ", chip: " << chip << ", prevHalf: " << prevHalf  << ", prevtrigtime: " << prevtrigtime << ", prevBx: " << uint16_t(bx) << ", globEventId: " << prev_globEventId << std::endl;
	TPGFEDataformat::HalfHgcrocData hrocdata;
	for(int ich=0;ich<charr.size();ich++) hrocdata.setChannel(ich, charr.at(ich).second);	
	hrocdata.setBx(uint16_t(bx));
	hrocdata.setSlinkBx(prevBx);
	pck.setZero();
	rocarr[prev_globEventId].push_back(std::make_pair(pck.packRocId(zside, sector, link, det, econd, selTC4, module, iroc, prevHalf),hrocdata));
	if(prev_globEventId==refEvent) hrocdata.print();
      }
      charr.clear();
      nofcorruption = 0;
    }//change of chip
    prevChip = chip;         
    prevHalf = half;
    prevtrigtime = trigtime;
    prevBx = bxcounter;
    prev_globEventId = globEventId_DAQ;
  }//event loop
  
  std::cout << "local rocarr.size(): " << rocarr.size() << std::endl;
  
}

void ArrangeTPGData(TPGFEConfiguration::Configuration& cfg, uint32_t modId, int ref_trigtime, std::unique_ptr<TTree>& trtpg, std::map<uint64_t,TPGFEDataformat::TcModuleBxPackets>& tpgarray)
{
  
  tpgarray.clear();
  TPGFEConfiguration::TPGFEIdPacking pck;
  pck.setModId(modId);
  
  uint32_t zside = pck.getZside();
  uint32_t sector = pck.getSector();
  uint32_t link = pck.getLink();
  uint32_t det = pck.getDetType();
  uint32_t econd = pck.getEconN();
  uint32_t econt = pck.getEconN();
  uint32_t selTC4 = pck.getSelTC4();
  uint32_t module = pck.getModule();
  uint32_t iroc = 0;
  
  Long64_t globEventId_TPG;
  UInt_t sourceId_TPG, contentId_TPG, stcenergy;
  UShort_t trigtime_TPG, bx, stcindex, stcenergyraw, largesttc, chip_TPG, bxecon;
  
  trtpg->SetBranchAddress("globalEventId" , &globEventId_TPG);
  trtpg->SetBranchAddress("chip" , &chip_TPG);
  trtpg->SetBranchAddress("sourceId" , &sourceId_TPG);
  trtpg->SetBranchAddress("contentId" , &contentId_TPG);
  trtpg->SetBranchAddress("trigtime" , &trigtime_TPG);
  trtpg->SetBranchAddress("stcenergy" , &stcenergy);
  trtpg->SetBranchAddress("stcindex" , &stcindex);
  trtpg->SetBranchAddress("stcenergyraw" , &stcenergyraw);
  trtpg->SetBranchAddress("largesttc" , &largesttc);
  trtpg->SetBranchAddress("bx" , &bx);
  trtpg->SetBranchAddress("bxecon" , &bxecon);
  
  std::vector<TPGFEDataformat::TcRawDataPacket> tcpktarr;
  TPGFEDataformat::TcRawDataPacket tcpkt;
  UShort_t prevChip = 100, prevBx = 100, prevBxEcon = 100, prevtrigtime = 100;
  Long64_t prev_globEventId = 0xffffffffffffffff;
  //trtpg->Scan("*");
  for(Long_t ievent = 0; ievent<trtpg->GetEntries(); ievent++){
  //for(Long_t ievent = 0; ievent<156000; ievent++){
    trtpg->GetEvent(ievent);
    
    // std::cout << "iEvent1 : " << ievent << ", tcpkt.size(): " << tcpkt.size()
    // 	      << ", globEventId_TPG: " << globEventId_TPG << ", prevGlobEent : " << prev_globEventId
    // 	      << ", trigtime_TPG: " << trigtime_TPG << ", prevTrigtime: " << prevtrigtime
    // 	      << ", chip: " << chip_TPG << ", prevChip: " << prevChip
    //   	      << ", bx: " << bx << ", prevBx: " << prevBx
    // 	      << ", "
    // 	      << std::endl;
    if(chip_TPG==1) {
      tcpkt.setTcData(TPGFEDataformat::STC4A, uint8_t(largesttc), uint16_t(stcenergyraw), uint64_t(stcenergyraw), 0, 0, 0);
      //   std::cout << "iEvent2 : " << ievent << ", tcpkt.size(): " << tcpkt.size() << ", bx: " << bx << ", prevBx: " << prevBx << ", tcpktarr.size(): " << tcpktarr.size() << std::endl;
    }    
    
    if(tcpkt.size()==12 and chip_TPG==1){    
      tcpkt.setBX(uint8_t(prevBxEcon));
      tcpkt.setType(TPGFEDataformat::STC4A);
      tcpktarr.push_back(tcpkt);
      //tcpkt.print();
      tcpkt.reset();
      // std::cout << "iEvent-3 : " << ievent << ", tcpktarr.size: " << tcpktarr.size()
      // 		<< ", tcpkt.size(): " << tcpkt.size() << ", bx: " << bx << ", prevBx: " << prevBx << ", trigtime: " << trigtime_TPG << ", prevtrigtime: " << prevtrigtime << ", chip: " << chip_TPG
      // 		<< std::endl;
    }
    
    if(tcpktarr.size()==15 and prevBx != bx){
      // std::cout << "iEvent-4 : " << ievent << ", tcpktarr.size: " << tcpktarr.size() << ", tcpkt.size(): " << tcpkt.size()
      // 		<< ", chip: " << chip_TPG << ", prevChip : " << prevChip 
      // 		<< ", bx: " << bx << ", prevBx: " << prevBx
      // 		<< ", trigtime_TPG: " << trigtime_TPG << ", prevtrigtime: "<< prevtrigtime << ", ref_trigtime: " << ref_trigtime
      // 		<< ", globEventId: " << prev_globEventId
      // 		<< std::endl;
      if(ref_trigtime==prevtrigtime){
	// std::cout << "iEvent-5 : " << ievent << std::endl;
	tpgarray[uint64_t(prev_globEventId)] = std::make_pair(modId,tcpktarr);
      }
      tcpktarr.clear();
    }//change of chip
    
    prevChip = chip_TPG;
    prevBx = bx;
    prevBxEcon = bxecon;
    prevtrigtime = trigtime_TPG;
    prev_globEventId = globEventId_TPG ; 
  }

    std::cout << "local tpgarray.size(): " << tpgarray.size() << std::endl;
}
