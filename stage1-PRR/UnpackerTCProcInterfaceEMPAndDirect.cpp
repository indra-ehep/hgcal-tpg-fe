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
#include "TFile.h"
#include "TTree.h"


l1thgcfirmware::HGCalTriggerCellSACollection convertOSPToTCs(std::vector<TPGBEDataformat::UnpackerOutputStreamPair> &upVec){
  l1thgcfirmware::HGCalTriggerCellSACollection theTCVec;
  for (auto& up : upVec){
      unsigned theModId_ = up.getModuleId();
      //std::cout<<"Module ID "<<theModId_<<std::endl;
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

        //std::cout<<"number "<<up.channelNumber(nStream,nTC)<<" address "<<up.channelNumber(nStream,nTC)<<" unpacked energy "<<up.unpackedChannelEnergy(nStream,nTC)<<std::endl;
      }
  }
  return theTCVec;
}



int main() {
  unsigned total_error_code=0;
  TPGStage1Emulation::Stage1IOFwCfg fwCfg;

  TFile fileout("TCProcessor_EmulationResults_Direct.root","recreate");
  TTree evtstree("Events","Tree");
  Int_t mod_,address_,col_,evt_;
  Long64_t energy_;
  evtstree.Branch("Event", &evt_,"evt_/I");
  evtstree.Branch("Module", &mod_,"mod_/I");
  evtstree.Branch("Address", &address_,"address_/I");
  evtstree.Branch("Column", &col_,"col_/I");
  evtstree.Branch("Energy", &energy_,"energy_/L");

  std::vector< std::pair<unsigned,std::vector<TPGFEDataformat::OrderedElinkPacket> > > empRx(60);
  unsigned linkN[128];

  unsigned ln(0);
  for(unsigned lp(0);lp<TPGStage1Emulation::Stage1IOFwCfg::MaximumLpgbtPairs;lp++) {
    bool connected(false);
    for(unsigned up(0);up<TPGStage1Emulation::Stage1IOFwCfg::MaximumUnpackersPerLpgbtPair;up++) {
      if(fwCfg.connected(lp,up)) connected=true;
    }

    if(connected) {
      assert(ln<60);
      linkN[lp]=ln;
      empRx[ln].first=lp;
      ln++;
    }
  }
  
  TPGFEDataformat::TcRawDataPacket vTc;
  TPGFEDataformat::OrderedElinkPacket vEl;
  TPGBEDataformat::UnpackerOutputStreamPair theOSP;
  

  //The following loop largely copied from GenerateEmpRxFile, to get some random data
  unsigned bxi;
  unsigned obx(8);

  for(unsigned ibx(0);ibx<128;ibx++) { //No messing with bx for test
    std::vector<TPGBEDataformat::UnpackerOutputStreamPair> theOutputStreams;
    unsigned bx=obx+ibx;

    if(bx==0 || bx>=3564) bxi=0xf;
    else bxi=(bx%8);

    std::cout << "BX loop = " << ibx << ", internal " << bxi << std::endl;

    unsigned nConnected=0; //Number of connected lpGBT/unpackers -> this will be used as module number. Works because some lpGBTs not connected at all! 
    for(unsigned lp(0);lp<TPGStage1Emulation::Stage1IOFwCfg::MaximumLpgbtPairs;lp++) {

      bool connected(false);

      for(unsigned up(0);up<TPGStage1Emulation::Stage1IOFwCfg::MaximumUnpackersPerLpgbtPair;up++) {
        if(fwCfg.connected(lp,up)) {
	          connected=true;

	          TPGFEDataformat::Type type=fwCfg.type(lp,up);
	          unsigned nTc=fwCfg.numberOfTCs(lp,up);

	          TPGFEModuleEmulation::ECONTEmulation::generateRandomTcRawData(ibx,type,nTc,vTc);

              TPGFEModuleEmulation::ECONTEmulation::convertToElinkData(bxi,vTc,vEl.data()+fwCfg.firstElink(lp,up));


            //Now pass the randomly created data to the unpacker. 
            //Doing this in a slightly dumb way (always convert immediately after creating the data, and append to a vector)
            TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(bxi,vTc,theOSP);
            theOSP.setModuleId(nConnected);
            theOutputStreams.push_back(theOSP);
            nConnected+=1;
	      }

      }
      if(connected) empRx[linkN[lp]].second.push_back(vEl);


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

    std::cout<<"Printing TCs with column, channel, frame mapping"<<std::endl;
    for (auto& tcobj : tcs_out_SA){
      std::cout<<"Mod ID "<<tcobj.moduleId()<<" address "<<tcobj.phi()<<" energy "<<tcobj.energy()<<" col "<<tcobj.column()<<" chn "<<tcobj.channel()<<" frame "<<tcobj.frame()<<std::endl;
      evt_=ibx;
      mod_=tcobj.moduleId();
	  address_=tcobj.phi();
	  energy_=tcobj.energy();
	  col_=tcobj.column();
	  evtstree.Fill();
    }

  }
  evtstree.Write();


     l1t::demo::BoardData boardData("ExampleBoard");
    for(auto theEmpOutput : empRx ){
        unsigned firstnr = theEmpOutput.first;
        l1t::demo::BoardData::Channel channelData_1;
        l1t::demo::BoardData::Channel channelData_2;
        for(auto theData : theEmpOutput.second){
        unsigned iFr=0;
        for(unsigned i=0; i<7;i++){
            l1t::demo::Frame frame;
            frame.data=theData[i];
            frame.valid = true;
            frame.startOfPacket = (iFr==0);
            //frame.startOfOrbit = (ibx==0&&iFr==0);
            frame.endOfPacket = (iFr==6);
            channelData_1.push_back(frame);
            iFr++;
        }
        if(iFr==7){
            l1t::demo::Frame frame;
            frame.data=0;
            frame.valid=false;
            channelData_1.push_back(frame);
        }
        iFr=0;
        for(unsigned i=7; i<14;i++){
            l1t::demo::Frame frame;
            frame.data=theData[i];
            frame.valid = true;
            frame.startOfPacket = (iFr==0);
            frame.endOfPacket = (iFr==6);
            channelData_2.push_back(frame);
            iFr++;
        }
        if(iFr==7){
            l1t::demo::Frame frame;
            frame.data=0;
            frame.valid=false;
            channelData_2.push_back(frame);
        }
        }
        boardData.add(theEmpOutput.first*2, channelData_1);
        boardData.add(theEmpOutput.first*2+1, channelData_2);
    }
    write(boardData, "stage1-PRR/RandomlyGenerated.txt.gz", l1t::demo::FileFormat::EMPv2);


return total_error_code;
}

