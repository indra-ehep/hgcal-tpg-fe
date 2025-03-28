/**********************************************************************
 Created on : 19/03/2025
 Purpose    : Read the input/output to/from Python emulator and use to compare the C++ emulator output
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
#include "Stage2.hh"
#include "TPGStage2Configuration.hh"

int main(int argc, char** argv)
{
  
  //============================================
  //Read Emulation input to the Python
  void ReadPhysDataCSV(std::string, std::vector<TPGBEDataformat::TcAccumulatorFW>&);
  std::string pyemulinfile = "input/stage2/EMPinputs/test_2025-03-12/tb_stimuli_for_PC_s180_0_vbf_jet_k_3.csv";
  std::vector<TPGBEDataformat::TcAccumulatorFW> physdata;  
  ReadPhysDataCSV(pyemulinfile, physdata);
  std::cout << "physdata.size() : " << physdata.size() << std::endl;
  //============================================
  
  //============================================
  //Read Emulation results by Python
  void ReadPyEmulOut(std::string, std::vector<l1thgcfirmware::HGCalCluster_HW>&);
  std::string pyemuloutfile = "input/stage2/EMPinputs/test_2025-03-12/tb_stimuli_for_PC_s180_0_vbf_jet_k_3_out.csv";
  std::vector<l1thgcfirmware::HGCalCluster_HW> pyemulout;  
  ReadPyEmulOut(pyemuloutfile, pyemulout);
  std::cout << "emulout.size() : " << pyemulout.size() << std::endl;
  //===========================================
  
  //============================================
  //Initialize Cpp emulation
  TPGStage2Emulation::Stage2 s2Clustering;
  //Set LUTs
  TPGStage2Configuration::ClusPropLUT cplut;
  cplut.readMuEtaLUT("input/stage2/configuration/mean_eta_LUT.csv");
  cplut.readSigmaEtaLUT("input/stage2/configuration/sigma_eta_LUT.csv");
  s2Clustering.setClusPropLUT(&cplut);
  l1thgcfirmware::HGCalCluster_HW L1TOutputEmul;
  uint64_t nof1stMM = 0, nof2ndMM = 0, nof3rdMM = 0;
  uint64_t nofEvents = 0;
  uint64_t checkevents[10] = {1806, 3402, 3562, 3899, 4111, 5152, 5401, 8199, 8346, 10556};
  std::vector<uint64_t> eventlist;
  for(int iev=0;iev<10;iev++) eventlist.push_back(checkevents[iev]);
  std::cout << "Check for total nevents : " << eventlist.size() << std::endl ;
  
  for(uint64_t ievent=0;ievent<physdata.size();ievent++){
    //std::cout << "Processing Event : " << ievent << std::endl;

    if ( std::find(eventlist.begin(), eventlist.end(), ievent) == eventlist.end() ) continue;
    std::cout << std::dec << "========= Processing Event : "<< ievent << " ============" << std::endl;
    L1TOutputEmul.clear();
    // C++ CP Emulation ===============================================
    //s2Clustering.ClusterProperties(physdata.at(ievent), L1TOutputEmul,true);
    // std::cout <<"float size : " << sizeof(float) << std::endl;
    // std::cout <<"double size : " << sizeof(double) << std::endl;
    s2Clustering.ClusterProperties(physdata.at(ievent), L1TOutputEmul);
    //==================================================================
    
    if(L1TOutputEmul.pack_firstWord()!=pyemulout.at(ievent).pack_firstWord()){
      //std::cout << "Mismatch in first word for event : " << ievent << std::endl;
      nof1stMM++;
    }
    if(L1TOutputEmul.pack_secondWord()!=pyemulout.at(ievent).pack_secondWord()){
      uint64_t XOR = L1TOutputEmul.pack_secondWord() xor pyemulout.at(ievent).pack_secondWord() ;
      std::cout << std::dec
      		<< "Mismatch in second word for event : " << ievent
      		<< std::hex
      		<< ", C++: " << L1TOutputEmul.pack_secondWord()
      		<< ", Python: " << pyemulout.at(ievent).pack_secondWord()
      		<< ", XOR: 0x"
      		<< std::setw(16) << std::setfill('0')
      		<< XOR
      		<< std::setw(4) << std::setfill(' ')
      		<< std::endl;
      
      std::cout << "========= Python Result ============" << std::endl;
      pyemulout.at(ievent).print();
      std::cout << "===================================" << std::endl;
      std::cout << "========= C++ Result ============" << std::endl;
      L1TOutputEmul.print();
      std::cout << "===================================" << std::endl;

      nof2ndMM++;
    }
    if(L1TOutputEmul.pack_thirdWord()!=pyemulout.at(ievent).pack_thirdWord()){
      
      uint64_t XOR = L1TOutputEmul.pack_thirdWord() xor pyemulout.at(ievent).pack_thirdWord() ;
      std::bitset<64> XOR_bitset = XOR;
      std::cout << std::dec
      		<< "Mismatch in third word for event : " << ievent
      		<< std::hex
      		<< ", C++: " << L1TOutputEmul.pack_thirdWord()
      		<< ", Python: " << pyemulout.at(ievent).pack_thirdWord()
      		<< ", XOR: 0x"
      		<< std::setw(16) << std::setfill('0')
      		<< XOR
      		<< std::setw(4) << std::setfill(' ')
      		<< std::endl;
      
      std::cout << "========= Python Result ============" << std::endl;
      pyemulout.at(ievent).print();
      std::cout << "===================================" << std::endl;
      std::cout << "========= C++ Result ============" << std::endl;
      L1TOutputEmul.print();
      std::cout << "===================================" << std::endl;
      
      nof3rdMM++;
    }
    nofEvents++;
  }//event loop
  std::cout << "Total number of events processed: " << std::dec << nofEvents << std::endl;
  std::cout << "Total number of mismatches in the first word : " << nof1stMM
	    <<", second word: " << nof2ndMM 
            << ", third word: " << nof3rdMM
            << std::endl;
  
  physdata.clear();
  pyemulout.clear();


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
      //event_ID, cluster_ID
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



void ReadPhysDataCSV(std::string inputfile, std::vector<TPGBEDataformat::TcAccumulatorFW>& physdata){
  
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
      //event_ID, cluster_ID,
      //N_TC,     E,      E_EM,  E_EM_core, E_H_early,
      //W,        N_TC_W, W2,    Wz,        Wphi,
      //Wroz,     Wz2,    Wphi2, Wroz2,     LayerBits,
      //Sat_TC,   ShapeQ
      //17,       0,
      //00000000000000d3,000000000000e025,000000000000a2c6,00000000000065f4,00000000000025c9,
      //0000000000001bbe,00000000000000d3,00000000000b293c,000000000049aca0,00000000008a6c48,
      //00000000017911f5,0000000146632bbc,00000002b3487988,0000001409642eab,00000003fff7f7c0,
      //0000000000000000,0000000000000001

      uint64_t eventId = std::strtoull(vsubstr.at(0).c_str(),&end,16) ;
      uint64_t clusterId = std::strtoull(vsubstr.at(1).c_str(),&end,16) ;
      TPGBEDataformat::TcAccumulatorFW accmulInput(3);
      for(int idata=2;idata<int(vsubstr.size());idata++){
	uint64_t data = std::strtoull(vsubstr.at(idata).c_str(),&end,16) ;
	// if(ievent==8501){
	//   std::cout<< "ievent : "<< ievent << ", eventId: "<< vsubstr.at(0).c_str() << ", clusterId: " << vsubstr.at(1).c_str() <<  "| idata: " << idata <<", data: " << data << std::endl;
	// }
	switch(idata){
	case 2:
	  accmulInput.setNumberOfTcs(data);  break;
	case 3:	  
	  accmulInput.setTotE(data);         break;
	case 4:
	  accmulInput.setCeeE(data);         break;
	case 5:
	  accmulInput.setCeeECore(data);     break;
	case 6:
	  accmulInput.setCeHEarly(data);     break;
	case 7:
	  accmulInput.setSumW(data);         break;
	case 8:
	  accmulInput.setNumberOfTcsW(data); break;
	case 9:
	  accmulInput.setSumW2(data);        break;
	case 10:
	  accmulInput.setSumWZ(data);        break;
	case 11:
	  accmulInput.setSumWPhi(data);      break;
	case 12:
	  accmulInput.setSumWRoZ(data);      break;
	case 13:
	  accmulInput.setSumWZ2(data);       break;
	case 14:
	  accmulInput.setSumWPhi2(data);     break;
	case 15:
	  accmulInput.setSumWRoZ2(data);     break;
	case 16:
	  accmulInput.setLayerBits(data);    break;
	case 17:
	  accmulInput.setsatTC(data);        break;
	case 18:
	  accmulInput.setshapeQ(data);       break;
	}
      }
      // if(ievent==8501)
      // 	accmulInput.printdetail(0);
      physdata.push_back(accmulInput);
      ievent++;
    }
    fin.close();
  
}
