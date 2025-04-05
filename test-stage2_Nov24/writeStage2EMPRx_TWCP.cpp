/**********************************************************************
 Created on : 12/03/2025
 Purpose    : Pepare EMP Rx files for fake tower and Physics CP inputs
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include "L1Trigger/DemonstratorTools/interface/utilities.h"

#include "TPGBEDataformat.hh"

int main(int argc, char** argv)
{
  
  void ReadPhysDataCSV(std::string, std::vector<std::array<uint64_t, 17>>&);
  //std::string infile = "input/stage2/EMPinputs/test_2025-03-12/tb_stimuli_for_PC_s180_0_vbf_jet_k_3.csv";
  std::string infile = "input/stage2/EMPinputs/test_2025-03-12/input_fp.csv";
  std::vector<std::array<uint64_t, 17>> physdata;  
  ReadPhysDataCSV(infile, physdata);
  std::cout << "physdata.size() : " << physdata.size() << std::endl;
  int ievent = 0;
  for(const std::array<uint64_t, 17>& data :  physdata){
    std::cout << "ievent : " << ievent << std::endl;
    for(int idata=0;idata<int(data.size());idata++){
      std::cout<< "\t idata: " << idata << ", data : " << std::hex << data[idata] << std::dec << std::endl;
    }
    ievent++;
  }
  
  l1t::demo::BoardData boardData("Stage2Board");
  
  // Number of frames
  const size_t numWordsBx = 162;
  const size_t nofBx = 6;

  // Channels to define
  std::vector<size_t> channels;
  for (int ich = 16 ; ich <= 111 ; ich++) channels.push_back(ich);

  uint64_t emptydata = 0;
  for (size_t channel : channels) {
    // Create a Channel object (vector of Frames)
    l1t::demo::BoardData::Channel channelData;
    ievent = 0;
    for (size_t ibx = 0; ibx < nofBx; ++ibx) {
      TPGBEDataformat::Stage1ToStage2Data s2InputBx;
      int s2i = 0;
      int s2i2 = 0;
      for (size_t i = 0; i < numWordsBx; ++i) {
  	if(i<=(numWordsBx-2) and (i+1)%3!=0){
  	  s2InputBx.setBit(s2i,60);
  	  for(int itc=0;itc<3;itc++) s2InputBx.setTC(s2i, itc, (3*i+itc)%48);
  	  //for(int iptt=0;iptt<2;iptt++) s2InputBx.setPTT(s2i, iptt, 0);
  	  s2InputBx.setPTT(s2i, 0, s2i2%256); //CEH
  	  s2InputBx.setPTT(s2i, 1, s2i); //CEE
	  int ich = channel - 16;
	  if(ich<17){
	    s2InputBx.replaceTCs(s2i, physdata.at(ievent).at(ich));
	  }
  	}
  	// Create a Frame object and populate it
  	l1t::demo::Frame frame;
  	frame.data = (i%3<=1) ? s2InputBx.getData(s2i) : emptydata;
  	frame.valid = ((i==(numWordsBx-1) or (i+1)%3==0) and i!=0)?false:true;
  	//frame.startOfOrbit = (i == 0);
  	frame.startOfOrbit = false;
  	frame.startOfPacket = (i == 0);
  	frame.endOfPacket = (i == (numWordsBx-1));
	
  	// Add the frame to the channel data
  	channelData.push_back(frame);
	
  	if(i%3<=1)s2i++;
  	if(i%3<=1)
  	  s2i2 += 2;
	if(i%3<=1) ievent++;
      }
      
    }//ibx

    // Add the channel to the BoardData object
    boardData.add(channel, channelData);
  }
  
  // Write the BoardData to file in EMP format
  write(boardData, "EMPStage2Input.txt", l1t::demo::FileFormat::EMPv2);
  
}

void ReadPhysDataCSV(std::string inputfile, std::vector<std::array<uint64_t, 17>>& physdata){
  
    std::ifstream fin(inputfile);
    std::string s;
    physdata.clear();
    std::vector<std::string> vsubstr;
    char* end = nullptr;
    int ievent = 0;
    while (std::getline(fin, s)){
      if(s.find("N_TC")!=std::string::npos) continue;
      std::istringstream ss(s);
      std::string substr;
      vsubstr.clear();
      while (std::getline(ss, substr, ',')) vsubstr.push_back(substr);
      if(vsubstr.size()<19) continue;
      // for(int idata=0;idata<int(vsubstr.size());idata++){
      // 	std::cout<< "\t idata: " << idata << ", data : " << std::hex << std::strtoull(vsubstr.at(idata).c_str(),&end,16) << std::dec << std::endl;
      // }
      std::array<uint64_t, 17> data;
      data.fill(0);
      for(int idata=0;idata<17;idata++) data[idata] = std::strtoull(vsubstr.at(idata+2).c_str(),&end,16);
      physdata.push_back(data);
      ievent++;
    }
    fin.close();
  
}
