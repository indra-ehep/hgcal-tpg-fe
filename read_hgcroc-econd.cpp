#include <iostream>

#include "TFileHandlerLocal.h"
#include "common/inc/FileReader.h"

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
    std::cout << "Word " << std::setw(3) << i << " ";
    std::cout << std::hex << std::setfill('0');
    std::cout << "0x" << std::setw(16) << p64[i] << std::endl;
    std::cout << std::dec << std::setfill(' ');
  }
  std::cout << std::endl;
}

bool is_cafe_word(const uint64_t word) {
  if(((word >> 32) & 0xFFFFFFFF) == 0xcafecafe)
    return true; //0xFECAFE                                                     
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

// typedef struct{
// } BCEventData;

class BCEventData {
public:
  uint32_t econt0_bc6_modsum[7];
  uint32_t econt0_bc6_energy[7][6];
  uint32_t econt0_bc6_channel[7][6];
  uint32_t bxid_econt0[7];

  void print(const char* type = "") {
    std::cout << "BCEventData(" << this << ")::print("<< type <<"), format = " << std::endl;
    
    for(unsigned ib(0);ib<7;ib++) {
      std::cout << " ib " << ib << std::endl
		<< "  MS  " << " = 0x"
		<< std::hex << ::std::setfill('0')
		<< std::setw(4) << econt0_bc6_modsum[ib]
		<< std::dec << ::std::setfill(' ')
		<< "  sum packed = " << std::setw(5) << econt0_bc6_modsum[ib]
		<< ",     BX = " << std::setw(2) << bxid_econt0[ib]
		<< std::endl;
      
      //for(unsigned i(0);i<3;i++) {//STC16
      for(unsigned i(0);i<6;i++) {
	std::cout << type << "  STC4A " << i 
		  << " channel, energy packed = "
		  << std::setw(5) << econt0_bc6_energy[ib][i]
	          << ", number = " << std::setw(2) << econt0_bc6_channel[ib][i]
		  << std::endl;
      }
    }
  }

};

int main(int argc, char** argv){
  //int econt_data_validation(unsigned relayNumber=1695822887, unsigned runNumber=1695822887){
  
  if(argc < 3){
    std::cerr << argv[0] << ": no relay and/or run numbers specified" << std::endl;
    return 1;
  }

  //Command line arg assignment
  //Assign relay and run numbers
  unsigned linkNumber(1);
  bool skipMSB(true);
  
  // ./econt_data_validation.exe $Relay $rname
  unsigned relayNumber(0);
  unsigned runNumber(0);
  std::istringstream issRelay(argv[1]);
  issRelay >> relayNumber;
  std::istringstream issRun(argv[2]);
  issRun >> runNumber;
  
  int runNumber1 = int(runNumber);


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
  
  uint64_t maxShowEvent = 2;
  uint64_t nEvents = 0;    
  //Keep a record of status of last 100 events
  uint64_t nofRStartErrors = 0, nofRStopErrors = 0;
  uint64_t nofBoEE = 0;
  uint64_t nofEoEE = 0;
  uint64_t nofL1aE = 0;

  const int maxEvents = 10;
  BCEventData elinkData[maxEvents];
  BCEventData unpackerData[maxEvents];
  const int maxEcons = 12;
  
  int ievent = 0;
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
      
      if (nEvents < 2) {
	rEvent->RecordHeader::print();
	boe->print();
  	eoe->print();
  	event_dump(rEvent);
      }
      
      //Increment event counter and reset error state
      nEvents++;
      //if(nEvents==1) continue;
      if(ievent>(maxEvents-1)) continue;
      cout<<"iEvent: " << ievent << endl;
      
      if(boe->boeHeader()!=boe->BoePattern) { nofBoEE++; continue;}
      if(eoe->eoeHeader()!=eoe->EoePattern) { nofEoEE++; continue;}
      uint16_t l1atype = boe->l1aType();      
      if(l1atype==0) { nofL1aE++; continue;}
      
      const uint64_t *p64(((const uint64_t*)rEvent)+1);
      //wordindex 0 and 1 are 128 bit slink headers
      //wordindex 2 is 64 capture block header -- follow miniDAQ documentation
      //Two capture blocks are expected in test-beam July-August 2024
      //wordindex 3 is 64 bit ECON-T header 
      if(nEvents<=maxShowEvent){
	cout<<"========= event : "<< nEvents << "=================="<< endl;
	cout<< std::hex   ;
	cout<<"0th : 0x"<< std::setfill('0') << setw(16) << p64[0] << endl;
	cout<<"1st : 0x" << p64[1] << endl;
	cout<<"2nd : 0x" << p64[2] << endl;
	cout<< std::dec << std::setfill(' ') ;
      }
      /////////////////////////////////////////////////////////////////
      bool isEcondp[maxEcons];
      int nofecons = 0;
      for(int iecon=0;iecon<maxEcons;iecon++) isEcondp[iecon] = false;
      for(int iecon=0;iecon<maxEcons;iecon++){
	uint8_t stat = (p64[2] >> uint8_t(iecon*3)) & 0x7;
	if(stat==0b000 or stat==0b010 or stat==0b011) {
	  isEcondp[iecon] = true ;
	  nofecons++;
	}
      }
      if(nEvents<=maxShowEvent) cout<<"Nof Econs in first capture block: " << nofecons << endl;
      uint32_t econ0h0 = (p64[3] >> 32) & 0xFFFFFFFF ;
      uint32_t econ0h1 = p64[3] & 0xFFFFFFFF ;
      bool isPassthrough = (econ0h0 >> 13 ) & 0x1 ;
      if(nEvents<=maxShowEvent) cout<<"========= End of event : "<< nEvents << "============="<< endl;
      ievent++;
    }//loop event  
  }//while read r
  
  delete r;
  
  return 0;
}
