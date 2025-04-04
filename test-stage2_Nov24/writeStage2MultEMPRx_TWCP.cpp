/**********************************************************************
 Created on : 04/04/2025
 Purpose    : Write multiple EMP Rx files with accumulator outputs.
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
  
  void ReadPhysDataCSV(std::vector<std::string>, std::vector<std::array<uint64_t, 17>>&);
  //std::string infile = "input/stage2/EMPinputs/test_2025-03-12/tb_stimuli_for_PC_s180_0_vbf_jet_k_3.csv";
  //std::string infile = "input/stage2/EMPinputs/test_2025-03-12/input_fp.csv";
  std::vector<std::string> infilelist;
  for(int findex=0;findex<6;findex++){
    std::string infile = "input/stage2/EMPinputs/test_2025-03-12/id_tb_stimuli_for_PC_s180_"+ std::to_string(findex) + "_vbf_jet_k_3.csv" ;
    infilelist.push_back(infile);
  }
  std::cout << "infilelist.size() : " << infilelist.size() << std::endl;
  
  std::vector<std::array<uint64_t, 17>> physdata;
  physdata.clear();
  ReadPhysDataCSV(infilelist, physdata);
  std::cout << "physdata.size() : " << physdata.size() << std::endl;
  infilelist.clear();
  
  int ievent = 0;
  // for(const std::array<uint64_t, 17>& data :  physdata){
  //   std::cout << "ievent : " << ievent << std::endl;
  //   for(int idata=0;idata<int(data.size());idata++){
  //     std::cout<< "\t idata: " << idata << ", data : " << std::hex << data[idata] << std::dec << std::endl;
  //   }
  //   ievent++;
  // }
  
  // Number of frames
  const size_t numWordsBx = 162;
  const size_t nofBx = 6;
  const size_t nofValidWordsPerBx = 108;
  const size_t nofValidWordsPerEMP = 648 ; //108*6

  long numberOfEMPs = (long)(std::floor( float(physdata.size()) / float(nofValidWordsPerEMP) ) ) ;
  std::cout << "Number of EMP files to be produced : " << numberOfEMPs << std::endl;
  
  // Channels to define
  std::vector<size_t> channels;
  for (int ich = 16 ; ich <= 111 ; ich++) channels.push_back(ich);

  uint64_t emptydata = 0;
  
  for(long iemp=0; iemp<numberOfEMPs; iemp++){
    
    l1t::demo::BoardData boardData("Stage2Board");
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

    std::string foutname = "EMPStage2Input_vbf_" + std::to_string(iemp) + ".txt" ;
    
    // Write the BoardData to file in EMP format
    write(boardData, foutname, l1t::demo::FileFormat::EMPv2);
  }//EMP loop
  
  physdata.clear();

  return true;
}

void ReadPhysDataCSV(std::vector<std::string> inputfilelist, std::vector<std::array<uint64_t, 17>>& physdata){

  for(unsigned findex=0;findex<inputfilelist.size();findex++){
    std::string inputfile = inputfilelist.at(findex);
    std::ifstream fin(inputfile);
    std::string s;
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
  
}
