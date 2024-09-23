#ifndef __HGCalLayer1PhiOrderFwConfig_h__
#define __HGCalLayer1PhiOrderFwConfig_h__

#include <vector>
#include <cstdint>  // uint32_t
#include <unordered_map>

namespace l1thgcfirmware {

  class HGCalLayer1PhiOrderFwConfig {
  public:
    HGCalLayer1PhiOrderFwConfig() {}

    void setSector120(const unsigned sector) { sector120_ = sector; }
    void setFPGAID(const uint32_t fpga_id) { fpga_id_ = fpga_id; }

    /*void
    configureMappingInfo() {  //creates a map between column number and a vector of pairs containing the available (channel,frame) combinations. Each frame accepts 4 TCs, so each (channel,frame) combination appears 4 times in the vector. We could optionally add a unique "slot" index to this vector, but it is not strictly necessary. The function also creates a vector of pairs between column number and number of TCs the column can accept.
      for (unsigned i = 0; i < 20; ++i) {
        //std::pair<unsigned, int> mod_col_pair = std::make_pair(dummyModId_,i);//
        for (unsigned j = 0; j < 2; ++j) {
          for (unsigned k = 0; k < 4; ++k) {  //number of slots per frame
            chn_frame_slots_per_mod_and_col_[dummyModId_][i].push_back(std::make_pair(i + 3, j));
          }
          for (unsigned k = 0; k < 4; ++k) {  //number of slots per frame
            chn_frame_slots_per_mod_and_col_[dummyModId_][i].push_back(std::make_pair(i + 4, j));
          }
        }
        max_tcs_per_module_and_column_[dummyModId_].push_back(
            std::make_pair(i, chn_frame_slots_per_mod_and_col_[dummyModId_][i].size()));
      }
    }*/

    void configureMappingInfo(){
        for(unsigned iModGroup=0; iModGroup<58;iModGroup++){
            for(unsigned j=0; j<4; j++){//BC low (mod 1) - 4 bins, 1TC/bin
                chn_frame_slots_per_mod_and_col_[1+iModGroup*5][j].push_back(std::make_pair(0,0));
                max_tcs_per_module_and_column_[1+iModGroup*5].push_back(std::make_pair(j,chn_frame_slots_per_mod_and_col_[1][j].size()));
            }
            for(unsigned j=0; j<7; j++){//BC high (mod 0) - 7 bins, 2 TC/bin bin 1-2; 1TC/bin rest
                chn_frame_slots_per_mod_and_col_[iModGroup*5][j].push_back(std::make_pair(0,0));
                if(j<2){
                    chn_frame_slots_per_mod_and_col_[iModGroup*5][j].push_back(std::make_pair(0,0));
                }
                max_tcs_per_module_and_column_[iModGroup*5].push_back(std::make_pair(j,chn_frame_slots_per_mod_and_col_[0][j].size()));
            }
            for(unsigned j=0; j<3; j++){//STC16 (mod 3-4) - 3 bins, 1TC/bin
                for(unsigned i=3; i<5; i++){
                    chn_frame_slots_per_mod_and_col_[i+iModGroup*5][j].push_back(std::make_pair(0,0));
                    max_tcs_per_module_and_column_[i+iModGroup*5].push_back(std::make_pair(j,chn_frame_slots_per_mod_and_col_[i][j].size()));
                }
            }
            for(unsigned j=0; j <7; j++){//STC4 (mod 2) - 7 bins, 2TC/bin bin 1-5; 1TC/bin rest
                chn_frame_slots_per_mod_and_col_[2+iModGroup*5][j].push_back(std::make_pair(0,0));
                if(j<5){
                    chn_frame_slots_per_mod_and_col_[2+iModGroup*5][j].push_back(std::make_pair(0,0));
                }
                max_tcs_per_module_and_column_[2+iModGroup*5].push_back(std::make_pair(j,chn_frame_slots_per_mod_and_col_[2][j].size()));
            }
        }
    }

