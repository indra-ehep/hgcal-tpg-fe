/**********************************************************************
 Created on : 18/10/2024
 Purpose    : Example to read TPG data for TC processor
 Author     : Indranil Das, Research Associate, Imperial
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
#include "HGCalLayer1PhiOrderFwConfig.h"
#include "HGCalLayer1PhiOrderFwImpl.h"
#include "HGCalTriggerCell_SA.h"

int main(int argc, char** argv)
{
  
  std::cout << "========= tpgdata_3T_tcproc================" << std::endl;

  if(argc < 3){
    std::cerr << argv[0] << ": no relay and/or run numbers specified" << std::endl;
    return false;
  }
  uint64_t getTotalNofEvents(uint32_t, uint32_t);
  //===============================================================================================================================
  //Assign relay,run and numbers of events from input
  //===============================================================================================================================
  uint32_t relayNumber(0);
  uint32_t runNumber(0);
  uint32_t linkNumber(0);
  uint64_t totEvents(0);
  uint64_t nofEvents(100);
  std::istringstream issRelay(argv[1]);
  issRelay >> relayNumber;
  //Notes:
  // 1. firmware test for third train 1727007636 (old ECON-T configuration so should not be analyzed)
  // 2. first pedestal with 3rd layer in configuration 1727033054 and installed 1727099251 (with correct ECON-T configuration)
  // 3. first technical with third layer : 1727099443 //https://cmsonline.cern.ch/webcenter/portal/cmsonline/Common/Elog?__adfpwp_action_portlet=683379043&__adfpwp_backurl=https%3A%2F%2Fcmsonline.cern.ch%3A443%2Fwebcenter%2Fportal%2Fcmsonline%2FCommon%2FElog%3F_afrRedirect%3D23225796808764440&_piref683379043.strutsAction=%2FviewMessageDetails.do%3FmsgId%3D1237840
  // 4. swap back roc2 config of mod16 for relays after 1727116899 (not necessary for this code, kept as bookkeeping)
  // 5. List of run flagged as green by Paul for TC Processor are Relay1727219172 to Run1727219175 and probably Relay1727211141
  

  if(relayNumber<1727099251){
    std::cerr << "Third layer is not properly configured" << std::endl;
    return false;
  }

  std::istringstream issRun(argv[2]);
  issRun >> runNumber;

  std::cout<<argc<<std::endl;
  if(argc == 4){
    std::istringstream issnEvents(argv[3]);
    issnEvents >> nofEvents;
  }

  totEvents = getTotalNofEvents(relayNumber, runNumber);
  if(nofEvents>totEvents){
    nofEvents = totEvents ;
    std::cout << "Setting number of events to process to " << nofEvents << std::endl;
  }
  //===============================================================================================================================
  //Helper functions
  l1thgcfirmware::HGCalTriggerCellSACollection convertOSPToTCs(std::vector<TPGBEDataformat::UnpackerOutputStreamPair> &);
  bool hasDifferentTCs(l1thgcfirmware::HGCalTriggerCellSACollection, l1thgcfirmware::HGCalTriggerCellSACollection);
  std::map<uint32_t,l1thgcfirmware::HGCalTriggerCellSACollection> createTCsFromTriggerData(uint32_t, TPGBEDataformat::TrigTCProcData);//ibx

  //===============================================================================================================================
  //Book histograms
  //===============================================================================================================================
  void BookHistograms(TDirectory*&, uint32_t);
  void FillHistogram( TDirectory*&, l1thgcfirmware::HGCalTriggerCellSACollection, l1thgcfirmware::HGCalTriggerCellSACollection);
  TFile *fout = new TFile(Form("Diff_TCProc_Relay-%u.root",relayNumber),"recreate");
  TDirectory *dir_diff = fout->mkdir("diff_plots");
  BookHistograms(dir_diff, relayNumber);
  //===============================================================================================================================
  //Read channel mapping
  //===============================================================================================================================
  TPGFEConfiguration::Configuration cfgs;
  cfgs.setSiChMapFile("cfgmap/WaferCellMapTraces.txt");
  cfgs.setSciChMapFile("cfgmap/channels_sipmontile_HDtypes.hgcal.txt");
  cfgs.initId();
  cfgs.readSiChMapping();
  cfgs.readSciChMapping();
  cfgs.loadModIdxToNameMapping();
  cfgs.loadMuxMapping();
  //===============================================================================================================================

  //===============================================================================================================================
  //Read ECON-T config : note that we are not reading any ECON-D config
  //===============================================================================================================================
  //First ready a dummy config
  uint32_t zside = 0, sector = 0, link = 0, det = 0;
  uint32_t econt = 0, selTC4 = 1, module = 0;
  uint32_t noflpGBTs = 4;
  TPGFEConfiguration::TPGFEIdPacking pck;
  for(int ilink=0;ilink<noflpGBTs;ilink++){
    int maxecons = (ilink<=1)?3:2;
    for(int iecont=0;iecont<maxecons ;iecont++){
      uint32_t idx = pck.packModId(zside, sector, ilink, det, iecont, selTC4, module); 
      cfgs.setModulePath(zside, sector, ilink, det, iecont, selTC4, module);
      cfgs.setEconTFile("cfgmap/init_econt.yaml"); //read a dummy econt since the original was not available earlier
      cfgs.readEconTConfigYaml();
    }//econt loop
  }//lpGBT loop

  //Step2: update all ECON-T configs corresponding to test-beam after the inclusion of third train
  link = 0; econt = 0; uint32_t lp0_bc0 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 0; econt = 1; uint32_t lp0_bc1 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 0; econt = 2; uint32_t lp0_bc2 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 1; econt = 0; uint32_t lp1_stc160 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 1; econt = 1; uint32_t lp1_stc161 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 1; econt = 2; uint32_t lp1_stc162 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 2; econt = 0; uint32_t lp2_stc4a0 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 2; econt = 1; uint32_t lp2_stc4a1 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 3; econt = 0; uint32_t lp3_stc4a0 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 3; econt = 1; uint32_t lp3_stc4a1 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  
  uint32_t density(0);
  uint32_t droplsb(1);
  std::map<uint32_t,TPGFEConfiguration::ConfigEconT>& econTPar =  cfgs.getEconTPara();
  for(const auto& it : econTPar){
    econTPar[it.first].setDensity(density);
    econTPar[it.first].setDropLSB(droplsb);
    
    if(it.first==lp0_bc0){              //// starting 1st train
      econTPar[it.first].setSelect(2);
      econTPar[it.first].setNElinks(3);      
    }else if (it.first==lp0_bc1){
      econTPar[it.first].setSelect(2);
      econTPar[it.first].setNElinks(2);     
    }else if (it.first==lp0_bc2){
      econTPar[it.first].setSelect(2);
      econTPar[it.first].setNElinks(2);
    }else if (it.first==lp1_stc160){    //// starting 2nd train
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(1);
    }else if (it.first==lp1_stc161){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(1);
    }else if (it.first==lp1_stc162){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(1);
    }else if (it.first==lp2_stc4a0){    //// starting 3rd train
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(4);
      econTPar[it.first].setSTCType(3);
    }else if (it.first==lp2_stc4a1){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(3);
    }else if (it.first==lp3_stc4a0){    //// starting motherboard
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(4);
      econTPar[it.first].setSTCType(3);
    }else if (it.first==lp3_stc4a1){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(3);
      econTPar[it.first].setSTCType(3);
    }
    std::cout << "Modtype " << it.first
	      << ", getOuttype: " << econTPar[it.first].getOutType()
	      << ", select: " << econTPar[it.first].getSelect()
      	      << ", stctype: " << econTPar[it.first].getSTCType()
	      << ", typename: " << TPGFEDataformat::tctypeName[econTPar[it.first].getOutType()]
	      << std::endl;
    econTPar[it.first].print();
  }//econt loop
  //===============================================================================================================================
  
  // ===============================================================================================================================
  // Set and Initialize the ECONT reader
  // ===============================================================================================================================
  TPGFEReader::ECONTReader econTReader(cfgs);
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGBEDataformat::Trig24Data>>> econtarray; //event,moduleId (from link)
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGBEDataformat::TrigTCProcData>>> tcprocarray; //event,moduleId (from link)
  econTReader.setNofCAFESep(14);
  // ===============================================================================================================================

  //===============================================================================================================================
  //Set mapping info for TC processor bins
  //===============================================================================================================================
  l1thgcfirmware::HGCalLayer1PhiOrderFwConfig theConfiguration_;
  theConfiguration_.configureSeptemberTestBeamMappingInfo();

  //===============================================================================================================================
  //Process certain reference eventlist
  //===============================================================================================================================
  /*TcTp1 events */ uint64_t refEvt[2] = {19, 137};
  std::vector<uint64_t> refEvents;
  for(int ievent=0;ievent<2;ievent++) refEvents.push_back(refEvt[ievent]);
  //when the following line is "commented" only the events filled in "refEvents" will be processed instead of nofevents set in the command line
  refEvents.resize(0);
  //===============================================================================================================================

  uint64_t minEventTPG, maxEventTPG;  
  long double nloopEvent =  100000;
  int nloop = TMath::CeilNint(nofEvents/nloopEvent) ;
  if(refEvents.size()>0) nloop = 1;
  std::cout<<"nloop: "<<nloop<<std::endl;

  for(int ieloop=0;ieloop<nloop;ieloop++){
    
    minEventTPG = ieloop*nloopEvent ;
    maxEventTPG = (ieloop+1)*nloopEvent;
    maxEventTPG = (maxEventTPG>uint64_t(nofEvents)) ? uint64_t(nofEvents) : maxEventTPG ;
    printf("iloop : %d, minEventTPG = %lu, maxEventTPG = %lu\n",ieloop,minEventTPG, maxEventTPG);
    std::cerr<<"iloop : "<< ieloop <<", minEventTPG = "<< minEventTPG <<", maxEventTPG = " << maxEventTPG << std::endl;

    econtarray.clear();
    tcprocarray.clear();
    
    econTReader.init(relayNumber,runNumber,0);
    std::cout<<"TRIG Before: elink and unpacker size : " << econtarray.size() << ", tcprocarray.size(): " << tcprocarray.size() <<std::endl;
    econTReader.getEvents(refEvents, minEventTPG, maxEventTPG, econtarray, tcprocarray);
    std::cout<<"TRIG After:  elink and unpacker size : " << econtarray.size() << ", tcprocarray.size(): " << tcprocarray.size() <<std::endl;
    econTReader.terminate();
    
    for( auto& it :  econtarray){ //this is essencially event loop
      uint64_t event = it.first ;
      std::cout << "Processing event: " << event << std::endl;
      std::map<uint32_t,l1thgcfirmware::HGCalTriggerCellSACollection> triggerCellsFromTriggerData;
      for(int ilink=0;ilink<noflpGBTs;ilink++){
	int maxecons = (ilink<=1)?3:2;
	for(int iecont=0;iecont<maxecons;iecont++){
	  uint32_t moduleId = pck.packModId(zside, sector, ilink, det, iecont, selTC4, module); 
	  std::vector<std::pair<uint32_t,TPGBEDataformat::Trig24Data>> econtdata =  econtarray[event];
    std::vector<std::pair<uint32_t,TPGBEDataformat::TrigTCProcData>> tcprocdata = tcprocarray[event];	  
	  TPGBEDataformat::UnpackerOutputStreamPair upemul,updata;
	  TPGFEDataformat::TcRawDataPacket vTCel;	
	  TPGBEDataformat::Trig24Data trdata;
    TPGBEDataformat::TrigTCProcData tcpdata;
    std::vector<TPGBEDataformat::UnpackerOutputStreamPair> theOutputStreams;
    l1thgcfirmware::HGCalTriggerCellSACollection theTCsFromOS;
    l1thgcfirmware::HGCalTriggerCellSACollection tcs_out_SA; //output from elink data
    l1thgcfirmware::HGCalTriggerCellSACollection tcs_out_tcproc_TB; //trigger cell output from tc processor in testbeam
    l1thgcfirmware::HGCalLayer1PhiOrderFwImpl theAlgo_;
	  
	  //for(const auto& econtit : econtdata){
    for(uint32_t ietd= 0; ietd < econtdata.size(); ietd++){
	    if(econtdata.at(ietd).first!=moduleId) continue;//Need to assume that order of econtdata and tcprocdata entries is the same.
	    trdata = econtdata.at(ietd).second ;
      tcpdata = tcprocdata.at(ietd).second;
      triggerCellsFromTriggerData = createTCsFromTriggerData(moduleId,tcpdata);
	    std::cout << "Dataloop:: event: " << event << ", moduleId : " << econtdata.at(ietd).first << " moduleId from tcproc data "<<tcprocdata.at(ietd).first << std::endl;
	    //for(int ibx=0;ibx<7;ibx++){
	    for(int ibx=3;ibx<4;ibx++){
	      const uint32_t *el = trdata.getElinks(ibx); 
	      uint32_t bx_2 = (el[0]>>28) & 0xF;
	      if(ilink==0) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::BestC, econTPar[moduleId].getNofTCs(), el, vTCel);
	      if(ilink==1) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC16, econTPar[moduleId].getNofSTCs(), el, vTCel);
	      if(ilink==2 or ilink==3) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC4A, econTPar[moduleId].getNofSTCs(), el, vTCel);
	      trdata.getUnpkStream(ibx,updata);
        updata.setModuleId(moduleId);
	      //==================================================================
	      //TC proc emulation something similar to below
	      //==================================================================
	      //TPGStage1Emulation::Stage1IO::convertUnpackerOutputStreamPairToTCProc(bx_2, updata, tcprocemul);
	      //==================================================================
	      std::cout << "========== ibx : " << ibx << " ==========" << std::endl;
	      std::cout << "========== TPG data: Stage1 unpacker output stream from firmware output for ibx : " << ibx << " ==========" << std::endl;
	      updata.print();
	      //==================================================================
	      //TC proc emulation comparison with data something similar to below
	      //==================================================================
        theOutputStreams.resize(0);

        theOutputStreams.push_back(updata);//Always a single UnpackerOutputStreamPair, vector only for generality

        //Convert unpacker output to standalone HGCal trigger cells
        theTCsFromOS.resize(0);
        theTCsFromOS = convertOSPToTCs(theOutputStreams);


        //Run TC processor emulator
        tcs_out_SA.resize(0);
        unsigned error_code = theAlgo_.run(theTCsFromOS, theConfiguration_, tcs_out_SA);

        tcs_out_tcproc_TB.resize(0);
        bool isDataTCsAssigned=false;
        if(triggerCellsFromTriggerData[ibx].size()>0) tcs_out_tcproc_TB=triggerCellsFromTriggerData[ibx]; isDataTCsAssigned = true;


            
        if(isDataTCsAssigned){//Only try to do comparison if we have read out TB data since some modules are missing
          bool diffTCs=hasDifferentTCs(tcs_out_SA,tcs_out_tcproc_TB);

          if(diffTCs || 1){
            std::cout << "========== Emulated TC proc results using firmware data of Stage1 unpacker output stream for ibx : " << ibx << " ==========" << std::endl;

            std::cout<<"Printing TCs with column, channel, frame mapping - after TCprocEmul, from ECON-T elink input"<<std::endl;
            for (auto& tcobj : tcs_out_SA){
              std::cout<<"Mod ID "<<tcobj.moduleId()<<" address "<<tcobj.phi()<<" energy "<<tcobj.energy()<<" col "<<tcobj.column()<<" chn "<<tcobj.channel()<<" frame "<<tcobj.frame()<<std::endl;
            }

	          std::cout << "========== TPG data: TC proc results as read in data link for ibx : " << ibx << " ==========" << std::endl;

            std::cout<<"Printing TCs with column, channel, frame mapping - as read out from TB data"<<std::endl;
            for (auto& tcobj : tcs_out_tcproc_TB){
              std::cout<<"Mod ID "<<tcobj.moduleId()<<" address "<<tcobj.phi()<<" energy "<<tcobj.energy()<<" col "<<tcobj.column()<<" chn "<<tcobj.channel()<<" frame "<<tcobj.frame()<<std::endl;
            }
          }
          FillHistogram(dir_diff,tcs_out_SA,tcs_out_tcproc_TB);
        }
	      //tcprocemul.print();
	      //trdata.getTCData(ibx,tcprocdata);
	      //tcprocdata.print();
	      //==================================================================
	      std::cout << "========== end of TPG data for ibx : " << ibx << " ==========" << std::endl;
	    }//bx loop
	    //if(eventCondn) trdata.print();
	  }//loop over econt data for a given run

	}//econt loop
      }//link loop
    }//this is essencially event loop
    
  }//ieloop

  fout->cd();
  dir_diff->Write();
  fout->Close();
  delete fout;
  
  return true;
}

