/**********************************************************************
 Created on : 12/03/2025
 Purpose    : Compare FW and emulation results
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/


#include <iostream>
#include "L1Trigger/DemonstratorTools/interface/utilities.h"


int main()
{
  void ReadEMPData(std::string fname, std::map<uint32_t,std::vector<uint64_t>>&, int offset);
  std::string inputEmulFileName = "EMPStage2Output.txt";
  //std::string inputFWFileName = "input/stage2/firmware-data/CaptureStage2_250214_1137/tx_summary.txt";
  std::string inputFWFileName = "input/stage2/firmware-data/CaptureStage2_250314_1218/tx_summary.txt";
  std::map<uint32_t,std::vector<uint64_t>> emuldata, fwdata;
  int emulframeoffset = 0, fwframeoffset = 199;
  ReadEMPData(inputEmulFileName,emuldata,emulframeoffset);
  ReadEMPData(inputFWFileName,fwdata,fwframeoffset);
  std::cout << "emuldata.size : " << emuldata.size() << std::endl;
  std::cout << "fwdata.size : " << fwdata.size() << std::endl;
  for(const auto& emd: emuldata){
    int iw = 0;
    std::cout  << "second size : " << emd.second.size() << std::endl;
    for(const auto& ed: emd.second){
      if(iw<(118-20)){
	uint64_t XOR = ed xor fwdata[emd.first].at(iw+20) ;
	std::cout << "link: " << std::setfill('0') << std::setw(2) << emd.first
		  << ", iw: " << std::setfill('0') << std::setw(2) << iw
		  << ", emuldata : 0x"  << std::setw(16) << std::hex << ed
		  << ", fwdata: 0x" << fwdata[emd.first].at(iw+20) 
		  << ", diff(emuldata - fwdata): 0x" << (ed-fwdata[emd.first].at(iw+20))
		  << ", (emuldata XOR fwdata): 0x" << XOR
		  << std::dec << std::endl;
      }
      iw++;
    }
    std::cout << std::endl;
  }
  return true;
}

void ReadEMPData(std::string fname, std::map<uint32_t,std::vector<uint64_t>>& linkwords, int offset)
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
