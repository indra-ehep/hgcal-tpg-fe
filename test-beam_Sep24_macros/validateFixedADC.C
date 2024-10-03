/**********************************************************************
 Created on : 02/10/2024
 Purpose    : Validate the ADC values in each event of fixed pattern run
 Author     : Indranil Das, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
#include <iostream>

#include "TFileHandlerLocal.h"
#include "FileReader.h"

#include "TPGFEDataformat.hh"
#include "TPGFEModuleEmulation.hh"
#include "Stage1IO.hh"

// Author : Indranil Das
// Email : Indranil.Das@cern.ch
// Affiliation : Imperial College, London

//Command to execute : 1. ./compile.sh
//                     2. ./read_lpGBT.exe $Relay $rname
//                     3. ./read_lpGBT.exe 1722081291 1722081291

using namespace std;


// print all (64-bit) words in event
void event_dump(const Hgcal10gLinkReceiver::RecordRunning *rEvent){
  const uint64_t *p64(((const uint64_t*)rEvent)+1);
  for(unsigned i(0);i<rEvent->payloadLength();i++){
    std::cout << "EventId : " << std::setw(3) << rEvent->slinkBoe()->eventId() ;
    std::cout << "\t Word " << std::setw(3) << i << " ";
    std::cout << std::hex << std::setfill('0');
    std::cout << "0x" << std::setw(16) << p64[i] << std::endl;
    std::cout << std::dec << std::setfill(' ');
  }
  std::cout << std::endl;
}

void event_dump32(const Hgcal10gLinkReceiver::RecordRunning *rEvent){
  const uint32_t *p32(((const uint32_t*)rEvent)+2);
  for(unsigned i(0);i<2*rEvent->payloadLength();i+=2){
    std::cout << "EventId : " << std::setw(3) << rEvent->slinkBoe()->eventId() ;
    std::cout << "\t Word " << std::setw(3) << i/2 << " ";
    std::cout << std::hex << std::setfill('0');
    std::cout << "MSB: 0x" << std::setw(8) << p32[i+1] ; 
    std::cout << ", LSB: 0x" << std::setw(8) << p32[i] << std::endl;
    std::cout << std::dec << std::setfill(' ');
  }
  std::cout << std::endl;
}

void event_dump32s(const Hgcal10gLinkReceiver::RecordRunning *rEvent){
  const uint32_t *p32(((const uint32_t*)rEvent)+2);
  for(unsigned i(0);i<2*rEvent->payloadLength();i++){
    std::cout << "EventId : " << std::setw(3) << rEvent->slinkBoe()->eventId() ;
    std::cout << "\t Word " << std::setw(3) << i << " ";
    std::cout << std::hex << std::setfill('0');
    std::cout << "0x" << std::setw(8) << p32[i] << std::endl;
    std::cout << std::dec << std::setfill(' ');
  }
  std::cout << std::endl;
}

bool is_cafe_word(const uint64_t word) {
  if(((word >> 32) & 0xFFFFFFFF) == 0xcafecafe)
    return true; //0xFECAFE                                                      // 0xFF FFFF masking to be there to compare with 16698110 (could have used the hex of this)
  else
    return false;
}
   

// returns location in this event of the n'th 0xfecafe... line
int find_cafe_word(const Hgcal10gLinkReceiver::RecordRunning *rEvent, int n, int cafe_word_loc = -1) {
  const uint64_t *p64(((const uint64_t*)rEvent)+1);

  if (cafe_word_loc > 0) {
    if (is_cafe_word(p64[cafe_word_loc])) {
      return cafe_word_loc;
    }else
      return cafe_word_loc;
  } 
  else {
    ;;//std::cout << "oops" << std::endl;
  }
 
  int cafe_counter = 0;
  int cafe_word_idx = -1;
  for(unsigned i(0);i<rEvent->payloadLength();i++){
    const uint64_t word = p64[i];                                                                                // Is it advisable to convert 64 bit to 32 this way without masking ?
    if (is_cafe_word(word)) { // if word == 0xfeca
      cafe_counter++;       
      if (cafe_counter == n){                                                                                    
        cafe_word_idx = i;
        break;
      }
    }
  }

  if (cafe_word_idx == -1) {
    //std::cerr << "Could not find cafe word" << std::endl;
    return 0;
  }else {
    return cafe_word_idx;
  }
}


int main(int argc, char** argv){
  //int econt_data_validation(unsigned relayNumber=1695822887, unsigned runNumber=1695822887){
  
  if(argc < 3){
    std::cerr << argv[0] << ": no relay and/or run numbers specified" << std::endl;
    return 1;
  }

  //Command line arg assignment
  //Assign relay and run numbers
  unsigned linkNumber(0);
  bool skipMSB(true);
  
  // ./econt_data_validation.exe $Relay $rname
  unsigned relayNumber(0);
  unsigned runNumber(0);
  std::istringstream issRelay(argv[1]);
  issRelay >> relayNumber;
  std::istringstream issRun(argv[2]);
  issRun >> runNumber;
  std::istringstream issLink(argv[3]);
  issLink >> linkNumber;
  if(linkNumber!=0 and linkNumber!=1){
    std::cerr << "Link number "<< argv[3] <<"is out of bound (use: 0 or 1)" << std::endl;
    return false;
  }
  
  uint32_t wordrange32[7][2] = {{0, 9}, {242, 247}, {480, 485}, {718, 725}, {958, 963}, {1196, 1201}, {1434, 1439}};
  uint32_t wordlist32[60] = {
     46,   49,   86,   87,  124,  127,  164,  165,  202,  205,
    284,  287,  324,  325,  362,  365,  402,  403,  440,  443,
    522,  525,  562,  563,  600,  203,  640,  641,  678,  681,
    762,  765,  802,  803,  840,  843,  880,  881,  918,  921,
   1000, 1003, 1040, 1041, 1078, 1081, 1118, 1119, 1156, 1159,
   1238, 1241, 1278, 1279, 1316, 1319, 1356, 1357, 1394, 1397
  };
  std::vector<uint32_t> skipWords;
  for(int ilpos=0;ilpos<60;ilpos++) skipWords.push_back(wordlist32[ilpos]);
  for(int irg=0;irg<7;irg++)
    for(int iwpos=wordrange32[irg][0];iwpos<=wordrange32[irg][1];iwpos++)
      skipWords.push_back(iwpos);
  std::sort(skipWords.begin(), skipWords.end());
  // for(auto iw : skipWords) std::cout <<"word to be skipped : " << iw << std::endl;
  
  //Create the file reader
  Hgcal10gLinkReceiver::FileReader _fileReader;

  //Make the buffer space for the records
  Hgcal10gLinkReceiver::RecordT<4095> *r(new Hgcal10gLinkReceiver::RecordT<4095>);

  //Set up specific records to interpet the formats
  const Hgcal10gLinkReceiver::RecordStarting *rStart((Hgcal10gLinkReceiver::RecordStarting*)r);
  const Hgcal10gLinkReceiver::RecordStopping *rStop ((Hgcal10gLinkReceiver::RecordStopping*)r);
  const Hgcal10gLinkReceiver::RecordRunning  *rEvent((Hgcal10gLinkReceiver::RecordRunning*) r);
  _fileReader.setDirectory(std::string("dat/Relay")+argv[1]);
  _fileReader.openRun(runNumber,linkNumber);
  
  uint64_t nEvents = 0;    
  //Keep a record of status of last 100 events
  uint64_t nofRStartErrors = 0, nofRStopErrors = 0;
  const uint32_t maxWord = 1440;
  uint32_t refWords32[maxWord];
  uint32_t tctp_r[maxWord];
  uint32_t adcseg_tctp01_r[maxWord];
  uint32_t adcseg_tctp23_r[maxWord];
  std::vector<uint64_t> nonZeroEvents;
  
  //Use the fileReader to read the records
  while(_fileReader.read(r)) {
    //Check the state of the record and print the record accordingly
    if( r->state()==Hgcal10gLinkReceiver::FsmState::Starting){
      if(!(rStart->valid())){
  	std::cerr << " FsmState::Starting validadity fails : rStart->valid() " << rStart->valid() << std::endl;
  	rStart->print();
  	std::cout << std::endl;
	nofRStartErrors++;
	continue;
      }
    }
    
    else if(r->state()==Hgcal10gLinkReceiver::FsmState::Stopping){
      if(!(rStop->valid())){
  	std::cerr << " FsmState::Stopping validadity fails : rStop->valid() " << rStop->valid() << std::endl;
  	rStop->print();
  	std::cout << std::endl;
	nofRStopErrors++;
	continue;
      }
    }
    //Else we have an event record 
    else{

      const Hgcal10gLinkReceiver::SlinkBoe *boe = rEvent->slinkBoe();
      const Hgcal10gLinkReceiver::SlinkEoe *eoe = rEvent->slinkEoe();
      const Hgcal10gLinkReceiver::BePacketHeader *beh =  rEvent->bePacketHeader();
      
      // if (beh->pattern()!=0xfe ) {
      // 	std::cout <<"=========================================================="<< std::endl;
      // 	std::cout <<"Event: " << boe->eventId() << std::endl;
      // 	rEvent->RecordHeader::print();
      // 	boe->print();
      // 	eoe->print();
      // 	beh->print();
      // 	std::cout <<"=========================================================="<< std::endl;
      // 	//event_dump(rEvent);
      // }
      
      if (nEvents < 2 ) {
	std::cout <<"Event: " << boe->eventId() << std::endl;
	rEvent->RecordHeader::print();
	boe->print();
	eoe->print();
	beh->print();
	event_dump(rEvent);
	event_dump32(rEvent);
	event_dump32s(rEvent);
      }
      if(nEvents<=2) cout<<"========= End of event : "<< nEvents << "============="<< endl;
      //Increment event counter and reset error state

      const uint32_t *p32(((const uint32_t*)rEvent)+2);
      
      if(rEvent->slinkBoe()->eventId()==1){
	if(maxWord<(2*rEvent->payloadLength())) {
	  std::cerr << "The payload dimension " << (2*rEvent->payloadLength()) << " words is more than maximum allowed dimension, i.e. " << maxWord <<" words." << std::endl;
	  _fileReader.close();
	  if(!r) delete r;
	  return -1;	  						    
	}
	for(unsigned i(0);i<2*rEvent->payloadLength();i++) {
	  refWords32[i] = p32[i];
	  tctp_r[i] = (refWords32[i] >> 30) & 0x3 ;
	  adcseg_tctp01_r[i] = (refWords32[i] >> 10) & 0xfffff ;
	  adcseg_tctp23_r[i] = (refWords32[i] >> 20) & 0x3ff ;
	}
      }
      
      for(unsigned i(0);i<2*rEvent->payloadLength();i++){
	if(std::find(skipWords.begin(),skipWords.end(),i) == skipWords.end()){
	  // std::cout << "EventId : " << std::setw(3) << rEvent->slinkBoe()->eventId() ;
	  // std::cout << "\t Word " << std::setw(3) << i << " ";
	  // std::cout << std::hex << std::setfill('0');
	  // std::cout << "0x" << std::setw(8) << p32[i] << std::endl;
	  // std::cout << std::dec << std::setfill(' ');
	  uint32_t tctp_i = (p32[i] >> 30) & 0x3 ;
	  uint32_t adcseg_tctp01_i = (p32[i] >> 10) & 0xfffff ;
	  uint32_t adcseg_tctp23_i = (p32[i] >> 20) & 0x3ff ;
	  uint32_t diff = ((tctp_r[i]==0 or tctp_r[i]==1) and (tctp_i==0 or tctp_i==1)) ? (adcseg_tctp01_i-adcseg_tctp01_r[i]) :  (adcseg_tctp23_i-adcseg_tctp23_r[i]) ;
	  //if((diff != 0) and (std::find(nonZeroEvents.begin(),nonZeroEvents.end(),rEvent->slinkBoe()->eventId()) == nonZeroEvents.end())) nonZeroEvents.push_back(rEvent->slinkBoe()->eventId());
	  if(diff != 0) nonZeroEvents.push_back(rEvent->slinkBoe()->eventId());
	}
      }//word loop
      
      if(nEvents%10000==0) std::cout << "Processing event : " << nEvents << std::endl;
      
      nEvents++;
    }//loop event
  }
  
  std::cout << "========== Total Events : " << nEvents << std::endl;
  
  std::cout<< "Nof Nonzero diff events are "<< nonZeroEvents.size() << std::endl;
  if(nonZeroEvents.size()>0) std::cout<< "/*Nonzero diff events*/ uint64_t refEvt["<< nonZeroEvents.size() <<"] = {";
  for(const uint64_t& totEvt : nonZeroEvents) std::cout<<totEvt << ", ";
  if(nonZeroEvents.size()>0) std::cout<< "};" << std::endl;
  
  _fileReader.close();
  if(!r) delete r;
  
  return 0;
}