uint64_t getTotalNofEvents(uint32_t relayNumber, uint32_t runNumber){

  uint64_t nEvents(0);
  //Create the file reader
  Hgcal10gLinkReceiver::FileReader fReader;
  
  //Make the buffer space for the records
  Hgcal10gLinkReceiver::RecordT<4095> *r(new Hgcal10gLinkReceiver::RecordT<4095>);
  

  //Set up specific records to interpet the formats
  const Hgcal10gLinkReceiver::RecordStarting *rStart((Hgcal10gLinkReceiver::RecordStarting*)r);
  const Hgcal10gLinkReceiver::RecordStopping *rStop ((Hgcal10gLinkReceiver::RecordStopping*)r);
  const Hgcal10gLinkReceiver::RecordRunning  *rEvent((Hgcal10gLinkReceiver::RecordRunning*) r);
  fReader.setDirectory(std::string("dat/Relay")+std::to_string(relayNumber));
  //fReader.openRun(runNumber,linkNumber);
  //We open linkNumber=0 which contains trigger data
  fReader.openRun(runNumber,0); 
  

  while(fReader.read(r)) {
    //Check the state of the record and print the record accordingly
    if( r->state()==Hgcal10gLinkReceiver::FsmState::Starting){
      if(!(rStart->valid())){
  	std::cerr << " FsmState::Starting validadity fails : rStart->valid() " << rStart->valid() << std::endl;
  	rStart->print();
  	std::cout << std::endl;
	continue;
      }
    }
    
    else if(r->state()==Hgcal10gLinkReceiver::FsmState::Stopping){
      if(!(rStop->valid())){
  	std::cerr << " FsmState::Stopping validadity fails : rStop->valid() " << rStop->valid() << std::endl;
  	rStop->print();
  	std::cout << std::endl;
	continue;
      }
    }
    //Else we have an event record 
    else{
      nEvents++;      
    }//loop event
  }
  
  std::cout << "========== Total Events : " << nEvents << std::endl;
  
  delete r;

  return nEvents;
}

