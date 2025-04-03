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
  //std::string inputFWFileName = "input/stage2/firmware-data/CaptureStage2_250314_1218/tx_summary.txt";
  std::string inputFWFileName = "input/stage2/firmware-data/CaptureStage2_250321_1305/tx_summary.txt";
  std::map<uint32_t,std::vector<uint64_t>> emuldata, fwdata;
  int emulframeoffset = 0, fwframeoffset = 199;
  ReadEMPData(inputEmulFileName,emuldata,emulframeoffset);
  ReadEMPData(inputFWFileName,fwdata,fwframeoffset);
  std::cout << "emuldata.size : " << emuldata.size() << std::endl;
  std::cout << "fwdata.size : " << fwdata.size() << std::endl;
  int offsetindex = 66;
  for(const auto& emd: emuldata){
    int iw = 0;
    for(const auto& ed: emd.second){
      if(iw<(118-offsetindex)){
  	uint64_t XOR = ed xor fwdata[emd.first].at(iw+offsetindex) ;
  	std::cout << std::dec
  		  << "link: " << emd.first
  		  << ", iw: " << iw
  		  << std::hex
  		  << ", emuldata : 0x"  
  		  << std::setw(16) << std::setfill('0')
  		  << ed
  		  << ", fwdata: 0x"
  		  << std::setw(16) << std::setfill('0')
  		  << fwdata[emd.first].at(iw+offsetindex) 
  		  // << ", diff(emuldata - fwdata): 0x"
  		  // << std::setw(16) << std::setfill('0')
  		  // << (ed-fwdata[emd.first].at(iw+offsetindex))
  		  << ", (emuldata XOR fwdata): 0x"
  		  << std::setw(16) << std::setfill('0')
  		  << XOR
  		  << std::setw(4) << std::setfill(' ')
  		  << std::endl;
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
