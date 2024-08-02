#include <iostream>
#include "TH1.h"
#include "TH2.h"

#include "TFile.h"
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
    std::cout << "EventId " << std::setw(3) << refEvent << " ";
    std::cout << "Word " << std::setw(3) << i << " ";
    std::cout << std::hex << std::setfill('0');
    std::cout << "0x" << std::setw(16) << p64[i] << std::endl;
    std::cout << std::dec << std::setfill(' ');
  }
  std::cout << std::endl;
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
  
  uint64_t maxShowEvent = 0;
  uint64_t nEvents = 0;    
  //uint64_t maxCAFESep = 6;
  const int maxCAFESeps = 7;
  
  uint64_t nofRStartErrors = 0, nofRStopErrors = 0;
  uint64_t nofBoEE = 0;
  uint64_t nofEoEE = 0;
  uint64_t nofL1aE = 0;
  uint64_t nofEventIdErrs = 0;
  uint64_t nofExcessFECAFEErrors = 0;  
  uint64_t nofFirstFECAFEErrors = 0;
  uint64_t nofBlockSizeErrors = 0;

  uint64_t total_phys_events = 0;
  uint64_t total_coinc_events = 0;
  uint64_t total_calib_events = 0;
  uint64_t total_random_events = 0;
  uint64_t total_soft_events = 0;
  uint64_t total_regular_events = 0;
  
  const int maxEvents = 200;
  const int nofEcontT = 3;
  const int noflpGBT = 2;
  BCEventData elinkData[noflpGBT][nofEcontT];
  BCEventData unpackerData[noflpGBT][nofEcontT];
  uint32_t nTC[nofEcontT] = {6, 6, 3};

  int econt01Diff[noflpGBT] ;
  int econt12Diff[noflpGBT] ;
  int econt20Diff[noflpGBT] ;
  
  uint64_t nofEcon012BxElinksErrors[noflpGBT] ;
  uint64_t nofEnErrors[noflpGBT][nofEcontT] ;
  uint64_t nofChErrors[noflpGBT][nofEcontT] ;
  uint64_t nofBxErrors[noflpGBT][nofEcontT] ;
  uint64_t nofMSErrors[noflpGBT][nofEcontT] ;
  uint64_t nofCBxErrors[noflpGBT][nofEcontT] ;        
  for(int ilp=0;ilp<noflpGBT;ilp++){
    nofEcon012BxElinksErrors[ilp] = 0;
    econt01Diff[ilp] = 0;
    econt12Diff[ilp] = 0;
    econt20Diff[ilp] = 0;
    for(int iecon=0;iecon<nofEcontT;iecon++){
      nofEnErrors[ilp][iecon] = 0 ;
      nofChErrors[ilp][iecon] = 0 ;
      nofBxErrors[ilp][iecon] = 0 ;
      nofMSErrors[ilp][iecon] = 0 ;
      nofCBxErrors[ilp][iecon] = 0 ;
    }
  }
  TH2F *bx_lp0_BCvsSTC4A = new TH2F("bx_lp0_BCvsSTC4A","BC6 vs STC4A",20,0,20, 20,0,20);
  TH2F *bx_lp0_BCvsSTC16 = new TH2F("bx_lp0_BCvsSTC16","BC6 vs STC16",20,0,20, 20,0,20);
  TH2F *bx_lp0_STC4AvsSTC16 = new TH2F("bx_lp0_STC4AvsSTC16","STC4A vs STC16",20,0,20, 20,0,20);
  TH2F *bx_lp0_IdmodvsBC = new TH2F("bx_lp0_IdmodvsBC","BxIdmod8 vs BC6",20,0,20, 20,0,20);
  TH2F *bx_lp0_IdmodvsSTC4A = new TH2F("bx_lp0_IdmodvsSTC4A","BxIdmod8 vs STC4A",20,0,20, 20,0,20);
  TH2F *bx_lp0_IdmodvsSTC16 = new TH2F("bx_lp0_IdmodvsSTC16","BxIdmod8 vs STC16",20,0,20, 20,0,20);

  TH2F *bx_lp1_BCvsSTC4A = new TH2F("bx_lp1_BCvsSTC4A","BC6 vs STC4A",20,0,20, 20,0,20);
  TH2F *bx_lp1_BCvsSTC16 = new TH2F("bx_lp1_BCvsSTC16","BC6 vs STC16",20,0,20, 20,0,20);
  TH2F *bx_lp1_STC4AvsSTC16 = new TH2F("bx_lp1_STC4AvsSTC16","STC4A vs STC16",20,0,20, 20,0,20);
  TH2F *bx_lp1_IdmodvsBC = new TH2F("bx_lp1_IdmodvsBC","BxIdmod8 vs BC6",20,0,20, 20,0,20);
  TH2F *bx_lp1_IdmodvsSTC4A = new TH2F("bx_lp1_IdmodvsSTC4A","BxIdmod8 vs STC4A",20,0,20, 20,0,20);
  TH2F *bx_lp1_IdmodvsSTC16 = new TH2F("bx_lp1_IdmodvsSTC16","BxIdmod8 vs STC16",20,0,20, 20,0,20);
  
  TH1F *bxen_lp0BC6 = new TH1F("bxen_lp0BC6","bxen_lp0BC6 (4E+3M)",7,-3,4);
  TH1F *bxen_lp0STC4 = new TH1F("bxen_lp0STC4","bxen_lp0STC4 (4E+3M)",7,-3,4);
  TH1F *bxen_lp0STC16 = new TH1F("bxen_lp0STC16","bxen_lp0STC16 (4E+3M)",7,-3,4);
  TH1F *bxen_lp1BC6 = new TH1F("bxen_lp1BC6","bxen_lp1BC6 (4E+3M)",7,-3,4);
  TH1F *bxen_lp1STC4 = new TH1F("bxen_lp1STC4","bxen_lp1STC4 (4E+3M)",7,-3,4);
  TH1F *bxen_lp1STC16 = new TH1F("bxen_lp1STC16","bxen_lp1STC16 (4E+3M)",7,-3,4);
  
  TH1F *en_unpacked_lp0BC6 = new TH1F("en_unpacked_lp0BC6","en_unpacked_lp0BC6",1000,0,1000);
  TH1F *en_unpacked_lp0STC4 = new TH1F("en_unpacked_lp0STC4","en_unpacked_lp0STC4",1000,0,1000);
  TH1F *en_unpacked_lp0STC16 = new TH1F("en_unpacked_lp0STC16","en_unpacked_lp0STC16",1000,0,1000);  
  TH1F *en_unpacked_lp1BC6 = new TH1F("en_unpacked_lp1BC6","en_unpacked_lp1BC6",1000,0,1000);
  TH1F *en_unpacked_lp1STC4 = new TH1F("en_unpacked_lp1STC4","en_unpacked_lp1STC4",1000,0,1000);
  TH1F *en_unpacked_lp1STC16 = new TH1F("en_unpacked_lp1STC16","en_unpacked_lp1STC16",1000,0,1000);
  
  TH2F *bx_BC6_lp0vs1 = new TH2F("bx_BC6_lp0vs1","BC6 lpGBT0 vs lpGBT1",20,0,20, 20,0,20);
  TH2F *bx_STC4_lp0vs1 = new TH2F("bx_STC4_lp0vs1","STC4 lpGBT0 vs lpGBT1",20,0,20, 20,0,20);
  TH2F *bx_STC16_lp0vs1 = new TH2F("bx_STC16_lp0vs1","STC16 lpGBT0 vs lpGBT1",20,0,20, 20,0,20);
  
  int ievent = 0;
  //Use the fileReader to read the records
  while(_fileReader.read(r)) {
    //Check the state of the record and print the record accordingly
    if( r->state()==Hgcal10gLinkReceiver::FsmState::Starting){
      if(!(rStart->valid())){
  	std::cerr <<"Relay: " << relayNumber << ", Run: "<< runNumber << " FsmState::Starting validadity fails : rStart->valid() " << rStart->valid() << std::endl;
  	rStart->print();
  	std::cout << std::endl;
	nofRStartErrors++;
	continue;
      }
    }
    
    else if(r->state()==Hgcal10gLinkReceiver::FsmState::Stopping){
      if(!(rStop->valid())){
  	std::cerr <<"Relay: " << relayNumber << ", Run: "<< runNumber << " FsmState::Stopping validadity fails : rStop->valid() " << rStop->valid() << std::endl;
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
      if(boe->boeHeader()!=boe->BoePattern) {
	std::cerr <<"Relay: " << relayNumber << ", Run: "<< runNumber << " Slink BoE header mismatch " << std::endl;
	nofBoEE++;
	continue;
      }
      if(eoe->eoeHeader()!=eoe->EoePattern) {
	std::cerr <<"Relay: " << relayNumber << ", Run: "<< runNumber << " Slink EoE header mismatch " << std::endl;
	nofEoEE++;
	continue;
      }
      
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
      if(nEvents<=maxShowEvent) cout << "L1aType : " << l1atype << ", bxId: " << bxId << endl;
      if(l1atype==0) { nofL1aE++; continue;}      

      if((Abs64(eventId,prevEvent) != Abs32(sequenceId, prevSequence)) and Abs64(eventId,prevEvent)>=2){
	std::cerr <<"Relay: " << relayNumber << ", Run: "<< runNumber << " Event: "<< eventId << ", l1aType : " << l1atype << ", and prevEvent  "<< prevEvent << ", nEvents : " << nEvents << " differs by "<< Abs64(eventId,prevEvent) <<" (sequence differs by [ "<< sequenceId << " - "<< prevSequence << " ] = " << Abs32(sequenceId, prevSequence) << "), EventID_Diff is more than 2 " << std::endl;
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
      
      int extra_cafe_word_loc = find_cafe_word(rEvent,(maxCAFESeps+1));
      if(extra_cafe_word_loc!=0){
	std::cerr <<"Relay: " << relayNumber << ", Run: "<< runNumber << " Event: " << eventId << " excess CAFE separator of " <<(maxCAFESeps+1)<< std::endl;
	nofExcessFECAFEErrors++ ;
	continue;
      }

      int first_cafe_word_loc = find_cafe_word(rEvent,1);
      if( first_cafe_word_loc != 2){
	std::cerr <<"Relay: " << relayNumber << ", Run: "<< runNumber << " Event: " << eventId << ", first_cafe_word_loc  "<< first_cafe_word_loc << std::endl;
	//Event_Dump(eventId, rEvent);
	nofFirstFECAFEErrors++ ;
	continue;
      }
      
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
      int loc[maxCAFESeps], size[maxCAFESeps];
      for(int iloc = 0 ; iloc < maxCAFESeps ; iloc++) {loc[iloc] = size[iloc] = -1;}
      cout<< std::dec << std::setfill(' ');
      for(int iloc = 0 ; iloc < maxCAFESeps ; iloc++){
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
      
      bool isExpectedBlockSize = true;
      for(int iloc = 0 ; iloc < maxCAFESeps ; iloc++) {
	bool checksize = false;
	if(iloc==(maxCAFESeps-1)){
	  int totsize = loc[iloc] + size[iloc] + 3 ;
	  checksize = (totsize==rEvent->payloadLength());
	  if(!checksize){
	    std::cerr <<"Relay: " << relayNumber << ", Run: "<< runNumber << " Event: "<< eventId << " has mismatch in last cafe position : " << iloc << ", size from block " << totsize << ", payload size " << rEvent->payloadLength() << std::endl;
	    // rEvent->RecordHeader::print();
	    // boe->print();
	    // eoe->print();
	    // Event_Dump(eventId, rEvent);
	    continue;
	  }
	}else{
	  int totsize = loc[iloc] + size[iloc] + 1;
	  checksize = (totsize==loc[iloc+1]);
	  if(!checksize){
	    std::cerr <<"Relay: " << relayNumber << ", Run: "<< runNumber << " Event: "<< eventId << " has mismatch in cafe position : " << iloc << std::endl;
	    //Event_Dump(eventId, rEvent);
	    continue;
	  }
	}
	if(!checksize) isExpectedBlockSize = false;
      }
      if(!isExpectedBlockSize){
	std::cerr <<"Relay: " << relayNumber << ", Run: "<< runNumber << " Event: "<< eventId << " has block size and location mismatch."<< std::endl;
	//Event_Dump(eventId, rEvent);
	nofBlockSizeErrors++ ;
	continue;
      }
      
      //if(nEvents==1) continue;
      //if(ievent>(maxEvents-1)) continue;
      if(nEvents<=maxShowEvent) cout<<"iEvent: " << ievent << endl;
      
      //////////// Read raw elink inputs for ch 1 /////////////////////
      int iblock = 1;
      int elBgnOffset = 0;
      int elIndx = 0;
      uint32_t elpckt[2][7][7]; //2:lpGBTs, the first 7 is for bx and second one for number of elinks
      uint32_t bx = 0xF;
      int iel = 0;
      int ibx = 0;
      for(int iw = loc[iblock]+1; iw <= (loc[iblock]+size[iblock]) ; iw++ ){
	uint32_t wMSB = (p64[iw] >> 32) & 0xFFFFFFFF ;
	uint32_t wLSB = p64[iw] & 0xFFFFFFFF ;
	if(elIndx>=elBgnOffset and (elIndx-elBgnOffset)%4==0) {
	  bx = (wMSB>>28) & 0xF ;
	  elinkData[0][0].econt_bxid[ibx] = bx;
	}
	if(elIndx>=elBgnOffset and (elIndx-elBgnOffset)%4==1) {
	  bx = (wLSB>>28) & 0xF ; //with (elIndx-elBgnOffset)%4==1,2 for STC4A and STC16
	  elinkData[0][1].econt_bxid[ibx] = bx;
	}
	if(elIndx>=elBgnOffset and (elIndx-elBgnOffset)%4==2) {
	  bx = (wLSB>>28) & 0xF ; //with (elIndx-elBgnOffset)%4==1,2 for STC4A and STC16
	  elinkData[0][2].econt_bxid[ibx] = bx;
	}
	if((elIndx-elBgnOffset)%4==0) iel = 0;
	if(elIndx>=elBgnOffset){
	  elpckt[0][ibx][iel] = wMSB;
	  if(iel<=6) elpckt[0][ibx][iel+1] = wLSB;
	  iel += 2;
	}//pick the first elink
	if(nEvents<=maxShowEvent)
	  cout<<"iloc: "<< iw << ", bx: " << bx << ", ibx: " << ibx
	      << std::hex
	      <<", MSB-word : 0x" << std::setfill('0') << setw(8) << wMSB
	      <<", LSB-word : 0x" << std::setfill('0') << setw(8) << wLSB
	      << std::dec << std::setfill(' ')
	      <<endl;
	elIndx++;
	if((elIndx-elBgnOffset)%4==0) ibx++;
      }
      /////////////////////////////////////////////////////////////////
      
      //////////// Read raw elink inputs for ch 2 /////////////////////
      iblock = 2;
      elBgnOffset = 0;
      elIndx = 0;
      bx = 0xF;
      iel = 0;
      ibx = 0;
      for(int iw = loc[iblock]+1; iw <= (loc[iblock]+size[iblock]) ; iw++ ){
	uint32_t wMSB = (p64[iw] >> 32) & 0xFFFFFFFF ;
	uint32_t wLSB = p64[iw] & 0xFFFFFFFF ;
	if(elIndx>=elBgnOffset and (elIndx-elBgnOffset)%4==0) {
	  bx = (wMSB>>28) & 0xF ;
	  elinkData[1][0].econt_bxid[ibx] = bx;
	}
	if(elIndx>=elBgnOffset and (elIndx-elBgnOffset)%4==1) {
	  bx = (wLSB>>28) & 0xF ; //with (elIndx-elBgnOffset)%4==1,2 for STC4A and STC16
	  elinkData[1][1].econt_bxid[ibx] = bx;
	}
	if(elIndx>=elBgnOffset and (elIndx-elBgnOffset)%4==2) {
	  bx = (wLSB>>28) & 0xF ; //with (elIndx-elBgnOffset)%4==1,2 for STC4A and STC16
	  elinkData[1][2].econt_bxid[ibx] = bx;
	}
	if((elIndx-elBgnOffset)%4==0) iel = 0;
	if(elIndx>=elBgnOffset){
	  elpckt[1][ibx][iel] = wMSB;
	  if(iel<=6) elpckt[1][ibx][iel+1] = wLSB;
	  iel += 2;
	}//pick the first elink
	if(nEvents<=maxShowEvent)
	  cout<<"iloc: "<< iw << ", bx: " << bx << ", ibx: " << ibx
	      << std::hex
	      <<", MSB-word : 0x" << std::setfill('0') << setw(8) << wMSB
	      <<", LSB-word : 0x" << std::setfill('0') << setw(8) << wLSB
	      << std::dec << std::setfill(' ')
	      <<endl;
	elIndx++;
	if((elIndx-elBgnOffset)%4==0) ibx++;
      }
      
      for(int ib = 0; ib < 7 ;ib++){
	bx_lp0_BCvsSTC4A->Fill(float(elinkData[0][0].econt_bxid[ib]), float(elinkData[0][1].econt_bxid[ib]));
	bx_lp0_BCvsSTC16->Fill(float(elinkData[0][0].econt_bxid[ib]), float(elinkData[0][2].econt_bxid[ib]));
	bx_lp0_STC4AvsSTC16->Fill(float(elinkData[0][1].econt_bxid[ib]), float(elinkData[0][2].econt_bxid[ib]));

	bx_lp1_BCvsSTC4A->Fill(float(elinkData[1][0].econt_bxid[ib]), float(elinkData[1][1].econt_bxid[ib]));
	bx_lp1_BCvsSTC16->Fill(float(elinkData[1][0].econt_bxid[ib]), float(elinkData[1][2].econt_bxid[ib]));
	bx_lp1_STC4AvsSTC16->Fill(float(elinkData[1][1].econt_bxid[ib]), float(elinkData[1][2].econt_bxid[ib]));
	
	if(ib==3){
	  uint32_t bxmodulo = (bxId==3564) ? 0xF : (bxId%8) ;
	  bx_lp0_IdmodvsBC->Fill(float(bxmodulo), float(elinkData[0][0].econt_bxid[ib]));
	  bx_lp0_IdmodvsSTC4A->Fill(float(bxmodulo), float(elinkData[0][1].econt_bxid[ib]));
	  bx_lp0_IdmodvsSTC16->Fill(float(bxmodulo), float(elinkData[0][2].econt_bxid[ib]));

	  bx_lp1_IdmodvsBC->Fill(float(bxmodulo), float(elinkData[1][0].econt_bxid[ib]));
	  bx_lp1_IdmodvsSTC4A->Fill(float(bxmodulo), float(elinkData[1][1].econt_bxid[ib]));
	  bx_lp1_IdmodvsSTC16->Fill(float(bxmodulo), float(elinkData[1][2].econt_bxid[ib]));
	  
	  bx_BC6_lp0vs1->Fill(float(elinkData[0][0].econt_bxid[ib]), float(elinkData[1][0].econt_bxid[ib]));
	  bx_STC4_lp0vs1->Fill(float(elinkData[0][1].econt_bxid[ib]), float(elinkData[1][1].econt_bxid[ib]));
	  bx_STC16_lp0vs1->Fill(float(elinkData[0][2].econt_bxid[ib]), float(elinkData[1][2].econt_bxid[ib]));
	}
      }
      
      // /////////////////////////////////////////////////////////////////
      // //Find the bx diffrence between the 
      // if(ievent==0) {
      // 	econt01Diff[0] = elinkData[0][0].econt_bxid[0] - elinkData[0][1].econt_bxid[0] ;
      // 	econt12Diff[0] = elinkData[0][1].econt_bxid[0] - elinkData[0][2].econt_bxid[0] ;
      // 	econt20Diff[0] = elinkData[0][2].econt_bxid[0] - elinkData[0][0].econt_bxid[0] ;
      // 	econt01Diff[1] = elinkData[1][0].econt_bxid[0] - elinkData[1][1].econt_bxid[0] ;
      // 	econt12Diff[1] = elinkData[1][1].econt_bxid[0] - elinkData[1][2].econt_bxid[0] ;
      // 	econt20Diff[1] = elinkData[1][2].econt_bxid[0] - elinkData[1][0].econt_bxid[0] ;
      // }
      // /////////////////////////////////////////////////////////////////
      
      ////// Print the energy/location and stage1 input also check back elink//////
      if(nEvents<=maxShowEvent)
	for(int ib=0;ib<7;ib++)
	  for(int iel=0;iel<7;iel++)
	    cout<<"ibx: "<< ib <<", elIndx: "<< iel
		<< std::hex
		<<", word : 0x" << std::setfill('0') << setw(8) << elpckt[0][ib][iel]
		<< std::dec << std::setfill(' ')
		<<endl;
      for(int ilp=0;ilp<2;ilp++){
	for(int ib=0;ib<7;ib++){
	  TPGFEDataformat::TcRawDataPacket vTC1, vTC2, vTC3;
	  TPGBEDataformat::UnpackerOutputStreamPair up1, up2,up3;

	  TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::TcRawData::BestC, nTC[0], elpckt[ilp][ib], vTC1);
	  TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(elinkData[ilp][0].econt_bxid[ib], vTC1, up1);

	  TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::TcRawData::STC4A, nTC[1], &elpckt[ilp][ib][3], vTC2);
	  TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(elinkData[ilp][1].econt_bxid[ib], vTC2, up2);
	
	  TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::TcRawData::STC16, nTC[2], &elpckt[ilp][ib][5], vTC3);
	  TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(elinkData[ilp][2].econt_bxid[ib], vTC3, up3);
	  
	  //TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(, vTcrdp);
	  
	  const std::vector<TPGFEDataformat::TcRawData> &vTc1(vTC1.second);
	  const std::vector<TPGFEDataformat::TcRawData> &vTc2(vTC2.second);
	  const std::vector<TPGFEDataformat::TcRawData> &vTc3(vTC3.second);
	  elinkData[ilp][0].econt_modsum[ib] = uint32_t(up1.moduleSum(0));
	  elinkData[ilp][0].econt_bxid[ib] = uint32_t(up1.bx(0));
	  for(int itc=0;itc<nTC[0];itc++){ //for BC and STC4A
	    elinkData[ilp][0].econt_energy[ib][itc] = uint32_t(up1.channelEnergy(0,itc));
	    elinkData[ilp][0].econt_channel[ib][itc] = uint32_t(up1.channelNumber(0,itc));
	    // TH1F *bxen_lp0BC6 = new TH1F("bxen_lp0BC6","bxen_lp0BC6 (4E+3M)",7,-3,4);
	    // TH1F *bxen_lp0STC4 = new TH1F("bxen_lp0STC4","bxen_lp0STC4 (4E+3M)",7,-3,4);
	    // TH1F *bxen_lp0STC166 = new TH1F("bxen_lp0STC166","bxen_lp0STC166 (4E+3M)",7,-3,4);
	    if(ilp==0){
	      bxen_lp0BC6->SetBinContent(ib+1,bxen_lp0BC6->GetBinContent(ib+1)+ (vTc1[itc+1].energy()));
	    }else{
	      bxen_lp1BC6->SetBinContent(ib+1,bxen_lp1BC6->GetBinContent(ib+1)+ (vTc1[itc+1].energy()));
	    }
	  }//itc

	  elinkData[ilp][1].econt_modsum[ib] = uint32_t(up2.moduleSum(0));
	  elinkData[ilp][1].econt_bxid[ib] = uint32_t(up2.bx(0));
	  for(int itc=0;itc<nTC[1];itc++){ //for BC and STC4A
	    elinkData[ilp][1].econt_energy[ib][itc] = uint32_t(up2.channelEnergy(0,itc));
	    elinkData[ilp][1].econt_channel[ib][itc] = uint32_t(up2.channelNumber(0,itc));
	    if(ilp==0){
	      bxen_lp0STC4->SetBinContent(ib+1,bxen_lp0STC4->GetBinContent(ib+1)+ (vTc2[itc+1].energy()));
	    }else{
	      bxen_lp1STC4->SetBinContent(ib+1,bxen_lp1STC4->GetBinContent(ib+1)+ (vTc2[itc+1].energy()));
	    }
	  }//itc
	  
	  elinkData[ilp][2].econt_modsum[ib] = 1;//uint32_t(up3.moduleSum(0));
	  elinkData[ilp][2].econt_bxid[ib] = uint32_t(up3.bx(0));
	  for(int itc=0;itc<nTC[2];itc++){
	    elinkData[ilp][2].econt_energy[ib][itc] = uint32_t(up3.channelEnergy(0,itc));
	    elinkData[ilp][2].econt_channel[ib][itc] = uint32_t(up3.channelNumber(0,itc));
	    if(ilp==0){
	      bxen_lp0STC16->SetBinContent(ib+1,bxen_lp0STC16->GetBinContent(ib+1)+(vTc3[itc+1].energy()));
	    }else{
	      bxen_lp1STC16->SetBinContent(ib+1,bxen_lp1STC16->GetBinContent(ib+1)+(vTc3[itc+1].energy()));
	    }
	  }//itc
	  
	  if(nEvents<=maxShowEvent) {
	    up1.print();
	    up2.print();
	    up3.print();
	  }
	}//ib
      }//lpGBT loop
      
      // if(nEvents<=maxShowEvent) {
      // 	elinkData[0][0].print(nTC[0], "TC", "elinks");
      // 	elinkData[0][1].print(nTC[1], "STC4A", "elinks");
      // 	elinkData[0][2].print(nTC[2], "STC16", "elinks");
      // }
      
      // // uint32_t elpckt0_test[3];
      // // if(nEvents<=maxShowEvent) TPGFEModuleEmulation::ECONTEmulation::convertToElinkData(refBx, vTcrdp, elpckt0_test); 
      // // if(nEvents<=maxShowEvent)
      // // 	for(int iel=0;iel<3;iel++)
      // // 	  cout<<"iel: "<< iel
      // // 	      << std::hex
      // // 	      <<", word : 0x" << std::setfill('0') << setw(8) << elpckt0_test[iel]
      // // 	      << std::dec << std::setfill(' ')
      // // 	      <<endl;
      // // /////////////////////////////////////////////////////////////////
      
      //////////// Print unpacker output for ch 1 /////////////////////
      iblock = 3;
      int unpkBgnOffset = 0;
      int unpkIndx = 0;
      uint32_t unpackedWord[2][3][7][8]; //2:lpGBT, 3:ECON-T, 7:bxs,8:words
      uint32_t iunpkw = 0;
      ibx = 0;
      for(int iw = loc[iblock]+1; iw <= (loc[iblock]+size[iblock]) ; iw++ ){
	uint32_t col0 = (p64[iw] >> (32+16)) & 0xFFFF ;
	uint32_t col1 = (p64[iw] >> 32) & 0xFFFF ;
	uint32_t col2 = (p64[iw] >> (32-16)) & 0xFFFF ;
	uint32_t col3 = p64[iw] & 0xFFFF ;
	if(unpkIndx>=unpkBgnOffset and (unpkIndx-unpkBgnOffset)%8==0) iunpkw=0;
	if(unpkIndx>=unpkBgnOffset){
	  unpackedWord[0][0][ibx][iunpkw] = col3; //BC6
	  unpackedWord[0][1][ibx][iunpkw] = col2; //STC4A
	  unpackedWord[0][2][ibx][iunpkw] = col1; //STC16
	  iunpkw++;
	}
	if(nEvents<=maxShowEvent)
	  cout<<"iloc: "<< iw
	      << std::hex
	      <<", col0 : 0x" << std::setfill('0') << setw(4) << col0 <<", "
	      <<", col1 : 0x" << std::setfill('0') << setw(4) << col1 <<", "
	      <<", col2 : 0x" << std::setfill('0') << setw(4) << col2 <<", "
	      <<", col3 : 0x" << std::setfill('0') << setw(4) << col3 <<", "
	      << std::dec << std::setfill(' ')
	      <<endl;
	
	unpkIndx++;
	if(unpkIndx>0 and (unpkIndx-unpkBgnOffset)%8==0) ibx++;
      }
      /////////////////////////////////////////////////////////////////

      //////////// Print unpacker output for ch 2 /////////////////////
      iblock = 4;
      unpkBgnOffset = 0;
      unpkIndx = 0;
      iunpkw = 0;
      ibx = 0;
      for(int iw = loc[iblock]+1; iw <= (loc[iblock]+size[iblock]) ; iw++ ){
	uint32_t col0 = (p64[iw] >> (32+16)) & 0xFFFF ;
	uint32_t col1 = (p64[iw] >> 32) & 0xFFFF ;
	uint32_t col2 = (p64[iw] >> (32-16)) & 0xFFFF ;
	uint32_t col3 = p64[iw] & 0xFFFF ;
	if(unpkIndx>=unpkBgnOffset and (unpkIndx-unpkBgnOffset)%8==0) iunpkw=0;
	if(unpkIndx>=unpkBgnOffset){
	  unpackedWord[1][0][ibx][iunpkw] = col3; //BC6
	  unpackedWord[1][1][ibx][iunpkw] = col2; //STC4A
	  unpackedWord[1][2][ibx][iunpkw] = col1; //STC16
	  iunpkw++;
	}
	if(nEvents<=maxShowEvent)
	  cout<<"iloc: "<< iw
	      << std::hex
	      <<", col0 : 0x" << std::setfill('0') << setw(4) << col0 <<", "
	      <<", col1 : 0x" << std::setfill('0') << setw(4) << col1 <<", "
	      <<", col2 : 0x" << std::setfill('0') << setw(4) << col2 <<", "
	      <<", col3 : 0x" << std::setfill('0') << setw(4) << col3 <<", "
	      << std::dec << std::setfill(' ')
	      <<endl;
	
	unpkIndx++;
	if(unpkIndx>0 and (unpkIndx-unpkBgnOffset)%8==0) ibx++;
      }
      /////////////////////////////////////////////////////////////////
      // for(int ilp=0;ilp<2;ilp++){
      // 	for(int iecon=0;iecon<3;iecon++){
      // 	  for(int ib=0;ib<7;ib++){
      // 	    for(int iupw=0;iupw<8;iupw++){ //BC6 and STC4A
      // 	      if(nEvents<=maxShowEvent)
      // 		cout<<"iloc: ("<<ilp<<"," << iecon << "," << ib << "," << iupw << ")"
      // 		    << std::hex
      // 		    <<", unpackedWord: 0x" << std::setfill('0') << setw(4) << unpackedWord[ilp][iecon][ib][iupw] <<", "
      // 		    << std::dec << std::setfill(' ')
      // 		    <<endl;
      // 	    }
      // 	  }
      // 	}
      // }
      // if(nEvents<=maxShowEvent) {
      // 	for(int ilp=0;ilp<2;ilp++){
      // 	  cout<<"ilp: "<<ilp<< ", Diff01 "<<econt01Diff[ilp]<<endl;
      // 	  cout<<"ilp: "<<ilp<< ", Diff12 "<<econt12Diff[ilp]<<endl;
      // 	  cout<<"ilp: "<<ilp<< ", Diff20 "<<econt20Diff[ilp]<<endl;
      // 	}
      // }
      uint32_t modsum = 0xFF;
      uint32_t unpkBx = 0xF;
      uint32_t isValid = 0;
      uint32_t energy, channel;
      for(int ilp=0;ilp<2;ilp++){
	for(int iecon=0;iecon<3;iecon++){
	for(int ib=0;ib<7;ib++){
	    for(int iupw=0;iupw<8;iupw++){ //BC6 and STC4A
	      //for(int iupw=0;iupw<4;iupw++){ //STC16
	      //if(iupw > (nTC[iecon]+1)) continue;
	      isValid = (unpackedWord[ilp][iecon][ib][iupw] >> 15) & 0x1;
	      if(iupw==0){
		modsum = (unpackedWord[ilp][iecon][ib][iupw] >> 6) & 0xFF ;
		unpkBx = unpackedWord[ilp][iecon][ib][iupw] & 0xF ;
		if(isValid){
		  unpackerData[ilp][iecon].econt_bxid[ib] = unpkBx;
		  unpackerData[ilp][iecon].econt_modsum[ib] = modsum;
		}
	      }else{
		energy = (unpackedWord[ilp][iecon][ib][iupw] >> 6) & 0x1FF ;
		channel = unpackedWord[ilp][iecon][ib][iupw]  & 0x3F ;
		if(isValid){
		  unpackerData[ilp][iecon].econt_energy[ib][iupw-1] = energy;
		  unpackerData[ilp][iecon].econt_channel[ib][iupw-1] = channel;
		  if(ilp==0){
		    if(iecon==0) en_unpacked_lp0BC6->Fill(energy);
		    if(iecon==1) en_unpacked_lp0STC4->Fill(energy);
		    if(iecon==2) en_unpacked_lp0STC16->Fill(energy);
		  }
		  if(ilp==1){
		    if(iecon==0) en_unpacked_lp1BC6->Fill(energy);
		    if(iecon==1) en_unpacked_lp1STC4->Fill(energy);
		    if(iecon==2) en_unpacked_lp1STC16->Fill(energy);
		  }
		}
	      }
	      if(nEvents<=maxShowEvent)
		if(iupw==0)
		  cout<<"iupw: "<< iupw
		      <<", word: 0x" << std::hex << std::setfill('0') << setw(4) <<unpackedWord[ilp][iecon][ib][iupw] << std::dec << std::setfill(' ')
		      <<", bx: " << unpkBx <<", modsum: "<<modsum << ", isValid: "<< isValid << endl;
		else
		  cout<<"iupw: "<< iupw
		      <<", word: 0x" << std::hex << std::setfill('0') << setw(4) <<unpackedWord[ilp][iecon][ib][iupw] << std::dec << std::setfill(' ')
		      <<", energy: " << energy <<", channel: "<<channel << ", isValid: "<< isValid << endl;
	    }//itc
	  }//ib
	}//iecon
      }//ilp
      // if(nEvents<=maxShowEvent) {
      // 	unpackerData[0][0].print(nTC[0],"TC","unpacked");
      // 	unpackerData[0][1].print(nTC[1],"STC4A","unpacked");
      // 	unpackerData[0][2].print(nTC[2],"STC16","unpacked");
      // }
      // /////////////////////////////////////////////////////////////////
      // for(int ilp=0;ilp<noflpGBT;ilp++){
      // 	for(int iecon=0;iecon<nofEcontT;iecon++){
      // 	  for(int ib=0;ib<7;ib++){
      // 	    for(int iupw=0;iupw<=nTC[iecon];iupw++){ //BC6 and STC4A
      // 	      if(iupw==0){
      // 		if(elinkData[ilp][iecon].econt_bxid[ib] != unpackerData[ilp][iecon].econt_bxid[ib]) nofBxErrors[ilp][iecon]++;
      // 		if(elinkData[ilp][iecon].econt_modsum[ib] != unpackerData[ilp][iecon].econt_modsum[ib]) nofMSErrors[ilp][iecon]++;
      // 	      }else{
      // 		if(elinkData[ilp][iecon].econt_energy[ib][iupw-1] != unpackerData[ilp][iecon].econt_energy[ib][iupw-1]) nofEnErrors[ilp][iecon]++;
      // 		if(elinkData[ilp][iecon].econt_channel[ib][iupw-1] != unpackerData[ilp][iecon].econt_channel[ib][iupw-1]) nofChErrors[ilp][iecon]++;
      // 	      }
      // 	    }//itc
      // 	  }//ib
      // 	  // if(bxId==3564)
      // 	  //   if(elinkData[ilp][iecon].econt_bxid[3] != 0xF) nofCBxErrors[ilp][iecon]++ ;
      // 	  // else
      // 	  //   if(bxId%8 != elinkData[ilp][iecon].econt_bxid[3]) nofCBxErrors[ilp][iecon]++ ;
      // 	}//iecon
	
      // 	// if((elinkData[ilp][0].econt_bxid[0]-elinkData[ilp][1].econt_bxid[0])!=econt01Diff[ilp]
      // 	//    or
      // 	//    (elinkData[ilp][1].econt_bxid[0]-elinkData[ilp][2].econt_bxid[0])!=econt12Diff[ilp]
      // 	//    or
      // 	//    (elinkData[ilp][2].econt_bxid[0]-elinkData[ilp][0].econt_bxid[0])!=econt01Diff[ilp]
      // 	//    )
      // 	//   nofEcon012BxElinksErrors[ilp]++;
      // }//ilp
      
      // if(nEvents<=maxShowEvent) {
      // 	elinkData[0][0].print(nTC[0], "TC", Form("Elinks0: event: %d",ievent));
      // 	elinkData[0][1].print(nTC[1], "STC4A", Form("Elinks0: event: %d",ievent));
      // 	elinkData[0][2].print(nTC[2], "STC16", Form("Elinks0: event: %d",ievent));
      // 	elinkData[1][0].print(nTC[0], "TC", Form("Elinks1: event: %d",ievent));
      // 	elinkData[1][1].print(nTC[1], "STC4A", Form("Elinks1: event: %d",ievent));
      // 	elinkData[1][2].print(nTC[2], "STC16", Form("Elinks1: event: %d",ievent));
      //}
      ////////////////// Validate Energy Channel /////////////////////
      if(nEvents<=maxShowEvent) cout<<"========= End of event : "<< nEvents << "============="<< endl;
      ievent++;
    }//loop event  
  }//while read r

  bx_lp0_IdmodvsBC->GetXaxis()->SetTitle("Bx mod 8");
  bx_lp0_IdmodvsSTC4A->GetXaxis()->SetTitle("Bx mod 8");
  bx_lp0_IdmodvsSTC16->GetXaxis()->SetTitle("Bx mod 8");
  bx_lp0_IdmodvsBC->GetYaxis()->SetTitle("Bx (BC6 of lpGBT0)");
  bx_lp0_IdmodvsSTC4A->GetYaxis()->SetTitle("Bx (STC4 of lpGBT0)");
  bx_lp0_IdmodvsSTC16->GetYaxis()->SetTitle("Bx (STC16 of lpGBT0)");
  
  bx_lp0_BCvsSTC4A->GetXaxis()->SetTitle("Bx (BC6 of lpGBT0)");
  bx_lp0_BCvsSTC16->GetXaxis()->SetTitle("Bx (BC6 of lpGBT0)");
  bx_lp0_STC4AvsSTC16->GetXaxis()->SetTitle("Bx (STC4 of lpGBT0)");
  bx_lp0_BCvsSTC4A->GetYaxis()->SetTitle("Bx (STC4 of lpGBT0)");
  bx_lp0_BCvsSTC16->GetYaxis()->SetTitle("Bx (STC16 of lpGBT0)");
  bx_lp0_STC4AvsSTC16->GetYaxis()->SetTitle("Bx (STC16 of lpGBT0)");
  
  bxen_lp0BC6->GetXaxis()->SetTitle("Bx (lpGBT0)");
  bxen_lp0STC4->GetXaxis()->SetTitle("Bx (lpGBT0)");
  bxen_lp0STC16->GetXaxis()->SetTitle("Bx (lpGBT0)");
  bxen_lp0BC6->GetYaxis()->SetTitle("Energy(4E+3M)");
  bxen_lp0STC4->GetYaxis()->SetTitle("Energy(4E+3M)");
  bxen_lp0STC16->GetYaxis()->SetTitle("Energy(4E+3M)");

  bx_BC6_lp0vs1->GetXaxis()->SetTitle("Bx (BC6 of lpGBT0)");
  bx_BC6_lp0vs1->GetYaxis()->SetTitle("Bx (BC6 of lpGBT1)");
  bx_STC4_lp0vs1->GetXaxis()->SetTitle("Bx (STC4 of lpGBT0)");
  bx_STC4_lp0vs1->GetYaxis()->SetTitle("Bx (STC4 of lpGBT1)");
  bx_STC16_lp0vs1->GetXaxis()->SetTitle("Bx (STC16 of lpGBT0)");
  bx_STC16_lp0vs1->GetYaxis()->SetTitle("Bx (STC16 of lpGBT1)");
  
  bx_lp1_IdmodvsBC->GetXaxis()->SetTitle("Bx mod 8");
  bx_lp1_IdmodvsSTC4A->GetXaxis()->SetTitle("Bx mod 8");
  bx_lp1_IdmodvsSTC16->GetXaxis()->SetTitle("Bx mod 8");
  bx_lp1_IdmodvsBC->GetYaxis()->SetTitle("Bx (BC6 of lpGBT1)");
  bx_lp1_IdmodvsSTC4A->GetYaxis()->SetTitle("Bx (STC4 of lpGBT1)");
  bx_lp1_IdmodvsSTC16->GetYaxis()->SetTitle("Bx (STC16 of lpGBT1)");
  
  bx_lp1_BCvsSTC4A->GetXaxis()->SetTitle("Bx (BC6 of lpGBT1)");
  bx_lp1_BCvsSTC16->GetXaxis()->SetTitle("Bx (BC6 of lpGBT1)");
  bx_lp1_STC4AvsSTC16->GetXaxis()->SetTitle("Bx (STC4 of lpGBT1)");
  bx_lp1_BCvsSTC4A->GetYaxis()->SetTitle("Bx (STC4 of lpGBT1)");
  bx_lp1_BCvsSTC16->GetYaxis()->SetTitle("Bx (STC16 of lpGBT1)");
  bx_lp1_STC4AvsSTC16->GetYaxis()->SetTitle("Bx (STC16 of lpGBT1)");

  bxen_lp1BC6->GetXaxis()->SetTitle("Bx (lpGBT1)");
  bxen_lp1STC4->GetXaxis()->SetTitle("Bx (lpGBT1)");
  bxen_lp1STC16->GetXaxis()->SetTitle("Bx (lpGBT1)");
  bxen_lp1BC6->GetYaxis()->SetTitle("Energy(4E+3M)");
  bxen_lp1STC4->GetYaxis()->SetTitle("Energy(4E+3M)");
  bxen_lp1STC16->GetYaxis()->SetTitle("Energy(4E+3M)");
  
  en_unpacked_lp0BC6->GetXaxis()->SetTitle("Energy (BC6 of lpGBT0 [5E+4M])");
  en_unpacked_lp0STC4->GetXaxis()->SetTitle("Energy (STC4A of lpGBT0 [5E+4M])");
  en_unpacked_lp0STC16->GetXaxis()->SetTitle("Energy (STC16 of lpGBT0 [5E+4M])");
  en_unpacked_lp1BC6->GetXaxis()->SetTitle("Energy (BC6 of lpGBT1 [5E+4M])");
  en_unpacked_lp1STC4->GetXaxis()->SetTitle("Energy (STC4A of lpGBT1 [5E+4M])");
  en_unpacked_lp1STC16->GetXaxis()->SetTitle("Energy (STC16 of lpGBT1 [5E+4M])");

  TFile *fout = new TFile(Form("out_%u.root",runNumber),"recreate");
  bx_lp0_IdmodvsBC->Write();
  bx_lp0_IdmodvsSTC4A->Write();
  bx_lp0_IdmodvsSTC16->Write();
  bx_lp0_BCvsSTC4A->Write();
  bx_lp0_BCvsSTC16->Write();
  bx_lp0_STC4AvsSTC16->Write();  
  en_unpacked_lp0BC6->Write();
  en_unpacked_lp0STC4->Write();
  en_unpacked_lp0STC16->Write();
  bxen_lp0BC6->Write();
  bxen_lp0STC4->Write();
  bxen_lp0STC16->Write();
  
  bx_BC6_lp0vs1->Write();
  bx_STC4_lp0vs1->Write();
  bx_STC16_lp0vs1->Write();
  
  bx_lp1_IdmodvsBC->Write();
  bx_lp1_IdmodvsSTC4A->Write();
  bx_lp1_IdmodvsSTC16->Write();
  bx_lp1_BCvsSTC4A->Write();
  bx_lp1_BCvsSTC16->Write();
  bx_lp1_STC4AvsSTC16->Write();
  en_unpacked_lp1BC6->Write();
  en_unpacked_lp1STC4->Write();
  en_unpacked_lp1STC16->Write();
  bxen_lp1BC6->Write();
  bxen_lp1STC4->Write();
  bxen_lp1STC16->Write();
  fout->Close();
  delete fout;


  delete r;
  
  return 0;
}
