/**********************************************************************
 Created on : 16/03/2025
 Purpose    : Read the Python Emulator output, then pack them in L1T words
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include "L1Trigger/DemonstratorTools/interface/utilities.h"
#include "DataFormats/L1THGCal/interface/HGCalCluster_HW.h"

#include "TPGBEDataformat.hh"


int main(int argc, char** argv)
{
  
  l1thgcfirmware::HGCalCluster_HW L1TOutputPyEmul;
  l1thgcfirmware::HGCalCluster_HW L1TOutputFW;

  //============================================
  //Read Emulation results by Python
  void ReadPyEmulOut(std::string, std::vector<l1thgcfirmware::HGCalCluster_HW>&);
  std::string infile = "input/stage2/EMPinputs/test_2025-03-12/tb_stimuli_for_PC_s180_0_vbf_jet_k_3_out.csv";
  std::vector<l1thgcfirmware::HGCalCluster_HW> emulout;  
  ReadPyEmulOut(infile, emulout);
  std::cout << "emulout.size() : " << emulout.size() << std::endl;
  //===========================================

  //===========================================
  //Read EMP file from FW
  void ReadEMPData(std::string fname, std::map<uint32_t,std::vector<uint64_t>>&);
  std::string inputFWFileName = "input/stage2/firmware-data/CaptureStage2_250314_1218/tx_summary.txt";
  std::map<uint32_t,std::vector<uint64_t>> fwdata;
  ReadEMPData(inputFWFileName,fwdata);
  std::cout << "fwdata.size : " << fwdata.size() << std::endl;
  //===========================================
  
  int ievent = 0;
  for(const l1thgcfirmware::HGCalCluster_HW& l1tdata :  emulout){
    if(ievent>66) continue;
    uint64_t xor_firstword = l1tdata.pack_firstWord() xor fwdata[4][ievent+51];
    std::cout << "ievent: " << ievent
	      << std::hex
	      << ", 1st word: " << l1tdata.pack_firstWord()
              << ", link word1 0x" << fwdata[4][ievent+51]
	      << ", XOR : " << xor_firstword
	      << std::dec
	      << std::endl;
    uint64_t xor_secondword = l1tdata.pack_secondWord() xor fwdata[5][ievent+51];
    std::cout << "ievent: " << ievent
	      << std::hex
	      << ", 2nd word: " << l1tdata.pack_secondWord()
              << ", link word1 0x" << fwdata[5][ievent+51]
	      << ", XOR : " << xor_secondword
	      << std::dec
	      << std::endl;
    uint64_t xor_thirdword = l1tdata.pack_thirdWord() xor fwdata[6][ievent+51];
    std::cout << "ievent: " << ievent
	      << std::hex
	      << ", 3nd word: " << l1tdata.pack_thirdWord()
              << ", link word1 0x" << fwdata[6][ievent+51]
	      << ", XOR : " << xor_thirdword
	      << std::dec
	      << std::endl;
    ievent++;
  }
  
  return true;
}

void ReadPyEmulOut(std::string inputfile, std::vector<l1thgcfirmware::HGCalCluster_HW>& emulout){

    
    std::ifstream fin(inputfile);
    std::string s;
    emulout.clear();
    std::vector<std::string> vsubstr;
    char* end = nullptr;
    int ievent = 0;
    while (std::getline(fin, s)){
      if(s.find("event_ID")!=std::string::npos) continue;
      std::istringstream ss(s);
      std::string substr;
      vsubstr.clear();
      while (std::getline(ss, substr, ',') and substr.find("[")==std::string::npos){
	if(substr.find("[")==std::string::npos)
	  vsubstr.push_back(substr);
      }
      
      //std::cout<< "ievent: " << ievent << ", data size : " << int(vsubstr.size()) << std::endl;

      //================================================================================
      //The Python output arranges the emulation output in following order
      //================================================================================
      //Eventid, clusterId,
      //ET, e_or_gamma_ET, GCT_e_or_gamma_Select_0, GCT_e_or_gamma_Select_1, GCT_e_or_gamma_Select_2, GCT_e_or_gamma_Select_3,
      //Fraction_in_CE_E, Fraction_in_core_CE_E, Fraction_in_front_CE_H,
      //First_Layer, ET_Weighted_Eta, ET_Weighted_Phi, ET_Weighted_Z, Number_of_TCs
      //Saturated_Trigger_Cell, Quality_of_Fraction_in_CE_E, Quality_of_Fraction_in_core_CE_E, Quality_of_Fraction_in_front_CE_H, Quality_of_Sigmas_and_Means, Saturated_Phi, Nominal_Phi,
      //Sigma_E, Last_Layer, Shower_Length,
      //Sigma_ZZ, Sigma_PhiPhi, Core_Shower_Length, Sigma_EtaEta, Sigma_RoZRoZ,
      //LayerBits
      //================================================================================
      //1,0
      //224, 163, 1, 1, 0, 1,
      //186.0, 160.0, 43.0,
      //1, 485, -94, 680, 211,
      //0, 1, 1, 1, 1, 0, 1
      //0, 28, 28
      //46, 34, 14, 5, 37,
      //"[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0]"
      //================================================================================
      
      if(vsubstr.size()<31) continue;
      l1thgcfirmware::HGCalCluster_HW L1TOutputPyEmul;
      for(int idata=2;idata<int(vsubstr.size());idata++){
	//int data = std::strtoi(vsubstr.at(idata).c_str(),&end,10) ;
	int data = atoi(vsubstr.at(idata).c_str()) ;
      	//std::cout<< " idata: " << idata <<", data: " << data << std::endl; 
	switch(idata){
	case 2:
	  L1TOutputPyEmul.e = data; 	          break;
	case 3:
	  L1TOutputPyEmul.e_em = data;            break;
	case 4:
	  L1TOutputPyEmul.gctBits[0] = data; 	  break;
	case 5:
	  L1TOutputPyEmul.gctBits[1] = data; 	  break;
	case 6:
	  L1TOutputPyEmul.gctBits[2] = data; 	  break;
	case 7:
	  L1TOutputPyEmul.gctBits[3] = data; 	  break;
	case 8:
	  L1TOutputPyEmul.fractionInCE_E = data;  break;
	case 9:
	  L1TOutputPyEmul.fractionInCoreCE_E = data;  	  break;
	case 10:
	  L1TOutputPyEmul.fractionInEarlyCE_E = data; 	  break;
	case 11:
	  L1TOutputPyEmul.firstLayer = data; 	  break;
	case 12:
	  L1TOutputPyEmul.w_eta = data;  	  break;
	case 13:
	  L1TOutputPyEmul.w_phi = data; 	  break;
	case 14:
	  L1TOutputPyEmul.w_z = data; 	          break;
	case 15:
	  L1TOutputPyEmul.nTC = data;      	  break;
	case 16:
	  L1TOutputPyEmul.qualFlags[0] = data;    break;
	case 17:
	  L1TOutputPyEmul.qualFlags[1] = data; 	  break;
	case 18:
	  L1TOutputPyEmul.qualFlags[2] = data;    break;
	case 19:
	  L1TOutputPyEmul.qualFlags[3] = data;    break;
	case 20:
	  L1TOutputPyEmul.qualFlags[4] = data;    break;
	case 21:
	  L1TOutputPyEmul.qualFlags[5] = data;    break;
	case 22:
	  L1TOutputPyEmul.qualFlags[6] = data;    break;
	case 23:
	  L1TOutputPyEmul.sigma_E = data;      	  break;
	case 24:
	  L1TOutputPyEmul.lastLayer = data;       break;
	case 25:
	  L1TOutputPyEmul.showerLength = data;    break;
	case 26:
	  L1TOutputPyEmul.sigma_z = data;         break;
	case 27:
	  L1TOutputPyEmul.sigma_phi = data;       break;
	case 28:
	  L1TOutputPyEmul.coreShowerLength = data;       break;
	case 29:
	  L1TOutputPyEmul.sigma_eta = data;       break;
	case 30:
	  L1TOutputPyEmul.sigma_roz = data;       break;
	}
      }
      //L1TOutputPyEmul.print();
      emulout.push_back(L1TOutputPyEmul);      
      ievent++;
    }
    fin.close();
  
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
	iFrame++;      
      }//firstevent
      //iFrame++;      
    }//loop over frames of a given channel
    
  }//loop over channels

}
