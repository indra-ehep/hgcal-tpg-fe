/**********************************************************************
 Created on : 26/01/2025
 Purpose    : Emulation of stage2 tower processing with EMP file as I/O
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
#include <iostream>
#include "L1Trigger/DemonstratorTools/interface/utilities.h"

#include "TPGBEDataformat.hh"
#include "Stage2.hh"

int main(int argc, char** argv)
{
    // ===============================================
    // Example for reading in data from a pattern file
    // ===============================================
    std::string inputFileName = "EMPStage2Input.txt";
    l1t::demo::BoardData inputs = l1t::demo::read( inputFileName, l1t::demo::FileFormat::EMPv2 );
    auto nChannels = inputs.size();
    const size_t numWordsBx = 162;
    const size_t nofBx = 6;
    std::vector<TPGBEDataformat::Stage1ToStage2Data> vS12[nofBx];
    
    std::cout << "Board data name : " << inputs.name() << ", and number of channels are: " << nChannels  << std::endl;
    for ( const auto& channel : inputs ) {
      //std::cout << "Data on channel : " << channel.first << std::endl;
      unsigned int iFrame = 0;
      int ibx = 0;
      int s2i = 0;
      TPGBEDataformat::Stage1ToStage2Data s2InputBx ; 
      for ( const auto& frame : channel.second ) {
	//std::cout << frame.startOfOrbit << frame.startOfPacket << frame.endOfPacket << frame.valid << " " << std::hex << frame.data << std::dec << std::endl;
	//if(iFrame%numWordsBx==0) s2InputBx = new TPGBEDataformat::Stage1ToStage2Data();
	if(iFrame%3<=1 and frame.valid){
	  s2InputBx.setData(s2i, frame.data);
	  s2i++;
	}
	iFrame++;
	if(iFrame%numWordsBx==0) {
	  vS12[ibx].push_back(s2InputBx);
	  ibx++;
	  s2i = 0;
	  s2InputBx.reset();
	  //delete s2InputBx;
	}
      }//loop over frames of a given channel
      
    }//loop over channels

    std::cout << "Nof links for Bx1 : " << vS12[0].size() << std::endl;
    std::cout << "Nof links for Bx2 : " << vS12[1].size() << std::endl;
    
    TPGStage2Emulation::Stage2 stage2TowerEmul;
    TPGBEDataformat::Stage2ToL1TData s2OutputBx[nofBx] ; 
    for(int ibx=0;ibx<nofBx;ibx++){
      stage2TowerEmul.run(vS12[ibx]);
      for(unsigned eta(0);eta<20;eta++) 
	for(unsigned phi(0);phi<24;phi++) 
	  s2OutputBx[ibx].setTowerLinkData(eta, phi, stage2TowerEmul.getTowerOutput(eta,phi));             
    }//bx loop

    //Write EMP output
    l1t::demo::BoardData boardData("L1TBoard");
  
    // Number of frames
    const size_t l1tnumWordsBx = 30;
  
    // Channels to define
    std::vector<size_t> channels;
    for (int ich = 4 ; ich <= 15 ; ich++) channels.push_back(ich);

    uint64_t emptydata = 0;
    for (size_t channel : channels) {
      // Create a Channel object (vector of Frames)
      l1t::demo::BoardData::Channel channelData;
      int ilink = channel%4;
      for (size_t ibx = 0; ibx < nofBx; ++ibx) {
	for (size_t i = 0; i < l1tnumWordsBx; ++i) {
	  // Create a Frame object and populate it
	  l1t::demo::Frame frame;
	  frame.data = s2OutputBx[ibx].getData(ilink,i);
	  frame.valid = true;
	  frame.startOfOrbit = false;
	  frame.startOfPacket = (i == 0);
	  frame.endOfPacket = (i == (l1tnumWordsBx-1));

	  // Add the frame to the channel data
	  channelData.push_back(frame);       
	}//loop 30 words      
      }//ibx

      // Add the channel to the BoardData object
      boardData.add(channel, channelData);
    }
  
    // Write the BoardData to file in EMP format
    write(boardData, "EMPStage2Output.txt", l1t::demo::FileFormat::EMPv2);

    
    return true;

}