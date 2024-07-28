#include <iostream>

#include "TFileHandlerLocal.h"
#include "common/inc/FileReader.h"

#include "TPGFEDataformat.hh"
#include "Stage1IO.hh"

// Author : Indranil Das
// Email : Indranil.Das@cern.ch
// Affiliation : Imperial College, London

//Command to execute : 1. ./compile.sh
//                     2. ./read_lpGBT.exe $Relay $rname

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

  
  uint64_t nEvents = 0;    
  //Keep a record of status of last 100 events
  uint64_t nofRStartErrors = 0, nofRStopErrors = 0;
  uint64_t nofBoEE = 0;
  uint64_t nofEoEE = 0;
  uint64_t nofL1aE = 0;

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
      
      if(boe->boeHeader()!=boe->BoePattern) { nofBoEE++; continue;}
      if(eoe->eoeHeader()!=eoe->EoePattern) { nofEoEE++; continue;}
      uint16_t l1atype = boe->l1aType();      
      if(l1atype==0) { nofL1aE++; continue;}
     
      const uint64_t *p64(((const uint64_t*)rEvent)+1);
      
      if(nEvents<=2){
	cout<<"========= event : "<< nEvents << "=================="<< endl;
	cout<< std::hex   ;
	cout<<"0th : 0x"<< std::setfill('0') << setw(16) << p64[0] << endl;
	cout<<"1st : 0x" << p64[1] << endl;
	cout<<"2nd : 0x" << p64[2] << endl;	
      }

      ////////////// First find the block details from header //////////
      int loc[6], size[6];
      for(int iloc = 0 ; iloc < 6 ; iloc++) {loc[iloc] = size[iloc] = -1;}
      cout<< std::dec << std::setfill(' ');
      for(int iloc = 0 ; iloc < 6 ; iloc++){
	loc[iloc] = find_cafe_word(rEvent, iloc+1);
	size[iloc] = p64[loc[iloc]] & 0xFF ;
	int chid = (p64[loc[iloc]] >> 8) & 0xFF ;
	int bufstat = (p64[loc[iloc]] >> 16) & 0xF ;
	int nofwd_perbx = (p64[loc[iloc]] >> 20) & 0xF ;
	if(nEvents<=2)
	  cout<<"iloc: "<< iloc
	      << std::hex
	      <<", word : 0x" << std::setfill('0') << setw(16) << p64[loc[iloc]]
	      << std::dec << std::setfill(' ')
	      << ", location : " << loc[iloc] << ", size: " << size[iloc] <<", ch_id : "<< chid << ", bufstat: " << bufstat << ", nofwd_perbx: "<< nofwd_perbx << endl;
      }
      /////////////////////////////////////////////////////////////////
      
      //////////// Read raw elink inputs for ch 1 /////////////////////
      int iblock = 1;
      int elBeginIndx = 3;
      int elIndx = 0;
      uint32_t elpckt0[7];
      int iel = 0;
      for(int iw = loc[iblock]+1; iw <= (loc[iblock]+size[iblock]) ; iw++ ){
	uint32_t wMSB = (p64[iw] >> 32) & 0xFFFFFFFF ;
	uint32_t wLSB = p64[iw] & 0xFFFFFFFF ;
	if(nEvents<=2)
	  cout<<"iloc: "<< iw
	      << std::hex
	      <<", word : 0x" << std::setfill('0') << setw(8) << wLSB
	      << std::dec << std::setfill(' ')
	      <<endl;
	if(elIndx>=elBeginIndx and elIndx<elBeginIndx+7){
	  elpckt0[iel] = wLSB;
	  iel++;
	}//pick the first elink
	elIndx++;
      }
      /////////////////////////////////////////////////////////////////

      //////////////// Print the energy/location and stage1 input//////
      if(nEvents<=2)
	for(int iel=0;iel<7;iel++)
	  cout<<"elIndx: "<< iel
	      << std::hex
	      <<", word : 0x" << std::setfill('0') << setw(8) << elpckt0[iel]
	      << std::dec << std::setfill(' ')
	      <<endl;
      TPGFEDataformat::TcRawDataPacket vTcrdp;
      TPGBEDataformat::UnpackerOutputStreamPair up;
      if(nEvents<=2) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::TcRawData::BestC, 6, elpckt0, vTcrdp);
      if(nEvents<=2) {
	TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(0, vTcrdp, up);
	up.print();
      }
      /////////////////////////////////////////////////////////////////
      
      //////////// Print unpacker output for ch 1 /////////////////////
      iblock = 3;
      for(int iw = loc[iblock]+1; iw <= (loc[iblock]+size[iblock]) ; iw++ ){
	uint32_t col0 = (p64[iw] >> (32+16)) & 0xFFFF ;
	uint32_t col1 = (p64[iw] >> 32) & 0xFFFF ;
	uint32_t col2 = (p64[iw] >> (32-16)) & 0xFFFF ;
	uint32_t col3 = p64[iw] & 0xFFFF ;
	if(nEvents<=2)
	  cout<<"iloc: "<< iw
	      << std::hex
	      <<", col0 : 0x" << std::setfill('0') << setw(4) << col0 <<", "
	      <<", col1 : 0x" << std::setfill('0') << setw(4) << col1 <<", "
	      <<", col2 : 0x" << std::setfill('0') << setw(4) << col2 <<", "
	      <<", col3 : 0x" << std::setfill('0') << setw(4) << col3 <<", "
	      << std::dec << std::setfill(' ')
	      <<endl;
      }
      /////////////////////////////////////////////////////////////////
      
      if(nEvents<=2) cout<<"========= End of event : "<< nEvents << "============="<< endl;
    }//loop event  
  }
  
  delete r;
  
  return 0;
}