l1thgcfirmware::HGCalTriggerCellSACollection convertOSPToTCs(std::vector<TPGBEDataformat::UnpackerOutputStreamPair> &upVec){
  l1thgcfirmware::HGCalTriggerCellSACollection theTCVec;
  for (auto& up : upVec){
      unsigned theModId_ = up.getModuleId();
      unsigned nChans_ = up.numberOfValidChannels();
      for (unsigned iChn = 0; iChn < nChans_; iChn++){
        unsigned nTC=0;
        unsigned nStream=0;
        if(nChans_<7){
            nTC=iChn;
        } else {
            nStream = iChn%2;
            nTC = (iChn-nStream)/2;
        } 
        theTCVec.emplace_back(1,1,0,up.channelNumber(nStream,nTC),0,up.channelEnergy(nStream,nTC));
        theTCVec.back().setModuleId(theModId_);
      }
  }
  return theTCVec;
}

bool hasDifferentTCs(l1thgcfirmware::HGCalTriggerCellSACollection tcOrig, l1thgcfirmware::HGCalTriggerCellSACollection tcToComp){
   if(tcOrig.size()!=tcToComp.size()) return true;
   for(unsigned i = 0; i<tcOrig.size(); i++){ 
       if (tcOrig.at(i).energy()!=tcToComp.at(i).energy() || tcOrig.at(i).phi()!=tcToComp.at(i).phi() || tcOrig.at(i).channel()!=tcToComp.at(i).channel() || tcOrig.at(i).frame()!=tcToComp.at(i).frame() || tcOrig.at(i).column()!=tcToComp.at(i).column()){
	       return true;
       }
   }
   return false;
}

