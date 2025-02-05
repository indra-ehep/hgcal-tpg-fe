/**********************************************************************
 Created on : 05/02/2025
 Purpose    : Compare the firmware and emulation outputs of stage2 tower processing
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/

#include <iostream>
#include "L1Trigger/DemonstratorTools/interface/utilities.h"


int main()
{
  void ReadEMPData(std::string fname, std::map<uint32_t,std::vector<uint64_t>>&);
  std::string inputEmulFileName = "EMPStage2Output.txt";
  std::string inputFWFileName = "input/stage2/firmware-data/CaptureStage2_250204_1424/tx_summary.txt";
  std::map<uint32_t,std::vector<uint64_t>> emuldata, fwdata;
  ReadEMPData(inputEmulFileName,emuldata);
  ReadEMPData(inputFWFileName,fwdata);
  std::cout << "emuldata.size : " << emuldata.size() << std::endl;
  std::cout << "fwdata.size : " << fwdata.size() << std::endl;
  for(const auto& emd: emuldata){
    int iw = 0;
    for(const auto& ed: emd.second){
      uint64_t XOR = ed xor fwdata[emd.first].at(iw) ;
      std::cout << "link: " << std::setfill('0') << std::setw(2) << emd.first
		<< ", iw: " << std::setfill('0') << std::setw(2) << iw
		<< ", emuldata : 0x"  << std::setw(16) << std::hex << ed
		<< ", fwdata: 0x" << fwdata[emd.first].at(iw)
		<< ", (emuldata XOR fwdata): 0x" << XOR
		<< std::dec << std::endl;
      iw++;
    }
    std::cout << std::endl;
  }
  return true;
}

void ReadEMPData(std::string fname, std::map<uint32_t,std::vector<uint64_t>>& linkwords)
{
  l1t::demo::BoardData inputs = l1t::demo::read( fname, l1t::demo::FileFormat::EMPv2 );
  auto nChannels = inputs.size();
  std::cout << "compareStage2TowerFWvsEmul::Filename : "<< fname <<", Board data name : " << inputs.name() << ", and number of channels are: " << nChannels  << std::endl;

  for ( const auto& channel : inputs ) {
    //std::cout << "Data on channel : " << channel.first << std::endl;
    unsigned iFrame = 0;
    bool isFirstValidEvent = false;
    bool startStore = false;
    std::vector<uint64_t> words ; 
    for ( const auto& frame : channel.second ) {
      //std::cout << frame.startOfOrbit << frame.startOfPacket << frame.endOfPacket << frame.valid << " " << std::hex << frame.data << std::dec << std::endl;
      if(!isFirstValidEvent){
	if(frame.startOfPacket) {
	  startStore = true;
	  //std::cout << "SoP : 0x" << std::hex << frame.data << std::dec << std::endl;
	  words.clear();
	}
	if(startStore and frame.valid) words.push_back(frame.data);
	if(startStore and frame.endOfPacket) {
	  isFirstValidEvent = true;
	  linkwords[channel.first] = words;
	}//endofpacket
	//std::cout << channel.first << ", frame : " << iFrame << "...hello there.." << std::endl;
	iFrame++;      
      }//firstevent
      //iFrame++;      
    }//loop over frames of a given channel
    
  }//loop over channels

}
