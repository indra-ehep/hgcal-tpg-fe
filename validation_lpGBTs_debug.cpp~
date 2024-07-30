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


uint64_t Abs32(uint32_t a, uint32_t b)
{
  if(a>b)
    return (a-b);
  else if(b>a)
    return (b-a);
  else
    return 0;
}

uint64_t Abs64(uint64_t a, uint64_t b)
{
  if(a>b)
    return (a-b);
  else if(b>a)
    return (b-a);
  else
    return 0;
}

// print all (64-bit) words in event
void Event_Dump(uint64_t refEvent, const Hgcal10gLinkReceiver::RecordRunning *rEvent){
  const uint64_t *p64(((const uint64_t*)rEvent)+1);
  for(unsigned i(0);i<rEvent->payloadLength();i++){
    std::cerr << "EventId " << std::setw(3) << refEvent << " ";
    std::cerr << "Word " << std::setw(3) << i << " ";
    std::cerr << std::hex << std::setfill('0');
    std::cerr << "0x" << std::setw(16) << p64[i] << std::endl;
    std::cerr << std::dec << std::setfill(' ');
  }
  std::cerr << std::endl;
}

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


class BCEventData {
public:
  uint32_t econt_bxid[7];
  uint32_t econt_modsum[7];
  uint32_t econt_energy[7][6];
  uint32_t econt_channel[7][6];

