/**********************************************************************
 Created on : 12/03/2025
 Purpose    : Compare FW and emulation results
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/


#include <iostream>
#include <algorithm>
#include "L1Trigger/DemonstratorTools/interface/utilities.h"


int main()
{
  void ReadEMPData(std::string fname, std::map<uint32_t,std::vector<l1t::demo::Frame>>&, int offset);
  std::string inputEmulFileName = "EMPStage2Output.txt";
  //std::string inputFWFileName = "input/stage2/firmware-data/CaptureStage2_250214_1137/tx_summary.txt";
  //std::string inputFWFileName = "input/stage2/firmware-data/CaptureStage2_250314_1218/tx_summary.txt";
  std::string inputFWFileName = "input/stage2/firmware-data/CaptureStage2_250321_1305/tx_summary.txt";
  std::map<uint32_t,std::vector<l1t::demo::Frame>> emuldataset, fwdataset;
  int emulframeoffset = 0, fwframeoffset = 199;
  ReadEMPData(inputEmulFileName,emuldataset,emulframeoffset);
  ReadEMPData(inputFWFileName,fwdataset,fwframeoffset);
  std::cout << "emuldata.size : " << emuldataset.size() << std::endl;
  std::cout << "fwdata.size : " << fwdataset.size() << std::endl;
  int offsetindex = 66;
  for(const auto& emullinkframe: emuldataset){
    int iw = 0;
    std::cout  << "Stage2 output link : " << emullinkframe.first 
	       << ", emul frame size: " << emullinkframe.second.size()
      	       << ", firmware frame size: " << fwdataset[emullinkframe.first].size();
    //<< std::endl;

    ////===========================================================
    //// Reorder the firmware data
    ////===========================================================
    // Collect only the clusterproperties output from the FW data
    //----------------------------------
    // Step1: Find the first nonzero word and first cluster word after the header
    //----------------------------------
    uint64_t firstword = 0x0;
    uint64_t firstClusAfterHeaderword = 0x0;
    int iframe = 0;
    int firstframeid = 0;
    int firstheaderframeid = 0;
    int nextNonZero = 0;
    bool hasFoundFirstWord = false;
    bool hasFoundNextNonZero = false;
    bool hasFoundHeaderWord = false;
    bool hasFoundClusAfterHeaderword = false;
    for( const auto& frame : fwdataset[emullinkframe.first]){
      if(frame.valid and !hasFoundFirstWord){
	firstword = frame.data ;
	firstframeid = iframe;
	hasFoundFirstWord = true;
      }
      if(hasFoundFirstWord and firstframeid!=iframe and frame.valid and frame.data!=0x0 and !hasFoundNextNonZero){
	nextNonZero = iframe;
	hasFoundNextNonZero = true;
      }
      if(frame.startOfPacket and frame.valid and !hasFoundHeaderWord){
	hasFoundHeaderWord = true;
	firstheaderframeid = iframe;
      }
      if(hasFoundHeaderWord and (iframe-firstheaderframeid)==31 and frame.valid and frame.data!=0x0 and !hasFoundClusAfterHeaderword){
	firstClusAfterHeaderword = frame.data;
	hasFoundClusAfterHeaderword = true;
      }
      
      iframe++;
    }
    if(hasFoundFirstWord and hasFoundNextNonZero and (nextNonZero-firstframeid)<30){
      ;//std::cout << "First word at index : " << firstframeid << std::hex << " and word 0x" << firstword << std::dec << std::endl;
    }else{
      std::cout << "First word has not been spotted " << std::endl;
      continue;
    }
    if(!hasFoundClusAfterHeaderword){
      std::cout << "First cluster word after the header has not been spotted " << std::endl;
      continue;
    }else{
      ;//std::cout << std::hex << "First cluster word after the header 0x" << firstClusAfterHeaderword << std::dec << std::endl;
    }
    //---------------------------------------------------------------------------
    // Step2: Check that the first word is repeated after a certain framegap
    //---------------------------------------------------------------------------
    int indexRepeat = firstframeid+int(emullinkframe.second.size());
    if(fwdataset[emullinkframe.first].at(indexRepeat).data != firstword) {
      std::cout << "Repeat word has not been found " << std::endl;
      continue;
    }
    //-------------------------------------------------------------------------------
    // Step3: Fill the relevant cluster properties output to the final array of fwdata
    //-------------------------------------------------------------------------------
    std::vector<uint64_t> fwdata;
    iframe = 0;
    for( const auto& frame : fwdataset[emullinkframe.first]){
      if(frame.valid and frame.data!=0x0 and !frame.startOfPacket and iframe>=firstframeid and iframe<indexRepeat)
	fwdata.push_back(frame.data);
      iframe++;
    }    
    std::cout  << ", fw cluster datasize " << fwdata.size() ;//<< std::endl;
    //-------------------------------------------------------------------------------
    // Step4: Rotate to bring the first cluster word after the header to the front
    //-------------------------------------------------------------------------------
    int foundMatchindex = -1;
    uint64_t fwword = firstClusAfterHeaderword;
    for(unsigned iw=0;iw<fwdata.size();iw++) if(fwdata.at(iw)==fwword) foundMatchindex = iw;
    if(foundMatchindex == -1){
      std::cout << "\n Match not found for the first cluster word after the header in the fwdata array" << std::endl;
      continue;
    }
    for(unsigned iw=0;iw<foundMatchindex;iw++) std::rotate(fwdata.begin(),fwdata.begin()+1,fwdata.end());    
    ////=====================================================================

    
    ////======================================================================
    //// Reorder the emulated data
    ////======================================================================
    // Step1: Collect only the clusterproperties output from the emulated data
    //------------------------------------------------------------------------
    std::vector<uint64_t> preemuldata;
    iframe = 0;
    int sopindex = 0;
    for( const auto& frame : emuldataset[emullinkframe.first]){
      if(frame.startOfPacket) sopindex = iframe;
      if(frame.valid and frame.data!=0x0 and !frame.startOfPacket and iframe>=(sopindex+31))
	preemuldata.push_back(frame.data);
      iframe++;
    }
    std::cout  << ", preemuldata size " << preemuldata.size() ;//<< std::endl;

    //-----------------------------------------------------------------------------------------
    // Step2: Rotate the preemul data to find the first cluster word after header in the fwdata
    //-----------------------------------------------------------------------------------------
    foundMatchindex = -1;
    fwword = firstClusAfterHeaderword;
    for(unsigned iw=0;iw<preemuldata.size();iw++) if(preemuldata.at(iw)==fwword) foundMatchindex = iw;
    if(foundMatchindex == -1){
      std::cout << "\n Match not found for the first cluster word after the header in the fwdata array in emuldata " << std::endl;
      continue;
    }
    for(unsigned iw=0;iw<foundMatchindex;iw++) std::rotate(preemuldata.begin(),preemuldata.begin()+1,preemuldata.end());
    
    //-----------------------------------------------------------------------------------------
    // Step3: Clip the first 88 out of 108 cluster words per bx
    //-----------------------------------------------------------------------------------------
    std::vector<uint64_t> emuldata;
    for(int ibx=0;ibx<6;ibx++){
      for(unsigned iw=0;iw<88;iw++){
	int coliw = 108*ibx + iw;
	emuldata.push_back(preemuldata.at(coliw));
      }
    }
    preemuldata.clear();
    std::cout  << ", emul cluster datasize " << emuldata.size() << std::endl;
    ////======================================================================

    ////======================================================================
    //// Compare the firmware and emulated results for each words
    ////======================================================================
    // Step1: The dimention of both arrays should be same
    //------------------------------------------------------------------------
    if(fwdata.size()!=emuldata.size()){
      std::cout<<" Skipping link " << emullinkframe.first << " due to mismatch and fwdata and emuldata dimension " << std::endl;
      continue;
    }

    //------------------------------------------------------------------------
    // Step2: Now compare word-by-word and print the mismatched words 
    //------------------------------------------------------------------------
    int nofmissed = 0;
    //std::cout << "Emuldata size : " << emuldata.size() << std::endl;
    for(unsigned iw=0;iw<emuldata.size();iw++){
      if(emuldata.at(iw)!=fwdata.at(iw)) {
	std::cout << "iw: " << iw << std::hex << ", FWdata : 0x" << fwdata.at(iw) << ", emuldata : 0x" << emuldata.at(iw) << std::dec << std::endl;
	nofmissed++;
      }
    }
    
    //------------------------------------------------------------------------
    // Step3: Print the number of mismatches or full match
    //------------------------------------------------------------------------
    if(nofmissed==0)
      std::cout << "The emulation and FW data are in complete agreement for Stage2 ouput link " << emullinkframe.first << std::endl;
    else
      std::cout << "The emulation and FW data are not matching for " << nofmissed << " cases." << std::endl;
    ////======================================================================
    fwdata.clear(); emuldata.clear();
    
  }//loop over Stage2 links
  return true;
}

void ReadEMPData(std::string fname, std::map<uint32_t,std::vector<l1t::demo::Frame>>& linkwords, int offset)
{
  l1t::demo::BoardData inputs = l1t::demo::read( fname, l1t::demo::FileFormat::EMPv2 );
  auto nChannels = inputs.size();
  std::cout << "compareStage2TowerFWvsEmul::Filename : "<< fname <<", Board data name : " << inputs.name() << ", and number of channels are: " << nChannels  << std::endl;
  
  for ( const auto& channel : inputs ) {
    //std::cout << "Data on channel : " << channel.first << std::endl;
    unsigned iFrame = 0;
    std::vector<l1t::demo::Frame> words ; 
    for ( const auto& frame : channel.second ) {
      //std::cout << frame.startOfOrbit << frame.startOfPacket << frame.endOfPacket << frame.valid << " " << std::hex << frame.data << std::dec << std::endl;
      words.push_back(frame);
      iFrame++;      
    }
    linkwords[channel.first] = words;	 
  }//loop over channels
  
}

