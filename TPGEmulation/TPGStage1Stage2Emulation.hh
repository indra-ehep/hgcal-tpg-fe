/**********************************************************************
 Created on : 29/01/2025
 Purpose    : Combined emulation of Stage1 and Stage2
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
#ifndef TPGStage1Stage2Emulation_h
#define TPGStage1Stage2Emulation_h

/////////////// Stage1 TC processor includes ///////////
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <random>

#include "TPGFEModuleEmulation.hh"
#include "Stage1IOFwCfg.hh"
#include "Stage1IO.hh"
#include "HGCalTriggerCell_SA.h"
#include "HGCalLayer1PhiOrderFwImpl.h"
#include "HGCalLayer1PhiOrderFwConfig.h"
#include "L1Trigger/DemonstratorTools/interface/utilities.h"
#include "TPGFEDataformat.hh"
#include "TPGBEDataformat.hh"
#include "Stage1IO.hh"
#include "TFile.h"
#include "TTree.h"
///////////////////////////////////////////////////////

/////////////// Stage1 Tower emulation includes ///////
#include "TowerSums.h"
#include "Utilities.h"

#include <filesystem>
#include <iostream>
#include <fstream>

// #include "L1Trigger/DemonstratorTools/interface/utilities.h"

#include "TPGFEConfiguration.hh"
// #include "TPGFEDataformat.hh"
// #include "TPGBEDataformat.hh"
// #include "Stage1IO.hh"

#include "yaml-cpp/yaml.h"
///////////////////////////////////////////////////////

/////////////// Stage2 Tower processor includes ///////
// #include <iostream>
// #include "L1Trigger/DemonstratorTools/interface/utilities.h"

// #include "TPGBEDataformat.hh"
#include "Stage2.hh"
///////////////////////////////////////////////////////


/////////////// Mapping includes //////////////////////
///////////////////////////////////////////////////////

namespace TPGStage1Stage2Emulation{
  
  class InputOutput{
  public:
    InputOutput() : inputmode(0) {}
    void SetStage1InputEMP(const char* inputfile){ inputEMPFile = inputfile; inputmode = 1; }
    void SetStage1InputRelay(const char* inputfile){ inputRelayFile = inputfile; inputmode = 2; }
    void SetStage1InputROOT(const char* inputfile){ inputROOTFile = inputfile; inputmode = 3; }
    void Stage1InputInfo(){
      switch(inputmode){
      case 1:
	std::cout<<"Input EMP file name is : " << inputEMPFile <<std::endl;
	break;
      case 2:
	std::cout<<"Input Relay file name is : " << inputRelayFile <<std::endl;
	break;
      case 3:
	std::cout<<"Input ROOT file name is : " << inputROOTFile <<std::endl;
	break;
      default:
	std::cout<<"No input file has been set." <<std::endl;
      }
    }
    void GetInputs(); //Read Inputs for one event and for 60 lpGBT pairs
    void GetEMPInputs() {;}
    void GetRelayInputs() {;}
    void GetROOTInputs() {;}
  private:
    uint16_t inputmode; //0:undefined, 1:EMP, 2:Relays, 3:ROOT
    std::string inputEMPFile;
    std::string inputRelayFile;
    std::string inputROOTFile;
  };
  
  void InputOutput::GetInputs(){
    switch(inputmode){
    case 1:
      GetEMPInputs();
      break;
    case 2:
      GetRelayInputs();
      break;
    case 3:
      GetROOTInputs();
      break;
    default:
      std::cout<<"No input mode has been set." <<std::endl;
    }    
  }
    
  class CombinedEmulation{
    
  };
  
}
#endif