  void print(unsigned nTC = 6, const char* tctype = "TC", const char* datasource = "") {
    std::cout << "BCEventData(" << this << ")::print("<< datasource <<"), format = " << std::endl;
    
    for(unsigned ib(0);ib<7;ib++) {
      std::cout << " ib " << ib << std::endl
		<< "  MS  " << " = 0x"
		<< std::hex << ::std::setfill('0')
		<< std::setw(4) << econt_modsum[ib]
		<< std::dec << ::std::setfill(' ')
		<< "  sum packed = " << std::setw(5) << econt_modsum[ib]
		<< ",     BX = " << std::setw(2) << econt_bxid[ib]
		<< std::endl;
      

      for(unsigned i(0);i<nTC;i++) {
	std::cout << datasource << " " << tctype << " " << i 
		  << " energy packed = "
		  << std::setw(5) << econt_energy[ib][i]
	          << ", channel  = " << std::setw(2) << econt_channel[ib][i]
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

  uint64_t prevEvent = 0;
  uint32_t prevSequence = 0;
  uint64_t eventId = 0;
  uint32_t sequenceId = 0;
  uint16_t bxId = 0;
  uint16_t l1atype = 0;
  
  uint64_t maxShowEvent = 2;
  uint64_t nEvents = 0;    
  
  uint64_t nofRStartErrors = 0, nofRStopErrors = 0;
  uint64_t nofBoEE = 0;
  uint64_t nofEoEE = 0;
  uint64_t nofL1aE = 0;
  uint64_t nofEventIdErrs = 0;
  uint64_t nofExcessFECAFEErrors = 0;  
  uint64_t nofFirstFECAFEErrors = 0;
  
  uint64_t total_phys_events = 0;
  uint64_t total_coinc_events = 0;
  uint64_t total_calib_events = 0;
  uint64_t total_random_events = 0;
  uint64_t total_soft_events = 0;
  uint64_t total_regular_events = 0;
  
  const int maxEvents = 10;
  BCEventData elinkData[maxEvents];
  BCEventData unpackerData[maxEvents];
  
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
      if(boe->boeHeader()!=boe->BoePattern) { nofBoEE++; continue;}
      if(eoe->eoeHeader()!=eoe->EoePattern) { nofEoEE++; continue;}
      
      eventId = boe->eventId();
      bxId = eoe->bxId();
      sequenceId = rEvent->RecordHeader::sequenceCounter(); 
      l1atype = boe->l1aType();
      if(l1atype==0x0001)
      	total_phys_events++;
      if(l1atype==0x0002)
      	total_calib_events++;
      if(l1atype==0x0003)
      	total_coinc_events++;
      if(l1atype==0x0004)
      	total_random_events++;
      if(l1atype==0x0008)
      	total_soft_events++;
      if(l1atype==0x0010)
      	total_regular_events++;
      if(nEvents<=maxShowEvent) cout << "L1aType : " << l1atype << endl;
      if(l1atype==0) { nofL1aE++; continue;}      

      if((Abs64(eventId,prevEvent) != Abs32(sequenceId, prevSequence)) and Abs64(eventId,prevEvent)>=2){
	std::cerr << "Event : "<< eventId << ", l1aType : " << l1atype << ", and prevEvent  "<< prevEvent << ", nEvents : " << nEvents << " differs by "<< Abs64(eventId,prevEvent) <<" (sequence differs by [ "<< sequenceId << " - "<< prevSequence << " ] = " << Abs32(sequenceId, prevSequence) << "), EventID_Diff is more than 2 " << std::endl;
	event_dump(rEvent);
	rEvent->RecordHeader::print();
	boe->print();
	eoe->print();
	nofEventIdErrs++;
	prevEvent = boe->eventId();
	prevSequence = rEvent->RecordHeader::sequenceCounter(); 
	continue;
	//break;
      }
      
      prevEvent = eventId;
      prevSequence = sequenceId;
      
      int seventh_cafe_word_loc = find_cafe_word(rEvent, 7);
      if(seventh_cafe_word_loc!=0){
	nofExcessFECAFEErrors++ ;
	continue;
      }
      
      int first_cafe_word_loc = find_cafe_word(rEvent,1);
      if( first_cafe_word_loc != 2){
	std::cerr << "Event : "<< eventId << ", l1aType : " << l1atype << ", first_cafe_word_loc  "<< first_cafe_word_loc << std::endl;
	// rEvent->RecordHeader::print();
	// boe->print();
  	// eoe->print();
	Event_Dump(eventId, rEvent);
	nofFirstFECAFEErrors++ ;
	continue;
      }
      
      if(eventId==1002670) Event_Dump(eventId, rEvent);
      
      const uint64_t *p64(((const uint64_t*)rEvent)+1);
      
      if(nEvents<=maxShowEvent){
	cout<<"========= event : "<< nEvents << "=================="<< endl;
	cout<< std::hex   ;
	cout<<"0th : 0x"<< std::setfill('0') << setw(16) << p64[0] << endl;
	cout<<"1st : 0x" << p64[1] << endl;
	cout<<"2nd : 0x" << p64[2] << endl;
	cout<< std::dec << std::setfill(' ') ;
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
	if(nEvents<=maxShowEvent)
	  cout<<"iloc: "<< iloc
	      << std::hex
	      <<", word : 0x" << std::setfill('0') << setw(16) << p64[loc[iloc]]
	      << std::dec << std::setfill(' ')
	      << ", location : " << loc[iloc] << ", size: " << size[iloc] <<", ch_id : "<< chid << ", bufstat: " << bufstat << ", nofwd_perbx: "<< nofwd_perbx << endl;
      }
      /////////////////////////////////////////////////////////////////

      //if(nEvents==1) continue;
      if(ievent>(maxEvents-1)) continue;
      cout<<"iEvent: " << ievent << endl;

      // //////////// Read raw elink inputs for ch 1 /////////////////////
      // int iblock = 1;
      // int elBgnOffset = 0;
      // int elIndx = 0;
      // uint32_t elpckt[7][7]; //the first 7 is for bx and second one for number of elinks
      // uint32_t bx = 0xF;
      // uint32_t refBx ;
      // int iel = 0;
      // int ibx = 0;
      // for(int iw = loc[iblock]+1; iw <= (loc[iblock]+size[iblock]) ; iw++ ){
      // 	uint32_t wMSB = (p64[iw] >> 32) & 0xFFFFFFFF ;
      // 	uint32_t wLSB = p64[iw] & 0xFFFFFFFF ;
      // 	if(elIndx>=elBgnOffset and (elIndx-elBgnOffset)%4==0) {
      // 	  bx = (wMSB>>28) & 0xF ;
      // 	  //bx = (wLSB>>28) & 0xF ; //with (elIndx-elBgnOffset)%4==1,2 for STC4A and STC16
      // 	  elinkData[ievent].econt_bxid[ibx] = bx;
      // 	}
      // 	if((elIndx-elBgnOffset)%4==0) iel = 0;
      // 	if(elIndx>=elBgnOffset){
      // 	  elpckt[ibx][iel] = wMSB;
      // 	  if(iel<=6) elpckt[ibx][iel+1] = wLSB;
      // 	  if(iel==0) refBx = bx;
      // 	  iel += 2;
      // 	}//pick the first elink
      // 	if(nEvents<=maxShowEvent)
      // 	  cout<<"iloc: "<< iw << ", bx: " << bx << ", ibx: " << ibx
      // 	      << std::hex
      // 	      <<", MSB-word : 0x" << std::setfill('0') << setw(8) << wMSB
      // 	      <<", LSB-word : 0x" << std::setfill('0') << setw(8) << wLSB
      // 	      << std::dec << std::setfill(' ')
      // 	      <<endl;
      // 	elIndx++;
      // 	if((elIndx-elBgnOffset)%4==0) ibx++;
      // }
      // /////////////////////////////////////////////////////////////////
      
      // ////// Print the energy/location and stage1 input also check back elink//////
      // if(nEvents<=maxShowEvent)
      // 	for(int ib=0;ib<7;ib++)
      // 	  for(int iel=0;iel<7;iel++)
      // 	    cout<<"ibx: "<< ib <<", elIndx: "<< iel
      // 		<< std::hex
      // 		<<", word : 0x" << std::setfill('0') << setw(8) << elpckt[ib][iel]
      // 		<< std::dec << std::setfill(' ')
      // 		<<endl;
      
      // for(int ib=0;ib<7;ib++){
      // 	TPGFEDataformat::TcRawDataPacket vTcrdp;
      // 	TPGBEDataformat::UnpackerOutputStreamPair up;
      // 	TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::TcRawData::BestC, 6, elpckt[ib], vTcrdp);
      // 	//TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::TcRawData::STC4A, 6, &elpckt[ib][3], vTcrdp);
      // 	//TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::TcRawData::STC16, 3, &elpckt[ib][5], vTcrdp);
      // 	TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(elinkData[ievent].econt_bxid[ib], vTcrdp, up);
      // 	elinkData[ievent].econt_modsum[ib] = uint32_t(up.moduleSum(0));
      // 	elinkData[ievent].econt_bxid[ib] = uint32_t(up.bx(0));
      // 	for(int itc=0;itc<6;itc++){ //for BC and STC4A
      // 	//for(int itc=0;itc<3;itc++){
      // 	  elinkData[ievent].econt_energy[ib][itc] = uint32_t(up.channelEnergy(0,itc));
      // 	  elinkData[ievent].econt_channel[ib][itc] = uint32_t(up.channelNumber(0,itc));
      // 	}//itc
      // 	if(nEvents<=maxShowEvent) up.print();
      // }//ib
      // if(nEvents<=maxShowEvent) elinkData[ievent].print("elinks");
      
      // if(nEvents<=maxShowEvent) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::TcRawData::BestC, 6, elpckt0, vTcrdp);
      // if(nEvents<=maxShowEvent) {
      // 	TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(refBx, vTcrdp, up); 
      // 	up.print();
      // }
      // uint32_t elpckt0_test[3];
      // if(nEvents<=maxShowEvent) TPGFEModuleEmulation::ECONTEmulation::convertToElinkData(refBx, vTcrdp, elpckt0_test); 
      // if(nEvents<=maxShowEvent)
      // 	for(int iel=0;iel<3;iel++)
      // 	  cout<<"iel: "<< iel
      // 	      << std::hex
      // 	      <<", word : 0x" << std::setfill('0') << setw(8) << elpckt0_test[iel]
      // 	      << std::dec << std::setfill(' ')
      // 	      <<endl;
      // /////////////////////////////////////////////////////////////////
      
      // //////////// Print unpacker output for ch 1 /////////////////////
      // iblock = 3;
      // int unpkBgnOffset = 0;
      // int unpkIndx = 0;
      // uint32_t unpackedWord[7][7];
      // uint32_t iunpkw = 0;
      // ibx = 0;
      // for(int iw = loc[iblock]+1; iw <= (loc[iblock]+size[iblock]) ; iw++ ){
      // 	uint32_t col0 = (p64[iw] >> (32+16)) & 0xFFFF ;
      // 	uint32_t col1 = (p64[iw] >> 32) & 0xFFFF ;
      // 	uint32_t col2 = (p64[iw] >> (32-16)) & 0xFFFF ;
      // 	uint32_t col3 = p64[iw] & 0xFFFF ;
      // 	if(unpkIndx>=unpkBgnOffset and (unpkIndx-unpkBgnOffset)%8==0) iunpkw=0;
      // 	if(unpkIndx>=unpkBgnOffset){
      // 	  unpackedWord[ibx][iunpkw] = col3; //BC6
      // 	  // unpackedWord[ibx][iunpkw] = col2; //STC4A
      // 	  // unpackedWord[ibx][iunpkw] = col1; //STC16
      // 	  iunpkw++;
      // 	}
      // 	if(nEvents<=maxShowEvent)
      // 	  cout<<"iloc: "<< iw
      // 	      << std::hex
      // 	      <<", col0 : 0x" << std::setfill('0') << setw(4) << col0 <<", "
      // 	      <<", col1 : 0x" << std::setfill('0') << setw(4) << col1 <<", "
      // 	      <<", col2 : 0x" << std::setfill('0') << setw(4) << col2 <<", "
      // 	      <<", col3 : 0x" << std::setfill('0') << setw(4) << col3 <<", "
      // 	      << std::dec << std::setfill(' ')
      // 	      <<endl;
	
      // 	unpkIndx++;
      // 	if((unpkIndx-unpkBgnOffset)%8==0) ibx++;
      // }
      // uint32_t modsum = 0xFF;
      // uint32_t unpkBx = 0xF;
      // uint32_t isValid = 0;
      // uint32_t energy, channel;
      // for(int ib=0;ib<7;ib++){
      // 	for(int iupw=0;iupw<7;iupw++){ //BC6 and STC4A
      // 	//for(int iupw=0;iupw<4;iupw++){ //STC16
      // 	  isValid = (unpackedWord[ib][iupw] >> 15) & 0x1;
      // 	  if(iupw==0){
      // 	    modsum = (unpackedWord[ib][iupw] >> 6) & 0xFF ;
      // 	    unpkBx = unpackedWord[ib][iupw] & 0xF ;
      // 	    unpackerData[ievent].econt_bxid[ib] = unpkBx;
      // 	    unpackerData[ievent].econt_modsum[ib] = modsum;
      // 	  }else{
      // 	    energy = (unpackedWord[ib][iupw] >> 6) & 0x1FF ;
      // 	    channel = unpackedWord[ib][iupw]  & 0x3F ;
      // 	    unpackerData[ievent].econt_energy[ib][iupw-1] = energy;
      // 	    unpackerData[ievent].econt_channel[ib][iupw-1] = channel;
      // 	  }
      // 	  if(nEvents<=maxShowEvent)
      // 	    if(iupw==0)
      // 	      cout<<"iupw: "<< iupw
      // 		  <<", word: 0x" << std::hex << std::setfill('0') << setw(4) <<unpackedWord[ib][iupw] << std::dec << std::setfill(' ')
      // 		  <<", bx: " << unpkBx <<", modsum: "<<modsum << endl;
      // 	    else
      // 	      cout<<"iupw: "<< iupw
      // 		  <<", word: 0x" << std::hex << std::setfill('0') << setw(4) <<unpackedWord[ib][iupw] << std::dec << std::setfill(' ')
      // 		  <<", energy: " << energy <<", channel: "<<channel << endl;
      // 	}//itc
      // }//ib
      // if(nEvents<=maxShowEvent) unpackerData[ievent].print("unpacked");
      // /////////////////////////////////////////////////////////////////
      
      if(nEvents<=maxShowEvent) cout<<"========= End of event : "<< nEvents << "============="<< endl;
      ievent++;
    }//loop event  
  }//while read r
  
  cout <<"Relay| Run| NofEvts| NofPhysT| NofCalT| NofCoinT| NofRandT| NofSoftT| NofRegT| RStrtE| RStpE| BoeE| EoeE| L1aE| EvtIdE| xscafeE| 1stcafeE|"<<endl;
  cout << relayNumber << "|"
       << runNumber << "|"
       << nEvents << "|"
       << total_phys_events << "|"
       << total_calib_events << "|"
       << total_coinc_events << "|"
       << total_random_events << "|"
       << total_soft_events << "|"
       << total_regular_events << "|"
       << nofRStartErrors << "|"
       << nofRStopErrors << "|"
       << nofBoEE << "|"
       << nofEoEE << "|"
       << nofL1aE << "|"
       << nofEventIdErrs << "|"
       << nofExcessFECAFEErrors << "|"
       << nofFirstFECAFEErrors << "|"
       <<endl;

  //   cout <<"Relay| Run| NofEvts| NofPhysT| NofCalT| NofCoinT| NofRandT| NofSoftT| NofRegT| RStrtE| RStpE| BoeE| EoeE| L1aE| EvtIdE| xscafeE| 1stcafeE| daqHE| NbxE| STCNumE| STCLocE| EngE| BxMME| BxCMME| nofSciCE| EmptyTCs| STCNumE1| STCLocE1| EngE1| BxMME1| BxCMME1| nofSciCE1| EmptyTCs1| PV|"<<endl;
  // cout << relayNumber << "|"
  //      << runNumber << "|"
  //      << nEvents << "|"
  //      << total_phys_events << "|"
  //      << total_calib_events << "|"
  //      << total_coinc_events << "|"
  //      << total_random_events << "|"
  //      << total_soft_events << "|"
  //      << total_regular_events << "|"
  //      << nofRStartErrors << "|"
  //      << nofRStopErrors << "|"
  //      << nofBoEE << "|"
  //      << nofEoEE << "|"
  //      << nofL1aE << "|"
  //      << nofEventIdErrs << "|"
  //      << nofExcessFECAFEErrors << "|" 
  //      << nofFirstFECAFEErrors << "|"
  //      << hDaqEvtMisMatch->GetEntries() << "|"
  //      << nofNbxMisMatches << "|"
  //      << nofEvcSTCNE << "|"
  //      << nofEvcSTCLE << "|"
  //      << nofEvcEngE << "|"
  //      << nofEvcBxE << "|"
  //      << nofEvcBxCE << "|"
  //      << nofSciCE << "|"
  //      << nofEmptyTCs << "|"
  //      << nofEvcSTCNE1 << "|"
  //      << nofEvcSTCLE1 << "|"
  //      << nofEvcEngE1 << "|"
  //      << nofEvcBxE1 << "|"
  //      << nofEvcBxCE1 << "|"
  //      << nofSciCE1 << "|"
  //      << nofEmptyTCs1 << "|"
  //      << std::hex << std::setfill('0')
  //      << "0x" << std::setw(4) << payload_version << "|"
  //      << std::dec << std::setfill(' ')
  //      <<endl;

  // for(int iev=0 ; iev < maxEvents ; iev++){
  //   cout<<"========= Compare event "<< iev << " elinks ============="<< endl;
  //   elinkData[iev].print(Form("Elinks: event: %d",iev));
  //   cout<<"========= Compare event "<< iev << " elinks ============="<< endl;
  // }
  
  // for(int iev=0 ; iev < maxEvents ; iev++){
  //   cout<<"========= Compare event "<< iev << " unpacker ============="<< endl;
  //   unpackerData[iev].print(Form("Unpacker: event: %d",iev));
  //   cout<<"========= Compare event "<< iev << " unpacker ============="<< endl;
  // }

  delete r;
  
  return 0;
}