std::map<uint32_t,l1thgcfirmware::HGCalTriggerCellSACollection> createTCsFromTriggerData(uint32_t moduleId, TPGBEDataformat::TrigTCProcData tcdata_){//ibx
    std::map<uint32_t,l1thgcfirmware::HGCalTriggerCellSACollection> theTCsFromTriggerData_;
    for(int ibx=0;ibx<7;ibx++){
      l1thgcfirmware::HGCalTriggerCellSACollection tmp_tcs;
      for(int ibin=0;ibin<9;ibin++){
        for(int iinstance=0;iinstance<2;iinstance++){
          int tcoutputword = tcdata_.getUnpkWord(ibx,ibin,iinstance);
          if(tcoutputword > -1){
            uint32_t tcout_energy = tcoutputword&0x1FF;
            uint32_t tcout_channel = (tcoutputword>>9)&0x3F;
            tmp_tcs.emplace_back(1,1,0,tcout_channel,0,tcout_energy);
            tmp_tcs.back().setColumn(ibin);
            tmp_tcs.back().setChannel(0);
            tmp_tcs.back().setFrame(0);
            tmp_tcs.back().setModuleId(moduleId);
          }
        }
      }
      theTCsFromTriggerData_[ibx]=tmp_tcs;
    }
  return theTCsFromTriggerData_;
}