    void configureTestBeamMappingInfo(){
      for(unsigned j=0;j<7; j++){ //Module 256, 8448 - BC (high occ for now)
        chn_frame_slots_per_mod_and_col_[256][j].push_back(std::make_pair(0,0));
        chn_frame_slots_per_mod_and_col_[8448][j].push_back(std::make_pair(0,0));
        if (j<2){
          chn_frame_slots_per_mod_and_col_[256][j].push_back(std::make_pair(0,0));
          chn_frame_slots_per_mod_and_col_[8448][j].push_back(std::make_pair(0,0));

        }
        max_tcs_per_module_and_column_[256].push_back(std::make_pair(j,chn_frame_slots_per_mod_and_col_[256][j].size()));
        max_tcs_per_module_and_column_[8448].push_back(std::make_pair(j,chn_frame_slots_per_mod_and_col_[8448][j].size()));

      }

      for(unsigned j=0; j<7;j++){ // Module 768,8960 - STC4A
        chn_frame_slots_per_mod_and_col_[768][j].push_back(std::make_pair(0,0));
        chn_frame_slots_per_mod_and_col_[8960][j].push_back(std::make_pair(0,0));
        if(j<5){
          chn_frame_slots_per_mod_and_col_[768][j].push_back(std::make_pair(0,0));
          chn_frame_slots_per_mod_and_col_[8960][j].push_back(std::make_pair(0,0));
        }
        max_tcs_per_module_and_column_[768].push_back(std::make_pair(j,chn_frame_slots_per_mod_and_col_[768][j].size()));
        max_tcs_per_module_and_column_[8960].push_back(std::make_pair(j,chn_frame_slots_per_mod_and_col_[8960][j].size()));
      }


      for(unsigned j=0; j<3;j++){ // Module 1280,9472 - CTC4A, 
        chn_frame_slots_per_mod_and_col_[1280][j].push_back(std::make_pair(0,0));
        max_tcs_per_module_and_column_[1280].push_back(std::make_pair(j,chn_frame_slots_per_mod_and_col_[1280][j].size()));
        chn_frame_slots_per_mod_and_col_[9472][j].push_back(std::make_pair(0,0));
        max_tcs_per_module_and_column_[9472].push_back(std::make_pair(j,chn_frame_slots_per_mod_and_col_[1280][j].size()));
      }
    }

    unsigned phiSector() const { return sector120_; }
    uint32_t fpgaID() const { return fpga_id_; }
    unsigned getColBudgetAtIndex(unsigned moduleId, unsigned theColumnIndex) const {
      return max_tcs_per_module_and_column_.at(moduleId).at(theColumnIndex).second;
    }  //Get TC budget for column at index theColumnIndex in the vector
    int getColFromBudgetMapAtIndex(unsigned moduleId, unsigned theColumnIndex) const {
      return max_tcs_per_module_and_column_.at(moduleId).at(theColumnIndex).first;
    }  //Get column number at index theColumnIndex in the vector
    unsigned getChannelAtIndex(unsigned moduleId, int theColumn, unsigned theChnFrameIndex) const {
      return chn_frame_slots_per_mod_and_col_.at(moduleId).at(theColumn).at(theChnFrameIndex).first;
    }  //Extract channel number for colnr theColumn, at given channel+frame index in the vector
    unsigned getFrameAtIndex(unsigned moduleId, int theColumn, unsigned theChnFrameIndex) const {
      return chn_frame_slots_per_mod_and_col_.at(moduleId).at(theColumn).at(theChnFrameIndex).second;
    }  //Extract frame number for colnr theColumn, at given channel+frame index in the vector

  private:
    unsigned sector120_;
    uint32_t fpga_id_;
    std::unordered_map<unsigned, std::vector<std::pair<int, unsigned>>> max_tcs_per_module_and_column_;
    std::unordered_map<unsigned, std::unordered_map<int, std::vector<std::pair<unsigned, unsigned>>>>
        chn_frame_slots_per_mod_and_col_;
    //const uint32_t dummyModId_ = 1879048191;  // Just to avoid filling maps for random module ID values. Temporary!!
  };

}  // namespace l1thgcfirmware

#endif