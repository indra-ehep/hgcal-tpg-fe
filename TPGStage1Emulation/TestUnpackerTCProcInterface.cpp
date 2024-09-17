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

l1thgcfirmware::HGCalTriggerCellSACollection convertOSPToTCs(std::vector<TPGBEDataformat::UnpackerOutputStreamPair> &upVec){
  l1thgcfirmware::HGCalTriggerCellSACollection theTCVec;
  for (auto& up : upVec){
      unsigned theModId_ = up.getModuleId();
      //std::cout<<"Module ID "<<theModId_<<" subtract per TC "<<subPerTC<<std::endl;
      //std::cout<<"Number of valid channels "<<up.numberOfValidChannels()<<std::endl;
      unsigned nChans_ = up.numberOfValidChannels();
      for (unsigned iChn = 0; iChn < nChans_; iChn++){
        unsigned nTC=0;
        unsigned nStream=0;
        if(nChans_<7){
            nTC=iChn;
        } else {
            nStream = iChn%2;
            nTC = (iChn-nStream)/2;
            //std::cout<<"for TC in iChn "<<iChn<<" nStream "<<nStream<<" nTC "<<nTC<<std::endl;
        }
        theTCVec.emplace_back(1,1,0,up.channelNumber(nStream,nTC),0,up.unpackedChannelEnergy(nStream,nTC));
        theTCVec.back().setModuleId(theModId_);

        //std::cout<<"number "<<up.channelNumber(nStream,nTC)<<" address "<<up.channelNumber(nStream,nTC)-iChn*subPerTC<<" unpacked energy "<<up.unpackedChannelEnergy(nStream,nTC)<<std::endl;
      }
  }
  return theTCVec;
}



int main() {
  TPGStage1Emulation::Stage1IOFwCfg fwCfg;

  unsigned ln(0);
  for(unsigned lp(0);lp<TPGStage1Emulation::Stage1IOFwCfg::MaximumLpgbtPairs;lp++) {
    bool connected(false);
    for(unsigned up(0);up<TPGStage1Emulation::Stage1IOFwCfg::MaximumUnpackersPerLpgbtPair;up++) {
      if(fwCfg.connected(lp,up)) connected=true;
    }
  }
  
  TPGFEDataformat::TcRawDataPacket vTc;
  TPGBEDataformat::UnpackerOutputStreamPair theOSP;
  std::vector<TPGBEDataformat::UnpackerOutputStreamPair> theOutputStreams;

  //The following loop largely copied from GenerateEmpRxFile, to get some random data
  unsigned bxi;
  unsigned obx(8);

  for(unsigned ibx(0);ibx<1;ibx++) { //No messing with bx for test
    unsigned bx=obx+ibx;
    //std::cout<<"bx "<<bx<<" obx "<<obx<<" ibx "<<ibx<<std::endl;

    if(bx==0 || bx>=3564) bxi=0xf;
    else bxi=(bx%8);

    //std::cout << "BX loop = " << bx << ", internal " << bxi << std::endl;

    unsigned nConnected=0; //Number of connected lpGBT/unpackers -> this will be used as module number. Works because some lpGBTs not connected at all! 
    for(unsigned lp(0);lp<TPGStage1Emulation::Stage1IOFwCfg::MaximumLpgbtPairs;lp++) {

      bool connected(false);

      for(unsigned up(0);up<TPGStage1Emulation::Stage1IOFwCfg::MaximumUnpackersPerLpgbtPair;up++) {
        if(fwCfg.connected(lp,up)) {
	          connected=true;
	          //vTc.second.resize(0);

	          TPGFEDataformat::Type type=fwCfg.type(lp,up);
	          unsigned nTc=fwCfg.numberOfTCs(lp,up);

	          TPGFEModuleEmulation::ECONTEmulation::generateRandomTcRawData(bx,type,nTc,vTc);

            //Print created TCs
	          /*std::cout << std::endl << "lp,up = " << lp << ", " << up
		        << ", TCs generated" << std::endl;
            vTc.print();*/

            //Now pass the randomly created data to the unpacker. 
            //Doing this in a slightly dumb way (always convert immediately after creating the data, and append to a vector)
            TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(bxi,vTc,theOSP);
            theOSP.setModuleId(nConnected);
            theOutputStreams.push_back(theOSP);
            nConnected+=1;
	      }

      }

    }

  }

  //Convert unpacker output to standalone HGCal trigger cells
  l1thgcfirmware::HGCalTriggerCellSACollection theTCsFromOS;
  theTCsFromOS = convertOSPToTCs(theOutputStreams);
 
  //Configure mapping (should match Florence's configuration)
  l1thgcfirmware::HGCalLayer1PhiOrderFwConfig theConfiguration_;
  theConfiguration_.configureMappingInfo();

  l1thgcfirmware::HGCalTriggerCellSACollection tcs_out_SA;
  l1thgcfirmware::HGCalLayer1PhiOrderFwImpl theAlgo_;

  //Run TC processor
  unsigned error_code = theAlgo_.run(theTCsFromOS, theConfiguration_, tcs_out_SA);

  std::cout<<"Printing TCs with column, channel, frame mapping"<<std::endl;
  for (auto& tcobj : tcs_out_SA){
    std::cout<<"Mod ID "<<tcobj.moduleId()<<" address "<<tcobj.phi()<<" energy "<<tcobj.energy()<<" col "<<tcobj.column()<<" chn "<<tcobj.channel()<<" frame "<<tcobj.frame()<<std::endl;
  }

return error_code;
}


