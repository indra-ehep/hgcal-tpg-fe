/**********************************************************************
 Created on : 13/08/2024
 Purpose    : Emulation for test-beam of July-August 2024
 Author     : Indranil Das, Visiting Fellow
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
#include <iostream>

#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TProfile.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TFile.h"

#include "TFileHandlerLocal.h"
#include "common/inc/FileReader.h"

#include "yaml-cpp/yaml.h"
#include <deque>

#include <string>
#include <fstream>
#include <cassert>
#include <bitset>

#include "TPGFEDataformat.hh"
#include "TPGBEDataformat.hh"
#include "TPGFEConfiguration.hh"
#include "TPGFEModuleEmulation.hh"
#include "TPGFEReader2024.hh"
#include "Stage1IO.hh"

int main(int argc, char** argv)
{
  //===============================================================================================================================
  // make
  // ./emul_Jul24.exe $Relay $rname $link_number $istrimming $density $dropLSB 
  //===============================================================================================================================  
  if(argc < 3){
    std::cerr << argv[0] << ": no relay and/or run numbers specified" << std::endl;
    return false;
  }
  if(argc < 4){
    std::cerr << argv[1] << ": no link number (1 or 2) is specified " << std::endl;
    return false;
  }
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Assign relay,run and link numbers from input
  //===============================================================================================================================
  uint32_t relayNumber(0);
  uint32_t runNumber(0);
  uint32_t linkNumber(0);
  bool isTrimming(0);
  uint32_t density(1);
  uint32_t droplsb(1);
  std::istringstream issRelay(argv[1]);
  issRelay >> relayNumber;
  std::istringstream issRun(argv[2]);
  issRun >> runNumber;
  std::istringstream issLink(argv[3]);
  issLink >> linkNumber;
  if(linkNumber!=1 and linkNumber!=2){
    std::cerr << "Link number "<< argv[3] <<"is out of bound (use: 1 or 2)" << std::endl;
    return false;
  }
  uint32_t trig_linkNumber = TMath::FloorNint((linkNumber-1)/2);
  std::istringstream issTrimming(argv[4]);
  issTrimming >> isTrimming;
  std::istringstream issDensity(argv[5]);
  issDensity >> density;
  std::istringstream issDropLSB(argv[6]);
  issDropLSB >> droplsb;
  //===============================================================================================================================

  //===============================================================================================================================
  //Book histograms
  //===============================================================================================================================
  void BookHistograms(TDirectory*&, uint32_t);
  void FillHistogram(bool, TDirectory*&, uint32_t, uint64_t, uint16_t, uint32_t, const uint32_t *, const uint32_t *, uint32_t, uint32_t,
		     TPGFEDataformat::TcRawDataPacket&, TPGFEDataformat::TcRawDataPacket&, const uint32_t *, TPGBEDataformat::UnpackerOutputStreamPair&);
  
  TFile *fout = new TFile(Form("Diff_Relay-%u.root",relayNumber), "recreate");
  TDirectory *dir_diff = fout->mkdir("diff_plots");
  BookHistograms(dir_diff, relayNumber);
  //===============================================================================================================================

  //===============================================================================================================================
  //Read channel mapping
  //===============================================================================================================================
  TPGFEConfiguration::Configuration cfgs;
  cfgs.setSiChMapFile("cfgmap/WaferCellMapTraces.txt");
  cfgs.setSciChMapFile("cfgmap/channels_sipmontile_HDtypes.hgcal.txt");
  cfgs.initId();
  cfgs.readSiChMapping();
  cfgs.readSciChMapping();
  cfgs.loadModIdxToNameMapping();
  cfgs.loadMuxMapping();
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Read ECON-T setting
  //===============================================================================================================================  
  uint32_t zside = 0, sector = 0, link = 0, det = 0;
  uint32_t econt = 0, selTC4 = 1, module = 0;
  TPGFEConfiguration::TPGFEIdPacking pck;
  
  std::string cfgrocname[2][3][3] = {
    {
      {"320MLF3WXIH0012_roc0_e2.yaml", "320MLF3WXIH0012_roc1_e2.yaml", "320MLF3WXIH0012_roc2_e2.yaml"},
      {"320MLF3WXIH0006_roc0_e1.yaml", "320MLF3WXIH0006_roc1_e1.yaml", "320MLF3WXIH0006_roc2_e1.yaml"},
      {"320MLF3WXIH0011_roc0_e0.yaml", "320MLF3WXIH0011_roc1_e0.yaml", "320MLF3WXIH0011_roc2_e0.yaml"}
    },
    {
      {"320MLF3WXIH0007_roc0_e2.yaml", "320MLF3WXIH0007_roc1_e2.yaml", "320MLF3WXIH0007_roc2_e2.yaml"},
      {"320MLF3WXIH0005_roc0_e1.yaml", "320MLF3WXIH0005_roc1_e1.yaml", "320MLF3WXIH0005_roc2_e1.yaml"},
      {"320MLF3WXIH0010_roc0_e0.yaml", "320MLF3WXIH0010_roc1_e0.yaml", "320MLF3WXIH0010_roc2_e0.yaml"}
    }
  };
  
  for(int ilink=0;ilink<2;ilink++){
    for(int iecond=0;iecond<3;iecond++){
      uint32_t idx = pck.packModId(zside, sector, ilink, det, iecond, selTC4, module); //we assume same ECONT and ECOND number for a given module
      
      cfgs.setModulePath(zside, sector, ilink, det, iecond, selTC4, module);

      switch(iecond){
      case 0:
	cfgs.setEconTFile("cfgmap/slow_control_configuration/init_econt_e2.yaml");
	break;
      case 1:
	cfgs.setEconTFile("cfgmap/slow_control_configuration/init_econt_e1.yaml");
	break;
      default:
	cfgs.setEconTFile("cfgmap/slow_control_configuration/init_econt.yaml");
	break;
      }
      cfgs.readEconTConfigYaml();
      
      cfgs.setEconDFile("cfgmap/slow_control_configuration/init_econd.yaml");
      cfgs.readEconDConfigYaml();
      
      if(relayNumber==1722870998 and runNumber==1722871004){
	if(ilink==1 and (iecond==1 or iecond==2)){
	  cfgs.setEconTFile("cfgmap/init_econt_e2_cal_setup2.yaml");
	  cfgs.readEconTConfigYaml();
	}
	if(ilink==1 and (iecond==0)){
	  cfgs.setEconTFile("cfgmap/init_econt_e2_mux_setup1_mod.yaml");
	  cfgs.readEconTConfigYaml();
	}
      }

      if(relayNumber==1722871297 and runNumber==1722871302){
	if(ilink==1 and (iecond==1 or iecond==2)){
	  cfgs.setEconTFile("cfgmap/init_econt_e1_cal_setup1.yaml");
	  cfgs.readEconTConfigYaml();
	}
	if(ilink==1 and (iecond==0)){
	  cfgs.setEconTFile("cfgmap/init_econt_e2_mux_setup1.yaml");
	  cfgs.readEconTConfigYaml();
	}
      }
      
      if(relayNumber==1722871625 and runNumber==1722871630){
	if(ilink==1 and (iecond==1 or iecond==2)){
	  cfgs.setEconTFile("cfgmap/init_econt_e1_cal_setup1.yaml");
	  cfgs.readEconTConfigYaml();
	  cfgs.setEconTFile("cfgmap/init_econt_e1_mux_setup2.yaml");
	  cfgs.readEconTConfigYaml();
	}
	if(ilink==1 and (iecond==0)){
	  cfgs.setEconTFile("cfgmap/init_econt_e2_mux_setup1.yaml");
	  cfgs.readEconTConfigYaml();
	}
      }
      
      if(relayNumber==1722871974 and runNumber==1722871979){
	if(ilink==1 and (iecond==1 or iecond==2)){
	  cfgs.setEconTFile("cfgmap/init_econt_e1_cal_setup1_mod.yaml");
	  cfgs.readEconTConfigYaml();
	  cfgs.setEconTFile("cfgmap/init_econt_e1_mux_setup2.yaml");
	  cfgs.readEconTConfigYaml();
	}
	if(ilink==1 and (iecond==0)){
	  cfgs.setEconTFile("cfgmap/init_econt_e2_cal_setup2.yaml");
	  cfgs.readEconTConfigYaml();
	  cfgs.setEconTFile("cfgmap/init_econt_e2_mux_setup1_mod.yaml");
	  cfgs.readEconTConfigYaml();
	}
      }
      
      for(uint32_t iroc=0;iroc<3;iroc++){
	std::cout<<"idx: "<<idx<<", ilink: " << ilink << ", iecond: "<<iecond<<", iroc: "<<iroc << ", fname : " << cfgrocname[ilink][iecond][iroc] << std::endl;
	uint32_t rocid_0 = pck.packRocId(zside, sector, ilink, det, iecond, selTC4, module, iroc, 0);
	uint32_t rocid_1 = pck.packRocId(zside, sector, ilink, det, iecond, selTC4, module, iroc, 1);
	//cfgs.setRocFile(Form("cfgmap/slow_control_configuration/configs_with_MasterTDC_and_TPGTRM/%s",cfgrocname[ilink][iecond][iroc].c_str()));
	cfgs.setRocFile(Form("cfgmap/bt2024-fe-config-trimmed/%s",cfgrocname[ilink][iecond][iroc].c_str()));
	cfgs.readRocConfigYaml(rocid_0, rocid_1);	
      }//roc loop
    }//econd loop
  }//lpGBT loop
  if(!isTrimming) cfgs.setPedThZero();
  link = 0; econt = 0;
  uint32_t testmodid = pck.packModId(zside, sector, link, det, econt, selTC4, module); //we assume same ECONT and ECOND number for a given module
  cfgs.printCfgPedTh(testmodid);
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Modify the ECON parameters for special cases
  //===============================================================================================================================
  std::map<uint32_t,TPGFEConfiguration::ConfigEconD>& econDPar =  cfgs.getEconDPara();
  for(const auto& it : econDPar){
    //std::cout << "it.first: "<< it.first << std::endl;
    econDPar.at(it.first).setPassThrough(true);
    econDPar.at(it.first).setNeRx(6);
  }
  link = 0; econt = 2;
  uint32_t lp0_stc16orctc4a = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 1; econt = 2;
  uint32_t lp1_stc16orctc4a = pck.packModId(zside, sector, link, det, econt, selTC4, module);

  std::map<uint32_t,TPGFEConfiguration::ConfigEconT>& econTPar =  cfgs.getEconTPara();
  for(const auto& it : econTPar){
    econTPar[it.first].setDensity(density);
    econTPar[it.first].setDropLSB(droplsb);
    if(it.first == lp0_stc16orctc4a or it.first == lp1_stc16orctc4a){
      uint32_t stc_type = 0;
      if(relayNumber>1722881092)
	stc_type = 1;
      else if(relayNumber==1722881092)
	stc_type = (it.first==lp0_stc16orctc4a) ? 1 : 2;
      else
	stc_type = 2;
      econTPar[it.first].setSTCType(stc_type);
    }
    std::cout << "Modtype " << it.first
	      << ", getOuttype: " << econTPar[it.first].getOutType()
	      << ", select: " << econTPar[it.first].getSelect()
      	      << ", stctype: " << econTPar[it.first].getSTCType()
	      << ", typename: " << TPGFEDataformat::tctypeName[econTPar[it.first].getOutType()]
	      << std::endl;
    econTPar[it.first].print();
  }//econt loop
  //===============================================================================================================================

  // ===============================================================================================================================
  // Set and Initialize the ECONT reader
  // ===============================================================================================================================
  TPGFEReader::ECONTReader econTReader(cfgs);
  //output array
  //econTReader.checkEvent(1);
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGBEDataformat::Trig24Data>>> econtarray; //event,moduleId (from link)
  // ===============================================================================================================================
  
  //===============================================================================================================================
  //Set and Initialize the ECOND reader
  //===============================================================================================================================
  TPGFEReader::ECONDReader econDReader(cfgs);
  econDReader.setTotUp(0);
  //econDReader.checkEvent(1);
  //econDReader.showFirstEvents(10);

  //===============================================================================================================================
  //Set and Initialize the Emulator
  //===============================================================================================================================
  TPGFEModuleEmulation::HGCROCTPGEmulation rocTPGEmul(cfgs);
  TPGFEModuleEmulation::ECONTEmulation econtEmul(cfgs);
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Set refernce eventlist
  //===============================================================================================================================
  //// ./emul_Jul24.exe 1722870998 1722871004 1 > temp.log  
  //uint64_t refEvt[3] = {12049, 16652, 17546}; //BC lpGBT:1
  //uint64_t refEvt[5] = {868, 6064, 6114, 6590, 8204}; //STC4A lpGBT:1
  //uint64_t refEvt[8] = {563, 11321, 12346, 12646, 17305, 19916, 22529, 22857}; //CTC4A lpGBT:1
  
  ////./emul_Jul24.exe 1722871974 1722871979 1 > temp1.log
  //uint64_t refEvt[2] = {9198, 16570}; // BC lpGBT:1
  //uint64_t refEvt[5] = {886, 2006, 2236, 2757, 3349}; //BC lpGBT:0
  //uint64_t refEvt[5] = {143, 567, 728, 1895, 1994}; // STC4A lpGBT:1
  //uint64_t refEvt[5] = {5152, 8783, 10290, 18288, 18458};
  //uint64_t refEvt[69] = {3, 14, 47, 58, 69, 80, 91, 102, 113, 135, 146, 179, 190, 212, 223, 234, 256, 267, 300, 311, 322, 333, 355, 366, 377, 388, 399, 410, 421, 432, 443, 454, 476, 487, 498, 509, 520, 542, 553, 564, 575, 597, 608, 630, 641, 674, 685, 696, 707, 718, 729, 751, 762, 795, 806, 817, 828, 839, 850, 861, 874, 894, 916, 927, 938, 949, 971, 982, 993};
  //uint64_t refEvt[1] = {874};
  //193
  //uint64_t refEvt[] = {1325, 1326, 1327, 1328, 1329, 1330, 2154, 2155, 2156, 1560, 2232, 2584, 2968, 3992, 4344, 5048, 5848, 6168, 8248, 9976, 10360, 10712, 11768, 12152, 12824, 13560, 13944, 14264, 14552, 14648, 14936, 15640, 15672, 16664, 16696, 16728, 17080, 17432, 18136, 18168, 18872, 19128, 19576, 19864, 19896, 20280, 20888, 21272, 21422, 21976, 22392, 22744, 23064, 23704, 23800, 26168, 27288, 28376, 28984, 29400, 30104, 30456, 30744, 30808, 30926, 31096, 31128, 31160, 31448, 31800, 32248, 32568, 32600, 33208, 34296, 34648, 35032, 35064, 36120, 37080, 37848, 38520, 38840, 38936, 39256, 39637, 40632, 41368, 42712, 42776, 43832, 44920, 45880, 46062, 46584, 46936, 47320, 47352, 48056, 48344, 48408, 49464, 50552, 50638, 52248, 52664, 53304, 53624, 53688, 53720, 54328, 54424, 54680, 55064, 56088, 56504, 56792, 57208, 57560, 57592, 58968, 59640, 60312, 60728, 61016, 62136, 63480, 63576, 64888, 65240, 65304, 65592, 66360, 66648, 67093, 69464, 71256, 71576, 71672, 71960, 72664, 73688, 73752, 74072, 75480, 75832, 76600, 76888, 77240, 77560, 77912, 78264, 78648, 78968, 79320, 79384, 79672, 80024, 81080, 81112, 81784, 82552, 82581, 82936, 83192, 83288, 83928, 84312, 84600, 85368, 86744, 87128, 87448, 87598, 87832, 88152, 88824, 88888, 89240, 89560, 89944, 90264, 90584, 90616, 91288, 91470, 91640, 91704, 92174, 92760, 93080, 94200, 95576, 96312, 97304, 97336, 97688, 98008, 99096, 99214, 99384, 99736, };
  uint64_t refEvt[] = {37848, 60142, 77424, 92760, 96344, 130808};
  //uint64_t refEvt[] = {1560, 2232, 2584, 2968, 3992, 4344, 5048, 5848, 6168, 8248, 9976};
  std::vector<uint64_t> refEvents;
  for(int ievent=0;ievent<6;ievent++) refEvents.push_back(refEvt[ievent]);
  refEvents.resize(0);
  //===============================================================================================================================
  
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>>> hrocarray; //event,rocId
  std::map<uint64_t,uint32_t> eventbx;
  std::vector<uint64_t> eventList;
  std::vector<uint64_t> nonZeroEvents;
  std::vector<uint64_t> shiftedBxEvent;
  
  uint64_t minEventDAQ, maxEventDAQ;  
  //const long double maxEvent = 882454  ; 
  //const long double maxEvent = 1038510 ;
  //const long double maxEvent = 6377139 ; 
  //const long double maxEvent = 1138510 ;
  //long double nloopEvent =  100000;
  const long double maxEvent = 1000000  ; //1722870998:24628, 1722871979:31599
  long double nloopEvent = 100000 ;
  int nloop = TMath::CeilNint(maxEvent/nloopEvent) ;
  if(refEvents.size()>0) nloop = 1;
  std::cout<<"nloop: "<<nloop<<std::endl;
  for(int ieloop=0;ieloop<nloop;ieloop++){
    
    minEventDAQ = ieloop*nloopEvent ;
    maxEventDAQ = (ieloop+1)*nloopEvent;
    maxEventDAQ = (maxEventDAQ>uint64_t(maxEvent)) ? uint64_t(maxEvent) : maxEventDAQ ;
    
    printf("iloop : %d, minEventDAQ = %lu, maxEventDAQ = %lu\n",ieloop,minEventDAQ, maxEventDAQ);

    hrocarray.clear();
    eventList.clear();
    econtarray.clear();
    nonZeroEvents.clear();
    shiftedBxEvent.clear();
    
    econTReader.init(relayNumber,runNumber,0);
    std::cout<<"TRIG Before Link "<<trig_linkNumber<<", size : " << econtarray.size() <<std::endl;
    econTReader.getEvents(refEvents, minEventDAQ, maxEventDAQ, econtarray);
    std::cout<<"TRIG After Link "<<trig_linkNumber<<", size : " << econtarray.size() <<std::endl;
    econTReader.terminate();

    econDReader.init(relayNumber,runNumber,linkNumber);
    std::cout<<"DAQ Before Link "<<linkNumber<<", size : " << hrocarray.size() <<std::endl;
    econDReader.getEvents(refEvents, minEventDAQ, maxEventDAQ, hrocarray, eventList); //Input <---> output
    std::cout<<"DAQ After Link "<<linkNumber<<", size : " << hrocarray.size() <<std::endl;
    econDReader.terminate();

    // for(uint64_t ievt = 0 ; ievt < eventList.size(); ievt++ ){
    //   uint64_t ievent = eventList[ievt] ;
    //   std::cout << "Event: " << ievent << ", ECONT: " << econtarray.at(ievent).size() << ", ROC: "<< hrocarray.at(ievent).size() << std::endl;
    // }
    // continue;
    
    //std::vector<uint64_t> totEvents;
    for(uint64_t ievt = 0 ; ievt < eventList.size(); ievt++ ){
      uint64_t ievent = eventList[ievt] ;
      if(econtarray.at(ievent).size()!=6 or hrocarray.at(ievent).size()!=36) continue;
      std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>> datalist = hrocarray[ievent] ;
      if(ievt<1000) std::cout << "ROC ievent: "<< ievent<< ", datalist size: "<< datalist.size() << std::endl;
      bool hasTOT = false;
      for(const auto& hrocit : datalist){
	const TPGFEDataformat::HalfHgcrocData& hrocdata = hrocit.second ;
	if(testmodid==pck.getModIdFromRocId(uint32_t(hrocit.first))){
	//if(event<100 and (iecond==1 and ilink==0)){
	  if(ievt<1000) std::cout << "ievent: "<< ievent<< "\t data for half-roc_id: "<< hrocit.first <<  std::endl;
	  if(ievt<1000) hrocdata.print();
	  if(hrocdata.hasTOT()) hasTOT = true;
	}
      }
      //if(hasTOT) totEvents.push_back(ievent);
    }
    
    //===============================================================================================================================
    // Emulate
    //===============================================================================================================================
    std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::ModuleTcData>>> modarray; //event,moduleId
    modarray.clear();
    std::map<uint32_t,TPGFEDataformat::HalfHgcrocData> rocdata;
    std::map<uint32_t,TPGFEDataformat::ModuleTcData> moddata;
    /////////////////////////////////////////////////////////////
    //// The following part should be in a loop over modules
    /////////////////////////////////////////////////////////////
    std::cout<<"eventList size: " << eventList.size() <<std::endl;

    for(uint64_t ievt = 0 ; ievt < eventList.size(); ievt++ ){
      uint64_t event = eventList[ievt] ;
      
      //first check that both econd and econt data has the same eventid otherwise skip the event
      if(econtarray.find(event) == econtarray.end() or hrocarray.find(event) == hrocarray.end() or econtarray.at(event).size()!=6 or hrocarray.at(event).size()!=36) continue;

      //if(econtarray.find(event) == econtarray.end()) break;

      //bool eventCondn = (event<1000 or event==1560 or event==2232 or event==2584 or event==2968 or event==3992);
      //bool eventCondn = (event<1000);
      bool eventCondn = (ievt<1000);
      if( eventCondn or event%100000==0)
	std::cout<<std::endl<<std::endl<<"=========================================================================="<<std::endl<<"Processing event: " << event <<std::endl<<std::endl<<std::endl;
    
      for(int ilink=0;ilink<2;ilink++){
	for(int iecond=0;iecond<3;iecond++){
	  uint32_t moduleId = pck.packModId(zside, sector, ilink, det, iecond, selTC4, module); //we assume same ECONT and ECOND number for a given module
	  
	  std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	  rocdata.clear();
	  int emul_bx;
	  uint32_t econd_bx, econd_bx_4b, emul_bx_4b;
	  uint32_t econt_central_bx_4b;
	  uint16_t econt_slink_bx, econd_slink_bx; 
	  std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>> hrocvec ;
	  // if(event==1560 or event==2232 or event==2584 or event==2968 or event==3992)
	  //   hrocvec =  hrocarray.at(event+1);
	  // else
	  hrocvec =  hrocarray.at(event);
	  
	  for(const auto& data : hrocvec){
	    rocdata[data.first] = data.second ;
	    const TPGFEDataformat::HalfHgcrocData& hrocdata = data.second ;
	    //if(moduleId==pck.getModIdFromRocId(uint32_t(data.first)))
	    
	    if(data.first==moduleId){
	      econd_bx = rocdata[data.first].getBx();
	      econd_slink_bx = rocdata[data.first].getSlinkBx();
	      emul_bx = econd_bx - 3;
	      if(emul_bx<0) emul_bx = (3564+emul_bx) + 1;
	      emul_bx_4b = (rocdata[data.first].getBx()==3564) ? 0xF : (emul_bx & 0xF);
	      econd_bx_4b = (rocdata[data.first].getBx()==3564) ? 0xF : (econd_bx & 0xF);
	      //tune the bx here to match the TPG
	      if(relayNumber==1723627575) emul_bx_4b += 2;
	      if(relayNumber==1722702638) emul_bx_4b += 7;
	      if(relayNumber==1722870998) emul_bx_4b += 2;
	      if(relayNumber==1722871974) emul_bx_4b += 2;
	    }
	  }
	  
	  //================================================
	  //HGCROC emulation for a given module
	  //================================================
	  bool isSim = false; //true for CMSSW simulation and false for beam-test analysis
	  rocTPGEmul.Emulate(isSim, event, moduleId, rocdata, modTcdata);
	  //================================================
	  
	  modarray[event].push_back(modTcdata);
	  if(event==0){
	    uint32_t modTcid = modTcdata.first;
	    TPGFEDataformat::ModuleTcData& mtcdata = modTcdata.second;
	    if(modTcid==moduleId) mtcdata.print();
	  }
	  
	  moddata.clear();
	  for(const auto& data : modarray.at(event))
	    if(data.first==moduleId) moddata[data.first] = data.second ;

	  // if(iecond==0)
	  //   econtEmul.setVerbose();
	  // else
	  //   econtEmul.setVerbose(false);
	  
	  //================================================
	  //ECONT emulation for a given module
	  //================================================
	  econtEmul.Emulate(isSim, event, moduleId, moddata);
	  TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	  //================================================
	  if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	  TcRawdata.second.setBX(emul_bx_4b%8);
	
	  uint32_t *elinkemul = new uint32_t[econTPar[moduleId].getNElinks()];
	  TPGFEModuleEmulation::ECONTEmulation::convertToElinkData(emul_bx_4b%8, TcRawdata.second, elinkemul);

	  std::vector<std::pair<uint32_t,TPGBEDataformat::Trig24Data>> econtdata =  econtarray[event];

	  TPGBEDataformat::UnpackerOutputStreamPair upemul,up1;
	  TPGFEDataformat::TcRawDataPacket vTC1;	
	  int refbx = -1;
	  TPGBEDataformat::Trig24Data trdata;
	  for(const auto& econtit : econtdata){
	    if(econtit.first!=TcRawdata.first) continue;
	    trdata = econtit.second ;
	    // if(eventCondn and iecond==0) std::cout << "event: " << event << ", moduleId : " << econtit.first << std::endl;
	    // if(eventCondn and iecond==0) TcRawdata.second.print();
	    // if(eventCondn and iecond==0) trdata.print();
	    bool hasMatched = false;
	    for(int ibx=0;ibx<7;ibx++){
	      const uint32_t *el = trdata.getElinks(ibx); 
	      uint32_t bx_2 = (el[0]>>28) & 0xF;
	      if(bx_2==(emul_bx_4b%8)) {
		refbx = ibx;
		hasMatched = true;
	      }//match found;
	    }//bx loop
	    if(hasMatched) break;
	  }//loop over econt data for a given run
	  if(refbx==-1){
	    //std::cout << "refbx: " << refbx << std::endl ;
	    if(std::find(shiftedBxEvent.begin(),shiftedBxEvent.end(),event) == shiftedBxEvent.end()) shiftedBxEvent.push_back(event);
	    econt_slink_bx = trdata.getSlinkBx();
	    FillHistogram(false, dir_diff, relayNumber, event, econt_slink_bx, econTPar[moduleId].getNElinks(), 0x0, elinkemul, ilink, iecond, vTC1, TcRawdata.second, 0x0, upemul);
	    delete []elinkemul;
	    continue;
	  }
	  // if(iecond==0) refbx = 5;
	  const uint32_t *elmid = trdata.getElinks(3);
	  econt_central_bx_4b = (elmid[0]>>28) & 0xF;
	  econt_slink_bx = trdata.getSlinkBx();
	  const uint32_t *eldata = trdata.getElinks(refbx);
	  const uint32_t *unpkWords = trdata.getUnpkWords(refbx);
	  uint32_t bx_data = (eldata[0]>>28) & 0xF;
	  TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TcRawdata.second.type(), TcRawdata.second.size(), eldata, vTC1);
	  if(relayNumber>1722881092 or iecond==0 or iecond==1){
	    TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(emul_bx_4b%8, TcRawdata.second, upemul);
	    TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(bx_data, vTC1, up1);
	  }

	  // if(iecond==0){
	  //   bool hasTC33emul = false, hasTC33data = false;
	  //   for(uint32_t itc=0;itc<vTC1.size();itc++){
	  //     if(vTC1.getTcData().at(itc).address()==33) hasTC33data = true;	      
	  //   }
	  //   for(uint32_t itc=0;itc<TcRawdata.second.size();itc++){
	  //     if(TcRawdata.second.getTcData().at(itc).address()==33) hasTC33emul = true;	      
	  //   }
	  //   if(hasTC33emul and hasTC33data){
	  //     std::cout << "TC energy comparison for  ievent: "<< event << ", moduleId : " << moduleId << std::endl;
	  //     TcRawdata.second.print();
	  //     vTC1.print();
	  //   }
	  //   if(hasTC33emul and !hasTC33data){
	  //     std::cout << "TC energy emul-only for  ievent: "<< event << ", moduleId : " << moduleId << std::endl;
	  //     TcRawdata.second.print();
	  //     vTC1.print();	      
	  //   }
	  //   if(!hasTC33emul and hasTC33data){
	  //     std::cout << "TC energy data-only for  ievent: "<< event << ", moduleId : " << moduleId << std::endl;
	  //     TcRawdata.second.print();
	  //     vTC1.print();	      
	  //   }
	  // }
	  bool hasTcTp1 = false;
	  if(TcRawdata.second.isTcTp1()) hasTcTp1 = true;
	  bool isLargeDiff = false;
	  for(uint32_t iel=0;iel<econTPar[moduleId].getNElinks();iel++){
	    if( ((eldata[iel]-elinkemul[iel]) != 0) and hasTcTp1==false) isLargeDiff = true;
	  }
	  if(isLargeDiff)
	    std::cout<<std::endl<<std::endl<<"=========================================================================="<<std::endl<<"Processing event: " << event <<std::endl<<std::endl<<std::endl;
	  if(eventCondn or isLargeDiff) std::cout << "Elink comparison for  ievent: "<< event << ", moduleId : " << moduleId << std::endl;
	  if(eventCondn or isLargeDiff) std::cout << "Event: "<< event
						  << ", ECOND:: (econd_slink_bx: " << econd_slink_bx <<", econd_bx: "<<econd_bx<<", econd econd_bx_4b : " <<  econd_bx_4b
						  << ") ECONT:: (econt_slink_bx: " << econt_slink_bx <<", econt_slink_bx%8: "<< (econt_slink_bx%8) <<", econt_central_bx_4b: " << econt_central_bx_4b
						  <<"), moduleId : " << moduleId <<", isLargeDiff: " << isLargeDiff << std::endl;
	  //if(eventCondn or isLargeDiff) modTcdata.second.print();
	  if(eventCondn or isLargeDiff) TcRawdata.second.print();
	  if(eventCondn or isLargeDiff) vTC1.print();
          //if(eventCondn or isLargeDiff) up1.print();
	  //if(eventCondn or isLargeDiff) upemul.print();
	  // 
	  for(uint32_t iel=0;iel<econTPar[moduleId].getNElinks();iel++){
	    if(eventCondn or isLargeDiff) std::cout<<"Elink emul : 0x" << std::hex << std::setfill('0') << std::setw(8) << elinkemul[iel] << std::dec ;
	    if(eventCondn or isLargeDiff) std::cout<<"\t Elink data : 0x" << std::hex << std::setfill('0') << std::setw(8) << eldata[iel] << std::dec ;//<< std::endl;
	    if(eventCondn or isLargeDiff) std::cout<<"\t Diff: " << std::setfill(' ') << std::setw(10) << (eldata[iel]-elinkemul[iel])  << ", XOR: " << std::bitset<32>{(eldata[iel] ^ elinkemul[iel])} << std::endl;
	    if( ((eldata[iel]-elinkemul[iel]) != 0) and (std::find(nonZeroEvents.begin(),nonZeroEvents.end(),event) == nonZeroEvents.end()) and hasTcTp1==false) nonZeroEvents.push_back(event);
	  }
	  uint16_t *unpkMsTc = new uint16_t[TcRawdata.second.size()+1];
	  if(relayNumber>1722881092 or iecond==0 or iecond==1){
	    uint32_t strm = 0;
	    for(uint32_t iw=0;iw<=TcRawdata.second.size();iw++){
	      unpkMsTc[iw] = (iw==0)? upemul.getMsData(strm) : upemul.getTcData(strm, iw-1);
	      bool diffcondn = (iw==0 and (TcRawdata.second.type()==TPGFEDataformat::STC4A or TcRawdata.second.type()==TPGFEDataformat::CTC4A or TcRawdata.second.type()==TPGFEDataformat::STC16));
	      uint32_t diff = (diffcondn)? ((unpkWords[iw]&0xf) - (unpkMsTc[iw]&0xf)) : (unpkWords[iw] - unpkMsTc[iw]) ;
	      uint32_t XOR = (diffcondn)? ((unpkWords[iw]&0xf) xor (unpkMsTc[iw]&0xf)) : (unpkWords[iw] xor unpkMsTc[iw]) ;
	      if(eventCondn or isLargeDiff) std::cout<<"Stage1 unpacker Words emul : 0x" << std::hex << ::std::setfill('0') << std::setw(4) << unpkWords[iw] << std::dec ;
	      if(eventCondn or isLargeDiff) std::cout<<"\t  data : 0x" << std::hex << ::std::setfill('0') << std::setw(4) << unpkMsTc[iw] << std::dec ;
	      if(eventCondn or isLargeDiff) std::cout<<"\t Diff: " << std::setfill(' ') << std::setw(10) << diff  << ", XOR: " << std::bitset<32>{XOR} << std::dec << std::endl;
	    }	    
	  }
	  FillHistogram(true, dir_diff, relayNumber, event, econt_slink_bx, econTPar[moduleId].getNElinks(), eldata, elinkemul, ilink, iecond, vTC1, TcRawdata.second, unpkWords, upemul);	  
	  delete []elinkemul;
	  delete []unpkMsTc;
	}//econd loop
      }//ilink loop    
    }//event loop
  }//ieloop
  //===============================================================================================================================
  if(nonZeroEvents.size()>0) std::cerr<< "/*Nonzero diff events*/ uint64_t refEvt["<< nonZeroEvents.size() <<"] = {";
  for(const uint64_t& totEvt : nonZeroEvents) std::cerr<<totEvt << ", ";
  if(nonZeroEvents.size()>0) std::cerr<< "};" << std::endl;

  if(shiftedBxEvent.size()>0) std::cerr<< "/*Shifted Bx/events */ uint64_t refEvt["<< shiftedBxEvent.size() <<"] = {";
  for(const uint64_t& totEvt : shiftedBxEvent) std::cerr<<totEvt << ", ";
  if(shiftedBxEvent.size()>0) std::cerr<< "};" << std::endl;

  fout->cd();
  dir_diff->Write();
  fout->Close();
  delete fout;
  
  return true;
}


