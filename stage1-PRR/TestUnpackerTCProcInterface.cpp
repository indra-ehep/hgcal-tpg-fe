/*
g++ -I TPGStage1Emulation -I. HGCalLayer1PhiOrderFwImpl.cc -I. TestUnpackerTCProcInterface.cpp -L /opt/local/lib  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o TestUnpackerTCProcInterface.exe
*/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <random>

#include "TPGFEModuleEmulation.hh"
#include "Stage1IOFwCfg.hh"
#include "Stage1IO.hh"
#include "HGCalTriggerCell_SA.h"
#include "HGCalLayer1PhiOrderFwImpl.h"
#include "HGCalLayer1PhiOrderFwConfig.h"
#include "L1Trigger/DemonstratorTools/interface/utilities.h"
#include "TPGFEDataformat.hh"
#include "TPGBEDataformat.hh"
#include "Stage1IO.hh"
#include "TFile.h"
#include "TTree.h"

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
        theTCVec.emplace_back(1,1,0,up.channelNumber(nStream,nTC),0,up.unpackedChannelEnergy(nStream,nTC));
        theTCVec.back().setModuleId(theModId_);

      }
  }
  return theTCVec;
}



int main(int argc, char** argv) {
  unsigned total_error_code=0;
  bool doPrint=false; //If turned on (by command line argument), print output to screen. Otherwise only create root file
  if(argc > 1 && atoi(argv[1])==1){
      doPrint=true;
  }

  TFile fileout("TCProcessor_EmulationResults.root","recreate");
  TTree evtstree("Events","Tree");
  Int_t mod_,link_,address_,col_,evt_;
  Long64_t energy_;
  evtstree.Branch("Event", &evt_,"evt_/I");
  evtstree.Branch("Module", &mod_,"mod_/I");
  evtstree.Branch("Link", &link_,"link_/I");
  evtstree.Branch("Address", &address_,"address_/I");
  evtstree.Branch("Column", &col_,"col_/I");
  evtstree.Branch("Energy", &energy_,"energy_/L");

  std::string inputFileName = "data_v11_rx_MsCounter/rx_summary.txt";

  l1t::demo::BoardData inputs = l1t::demo::read( inputFileName, l1t::demo::FileFormat::EMPv2 );

  auto nChannels = inputs.size();
  std::vector<unsigned int> channel_ids;
  for ( const auto& channel : inputs ) {
      channel_ids.push_back(channel.first);
  }
  uint32_t elinks[128][290][4]; //128 events, 290 modules, maximum of 4 elinks
  for(unsigned int nChn = 0; nChn<(channel_ids.size()/2); nChn+=1){
      unsigned thechnnr = channel_ids.at(2*nChn);
      for (unsigned int iEvt=0; iEvt < 128 ; iEvt++){
        assert(inputs.at(thechnnr).at(iEvt*8).startOfPacket);
        elinks[iEvt][nChn*5+0][0] = inputs.at(thechnnr).at(iEvt*8+0).data;
        elinks[iEvt][nChn*5+2][1] = inputs.at(thechnnr+1).at(iEvt*8+0).data;
        elinks[iEvt][nChn*5+0][1] = inputs.at(thechnnr).at(iEvt*8+1).data;
        elinks[iEvt][nChn*5+2][2] = inputs.at(thechnnr+1).at(iEvt*8+1).data;
        elinks[iEvt][nChn*5+0][2] = inputs.at(thechnnr).at(iEvt*8+2).data;
        elinks[iEvt][nChn*5+2][3] = inputs.at(thechnnr+1).at(iEvt*8+2).data;
        elinks[iEvt][nChn*5+0][3] = inputs.at(thechnnr).at(iEvt*8+3).data;
        elinks[iEvt][nChn*5+3][0] = inputs.at(thechnnr+1).at(iEvt*8+3).data;
        elinks[iEvt][nChn*5+1][0] = inputs.at(thechnnr).at(iEvt*8+4).data;
        elinks[iEvt][nChn*5+3][1] = inputs.at(thechnnr+1).at(iEvt*8+4).data;
        elinks[iEvt][nChn*5+1][1] = inputs.at(thechnnr).at(iEvt*8+5).data;
        elinks[iEvt][nChn*5+4][0] = inputs.at(thechnnr+1).at(iEvt*8+5).data;
        elinks[iEvt][nChn*5+2][0] = inputs.at(thechnnr).at(iEvt*8+6).data;
        elinks[iEvt][nChn*5+4][1] = inputs.at(thechnnr+1).at(iEvt*8+6).data;
      }
  }

    for(unsigned int iEvt = 0; iEvt <128 ;iEvt++){
      std::vector<TPGBEDataformat::UnpackerOutputStreamPair> theOutputStreams;
      if(doPrint)
      std::cout<<"==========EVENT "<<iEvt<<"==========="<<std::endl;
      evt_ = iEvt;
      for(unsigned int iMod=0; iMod<290; iMod++){
        TPGFEDataformat::TcRawDataPacket vTc;
        TPGBEDataformat::UnpackerOutputStreamPair theOSP;
        if(iMod%5==0){
           TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::BestC, 9, elinks[iEvt][iMod], vTc);
        } else if (iMod%5==1){
           TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::BestC, 4, elinks[iEvt][iMod], vTc);
        } else if (iMod%5==2){
           TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC4A, 12, elinks[iEvt][iMod], vTc);
        } else if (iMod%5==3){
           TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC16, 3, elinks[iEvt][iMod], vTc);
        } else if (iMod%5==4){
           TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC16, 3, elinks[iEvt][iMod], vTc);
        }
        TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(1,vTc,theOSP);
        theOSP.setModuleId(iMod);
        theOutputStreams.push_back(theOSP);
      }
        
      //Convert unpacker output to standalone HGCal trigger cells
      l1thgcfirmware::HGCalTriggerCellSACollection theTCsFromOS;
      theTCsFromOS = convertOSPToTCs(theOutputStreams);
    
      //Configure mapping (should match Florence's configuration)
      l1thgcfirmware::HGCalLayer1PhiOrderFwConfig theConfiguration_;
      theConfiguration_.configureTestSetupMappingInfo();
      theConfiguration_.configureTestSetupTDAQReadoutInfo();
  
      l1thgcfirmware::HGCalTriggerCellSACollection tcs_out_SA;
      l1thgcfirmware::HGCalLayer1PhiOrderFwImpl theAlgo_;

      //Run TC processor
      unsigned error_code = theAlgo_.run(theTCsFromOS, theConfiguration_, tcs_out_SA);
  
      total_error_code+=error_code;
  
      if(doPrint)
      std::cout<<"Printing TCs with column, channel, frame mapping"<<std::endl;
      for (auto& tcobj : tcs_out_SA){
	if(doPrint)
        std::cout<<"Mod ID "<<tcobj.moduleId()<<" address "<<tcobj.phi()<<" energy "<<tcobj.energy()<<" col "<<tcobj.column()<<" chn "<<tcobj.channel()<<" frame "<<tcobj.frame()<<std::endl;
	mod_=tcobj.moduleId();
	address_=tcobj.phi();
	energy_=tcobj.energy();
	col_=tcobj.column();
	link_=channel_ids.at(2*(mod_/5));//This is the start
	evtstree.Fill();
      }
    }
evtstree.Write();
return total_error_code;
}


