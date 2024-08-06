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

// print all (32-bit) words in event
void Event_Dump32(uint64_t refEvent, const Hgcal10gLinkReceiver::RecordRunning *rEvent){
  const uint32_t *p32(((const uint32_t*)rEvent));
  for(unsigned i(0);i<2*rEvent->payloadLength();i++){
    std::cout << "EventId " << std::setw(3) << refEvent << " ";
    std::cout << "Word " << std::setw(3) << i << " ";
    std::cout << std::hex << std::setfill('0');
    std::cout << "0x" << std::setw(8) << p32[i] << std::endl;
    std::cout << std::dec << std::setfill(' ');
  }
  std::cout << std::endl;
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

bool is_ffsep_word(const uint64_t word, int& isMSB) {

  if( (word & 0xFFFFFFFF) == 0xFFFFFFFF){
    isMSB = 0;
    return true; //0xFFFF 0xFFFF
  }else if((word >> 32) == 0xFFFFFFFF){
    isMSB = 1;
    return true; //0xFFFF 0xFFFF
  }else{
    isMSB = -1;
    return false;
  }
}

// returns location in this event of the n'th 0xfecafe... line
int find_ffsep_word(const Hgcal10gLinkReceiver::RecordRunning *rEvent, int& isMSB, int n, int ffsep_word_loc = -1) {
  const uint64_t *p64(((const uint64_t*)rEvent)+1);      
  if (ffsep_word_loc > 0) {
    if (is_ffsep_word(p64[ffsep_word_loc], isMSB)) {
      return ffsep_word_loc;
    }else
      return ffsep_word_loc;
  } 
  else {
    ;
  }
 
  int sep_counter = 0;
  int sep_word_idx = -1;
  for(uint32_t i(0);i<rEvent->payloadLength();i++){
    const uint64_t word = p64[i];
    if (is_ffsep_word(word,isMSB)) { // if word == 0xfeca
      sep_counter++;       
      if (sep_counter == n){                                                                                    
	sep_word_idx = i;
	break;
      }
    }
  }

  if (sep_word_idx == -1) {
    //std::cerr << "Could not find sep word" << std::endl;
    return 0;
  }else {
    return sep_word_idx;
  }
}

bool is_empty_word(const uint64_t word, int& isMSB) {
  if( (word & 0xFFFFFFFF) == 0x0){
    isMSB = 0;
    return true; 
  }else if((word >> 32) == 0x0){
    isMSB = 1;
    return true; 
  }else{
    isMSB = -1;
    return false;
  }
}

bool found_empty_word(const Hgcal10gLinkReceiver::RecordRunning *rEvent, int& isMSB, int ffsep_word_loc = -1) {
  const uint64_t *p64(((const uint64_t*)rEvent)+1);
  if (ffsep_word_loc > 0) 
    if (is_empty_word(p64[ffsep_word_loc], isMSB)) 
      return true;
    else
      return false;
  else
    return false;
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

void printEconDEvents(uint64_t nEvents, uint64_t  maxShowEvent, std::map<uint32_t,std::vector<uint32_t>>& econheaderpos_lst, std:: map<uint32_t,std::vector<uint32_t>>& zeropos_lst, const Hgcal10gLinkReceiver::RecordRunning *rEvent){

  const uint64_t *p64(((const uint64_t*)rEvent)+1);
  const uint32_t *p32(((const uint32_t*)rEvent)+2);
  int index = 3;
  int nRx = 6;
  bool isTOTUP = false;
  uint32_t rocn = 0;
  uint32_t half = 0;
  if(nEvents<=maxShowEvent){
    cout<< std::hex   ;
    cout<<"word-64 0th : 0x"<< std::setfill('0') << setw(16) << p64[index] ;
    cout<<", word-32 0th : 0x"<< std::setfill('0') << setw(8) << p32[2*index+1] ;
    cout<<", 0x" << p32[2*index] << endl;
    cout<< std::dec << std::setfill(' ') ;
  }
  for (const auto&  econhpos : econheaderpos_lst){
    uint32_t icapblk = econhpos.first;
    vector<uint32_t> econhloc = econhpos.second;
    for(uint32_t iecond = 0 ; iecond < econhloc.size() ; iecond++){
      cout<< std::hex ;
      if(nEvents<=maxShowEvent) cout<<"For econ " << iecond << ", word is 0x"<< std::setfill('0') << setw(16) <<p64[econhloc.at(iecond)]<<endl;
      if(nEvents<=maxShowEvent) cout<<"For econ " << iecond << ", zero word is 0x"<< std::setfill('0') << setw(16) <<p64[zeropos_lst[icapblk].at(iecond)]<<endl;
      cout<< std::dec << std::setfill(' ') ;
      if(nEvents<=maxShowEvent) cout<<"econd words position for econ " << iecond << ", is "<< std::setfill('0') << setw(2) <<econhloc.at(iecond)<<endl;
      if(nEvents<=maxShowEvent) cout<<"zero words position for econ " << iecond << ", is "<< std::setfill('0') << setw(2) << zeropos_lst[icapblk].at(iecond)  <<endl;
      for(int iw=econhloc.at(iecond)+1 ; iw <= zeropos_lst[icapblk].at(iecond) and iecond == 0 and nEvents<=maxShowEvent ; iw++ ){
	cout<<"word-64 "<< iw << std::hex <<"-th : 0x"<< std::setfill('0') << setw(16) << p64[iw] ;
	cout<<",       word-32 0th : 0x"<< std::setfill('0') << setw(8) << p32[2*iw+1] ;
	cout<<", 0x" << std::setfill('0') << setw(8) << p32[2*iw] << endl;
	cout<< std::dec << std::setfill(' ') ;

	int isMSB[12];
	int ffsep_word_loc[12];
	bool is_ff_sep_notfound[12];
	bool has_all_sep_found = true;;
	for(int iloc=0;iloc<nRx;iloc++){
	  isMSB[iloc] = -1;
	  is_ff_sep_notfound[iloc] = false;
	  ffsep_word_loc[iloc] = find_ffsep_word(rEvent, isMSB[iloc], iloc+1);
	  if(isMSB[iloc] == -1) {
	    is_ff_sep_notfound[iloc] = true;
	    has_all_sep_found = false ;
	  }
	}

	int expctd_empty_word_loc = zeropos_lst[icapblk].at(iecond);
	int empty_isMSB = -1;
	bool hasfound_emptry_word = found_empty_word(rEvent, empty_isMSB, expctd_empty_word_loc) ;      
	if(!hasfound_emptry_word) continue;
	if(!has_all_sep_found) continue;
      
	int ichip = 0;
	// const uint64_t *p64(((const uint64_t*)rEvent)+1);
      
	// uint32_t wordH = ((p64[ffsep_word_loc[0]-1] >> 32) & 0xFFFFFFFF) ;
	// int daq_header_payload_length = int(((wordH>>14) & 0x1FF));
	// daq_header_payload_length -= 1 ; //1 for DAQ header
	// if ((nEvents < nShowEvents) or (scanMode and boe->eventId()==inspectEvent)){
	//   std::cout << std::hex << std::setfill('0');
	//   std::cout << "Event : " << nEvents << ", WordH : 0x" << wordH  ;
	//   std::cout << std::dec << std::setfill(' ');
	//   std::cout<<", Size in ECON-D header : " << daq_header_payload_length << std::endl;
	// }
      
	// int record_header_sizeinfo = int(rEvent->payloadLength()); //in 64-bit format
	// int reduced_record_header_sizeinfo = record_header_sizeinfo - (4+2+1) ; //exclude 4 64-bit words for slink header/trailer + 2 DAQ header + 1 for DAQ trailer
	// int record_header_sizeinfo_32bit = 2*reduced_record_header_sizeinfo ; //change to 32-bit format

	// if(record_header_sizeinfo_32bit!=daq_header_payload_length) continue;
      
	// if(boe->eventId()>=minEventDAQ and boe->eventId()<maxEventDAQ){

	
	for(int iloc=0;iloc<nRx;iloc++){ //For full LD/HD modules : 6/12 respectively
	  uint32_t max_sep_word = (iloc==(nRx-1)) ? expctd_empty_word_loc : ffsep_word_loc[iloc+1] ;
	    bool ishalf = (iloc%2==0) ? false : true ;
	    if(isMSB[iloc]==1) max_sep_word++;
	    int index = 0;
	    uint32_t ch = 0;
	    uint32_t rocpin = 0;
	    int adcmL = 0, adcL = 0, toaL = 0, totL = 0;
	    int adcmM = 0, adcM = 0, toaM = 0, totM = 0;
	    rocn = uint32_t(ichip);
	    half = uint32_t(ishalf);
	    cout<<"iloc : " << iloc << endl;
	    //TPGFEDataformat::HalfHgcrocChannelData chdata[36];
	    for(uint32_t i = ffsep_word_loc[iloc]+1 ; i< max_sep_word ; i++){	  //For 37 seqs for full half ROC
	    
	      uint32_t wordL = 0;
	      uint32_t wordM = 0;
	      uint32_t seqL = 0;		
	      uint32_t seqM = 0;
	      if(isMSB[iloc]==0){
		wordL = (p64[i] & 0xFFFFFFFF) ;
		wordM = ((p64[i] >> 32) & 0xFFFFFFFF) ;
		seqM = 2*index;
		seqL = 2*index+1;		
	      }else if(isMSB[iloc]==1){
		wordL = (p64[i-1] & 0xFFFFFFFF) ;	  
		wordM = ((p64[i] >> 32) & 0xFFFFFFFF) ;
		seqL = 2*index;
		seqM = 2*index+1;		
	      }
	      adcmL = 0; adcL = 0; toaL = 0; totL = 0;
	      adcmM = 0; adcM = 0; toaM = 0; totM = 0;
	      
	      const uint16_t trigflagL = (wordL>>30) & 0x3;
	      if(trigflagL<=1){ //0 or 1
		adcmL = (wordL>>20) & 0x3FF;
		adcL = (wordL>>10) & 0x3FF;
		toaL = wordL & 0x3FF;
	      }else if(trigflagL==2){
		adcmL = (wordL>>20) & 0x3FF;
		totL = (wordL>>10) & 0x3FF;
		toaL = wordL & 0x3FF;	    
	      }else if(trigflagL==3){
		adcmL = (wordL>>20) & 0x3FF;
		totL = (wordL>>10) & 0x3FF;
		toaL = wordL & 0x3FF;
	      }
	      if(totL>>0x9==1)  totL = (totL & 0x1ff) << 0x3 ; //10-bit to 12-bit conversion
	      if(isTOTUP) totL += 0x7;
	      
	      const uint16_t trigflagM = (wordM>>30) & 0x3;
	      if(trigflagM<=1){ //0 or 1
		adcmM = (wordM>>20) & 0x3FF;
		adcM = (wordM>>10) & 0x3FF;
		toaM = wordM & 0x3FF;
	      }else if(trigflagM==2){
		adcmM = (wordM>>20) & 0x3FF;
		totM = (wordM>>10) & 0x3FF;
		toaM = wordM & 0x3FF;	    
	      }else if(trigflagM==3){
		adcmM = (wordM>>20) & 0x3FF;
		totM = (wordM>>10) & 0x3FF;
		toaM = wordM & 0x3FF;
	      }
	      if(totM>>0x9==1)  totM = (totM & 0x1ff) << 0x3 ; //10-bit to 12-bit conversion
	      if(isTOTUP) totM += 0x7;
	      
	      if ( nEvents<=maxShowEvent ){ 
		if(isMSB[iloc]==0){
		  std::cout<<"\ti : "<<i<<", index : "<<index<<std::endl;
		  std::cout << std::hex << std::setfill('0');
		  std::cout<<"\tWordM : 0x" << std::setw(8) << wordM <<", wordL : 0x" << std::setw(8) << wordL<< std::endl;
		  std::cout << std::dec << std::setfill(' ');
		  std::cout<<"\tM:(flag,adc,tot,toa) : (" << trigflagM <<", " << adcM << ", " << totM <<", " << toaM << "), \t"
			   <<"L:(flag,adc,tot,toa) : (" << trigflagL <<", " << adcL << ", " << totL <<", " << toaL << ")" << std::endl;
		}else{
		  std::cout<<"\ti : "<<i<<", index : "<<index<<std::endl;
		  std::cout << std::hex << std::setfill('0');
		  std::cout<<"\tWordL[i-1] : 0x" << std::setw(8) << wordL<<", wordM : 0x" << std::setw(8) << wordM << std::endl;
		  std::cout << std::dec << std::setfill(' ');
		  std::cout<<"\tL:(flag,adc,tot,toa)[i-1] : (" << trigflagL <<", " << adcL << ", " << totL <<", " << toaL << "), \t"
			   <<"M:(flag,adc,tot,toa) : (" << trigflagM <<", " << adcM << ", " << totM <<", " << toaM << ")" << std::endl;
		}
	      }
	      
	      // if(seqToRocpin.find(std::make_pair(modName,std::make_tuple(rocn,half,seqM)))!=seqToRocpin.end()){
	      // 	uint32_t rocpin = seqToRocpin.at(std::make_pair(modName,std::make_tuple(rocn,half,seqM))) ;
	      // 	ch = rocpin%36 ; //36 for halfroc
	      // 	if(trigflagM>=0x2)
	      // 	  chdata[ch].setTot(uint16_t(totM),uint16_t(trigflagM));
	      // 	else if(trigflagM==0)
	      // 	  chdata[ch].setAdc(uint16_t(adcM),uint16_t(trigflagM));
	      // 	else if(trigflagM==0x1)
	      // 	  chdata[ch].setAdc(0,uint16_t(trigflagM)); //uint16_t(adcM)
	      // 	else
	      // 	  chdata[ch].setZero();
	      // }
	      // if(seqToRocpin.find(std::make_pair(modName,std::make_tuple(rocn,half,seqL)))!=seqToRocpin.end()){
	      // 	uint32_t rocpin = seqToRocpin.at(std::make_pair(modName,std::make_tuple(rocn,half,seqL))) ;
	      // 	ch = rocpin%36 ; //36 for halfroc
	      // 	if(trigflagL>=0x2)
	      // 	  chdata[ch].setTot(uint16_t(totL),uint16_t(trigflagL));
	      // 	else if(trigflagL==0)
	      // 	  chdata[ch].setAdc(uint16_t(adcL),uint16_t(trigflagL));
	      // 	else if(trigflagL==0x1)
	      // 	  chdata[ch].setAdc(0,uint16_t(trigflagL)); //uint16_t(adcL)
	      // 	else
	      // 	  chdata[ch].setZero();
	      // }
	      index++;
	    }//end of loop for 37 seq channels
	    // TPGFEDataformat::HalfHgcrocData hrocdata;
	    // hrocdata.setChannels(chdata);
	    // pck.setZero();
	    // hrocarray[boe->eventId()].push_back(std::make_pair(pck.packRocId(zside, sector, link, det, econt, selTC4, module, rocn, half),hrocdata));
	    //std::cout<<std::endl;
	    if(iloc%2==1)ichip++;
	  }
	//   events.push_back(boe->eventId());
	// }


      }
    }

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
  
  vector<uint64_t> captureheader_lst;
  map<uint32_t,vector<uint64_t>> econheader_lst; //
  vector<uint32_t> captureheaderpos_lst;
  map<uint32_t,vector<uint32_t>> econheaderpos_lst;
  map<uint32_t,vector<uint32_t>> zeropos_lst;
  void printEconDEvents(uint64_t, uint64_t,std:: map<uint32_t,std::vector<uint32_t>>&, std:: map<uint32_t,std::vector<uint32_t>>&, const Hgcal10gLinkReceiver::RecordRunning *);
  
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
      
      if (nEvents < 0) {
	rEvent->RecordHeader::print();
	boe->print();
  	eoe->print();
  	event_dump(rEvent);
      }
      
      //Increment event counter and reset error state
      nEvents++;
      //if(nEvents==1) continue;
      //if(ievent>(maxEvents-1)) continue;
      //cout<<"iEvent: " << ievent << endl;
      
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
	cout<<"3rd : 0x" << p64[3] << endl;
	cout<<"4th : 0x" << p64[4] << endl;
	cout<<"5th : 0x" << p64[5] << endl;
	cout<< std::dec << std::setfill(' ') ;
      }
      /////////////////////////////////////////////////////////////////
      // vector<uint64_t> captureheader_lst;
      // map<uint32_t,vector<uint64_t>> econheader_lst; //
      // vector<uint32_t> captureheaderpos_lst;
      // map<uint32_t,vector<uint32_t>> econheaderpos_lst;
      /////////////////////////////////////////////////////////////////
      captureheader_lst.clear();
      econheader_lst.clear();
      captureheaderpos_lst.clear();
      econheaderpos_lst.clear();
      zeropos_lst.clear();
      
      uint32_t icapblk = 0;
      uint32_t capturepos = 2;
      while( (capturepos+2) < rEvent->payloadLength()){
	uint64_t captureblkheader = p64[capturepos];
	captureheader_lst.push_back(captureblkheader);
	captureheaderpos_lst.push_back(capturepos);
      
	bool isEcondp[maxEcons];
	bool isPassthrough[maxEcons];
	uint32_t zeroloc[maxEcons];
	int nofecons = 0;
	for(int iecon=0;iecon<maxEcons;iecon++) {
	  isEcondp[iecon] = false;
	  isPassthrough[iecon]  = false;
	  zeroloc[iecon] = 0;
	}
	for(int iecon=0;iecon<maxEcons;iecon++){
	  uint8_t stat = (p64[capturepos] >> uint8_t(iecon*3)) & 0x7;
	  if(stat==0b000 or stat==0b010 or stat==0b011) {
	    isEcondp[iecon] = true ;
	    nofecons++;
	  }
	}
	//assert(nofecons!=0);
	if(nofecons==0) {
	  continue;
	} //update the feedback from Martim
	
	uint32_t next_econd_pos = capturepos + 1;
	for(int iecond = 0 ; iecond < nofecons ; iecond++){
	  uint32_t econpos = next_econd_pos;
	  uint32_t econh0 = (p64[econpos] >> 32) & 0xFFFFFFFF ;
	  uint32_t econh1 = p64[econpos] & 0xFFFFFFFF ;
	  econheader_lst[icapblk].push_back(p64[econpos]); 
	  econheaderpos_lst[icapblk].push_back(econpos);
	  isPassthrough[iecond] = (econh0 >> 13 ) & 0x1 ;
	  uint32_t econsize = (econh0 >> 14 ) & 0x1FF ; //specifies length in 32b-words+1 of eRx sub-packets and CRC trailer
	  assert(econsize>1);
	  zeroloc[iecond] = (econsize-1)/2 + (econpos+1) ; //(econ0pos+1): +1 to start from first eRx header
	  zeropos_lst[icapblk].push_back(zeroloc[iecond]);
	  next_econd_pos = zeroloc[iecond]+1 ;
	}
	for(int iecond = 0 ; iecond < nofecons ; iecond++){
	  if(nEvents<=maxShowEvent) cout<<"icapblk: "<<icapblk<<" zero position for econ " << iecond << ", "<<zeroloc[iecond]<<endl;
	}
	capturepos = zeroloc[nofecons-1] + 1;
	icapblk++;
      }//find the block positions
      
      //getEvents(uint64_t& minEventDAQ, uint64_t& maxEventDAQ, std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>>>& hrocarray, std::vector<uint64_t>& events){
      //printEconDEvents(nEvents, maxShowEvent, econheaderpos_lst, rEvent);
      printEconDEvents(nEvents, maxShowEvent, econheaderpos_lst, zeropos_lst, rEvent);
      if(nEvents<=maxShowEvent) cout<<"========= End of event : "<< nEvents << "============="<< endl;
      ievent++;
    }//loop event  
  }//while read r
  cout << "total events " << ievent << endl;
  rEvent->RecordHeader::print();
  delete r;
  
  return 0;
}
