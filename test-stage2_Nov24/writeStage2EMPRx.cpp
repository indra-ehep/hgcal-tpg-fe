/**********************************************************************
 Created on : 24/01/2025
 Purpose    : Write the EMP RX file for Stage2 input
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
#include <iostream>
#include "L1Trigger/DemonstratorTools/interface/utilities.h"

#include "TPGBEDataformat.hh"


int main(int argc, char** argv)
{
    // // ===============================================
    // // Example for reading in data from a pattern file
    // // ===============================================
    // std::string inputFileName = "EMPTools/testInput.txt";
    // l1t::demo::BoardData inputs = l1t::demo::read( inputFileName, l1t::demo::FileFormat::EMPv2 );
    // auto nChannels = inputs.size();
    // std::cout << "Board data name : " << inputs.name() << std::endl;
    // for ( const auto& channel : inputs ) {
    //     std::cout << "Data on channel : " << channel.first << std::endl;
    //     unsigned int iFrame = 0;
    //     for ( const auto& frame : channel.second ) {
    // 	  std::cout << frame.startOfOrbit << frame.startOfPacket << frame.endOfPacket << frame.valid << " " << std::hex << frame.data << std::dec << std::endl;
    //         ++iFrame;
    //         if (iFrame > 5 ) break;
    //     }
    // }
  
    // ==============================================
    // Example for writing out data to a pattern file
    // ==============================================
  l1t::demo::BoardData boardData("ExampleBoard");

  // Number of frames
  const size_t numWordsBx = 11;
  const size_t numFrames = 11;

  // Channels to define
  std::vector<size_t> channels = {4, 5, 6, 7};

  for (size_t channel : channels) {
    // Create a Channel object (vector of Frames)
    l1t::demo::BoardData::Channel channelData;
    
    TPGBEDataformat::TestStage2Input s2InputBx;
    for (size_t i = 0; i < numWordsBx; ++i) {
      if(i>0 and i<=(numWordsBx-2) and i%3!=0){
	for(int itc=0;itc<3;itc++) s2InputBx.setTC(i, itc, 3*i+itc);
	for(int iptt=0;iptt<2;iptt++) s2InputBx.setPTT(i, iptt, 0);
      }else if(i==(numWordsBx-1)){
	for(int itc=0;itc<3;itc++) s2InputBx.setTC(i, itc, 0xffff);
	for(int iptt=0;iptt<2;iptt++) s2InputBx.setPTT(i, iptt, 0xff);	
      }
      // Create a Frame object and populate it
      l1t::demo::Frame frame;
      frame.data = s2InputBx.getData(i);
      frame.valid = ((i==(numWordsBx-1) or i%3==0) and i!=0)?false:true;
      frame.startOfOrbit = (i == 0);
      frame.endOfPacket = (i == (numWordsBx-2));

      // Add the frame to the channel data
      channelData.push_back(frame);
    }

    // Add the channel to the BoardData object
    boardData.add(channel, channelData);
  }
  
  // Write the BoardData to file in EMP format
  write(boardData, "EMPStage2Input.txt", l1t::demo::FileFormat::EMPv2);
  
}
