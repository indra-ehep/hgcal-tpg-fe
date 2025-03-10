/**********************************************************************
 Created on : 04/02/2025
 Purpose    : Read the events of a given lpGBT pair and print the TC values
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/

#include <filesystem>
#include <iostream>
#include <fstream>

#include "L1Trigger/DemonstratorTools/interface/utilities.h"

#include "TPGFEConfiguration.hh"
#include "TPGFEDataformat.hh"
#include "TPGBEDataformat.hh"
#include "Stage1IO.hh"

#include "yaml-cpp/yaml.h"

using namespace std;

int main(int argc, char** argv)
{
  std::string inputFileName = "./input/stage1-PRR/EMPSample/rx_summary_60.txt";
  l1t::demo::BoardData inputs = l1t::demo::read( inputFileName, l1t::demo::FileFormat::EMPv2 );
  auto nChannels = inputs.size();
  std::cout << "Board name : " << inputs.name() << std::endl;
  std::cout << "Number of lpGBTs : " << inputs.size() << std::endl;
  std::cout << "Number of lpGBT pairs : " << inputs.size()/2 << std::endl;

  uint32_t zside = 0, sector = 0, link = 0, det = 0;
  uint32_t econt = 0, selTC4 = 1, module = 0;
  std::map<uint32_t,TPGFEConfiguration::ConfigEconT> econTPar;
  TPGFEConfiguration::ConfigEconT econTConf[5];
  TPGFEDataformat::TcRawDataPacket vTCel[5];
  TPGBEDataformat::UnpackerOutputStreamPair up1[5];
  std::vector<uint64_t> bcElist;
  int lpGBTpairs = inputs.size()/2;
  int stcydim = 3*lpGBTpairs ; //nof ECONTs with STC is 3
  std::vector<std::vector<uint64_t>> stcElist(12, std::vector<uint64_t>(stcydim, 0)); //12 corresponds to the maximum STC among the three ECONT types
  
  uint32_t Elinks[14], iel;
  typedef struct{
    uint32_t Elinks[14];
  }ElEvent;
  std::map<uint64_t,ElEvent> elEvents;
  
  std::map<uint32_t,uint32_t> lpGBTPair;
  std::map<size_t, l1t::demo::BoardData::Channel>::const_iterator itrlp;
  TPGFEConfiguration::TPGFEIdPacking pck;
  int ilppair = 0;
  uint64_t ievent = 0;

  for (itrlp = inputs.begin() ;  itrlp != inputs.end() and ilppair<1 ; itrlp++ ) { //loop on lpGBT pairs
    uint32_t lpGBT0 = itrlp->first;
    std::cout << "lpGBT0 : " << lpGBT0 << std::endl;
    l1t::demo::BoardData::Channel lp0 = itrlp->second;
    unsigned int iFrame0 = 0;
    unsigned int iFrame1 = 0;
    iel = 0;
    ElEvent elevt;
    for (unsigned int ifr0 = 0 ;  ifr0 < lp0.size() ; ifr0++ ) { //loop on lpGBT pairs
      l1t::demo::Frame frame0 = lp0.at(ifr0);
      if(frame0.startOfPacket) iel = 0;
      //std::cout << frame0.startOfOrbit << frame0.startOfPacket << frame0.endOfPacket << frame0.valid << " " << std::hex << frame0.data << std::dec << std::endl;
      if(frame0.valid){
	elevt.Elinks[iel] = frame0.data;
	iel++;
      }
      if(frame0.endOfPacket) {
	elEvents[ievent] = elevt ;
	ievent++;
      }
      iFrame0++;
    }
    cout << "Nof Events : " << elEvents.size() << endl;
    itrlp++;
    uint32_t lpGBT1 = itrlp->first;
    std::cout << "lpGBT1 : " << lpGBT1 << std::endl;
    l1t::demo::BoardData::Channel lp1 = itrlp->second;
    ievent = 0;
    for (unsigned int ifr1 = 0 ;  ifr1 < lp0.size() ; ifr1++ ) { //loop on lpGBT pairs
      l1t::demo::Frame frame1 = lp1.at(ifr1);
      if(frame1.startOfPacket) iel = 7;
      //Elinks[iel++] = frame1.data;
      //std::cout << frame1.startOfOrbit << frame1.startOfPacket << frame1.endOfPacket << frame1.valid << " " << std::hex << frame1.data << std::dec << std::endl;
      if(frame1.valid){
	elEvents[ievent].Elinks[iel] = frame1.data;
	iel++;
      }
      if(frame1.endOfPacket) ievent++;
      iFrame1++;
    }
    std::cout << std::endl;    
    lpGBTPair[lpGBT0] = lpGBT1;
    ilppair++;
    
  }

  
  for(const auto& elevent : elEvents){
    for(int i=0;i<70;i++) std::cout << "=" ;
    std::cout << std::endl;
    std::cout << "Event: " << elevent.first << std::endl;

    ///// Elinks words ///////
    for(int i=0;i<35;i++) std::cout << "=" ;
    std::cout << std::endl;
    std::cout << "Elink words: "<< std::endl;
    for(uint32_t iel1=0;iel1<iel;iel1++) {
      std::cout << std::setfill('0') << std::setw(2) << iel1 << ": 0x" << std::hex << std::setfill('0') << setw(8) << elevent.second.Elinks[iel1] << std::dec << ", "<< std::endl;
    }
    for(int i=0;i<35;i++) std::cout << "=" ;
    std::cout << std::endl;
    //////////////////////////

    econt = 0; uint32_t bc0 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
    econt = 1; uint32_t bc1 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
    econt = 2; uint32_t stc4a = pck.packModId(zside, sector, link, det, econt, selTC4, module);
    econt = 3; uint32_t stc160 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
    econt = 4; uint32_t stc161 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
    econTConf[0].setSelect(2);
    econTConf[0].setNElinks(4);
    econTPar[bc0] = econTConf[0];
    econTConf[1].setSelect(2);
    econTConf[1].setNElinks(2);
    econTPar[bc1] = econTConf[1];
    econTConf[2].setSelect(1);
    econTConf[2].setNElinks(4);
    econTConf[2].setSTCType(3);
    econTPar[stc4a] = econTConf[2];
    econTConf[3].setSelect(1);
    econTConf[3].setNElinks(2);
    econTConf[3].setSTCType(1);
    econTPar[stc160] = econTConf[3];
    econTConf[4].setSelect(1);
    econTConf[4].setNElinks(2);
    econTConf[4].setSTCType(1);
    econTPar[stc161] = econTConf[4];
        
    TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::BestC, econTConf[0].getNofTCs(), &(elevent.second.Elinks[0]), vTCel[0]);
    TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::BestC, econTConf[1].getNofTCs(), &(elevent.second.Elinks[4]), vTCel[1]);
    TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC4A, econTConf[2].getNofSTCs(), &(elevent.second.Elinks[6]), vTCel[2]);
    TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC16, econTConf[3].getNofSTCs(), &(elevent.second.Elinks[10]), vTCel[3]);
    TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC16, econTConf[3].getNofSTCs(), &(elevent.second.Elinks[12]), vTCel[4]);
    for(int iecon=0;iecon<5;iecon++) {
      if(iecon<2) std::cout << "EconT : " << iecon << ", nof TCs : " << econTConf[iecon].getNofTCs() << std::endl;
      else std::cout << "EconT : " << iecon << ", nof STCs : " << econTConf[iecon].getNofSTCs() << std::endl;
      vTCel[iecon].print();
    }
    for(int i=0;i<70;i++) std::cout << "=" ;
    std::cout << std::endl;
    
    for(int iecon=0;iecon<5;iecon++) TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(vTCel[iecon].bx(), vTCel[iecon], up1[iecon]);
    for(int iecon=0;iecon<5;iecon++) up1[iecon].print();

    for(int i=0;i<70;i++) std::cout << "=" ;
    std::cout << std::endl;

    // for(int iecon=0;iecon<2;iecon++) bcElist.push_back(up1[iecon].moduleSum(0));
    // for(int iecon=2;iecon<5;iecon++){
    //   int ix = ilppair*3 + (iecon-2);
    //   int iy = 0;
    //   for(int is = 0 ; is < 2 ; is++){
    // 	for(int itc = 0 ; itc<TPGBEDataformat::UnpackerOutputStreamPair::NumberOfTCs ; itc++){
    // 	  if(up1[iecon].isValidChannel(is, itc)) stcElist[iy++][ix] = up1[iecon].channelEnergy(is,itc);
    // 	}//tc loop
    //   }//stream loop
    // }//econ loop
    
  }//event loop
  return true;
}



    
