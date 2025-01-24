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
  
  // ==============================================
  // Example for writing out data to a pattern file
  // ==============================================
  l1t::demo::BoardData boardData("Stage2Board");
  
  // Number of frames
  const size_t numWordsBx = 162;
  const size_t nofBx = 6;
  
  // Channels to define
  std::vector<size_t> channels;
  for (int ich = 16 ; ich <= 111 ; ich++) channels.push_back(ich);
  
  for (size_t channel : channels) {
    // Create a Channel object (vector of Frames)
    l1t::demo::BoardData::Channel channelData;
    
    for (size_t ibx = 0; ibx < nofBx; ++ibx) {
      TPGBEDataformat::TestStage2Input s2InputBx;
      for (size_t i = 0; i < numWordsBx; ++i) {
	if(i<=(numWordsBx-2) and (i+1)%3!=0){
	  s2InputBx.setBit(i,60);
	  for(int itc=0;itc<3;itc++) s2InputBx.setTC(i, itc, (3*i+itc)%48);
	  for(int iptt=0;iptt<2;iptt++) s2InputBx.setPTT(i, iptt, 0);
	}
	// Create a Frame object and populate it
	l1t::demo::Frame frame;
	frame.data = s2InputBx.getData(i);
	frame.valid = ((i==(numWordsBx-1) or (i+1)%3==0) and i!=0)?false:true;
	//frame.startOfOrbit = (i == 0);
	frame.startOfOrbit = false;
	frame.startOfPacket = (i == 0);
	frame.endOfPacket = (i == (numWordsBx-1));

	// Add the frame to the channel data
	channelData.push_back(frame);
      }
    }//ibx

    // Add the channel to the BoardData object
    boardData.add(channel, channelData);
  }
  
  // Write the BoardData to file in EMP format
  write(boardData, "EMPStage2Input.txt", l1t::demo::FileFormat::EMPv2);
  
}