#include <iostream>
#include "L1Trigger/DemonstratorTools/interface/utilities.h"
#include "TFile.h"
#include "TTree.h"
#include <boost/program_options.hpp>
namespace po = boost::program_options;

uint16_t extractTC(uint64_t data, int tcIndex) {
    int bitOffset = tcIndex*16;
    return static_cast<uint16_t>((data >> bitOffset) & 0x7FFF);
  }

int getModuleNumber(int frameNum_, int channelNum_, int framesToIgnore_, int channelsToIgnore_){
    assert(frameNum_>(framesToIgnore_-1)&&channelNum_>(channelsToIgnore_-1));//Check that we are not in the first 4 output channels or the first two output frames
    int bareModuleNumber = -999;
    if(channelNum_%2==1 && ((frameNum_-framesToIgnore_)%8==0)) bareModuleNumber=4;//If channel number mod 2 is 1 then we are on the second link of the link pair and have the output of the fifth module (number 4)
    else if ( (channelNum_%2)==0 && ((frameNum_-framesToIgnore_)%8<3)) bareModuleNumber=0;
    else if ( (channelNum_%2)==0 && ((frameNum_-framesToIgnore_)%8==3)) bareModuleNumber=1;
    else if ( (channelNum_%2)==0 && ((frameNum_-framesToIgnore_)%8==4)) bareModuleNumber=2;
    else if ( (channelNum_%2)==0 && ((frameNum_-framesToIgnore_)%8==5)) bareModuleNumber=3;

    if(bareModuleNumber > -999) bareModuleNumber+=((channelNum_-channelsToIgnore_)/2)*5; //Channel number added to adjust module numbering: the first instrumented channels (4,5) contain modules 0-4, channels 6,7 have modules 5-9, etc. 
    return bareModuleNumber;
}

int getInputEvent(int moduleNum_, int outEvt_, int bchiOff_, int bclowOff_, int stcOff_){
   int inputEvt=0;
   int bareModuleNum = moduleNum_%5;
  if(bareModuleNum==0) inputEvt=outEvt_-bchiOff_;
  else if(bareModuleNum==1) inputEvt=outEvt_-bclowOff_;
  else if(bareModuleNum==2||bareModuleNum==3||bareModuleNum==4) inputEvt=outEvt_-stcOff_;
  return inputEvt;
}

int getColumnNumber(int moduleNum_,int tcNum_, int frameNum_, int framesToIgnore_){
    int bareModuleNum_ = moduleNum_%5;
    int colNum_=0;
    if(bareModuleNum_==1||bareModuleNum_==2||bareModuleNum_==3||bareModuleNum_==4) colNum_=tcNum_;
    else if(bareModuleNum_==0){
        if((frameNum_-framesToIgnore_)%8==0){//Need to adjust by a certain number of frames at the start of the output file
            if(tcNum_==0||tcNum_==1) colNum_=0;
            else colNum_=1;
        } else colNum_=tcNum_+(((frameNum_-framesToIgnore_)%8))*4-2;//column number is 2-5 for the n+1 frame and 6-8 for the n+2 frame. Bit clunky but this gives that
    }
    if((bareModuleNum_==3||bareModuleNum_==4) && colNum_==3) colNum_=-999;//For these modules, there are only 3 columns so need to set the fourth column number to something that will be considered invalid
    if(bareModuleNum_==0&&colNum_>6) colNum_=-999;// For this module, have 7 columns (0-6), so need to set the column number for the remaining cols to something that will be considered invalid.
    return colNum_;
}