void BookHistograms(TDirectory*& dir_diff, uint32_t relay){
  
  TH1D *hElinkDiff = new TH1D("hElinkDiff", Form("Elink diff for Relay: %u",relay), 1000, -10., 10.);
  hElinkDiff->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hElinkDiff->GetYaxis()->SetTitle("Entries");
  hElinkDiff->SetDirectory(dir_diff);
  
  TH1D *hElinkDiff_0 = new TH1D("hElinkDiff_0", Form("Elink diff for Relay: %u, for tctp case : 0",relay), 1000, -10., 10.);
  hElinkDiff_0->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hElinkDiff_0->GetYaxis()->SetTitle("Entries");
  hElinkDiff_0->SetDirectory(dir_diff);

  TH1D *hElinkDiff_1 = new TH1D("hElinkDiff_1", Form("Elink diff for Relay: %u, for tctp case : 1",relay), 1000, -10., 10.);
  hElinkDiff_1->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hElinkDiff_1->GetYaxis()->SetTitle("Entries");
  hElinkDiff_1->SetDirectory(dir_diff);

  TH1D *hUWord = new TH1D("hUWord", Form("Unpacker word diff for Relay: %u",relay), 1000, -10., 10.);
  hUWord->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hUWord->GetYaxis()->SetTitle("Entries");
  hUWord->SetDirectory(dir_diff);

  TH1D *hUWord_0 = new TH1D("hUWord_0", Form("Unpacker word diff for Relay: %u, for tctp case : 0",relay), 1000, -10., 10.);
  hUWord_0->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hUWord_0->GetYaxis()->SetTitle("Entries");
  hUWord_0->SetDirectory(dir_diff);

  TH1D *hUWord_1 = new TH1D("hUWord_1", Form("Unpacker word diff for Relay: %u, for tctp case : 0",relay), 1000, -10., 10.);
  hUWord_1->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hUWord_1->GetYaxis()->SetTitle("Entries");
  hUWord_1->SetDirectory(dir_diff);

  TH1D *hElDiff[2][2][3]; //2 modes(tctp==any or 1), 2 lpGBTs, 3 modules in each lpGBT
  TH1D *hUnpkWordDiff[2][2][3][8]; //2 modes(tctp==any or 1), 2 lpGBTs, 3 modules in each lpGBT 8 words max
  TH1D *hTCEDiff[2][2][3][8]; //2 modes(tctp==any or 1), 2 lpGBTs, 3 modules in each lpGBT, 8 TC energies 
  TH1D *hTCADiff[2][2][3][8]; //2 modes(tctp==any or 1), 2 lpGBTs, 3 modules in each lpGBT, 8 TC addresses
  
  for(int imode=0;imode<2;imode++)
    for(int ilp=0;ilp<2;ilp++)
      for(int imdl=0;imdl<3;imdl++){
	//compressed elinks
	hElDiff[imode][ilp][imdl] = new TH1D(Form("hElDiff_%d_%d_%d",imode,ilp,imdl),Form("%u: (ECONT - Emulator) for tctp case : %d, ilp:%d, module:%d",relay,imode,ilp,imdl), 200, -99, 101);
	hElDiff[imode][ilp][imdl]->SetMinimum(1.e-1);
	hElDiff[imode][ilp][imdl]->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
	hElDiff[imode][ilp][imdl]->GetYaxis()->SetTitle("Entries");
	if(imode==0)
	  hElDiff[imode][ilp][imdl]->SetLineColor(kBlue);
	else
	  hElDiff[imode][ilp][imdl]->SetLineColor(kRed);
	hElDiff[imode][ilp][imdl]->SetDirectory(dir_diff);
      }//imdl loop
  
  
  for(int imode=0;imode<2;imode++)
    for(int ilp=0;ilp<2;ilp++)
      for(int imdl=0;imdl<3;imdl++)
	for(int itc=0;itc<8;itc++){
	  
	  //compressed energy
	  hTCEDiff[imode][ilp][imdl][itc] = new TH1D(Form("hTCEDiff_%d_%d_%d_%d",imode,ilp,imdl,itc),Form("%u: (ECONT - Emulator) encoded energy for tctp case : %d, ilp:%d, module:%d, itc:%d",relay,imode,ilp,imdl,itc), 200, -99, 101);
	  hTCEDiff[imode][ilp][imdl][itc]->SetMinimum(1.e-1);
	  hTCEDiff[imode][ilp][imdl][itc]->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
	  hTCEDiff[imode][ilp][imdl][itc]->GetYaxis()->SetTitle("Entries");
	  if(imode==0)
	    hTCEDiff[imode][ilp][imdl][itc]->SetLineColor(kBlue);
	  else
	    hTCEDiff[imode][ilp][imdl][itc]->SetLineColor(kRed);
	  hTCEDiff[imode][ilp][imdl][itc]->SetDirectory(dir_diff);
	  
	  //address
	  hTCADiff[imode][ilp][imdl][itc] = new TH1D(Form("hTCADiff_%d_%d_%d_%d",imode,ilp,imdl,itc),Form("%u: (ECONT - Emulator) address for tctp case : %d, ilp:%d, module:%d, itc:%d",relay,imode,ilp,imdl,itc), 200, -99, 101);
	  hTCADiff[imode][ilp][imdl][itc]->SetMinimum(1.e-1);
	  hTCADiff[imode][ilp][imdl][itc]->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
	  hTCEDiff[imode][ilp][imdl][itc]->GetYaxis()->SetTitle("Entries");
	  if(imode==0)
	    hTCADiff[imode][ilp][imdl][itc]->SetLineColor(kBlue);
	  else
	    hTCADiff[imode][ilp][imdl][itc]->SetLineColor(kRed);
	  hTCADiff[imode][ilp][imdl][itc]->SetDirectory(dir_diff);
	  
	}//TC/STC loop
      

  for(int imode=0;imode<2;imode++)
    for(int ilp=0;ilp<2;ilp++)
      for(int imdl=0;imdl<3;imdl++)
	for(int iw=0;iw<8;iw++){
	  //compressed word
	  hUnpkWordDiff[imode][ilp][imdl][iw] = new TH1D(Form("hUnpkWordDiff_%d_%d_%d_%d",imode,ilp,imdl,iw),Form("%u: (ECONT - Emulator) unpacker word for tctp case : %d, ilp:%d, module:%d, iw:%d",relay,imode,ilp,imdl,iw), 200, -99, 101);
	  hUnpkWordDiff[imode][ilp][imdl][iw]->SetMinimum(1.e-1);
	  hUnpkWordDiff[imode][ilp][imdl][iw]->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
	  hUnpkWordDiff[imode][ilp][imdl][iw]->GetYaxis()->SetTitle("Entries");
	  if(imode==0)
	    hUnpkWordDiff[imode][ilp][imdl][iw]->SetLineColor(kMagenta);
	  else
	    hUnpkWordDiff[imode][ilp][imdl][iw]->SetLineColor(kRed);
	  hUnpkWordDiff[imode][ilp][imdl][iw]->SetDirectory(dir_diff);

      }//iword


  TH1D *hTC33EDiff = new TH1D("hTC33EDiff", Form("Energy difference for TC 33 for Relay: %u",relay), 1000, -10., 10.);
  hTC33EDiff->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hTC33EDiff->GetYaxis()->SetTitle("Entries");
  hTC33EDiff->SetDirectory(dir_diff);
  
  TH1D *hTC33EDiff_0 = new TH1D("hTC33EDiff_0", Form("Energy difference for TC 33 for Relay: %u for tctp case 0",relay), 1000, -10., 10.);
  hTC33EDiff_0->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hTC33EDiff_0->GetYaxis()->SetTitle("Entries");
  hTC33EDiff_0->SetDirectory(dir_diff);

  TH1D *hTC33EDiff_1 = new TH1D("hTC33EDiff_1", Form("Energy difference for TC 33 for Relay: %u for tctp case 1",relay), 1000, -10., 10.);
  hTC33EDiff_1->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hTC33EDiff_1->GetYaxis()->SetTitle("Entries");
  hTC33EDiff_1->SetDirectory(dir_diff);

  TH1D *hTCEmulfor33 = new TH1D("hTCEmulfor33", Form("Emul TC for data TC 33 for Relay: %u",relay), 50, 0, 50.);
  hTCEmulfor33->GetXaxis()->SetTitle("Emul TC");
  hTCEmulfor33->GetYaxis()->SetTitle("Entries");
  hTCEmulfor33->SetDirectory(dir_diff);

  TH1D *hTC33DataE = new TH1D("hTC33DataE", Form("Matched: Data energy of TC 33 for Relay: %u",relay), 400, -1., 399.);
  hTC33DataE->GetXaxis()->SetTitle("Data energy for TC 33");
  hTC33DataE->GetYaxis()->SetTitle("Entries");
  hTC33DataE->SetDirectory(dir_diff);

  TH1D *hTC33EmulE = new TH1D("hTC33EmulE", Form("Matched: Emul energy of TC 33 for Relay: %u",relay), 400, -1., 399.);
  hTC33EmulE->GetXaxis()->SetTitle("Emul energy for TC 33");
  hTC33EmulE->GetYaxis()->SetTitle("Entries");
  hTC33EmulE->SetDirectory(dir_diff);

  TH1D *hTC33EmulENM = new TH1D("hTC33EmulENM", Form("NonMatching: Emul energy of TC 33 for Relay: %u",relay), 400, -1., 399.);
  hTC33EmulENM->GetXaxis()->SetTitle("Emul energy for TC 33");
  hTC33EmulENM->GetYaxis()->SetTitle("Entries");
  hTC33EmulENM->SetDirectory(dir_diff);

  TH1D *hOccuTCData = new TH1D("hOccuTCData", Form("TC occupancy in data for Relay: %u",relay), 50, -0.5, 49.5);
  hOccuTCData->GetXaxis()->SetTitle("TC");
  hOccuTCData->GetYaxis()->SetTitle("Entries");
  hOccuTCData->SetDirectory(dir_diff);

  TH1D *hOccuTCEmul = new TH1D("hOccuTCEmul", Form("TC occupancy in emulation for Relay: %u",relay), 50, -0.5, 49.5);
  hOccuTCEmul->GetXaxis()->SetTitle("TC");
  hOccuTCEmul->GetYaxis()->SetTitle("Entries");
  hOccuTCEmul->SetDirectory(dir_diff);

  TH2D *hMissedTCs = new TH2D("hMissedTCs", Form("Missed TCs in emulation and data  for Relay: %u",relay), 50, -0.5, 49.5, 50, -0.5, 49.5);
  hMissedTCs->GetXaxis()->SetTitle("Emulation");
  hMissedTCs->GetYaxis()->SetTitle("Data");
  hMissedTCs->SetDirectory(dir_diff);

  TH1D *hSlinkBx8Z = new TH1D("hSlinkBx8Z", Form("ZeroDiff:(Slink BxId)-mod-8 for Relay: %u",relay), 16, -0.5, 15.5);
  hSlinkBx8Z->GetXaxis()->SetTitle("Bx");
  hSlinkBx8Z->GetYaxis()->SetTitle("Entries");
  hSlinkBx8Z->SetDirectory(dir_diff);

  TH1D *hSlinkBx8NZ = new TH1D("hSlinkBx8NZ", Form("NonZeroDiff:(Slink BxId)-mod-8 for Relay: %u",relay), 16, -0.5, 15.5);
  hSlinkBx8NZ->GetXaxis()->SetTitle("Bx");
  hSlinkBx8NZ->GetYaxis()->SetTitle("Entries");
  hSlinkBx8NZ->SetDirectory(dir_diff);
  
  TH1D *hSlinkBx8Matched = new TH1D("hSlinkBx8Matched", Form("ZeroDiff:(Slink BxId)-mod-8 for Relay: %u",relay), 16, -0.5, 15.5);
  hSlinkBx8Matched->GetXaxis()->SetTitle("Bx");
  hSlinkBx8Matched->GetYaxis()->SetTitle("Entries");
  hSlinkBx8Matched->SetDirectory(dir_diff);

  TH1D *hSlinkBx8NoTcTp1 = new TH1D("hSlinkBx8NoTcTp1", Form("ZeroDiff:(Slink BxId)-mod-8 for Relay: %u",relay), 16, -0.5, 15.5);
  hSlinkBx8NoTcTp1->GetXaxis()->SetTitle("Bx");
  hSlinkBx8NoTcTp1->GetYaxis()->SetTitle("Entries");
  hSlinkBx8NoTcTp1->SetDirectory(dir_diff);

  TH1D *hSlinkBx8TcTp1 = new TH1D("hSlinkBx8TcTp1", Form("ZeroDiff:(Slink BxId)-mod-8 for Relay: %u",relay), 16, -0.5, 15.5);
  hSlinkBx8TcTp1->GetXaxis()->SetTitle("Bx");
  hSlinkBx8TcTp1->GetYaxis()->SetTitle("Entries");
  hSlinkBx8TcTp1->SetDirectory(dir_diff);

  TH1D *hSlinkBx8All = new TH1D("hSlinkBx8All", Form("ZeroDiff:(Slink BxId)-mod-8 for Relay: %u",relay), 16, -0.5, 15.5);
  hSlinkBx8All->GetXaxis()->SetTitle("Bx");
  hSlinkBx8All->GetYaxis()->SetTitle("Entries");
  hSlinkBx8All->SetDirectory(dir_diff);

  TH2D *hNZModules = new TH2D("hNZModules", Form("Non-zero modules for Relay: %u",relay), 2, -0.5, 1.5, 3, -0.5, 2.5);
  hNZModules->GetXaxis()->SetTitle("Emulation");
  hNZModules->GetYaxis()->SetTitle("Data");
  hNZModules->SetDirectory(dir_diff);

}