void BookHistograms(TDirectory*& dir_diff, uint32_t relay){
  TH1D *hTCProcDiff_nonzero_size = new TH1D("hTCProcDiff_nonzero_size", Form("TC processor difference in output size if both TC lists are nonzero in length for Relay: %u", relay),51,-25.5,25.5);
  hTCProcDiff_nonzero_size->GetXaxis()->SetTitle("Difference between firmware TC processor and emulator (nonzero TC list length)");
  hTCProcDiff_nonzero_size->GetYaxis()->SetTitle("Entries");
  hTCProcDiff_nonzero_size->SetDirectory(dir_diff);

  TH1D *hTCProcDiff_size = new TH1D("hTCProcDiff_size", Form("TC processor difference in output size for Relay: %u", relay),51,-25.5,25.5);
  hTCProcDiff_size->GetXaxis()->SetTitle("Difference between firmware TC processor and emulator");
  hTCProcDiff_size->GetYaxis()->SetTitle("Entries");
  hTCProcDiff_size->SetDirectory(dir_diff);

  TH1D *hTCProcDiff_quantities = new TH1D("hTCProcDiff_quantities", Form("TC processor difference in output quantities for Relay: %u", relay),52,-0.5,25.5);
  hTCProcDiff_quantities->GetXaxis()->SetTitle("Difference between firmware TC processor and emulator");
  hTCProcDiff_quantities->GetYaxis()->SetTitle("Entries");
  hTCProcDiff_quantities->SetDirectory(dir_diff);
}