int main(int argc, char **argv)
{

  std::string inputFWFileName;
  std::string outputFileName;
  int ignoreFirstNFrame_;
  int ignoreFirstNChn_;
  int offsetBCHi_;
  int offsetBCLo_;
  int offsetSTC_;
  bool doPrint_;

  po::options_description desc("Allowed options");
  desc.add_options()
     ("help", "Print available options")
     ("ignore-first-frames",po::value<int>(&ignoreFirstNFrame_)->default_value(2),"Numer of frames at the start of the tx file to be ignored")
    ("ignore-first-channels",po::value<int>(&ignoreFirstNChn_)->default_value(4),"Numer of channels at the start of the tx file to be ignored")
     ("offset-bch",po::value<int>(&offsetBCHi_)->default_value(10),"BC high occupancy latency")
     ("offset-bcl",po::value<int>(&offsetBCLo_)->default_value(4),"BC low occupancy latency")
     ("offset-stc",po::value<int>(&offsetSTC_)->default_value(12),"STC latency")
     ("input-file",po::value<std::string>(&inputFWFileName)->default_value(""),"Input filename")
     ("output-file",po::value<std::string>(&outputFileName)->default_value("FirmwareResults.root"),"Output filename")
     ("print",po::value<bool>(&doPrint_)->default_value(false),"Print TCs to screen");

  po::variables_map vm;
  po::store(po::parse_command_line(argc,argv,desc),vm);
  po::notify(vm);

  if (vm.count("help")){
    std::cout<<desc<< "\n";
    return 1;
  }


  TTree evtstree("Events", "Tree");
  Int_t mod_, address_, col_, evtin_, evtout_;
  Long64_t energy_;
  evtstree.Branch("EventIn", &evtin_, "evtin_/I");
  evtstree.Branch("EventOut", &evtout_, "evtout_/I");
  evtstree.Branch("Module", &mod_, "mod_/I");
  evtstree.Branch("Address", &address_, "address_/I");
  evtstree.Branch("Column", &col_, "col_/I");
  evtstree.Branch("Energy", &energy_, "energy_/L");
  
  l1t::demo::BoardData inputs = l1t::demo::read( inputFWFileName, l1t::demo::FileFormat::EMPv2 );

  for ( const auto& channel : inputs ) {
    unsigned iFrame = 0;
    //write these straight to a tree:
    //output bx; corresponding module number; corresponding input bx; bin ID
    for ( const auto& frame : channel.second ) {
        if(!(frame.data==0||frame.data==432349964455149568||frame.data==1008819510838298624||frame.data==4096||frame.data==864699926762782720||frame.data==4400227581952||frame.data==0x8000040082008000)){//Make sure we skip 'empty' data 
            if(iFrame > (ignoreFirstNFrame_-1) && channel.first>(ignoreFirstNChn_-1)){
                mod_ = getModuleNumber(iFrame,channel.first,ignoreFirstNFrame_,ignoreFirstNChn_);
                evtout_ = (iFrame-ignoreFirstNFrame_)/8;//The output BX number. Assume we always have 8 frames per BX in the output file. 
                evtin_ = getInputEvent(mod_,evtout_,offsetBCHi_,offsetBCLo_,offsetSTC_);//Input BX number - ie the output BX adjusted for the latency of each specific unpacker
                if(evtin_>-1){//Only fill the tree if the input event number is at least 0
                    for(unsigned tcno_=0; tcno_<4;tcno_++){
                        energy_ = extractTC(frame.data,tcno_)&0x1FF;
                        address_= (extractTC(frame.data,tcno_)>>9)&0x3F;
                        col_ = getColumnNumber(mod_,tcno_,iFrame,ignoreFirstNFrame_);
                        if(mod_!=-999&&col_!=-999) evtstree.Fill();//Only fill the tree if the module number is valid and if the column number is valid. 
                    }
                }
            }
        }

      if(doPrint_){
        std::cout << "channel "<<channel.first<<" frame "<<iFrame<< "start orbit "<<frame.startOfOrbit <<" start packet "<< frame.startOfPacket << " end packet "<< frame.endOfPacket << "valid "<< frame.valid << " data  " << std::hex << frame.data << std::dec << std::endl;
        std::cout<<"TCs 0 "<< extractTC(frame.data,0) << " 1 "<< extractTC(frame.data,1) <<" 2 "<<extractTC(frame.data,2) << " 3 "<<extractTC(frame.data,3)<<std::endl;
        uint32_t tc_energy_0 = extractTC(frame.data,0)&0x1FF;
        uint32_t tc_address_0 = (extractTC(frame.data,0)>>9)&0x3F;
        std::cout<<"TC 0 energy "<< tc_energy_0<<" TC 0 address "<<tc_address_0<<std::endl;
        uint32_t tc_energy_1 = extractTC(frame.data,1)&0x1FF;
        uint32_t tc_address_1 = (extractTC(frame.data,1)>>9)&0x3F;
        std::cout<<"TC 1 energy "<< tc_energy_1<<" TC 1 address "<<tc_address_1<<std::endl;
        uint32_t tc_energy_2 = extractTC(frame.data,2)&0x1FF;
        uint32_t tc_address_2 = (extractTC(frame.data,2)>>9)&0x3F;
        std::cout<<"TC 2 energy "<< tc_energy_2<<" TC 2 address "<<tc_address_2<<std::endl;
        uint32_t tc_energy_3 = extractTC(frame.data,3)&0x1FF;
        uint32_t tc_address_3 = (extractTC(frame.data,3)>>9)&0x3F;
        std::cout<<"TC 3 energy "<< tc_energy_3<<" TC 3 address "<<tc_address_3<<std::endl;
      }
      iFrame++;      
    }//loop over frames of a given channel
    
  }//loop over channels


  TFile fileout(outputFileName.c_str(), "recreate");
  evtstree.Write();
}