void FillHistogram(bool matchFound, TDirectory*& dir_diff, uint32_t relayNumber, uint64_t event, uint16_t inputbxId,
		   uint32_t nelinks,  const uint32_t *eldata, const uint32_t *elemul,
		   uint32_t ilp, uint32_t imdl,
		   TPGFEDataformat::TcRawDataPacket& tcdata, TPGFEDataformat::TcRawDataPacket& tcemul,
		   const uint32_t *unpkWords, TPGBEDataformat::UnpackerOutputStreamPair& upemul){
  
  int imode = (tcemul.isTcTp1()) ? 1 : 0 ;
  TList *list = (TList *)dir_diff->GetList();

  uint16_t bxIdmod8 = (inputbxId==3564) ? 0xf : (inputbxId%8);
  if(imdl==0 and ilp==0){
    ((TH1D *) list->FindObject("hSlinkBx8All"))->Fill( bxIdmod8 );
    if(imode==0)
      ((TH1D *) list->FindObject("hSlinkBx8NoTcTp1"))->Fill( bxIdmod8 );
    else
      ((TH1D *) list->FindObject("hSlinkBx8TcTp1"))->Fill( bxIdmod8 );
  }
  
  if(matchFound){
    bool hasDiff = false;
    for(uint32_t iel=0;iel<nelinks;iel++){
      ((TH1D *) list->FindObject("hElinkDiff"))->Fill( (eldata[iel] - elemul[iel]) );
      if(imode==0)
	((TH1D *) list->FindObject("hElinkDiff_0"))->Fill( (eldata[iel] - elemul[iel]) );
      else
	((TH1D *) list->FindObject("hElinkDiff_1"))->Fill( (eldata[iel] - elemul[iel]) );
      ((TH1D *) list->FindObject(Form("hElDiff_%d_%d_%d",imode,ilp,imdl)))->Fill( (eldata[iel] - elemul[iel]) );
      if( ((eldata[iel] - elemul[iel]) != 0) and imode==0) hasDiff = true;
    }
    if(hasDiff)
      ((TH2D *) list->FindObject("hNZModules"))->Fill( ilp, imdl );
  
    if(imode==0 and imdl==0 and ilp==0){
      if(hasDiff)
	((TH1D *) list->FindObject("hSlinkBx8NZ"))->Fill( bxIdmod8 );
      else
	((TH1D *) list->FindObject("hSlinkBx8Z"))->Fill( bxIdmod8 );
      ((TH1D *) list->FindObject("hSlinkBx8Matched"))->Fill( bxIdmod8 );
    }
  
    std::vector<int> emulist,econtlist;
    for(uint32_t itc=0;itc<tcdata.size();itc++){
      int imodeloc = (tcemul.getTcData().at(itc).isTcTp1()) ? 1 : 0 ;
      ((TH1D *) list->FindObject(Form("hTCEDiff_%d_%d_%d_%d",imodeloc,ilp,imdl,itc)))->Fill( (tcdata.getTcData().at(itc).energy() - tcemul.getTcData().at(itc).energy()) );
      ((TH1D *) list->FindObject(Form("hTCADiff_%d_%d_%d_%d",imodeloc,ilp,imdl,itc)))->Fill( (tcdata.getTcData().at(itc).address() - tcemul.getTcData().at(itc).address()) );
    
      if(tcdata.getTcData().at(itc).address()==33 and imdl==0){
	((TH1D *) list->FindObject("hTC33EDiff"))->Fill( (tcdata.getTcData().at(itc).energy() - tcemul.getTcData().at(itc).energy()) );
	((TH1D *) list->FindObject("hTCEmulfor33"))->Fill( double(tcemul.getTcData().at(itc).address()) );
	if(imodeloc==0)
	  ((TH1D *) list->FindObject("hTC33EDiff_0"))->Fill( (tcdata.getTcData().at(itc).energy() - tcemul.getTcData().at(itc).energy()) );
	else
	  ((TH1D *) list->FindObject("hTC33EDiff_1"))->Fill( (tcdata.getTcData().at(itc).energy() - tcemul.getTcData().at(itc).energy()) );
	if((tcdata.getTcData().at(itc).energy() - tcemul.getTcData().at(itc).energy())!=0 and tcemul.getTcData().at(itc).address()==33) std::cout<<"CHECK event : " << event << std::endl;
      
	((TH1D *) list->FindObject("hTC33DataE"))->Fill( tcdata.getTcData().at(itc).energy() );
	((TH1D *) list->FindObject("hTC33EmulE"))->Fill( tcemul.getTcData().at(itc).energy() );
      }
      if(tcemul.getTcData().at(itc).address()==33 and  tcdata.getTcData().at(itc).address()!=33){
	((TH1D *) list->FindObject("hTC33EmulENM"))->Fill( tcemul.getTcData().at(itc).energy() );
	//std::cout<<"No data for TC-33 in emulation: " << event << std::endl;
      }
      if(imdl==0){
	((TH1D *) list->FindObject("hOccuTCData"))->Fill( tcdata.getTcData().at(itc).address() );
	((TH1D *) list->FindObject("hOccuTCEmul"))->Fill( tcemul.getTcData().at(itc).address() );
	emulist.push_back(tcemul.getTcData().at(itc).address());
	econtlist.push_back(tcdata.getTcData().at(itc).address());
      }
    }
    if(imdl==0){
      int missingEmulTC = -1;
      int nofmismatches = 0;
      for(const auto& itcemul : emulist){
	if(std::find(econtlist.begin(), econtlist.end(), itcemul) == econtlist.end()) {
	  missingEmulTC = itcemul;
	  nofmismatches++;
	}
      }
      int missingDataTC = -1;
      for(const auto& itcdata : econtlist){
	if(std::find(emulist.begin(), emulist.end(), itcdata) == emulist.end()) {
	  missingDataTC = itcdata;
	}
      }
      if(nofmismatches==1 and missingDataTC!=-1) {
	((TH2D *) list->FindObject("hMissedTCs"))->Fill( missingEmulTC, missingDataTC );
	//std::cout<<"Event with single mismatch : " << event << std::endl;
      }
    
    }
  
    uint16_t *unpkMsTc = new uint16_t[tcdata.size()+1];
    if(relayNumber>1722881092 or imdl==0 or imdl==1){
      uint32_t strm = 0;
      for(uint32_t iw=0;iw<=tcdata.size();iw++){
	unpkMsTc[iw] = (iw==0)? upemul.getMsData(strm) : upemul.getTcData(strm, iw-1);
	uint32_t diff = (iw==0 or tcdata.type()!=TPGFEDataformat::BestC)? ((unpkWords[iw]&0xf) - (unpkMsTc[iw]&0xf)) : (unpkWords[iw] - unpkMsTc[iw]) ;
	//uint32_t XOR = (iw==0 or tcdata.type()!=TPGFEDataformat::BestC)? ((unpkWords[iw]&0xf) xor (unpkMsTc[iw]&0xf)) : (unpkWords[iw] xor unpkMsTc[iw]) ;
      
	int imodeloc = (iw==0) ? ((tcemul.isTcTp1())?1:0) : ((tcemul.getTcData().at(iw-1).isTcTp1())?1:0) ;
	((TH1D *) list->FindObject("hUWord"))->Fill( diff );
	if(imodeloc==0)
	  ((TH1D *) list->FindObject("hUWord_0"))->Fill( diff );
	else
	  ((TH1D *) list->FindObject("hUWord_1"))->Fill( diff );
	if(iw>0) ((TH1D *) list->FindObject( Form("hUnpkWordDiff_%d_%d_%d_%d",imodeloc,ilp,imdl,iw-1) ))->Fill( diff );
	//hUnpkWordDiff[imode][ilp][imdl][iw] = new TH1D(,
      }	    
    }
    delete []unpkMsTc;
  }//if matched
  
}
