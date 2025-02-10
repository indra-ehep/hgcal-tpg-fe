/**********************************************************************
 Created on : 03/02/2025
 Purpose    : Access the TPG Stage2 configuration
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
#ifndef TPGStage2Configuration_h
#define TPGStage2Configuration_h

#include "yaml-cpp/yaml.h"
#include <cassert>

namespace TPGStage2Configuration{
  class Stage2Board{
  public:
    Stage2Board() : tmuxIndex(0), boardId(0), scaleFactor(0x1000), pageOffset(1), counterOffset(0) {
      thresh[0] = 0x10000;
      thresh[1] = 0x09000;
      thresh[2] = 0x08000;
      thresh[3] = 0x07000;
      thresh[4] = 0x06000;
      thresh[5] = 0x05000;
      thresh[6] = 0x04000;
    }
    void readConfigYaml(const char* inputfile);
    
    uint16_t getTmuxIndex() const {return uint16_t(tmuxIndex);}
    uint16_t getBoardId() const {return boardId;}
    uint64_t getThresh(int i) const { assert(i>=0 and i<=6); return thresh[i];}
    uint64_t getScaleFactor() const {return scaleFactor;}
    uint16_t getPageOffset() const {return pageOffset;}
    uint16_t getCounterOffset() const {return counterOffset;}

    void print() {
      std::cout << "TPGStage2Configuration::Stage2Board TmIndex: " << getTmuxIndex() << std::endl
		<< "TPGStage2Configuration::Stage2Board BoardId: " << getBoardId() << std::endl
		<< "TPGStage2Configuration::Stage2Board ScaleFactor: " << getScaleFactor() << std::endl;
      for(int ithr=0; ithr<=6; ithr++){
	std::cout << "TPGStage2Configuration::Stage2Board Threshold"<< ithr <<": " << getThresh(ithr) << std::endl;
      }
    }
    void setTmuxIndex(uint8_t itmux) { tmuxIndex = itmux;}
    void setBoardId(uint16_t bId) { boardId = bId;}
    void setThresh(int i, uint64_t thr) { assert(i>=0 and i<=6); thresh[i] = thr;}
    void setScaleFactor(uint64_t sf) { scaleFactor = sf;}
    void setPageOffset(uint16_t poffset) { pageOffset = poffset;}
    void setCounterOffset(uint16_t coffset) { counterOffset = coffset;}

  private:
    uint8_t tmuxIndex;
    uint16_t boardId;
    uint64_t thresh[7];
    uint64_t scaleFactor;
    uint16_t pageOffset;
    uint16_t counterOffset;
  };
  void Stage2Board::readConfigYaml(const char* inputfile){
    
    YAML::Node node(YAML::LoadFile(inputfile));
    if(node["LinkMuxNode"]){
      if(node["LinkMuxNode"]["TmIndex"]) setTmuxIndex(node["LinkMuxNode"]["TmIndex"].as<uint8_t>());
      if(node["LinkMuxNode"]["BoardId"]) setBoardId(node["LinkMuxNode"]["BoardId"].as<uint16_t>());
    }
    if(node["FormatTowersNode"]){
      for(int ithr=0; ithr<=6; ithr++){
	std::string thname = "Threshold" + std::to_string(ithr) ;  
	if(node["FormatTowersNode"][thname]) setThresh(ithr, node["FormatTowersNode"][thname].as<uint64_t>());
      }      
      if(node["FormatTowersNode"]["ScaleFactor"]) setScaleFactor(node["FormatTowersNode"]["ScaleFactor"].as<uint64_t>());
    }
    
  }
}
#endif