void FillHistogram(TDirectory*& dir_diff, l1thgcfirmware::HGCalTriggerCellSACollection tcProcEmulator, l1thgcfirmware::HGCalTriggerCellSACollection tcProcTB){
    TList *list = (TList *)dir_diff->GetList();

    int theTCListSizeDiff = tcProcEmulator.size()-tcProcTB.size();
    if (tcProcEmulator.size()>0 && tcProcTB.size()>0){
      int theTCListNonzeroSizeDiff = tcProcEmulator.size()-tcProcTB.size();
      ((TH1D *) list->FindObject("hTCProcDiff_nonzero_size"))->Fill(theTCListNonzeroSizeDiff);
    }
    ((TH1D *) list->FindObject("hTCProcDiff_size"))->Fill(theTCListSizeDiff);
    unsigned theSizeToTest = std::min(tcProcEmulator.size(),tcProcTB.size());
    for(unsigned i=0; i<theSizeToTest; i++){
      unsigned diffsize = std::abs(int(tcProcEmulator.at(i).energy()-tcProcTB.at(i).energy()))+std::abs(int(tcProcEmulator.at(i).phi()-tcProcTB.at(i).phi()))+std::abs(int(tcProcEmulator.at(i).channel()-tcProcTB.at(i).channel()))+std::abs(int(tcProcEmulator.at(i).frame()-tcProcTB.at(i).frame()))+std::abs(int(tcProcEmulator.at(i).column()-tcProcTB.at(i).column()));
      ((TH1D *) list->FindObject("hTCProcDiff_quantities"))->Fill(diffsize);
    }
}
