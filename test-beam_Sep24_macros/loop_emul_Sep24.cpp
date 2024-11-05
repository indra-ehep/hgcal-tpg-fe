/**********************************************************************
 Created on : 16/10/2024
 Purpose    : Loop over Sep-24 HGCAL testbeam runs for emulation tests
 Author     : Indranil Das, Research Associate
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
#include "FileReader.h"

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
#include "TPGFEReaderSep2024.hh"
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
  const uint32_t noflpGBTs(2);
  bool isTrimming(0);
  uint32_t density(0);
  uint32_t droplsb(1);
  long double maxEvent = 528447 ;         
  std::istringstream issRelay(argv[1]);
  issRelay >> relayNumber;
  std::istringstream issRun(argv[2]);
  issRun >> runNumber;
  std::istringstream issEvents(argv[3]);
  issEvents >> maxEvent;
  std::istringstream issLink(argv[4]);
  issLink >> linkNumber;
  if(linkNumber!=1 and linkNumber!=2){
    std::cerr << "Link number "<< argv[3] <<"is out of bound (use: 1 or 2)" << std::endl;
    return false;
  }
  uint32_t trig_linkNumber = TMath::FloorNint((linkNumber-1)/2);
  std::istringstream issTrimming(argv[5]);
  issTrimming >> isTrimming;
  // std::istringstream issDensity(argv[5]);
  // issDensity >> density;
  // std::istringstream issDropLSB(argv[6]);
  // issDropLSB >> droplsb;
  //===============================================================================================================================

  //===============================================================================================================================
  //Book histograms
  //===============================================================================================================================
  void BookHistograms(TDirectory*&, uint32_t);
  void FillHistogram(bool, bool, TDirectory*&, uint32_t, uint64_t, uint16_t, uint32_t, const uint32_t *, const uint32_t *, uint32_t, uint32_t,
		     TPGFEDataformat::TcRawDataPacket&, TPGFEDataformat::TcRawDataPacket&, const uint32_t *, const uint32_t *,
		     TPGBEDataformat::UnpackerOutputStreamPair&, TPGBEDataformat::UnpackerOutputStreamPair&,
		     std::vector<uint64_t>&, std::vector<uint64_t>&, std::vector<uint64_t>&);
		     //std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>>>&);
  
  TFile *fout = new TFile(Form("Diff_Relay-%u-%u.root",relayNumber,runNumber), "recreate");
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
  
  std::string cfgrocname[2][3][3] = { //nof lpGBTs, nofModules, nofHGCROC per modules
    {
      {"320MLF3WXIH0019_roc0_e2.yaml", "320MLF3WXIH0019_roc1_e2.yaml", "320MLF3WXIH0019_roc2_e2.yaml"},
      {"320MLF3WXIH0020_roc0_e1.yaml", "320MLF3WXIH0020_roc1_e1.yaml", "320MLF3WXIH0020_roc2_e1.yaml"},
      {"320MLF3WXIH0018_roc0_e0.yaml", "320MLF3WXIH0018_roc1_e0.yaml", "320MLF3WXIH0018_roc2_e0.yaml"}
    },
    {
      {"320MLF3WXIH0016_roc0_e2.yaml", "320MLF3WXIH0016_roc1_e2.yaml", "320MLF3WXIH0016_roc1_e2.yaml"},
      {"320MLF3WXIH0017_roc0_e1.yaml", "320MLF3WXIH0017_roc1_e1.yaml", "320MLF3WXIH0017_roc2_e1.yaml"},
      {"320MLF3WXIH0014_roc0_e0.yaml", "320MLF3WXIH0014_roc1_e0.yaml", "320MLF3WXIH0014_roc2_e0.yaml"}
    }
  };

  // std::string cfgrocname[2][3][3] = { //nof lpGBTs, nofModules, nofHGCROC per modules
  //   {
  //    {"320MLF3WXIH0019_roc0_e2.yaml", "320MLF3WXIH0019_roc1_e2.yaml", "320MLF3WXIH0019_roc2_e2.yaml"},
  //    {"320MLF3WXIH0020_roc0_e1.yaml", "320MLF3WXIH0020_roc1_e1.yaml", "320MLF3WXIH0020_roc2_e1.yaml"},
  //    {"320MLF3WXIH0018_roc0_e0.yaml", "320MLF3WXIH0018_roc1_e0.yaml", "320MLF3WXIH0018_roc2_e0.yaml"}
  //   },
  //   {
  //    {"320MLF3WXIH0016_roc0_e2.yaml", "320MLF3WXIH0016_roc1_e2.yaml", "320MLF3WXIH0016_roc2_e2.yaml"},
  //    {"320MLF3WXIH0017_roc0_e1.yaml", "320MLF3WXIH0017_roc1_e1.yaml", "320MLF3WXIH0017_roc2_e1.yaml"},
  //    {"320MLF3WXIH0014_roc0_e0.yaml", "320MLF3WXIH0014_roc1_e0.yaml", "320MLF3WXIH0014_roc2_e0.yaml"}
  //   }
  // };

  for(int ilink=0;ilink<noflpGBTs;ilink++){
    for(int iecond=0;iecond<3;iecond++){
      uint32_t idx = pck.packModId(zside, sector, ilink, det, iecond, selTC4, module); //we assume same ECONT and ECOND number for a given module
      
      cfgs.setModulePath(zside, sector, ilink, det, iecond, selTC4, module);

      switch(ilink){
      case 0:
	switch(iecond){
	case 0:
	  cfgs.setEconTFile("cfgmap/init_econt_e2.yaml");
	  break;
	case 1:
	  cfgs.setEconTFile("cfgmap/init_econt_e1.yaml");
	  break;
	default:
	  cfgs.setEconTFile("cfgmap/init_econt.yaml");
	  break;
	}
      case 1:
	switch(iecond){
	case 0:
	  cfgs.setEconTFile("cfgmap/init_econt_e2.yaml");
	  break;
	case 1:
	  cfgs.setEconTFile("cfgmap/init_econt_e1.yaml");
	  break;
	default:
	  cfgs.setEconTFile("cfgmap/init_econt.yaml");
	  break;
	}
	default:
	  cfgs.setEconTFile("cfgmap/init_econt.yaml");
	  break;
      }
      cfgs.readEconTConfigYaml();

      // if(ilink==1 and (iecond==0)){
      // 	cfgs.setEconTFile("cfgmap/init_econt_mux_test1.yaml");
      // 	cfgs.readEconTConfigYaml();
      // }
      
      cfgs.setEconTFile("cfgmap/init_econt_mux_test1.yaml");
      cfgs.readEconTConfigYaml();

      cfgs.setEconDFile("cfgmap/init_econd.yaml");
      cfgs.readEconDConfigYaml();      
      
      for(uint32_t iroc=0;iroc<3;iroc++){
	std::cout<<"idx: "<<idx<<", ilink: " << ilink << ", iecond: "<<iecond<<", iroc: "<<iroc << ", fname : " << cfgrocname[ilink][iecond][iroc] << std::endl;
	uint32_t rocid_0 = pck.packRocId(zside, sector, ilink, det, iecond, selTC4, module, iroc, 0);
	uint32_t rocid_1 = pck.packRocId(zside, sector, ilink, det, iecond, selTC4, module, iroc, 1);
	//cfgs.setRocFile(Form("cfgmap/slow_control_configuration/configs_with_MasterTDC_and_TPGTRM/%s",cfgrocname[ilink][iecond][iroc].c_str()));
	//cfgs.setRocFile(Form("cfgmap/bt2024-fe-config-trimmed/%s",cfgrocname[ilink][iecond][iroc].c_str()));
	cfgs.setRocFile(Form("cfgmap/configs_v3b_full/%s",cfgrocname[ilink][iecond][iroc].c_str()));
	//cfgs.setRocFile(Form("cfgmap/configs_v3b_full_aggressive/%s",cfgrocname[ilink][iecond][iroc].c_str()));
	cfgs.readRocConfigYaml(rocid_0, rocid_1);	
      }//roc loop
    }//econd loop
  }//lpGBT loop
  if(!isTrimming){
    if(relayNumber==1726581356)
      cfgs.setPedZero();
    else
      cfgs.setPedThZero();
  }
  link = 1; econt = 2;
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
  link = 0; econt = 0;
  uint32_t lp0_bc0 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 0; econt = 1;
  uint32_t lp0_bc1 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 0; econt = 2;
  uint32_t lp0_bc2 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 1; econt = 0;
  uint32_t lp1_stc4a0 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 1; econt = 1;
  uint32_t lp1_stc4a1 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 1; econt = 2;
  uint32_t lp1_stc4a2 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  
  std::map<uint32_t,TPGFEConfiguration::ConfigEconT>& econTPar =  cfgs.getEconTPara();
  for(const auto& it : econTPar){
    econTPar[it.first].setDensity(density);
    econTPar[it.first].setDropLSB(droplsb);
    
    if(it.first==lp0_bc0){
      econTPar[it.first].setSelect(2);
      econTPar[it.first].setNElinks(3);      
    }else if (it.first==lp0_bc1){
      econTPar[it.first].setSelect(2);
      econTPar[it.first].setNElinks(2);     
    }else if (it.first==lp0_bc2){
      econTPar[it.first].setSelect(2);
      econTPar[it.first].setNElinks(2);
    }else if (it.first==lp1_stc4a0){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(3);
      econTPar[it.first].setSTCType(3);
      if(relayNumber>=1727111828){ //1727211141
	econTPar[it.first].setNElinks(2);
	econTPar[it.first].setSTCType(1);
      }
    }else if (it.first==lp1_stc4a1){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(3);
      if(relayNumber>=1727111828){ //1727211141
	econTPar[it.first].setNElinks(2);
	econTPar[it.first].setSTCType(1);
      }
    }else if (it.first==lp1_stc4a2){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(3);
      if(relayNumber>=1727111828){ //1727211141
	econTPar[it.first].setNElinks(2);
	econTPar[it.first].setSTCType(1);
      }
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

  //===============================================================================================================================
  //Modify the HGCROC parameters for special cases
  //===============================================================================================================================
  // std::map<uint64_t,TPGFEConfiguration::ConfigCh>& hrocchcfg =  cfgs.getChPara();
  link = 1; econt = 0; uint32_t roc = 2, hroc = 0;
  uint32_t rocid_0 = pck.packRocId(zside, sector, link, det, econt, selTC4, module, roc, hroc);  
  std::map<uint32_t,TPGFEConfiguration::ConfigHfROC>& hroccfg =  cfgs.getRocPara();
  for(auto const& hroc : hroccfg){
    if(hroc.first == rocid_0) {
      hroccfg[hroc.first].setTotP(2, 13);
      hroccfg[hroc.first].setTotP(3, 0);
      hroccfg[hroc.first].setTotTH(2, 107);
    }
  }
  // ===============================================================================================================================

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
  // econDReader.setTotUp(0);
  // econDReader.checkEvent(1);
  // econDReader.showFirstEvents(10);
  //===============================================================================================================================
  //Set and Initialize the Emulator
  //===============================================================================================================================
  TPGFEModuleEmulation::HGCROCTPGEmulation rocTPGEmul(cfgs);
  TPGFEModuleEmulation::ECONTEmulation econtEmul(cfgs);
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Set refernce eventlist
  //===============================================================================================================================
  /*TcTp2 events */ uint64_t refEvt[50] = {66235, 127532, 145302, 151731, 194816, 260756, 269698, 383736, 391481, 397781, 471496, 534349, 605680, 613325, 694404, 873524, 919213, 1085378, 1124673, 1164106, 1222670, 1247896, 1278016, 1322821, 1453497, 1486909, 1533310, 1623036, 1661345, 1763515, 1769677, 1809836, 1829961, 1871023, 1932945, 1967257, 1968826, 1981516, 1984675, 2043998, 2060794, 2068723, 2165732, 2172380, 2237980, 2245408, 2259447, 2390648, 2462559, 2514946};
  // /*Nonzero diff events*/ uint64_t refEvt[6] = {66235, 471496, 1763515, 1968826, 2245408, 2390648, };
  // /*Unexpect TC in Data in event*/ uint64_t refEvt[48] = {66235, 127532, 145302, 151731, 194816, 260756, 269698, 383736, 391481, 397781, 471496, 534349, 605680, 613325, 694404, 873524, 919213, 1085378, 1124673, 1164106, 1222670, 1247896, 1278016, 1322821, 1453497, 1486909, 1533310, 1623036, 1661345, 1763515, 1769677, 1829961, 1871023, 1967257, 1968826, 1981516, 1984675, 2043998, 2060794, 2068723, 2165732, 2172380, 2237980, 2245408, 2259447, 2390648, 2462559, 2514946, };
  // /*Unexpect TC in Emul in event*/ uint64_t refEvt[48] = {66235, 127532, 145302, 151731, 194816, 260756, 269698, 383736, 391481, 397781, 471496, 534349, 605680, 613325, 694404, 873524, 919213, 1085378, 1124673, 1164106, 1222670, 1247896, 1278016, 1322821, 1453497, 1486909, 1533310, 1623036, 1661345, 1763515, 1769677, 1829961, 1871023, 1967257, 1968826, 1981516, 1984675, 2043998, 2060794, 2068723, 2165732, 2172380, 2237980, 2245408, 2259447, 2390648, 2462559, 2514946, };
  // /*Difference in elink and Stage1 output in event*/ uint64_t refEvt[21] = {66235, 151731, 471496, 633065, 873524, 1366513, 1589006, 1623036, 1763515, 1968826, 1984675, 2000515, 2091972, 2165732, 2245408, 2250959, 2259447, 2264639, 2335560, 2390648, 2489154, };
  
  // /*Nonzero diff events*/ uint64_t refEvt[270] = {501, 1228, 1979, 2380, 3681, 8947, 9150, 14269, 16630, 17084, 18150, 18317, 21495, 22730, 23262, 23752, 25429, 26124, 29583, 31233, 32440, 32477, 33355, 37255, 37449, 40088, 43779, 44198, 44846, 45149, 47482, 47827, 52122, 53122, 53246, 53594, 60112, 61817, 61920, 61998, 62007, 62759, 69903, 72231, 74371, 76238, 77216, 79129, 81230, 82271, 83465, 86466, 92519, 93599, 96274, 101000, 101567, 102974, 105747, 107379, 108193, 112965, 113587, 116182, 118269, 119496, 122610, 123616, 126640, 130274, 133164, 136833, 138315, 138454, 144578, 148644, 149259, 149327, 154429, 155116, 156033, 156548, 158696, 164637, 166925, 168467, 169855, 171068, 172813, 174742, 175530, 178176, 179676, 180198, 183192, 183301, 183870, 185770, 185850, 186747, 187132, 188741, 191168, 191601, 193457, 195624, 195704, 200596, 201380, 205918, 214989, 216693, 217454, 220286, 221250, 226654, 228898, 230011, 234331, 235128, 239364, 239522, 244280, 245480, 246189, 248438, 253417, 253444, 253639, 254473, 255453, 255539, 257294, 259392, 260267, 266090, 266339, 266375, 268465, 270534, 272028, 275391, 275586, 275684, 280761, 282453, 288238, 288265, 288671, 288952, 293754, 294123, 304376, 307535, 308665, 310190, 311899, 312449, 312885, 312989, 313814, 314962, 315386, 318294, 319656, 319784, 326650, 328145, 331944, 333993, 337060, 339686, 341165, 344105, 344339, 344803, 347128, 347180, 350848, 354262, 354825, 359931, 360062, 360889, 362145, 362156, 363223, 363859, 365343, 368827, 369290, 375588, 377732, 380036, 380556, 381311, 382065, 383543, 386925, 388552, 389500, 392122, 402780, 405279, 405973, 406277, 407570, 409453, 409685, 414396, 415372, 417436, 419979, 421046, 422401, 425935, 430696, 431549, 432358, 433485, 436630, 437676, 442969, 446320, 446744, 447255, 448814, 450377, 450818, 452659, 452790, 453222, 453694, 453747, 457618, 458773, 458791, 460826, 464808, 464810, 465623, 467478, 467591, 468038, 468130, 471782, 472300, 473956, 474238, 475435, 476611, 477610, 480893, 488623, 490151, 490796, 495806, 504806, 507106, 509698, 512441, 512946, 516025, 521430, 521858, 522489, 523397, 523916, 525766, 526835};
  // /*Possible corruped events */ uint64_t refEvt[1] = {337371, };

  ///*Nonzero diff events*/ uint64_t refEvt[392] = {2178, 2380, 4379, 4388, 5466, 8940, 8947, 9052, 10813, 13445, 13813, 16706, 17084, 17704, 18255, 18895, 21877, 22923, 24340, 24452, 26322, 26570, 29583, 30058, 31233, 32846, 32899, 33033, 33892, 34767, 37255, 37449, 39264, 40288, 42198, 42355, 43779, 45149, 47482, 48283, 48989, 50258, 50307, 51188, 51867, 52433, 53246, 55754, 55863, 56248, 57255, 59359, 59448, 60205, 61344, 61954, 61998, 64261, 69530, 69903, 70944, 71777, 71951, 71962, 73507, 74296, 74363, 75623, 76620, 77410, 80132, 80627, 80773, 86179, 91284, 91508, 92434, 92762, 93652, 93813, 95240, 96971, 100078, 102935, 103251, 103638, 104323, 105362, 107379, 109121, 109452, 110169, 114239, 116244, 120113, 120173, 125292, 127434, 130092, 130274, 130723, 131459, 132242, 132429, 132452, 133164, 133194, 133369, 133603, 134263, 136255, 136404, 138428, 139678, 140837, 142420, 144039, 147351, 147996, 148028, 149684, 150533, 151614, 153956, 154429, 154450, 156612, 161478, 161675, 161933, 163438, 164637, 166176, 166827, 167621, 167748, 168309, 168887, 169855, 172554, 173705, 174416, 174742, 177883, 180228, 181490, 182946, 183896, 184711, 185876, 186747, 187854, 188741, 192468, 195704, 198607, 199148, 200205, 200403, 200596, 201406, 202761, 204234, 206319, 209122, 210303, 212292, 212937, 213354, 214575, 216693, 218165, 219202, 219535, 223128, 223259, 224638, 225720, 225932, 226536, 226654, 227974, 229216, 229523, 233745, 233988, 234410, 236719, 236766, 238963, 239850, 241031, 242335, 247629, 248210, 248438, 248902, 249448, 250259, 250426, 251375, 254308, 254473, 255548, 257396, 258822, 258930, 261438, 261905, 264202, 266090, 268465, 272028, 273200, 273343, 273574, 275557, 276466, 276484, 278236, 279523, 280685, 280694, 281956, 287568, 288265, 288474, 290171, 293324, 302456, 304376, 308665, 310190, 311004, 312989, 313814, 316604, 317961, 318503, 319784, 320711, 320986, 326464, 326650, 326713, 327681, 329127, 331681, 334678, 336428, 338253, 338456, 339257, 341109, 342017, 346261, 347128, 347724, 348755, 353024, 354049, 355068, 357923, 360906, 361197, 361521, 362820, 363859, 363901, 366179, 367996, 369290, 371662, 371842, 373486, 378566, 379465, 379496, 379506, 380675, 381311, 386388, 387775, 388552, 389355, 390063, 390581, 390696, 392953, 394361, 395645, 397333, 397545, 398689, 402780, 403039, 404389, 406145, 406863, 407752, 408287, 408729, 411123, 411422, 413127, 415172, 416492, 417178, 419395, 419623, 420923, 421046, 424883, 425337, 425660, 425853, 427105, 429577, 429637, 431857, 436520, 439479, 439974, 442913, 443218, 447611, 447909, 448422, 448814, 452790, 453222, 457125, 457277, 457618, 458791, 460125, 460826, 464846, 465169, 470938, 471782, 473416, 473956, 474238, 474249, 474738, 475435, 477610, 477805, 480346, 480893, 481754, 482813, 484218, 485041, 487451, 490341, 491044, 491658, 491979, 493966, 494270, 494437, 494695, 497785, 499102, 499573, 502922, 502997, 503640, 504806, 505895, 506899, 507239, 507365, 509713, 510766, 511146, 511186, 512441, 514361, 514437, 514846, 516025, 516090, 516124, 521858, 522127, 523566, 523916, 524586, 527124};

  // /*Nonzero diff events*/ uint64_t refEvt[63] = {2380, 8947, 17084, 29583, 31233, 37255, 37449, 43779, 45149, 47482, 53246, 61998, 69903, 107379, 130274, 133164, 154429, 164637, 169855, 174742, 186747, 188741, 195704, 200596, 216693, 226654, 248438, 254473, 266090, 268465, 272028, 288265, 304376, 308665, 310190, 312989, 313814, 319784, 326650, 347128, 363859, 369290, 381311, 388552, 402780, 421046, 448814, 452790, 453222, 457618, 458791, 460826, 471782, 473956, 474238, 475435, 477610, 480893, 504806, 512441, 516025, 521858, 523916};
  
  std::vector<uint64_t> refEvents;
  //for(int ievent=0;ievent<1020;ievent++) refEvents.push_back(refEvt[ievent]);
  for(int ievent=0;ievent<50;ievent++) refEvents.push_back(refEvt[ievent]);
  refEvents.resize(0);
  //===============================================================================================================================
  
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>>> hrocarray; //event,rocId
  std::map<uint64_t,uint32_t> eventbx;
  std::vector<uint64_t> eventList;
  std::vector<uint64_t> nonZeroEvents;
  std::vector<uint64_t> shiftedBxEvent;
  std::vector<uint64_t> corruptEvent;
  std::vector<uint64_t> unExDataTC;
  std::vector<uint64_t> unExEmulTC;
  std::vector<uint64_t> elStg1Event;
  std::vector<uint64_t> TcTp1Events;
  std::vector<uint64_t> TcTp2Events;
  std::vector<uint64_t> TcTp3Events;
  std::vector<uint64_t> TcTp1TrigEvents;
  std::vector<uint64_t> TcTp2TrigEvents;
  std::vector<uint64_t> TcTp3TrigEvents;
  std::vector<uint64_t> nonZeroTcTp0Events;
  std::vector<uint64_t> nonZeroTcTp1Events;
  std::vector<uint64_t> nonZeroTcTp2Events;
  std::vector<uint64_t> nonZeroTcTp3Events;
  
  uint64_t minEventDAQ, maxEventDAQ;  
  //const long double maxEvent = 1000000 ;
  //const long double maxEvent = 2557415 ;     //relay:1726581356
  //const long double maxEvent = 528447 ;         //relay:1727211141
  long double nloopEvent =  100000;
  //const long double maxEvent = 1000  ; //1722870998:24628, 1722871979:31599
  //long double nloopEvent = 1000 ;
  int nloop = TMath::CeilNint(maxEvent/nloopEvent) ;
  if(refEvents.size()>0) nloop = 1;
  std::cout<<"nloop: "<<nloop<<std::endl;
  std::cerr<<"nloop: "<<nloop<<std::endl;
  uint64_t processed_events = 0;
  for(int ieloop=0;ieloop<nloop;ieloop++){
    
    minEventDAQ = ieloop*nloopEvent ;
    maxEventDAQ = (ieloop+1)*nloopEvent;
    maxEventDAQ = (maxEventDAQ>uint64_t(maxEvent)) ? uint64_t(maxEvent) : maxEventDAQ ;
    
    printf("iloop : %d, minEventDAQ = %lu, maxEventDAQ = %lu\n",ieloop,minEventDAQ, maxEventDAQ);
    std::cerr<<"iloop : "<< ieloop <<", minEventDAQ = "<< minEventDAQ <<", maxEventDAQ = " << maxEventDAQ << std::endl;

    hrocarray.clear();
    eventList.clear();
    econtarray.clear();
    
    econTReader.setNofCAFESep(11);
    if(relayNumber>=1727211141) econTReader.setNofCAFESep(14);
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
    
    for(uint64_t ievt = 0 ; ievt < eventList.size(); ievt++ ){
      uint64_t ievent = eventList[ievt] ;
      if(econtarray.at(ievent).size()!=6 or hrocarray.at(ievent).size()!=36) continue;
      std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>> datalist = hrocarray[ievent] ;
      if(ievt<10) std::cout << "ROC ievent: "<< ievent<< ", datalist size: "<< datalist.size() << std::endl;
      for(const auto& hrocit : datalist){
	const TPGFEDataformat::HalfHgcrocData& hrocdata = hrocit.second ;
	//if(testmodid==pck.getModIdFromRocId(uint32_t(hrocit.first))){
	//if(event<100 and (iecond==1 and ilink==0)){
	if(ievt<10) std::cout << "ievent: "<< ievent<< "\t data for modid : "<< pck.getModIdFromRocId(uint32_t(hrocit.first)) << ",\t half-roc_id: "<< hrocit.first <<  std::endl;
	if(ievt<10) hrocdata.print();
	if(hrocdata.hasTcTp(1) and (std::find(TcTp1Events.begin(),TcTp1Events.end(),ievent) == TcTp1Events.end())) TcTp1Events.push_back(ievent);
	if(hrocdata.hasTcTp(2) and (std::find(TcTp2Events.begin(),TcTp2Events.end(),ievent) == TcTp2Events.end())) TcTp2Events.push_back(ievent);
	if(hrocdata.hasTcTp(3) and (std::find(TcTp3Events.begin(),TcTp3Events.end(),ievent) == TcTp3Events.end())) TcTp3Events.push_back(ievent);
      }
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
      if(econtarray.find(event) == econtarray.end() or hrocarray.find(event) == hrocarray.end() or econtarray.at(event).size()!=6 or hrocarray.at(event).size()!=36) {
	if(std::find(corruptEvent.begin(),corruptEvent.end(),event) == corruptEvent.end()) corruptEvent.push_back(event);
	continue;
      }
      //if(econtarray.find(event) == econtarray.end()) break;

      //bool eventCondn = (event<1000 or event==1560 or event==2232 or event==2584 or event==2968 or event==3992);
      //bool eventCondn = (event<1000);
      bool eventCondn = (refEvents.size()==0) ? (ievt<10) : (ievt<refEvents.size());
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
	      emul_bx = econd_bx - 0;
	      if(emul_bx<0) emul_bx = (3564+emul_bx) + 1;
	      emul_bx_4b = (rocdata[data.first].getBx()==3564) ? 0xF : (emul_bx%8);
	      econd_bx_4b = (rocdata[data.first].getBx()==3564) ? 0xF : (econd_bx%8);
	      //tune the bx here to match the TPG
	      if(relayNumber==1726581356)
		if(rocdata[data.first].getBx()==3564)
		  emul_bx_4b = 1;
		else if(rocdata[data.first].getBx()==3563)
		  emul_bx_4b = 0xF;
		else if(emul_bx_4b==7)
		  emul_bx_4b = 0x0;
		else
		  emul_bx_4b += 1;	      
	      if(relayNumber==1726435152) emul_bx_4b += 0;
	      if(relayNumber==1722698259) emul_bx_4b += 7;
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
	  
	  // if(ilink==1 and iecond==0)
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
	  TcRawdata.second.setBX(emul_bx_4b);
	
	  uint32_t *elinkemul = new uint32_t[econTPar[moduleId].getNElinks()];
	  TPGFEModuleEmulation::ECONTEmulation::convertToElinkData(emul_bx_4b, TcRawdata.second, elinkemul);
	  
	  if(TcRawdata.second.isTcTp1() and (std::find(TcTp1TrigEvents.begin(),TcTp1TrigEvents.end(),event) == TcTp1TrigEvents.end())) TcTp1TrigEvents.push_back(event);
	  if(TcRawdata.second.isTcTp2() and (std::find(TcTp2TrigEvents.begin(),TcTp2TrigEvents.end(),event) == TcTp2TrigEvents.end())) TcTp2TrigEvents.push_back(event);
	  if(TcRawdata.second.isTcTp3() and (std::find(TcTp3TrigEvents.begin(),TcTp3TrigEvents.end(),event) == TcTp3TrigEvents.end())) TcTp3TrigEvents.push_back(event);
	  
	  std::vector<std::pair<uint32_t,TPGBEDataformat::Trig24Data>> econtdata =  econtarray[event];
	  
	  TPGBEDataformat::UnpackerOutputStreamPair upemul,up1;
	  TPGFEDataformat::TcRawDataPacket vTC1;	
    	  int refbx = -1;
	  TPGBEDataformat::Trig24Data trdata;
	  int tpg_m3_bxid = -1, tpg_p3_bxid = -1; 
	  for(const auto& econtit : econtdata){
	    if(econtit.first!=moduleId) continue;
	    trdata = econtit.second ;
	    if(eventCondn) std::cout << "event: " << event << ", moduleId : " << econtit.first << std::endl;
	    //if(eventCondn and iecond==0 and ilink==0) TcRawdata.second.print();
	    bool hasMatched = false;
	    for(int ibx=0;ibx<7;ibx++){
	      const uint32_t *el = trdata.getElinks(ibx); 
	      uint32_t bx_2 = (el[0]>>28) & 0xF;
	      if(ibx==0) tpg_m3_bxid = bx_2;
	      if(ibx==6) tpg_p3_bxid = bx_2;
	      if(bx_2==emul_bx_4b) {
	      //if(ibx==3) {
		refbx = ibx;
		hasMatched = true;
	      }//match found;
	      if(eventCondn and iecond==0 and ilink==0) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::BestC, 6, el, vTC1);
	      if(eventCondn and iecond==1 and ilink==0) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::BestC, 4, el, vTC1);
	      if(eventCondn and iecond==2 and ilink==0) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::BestC, 4, el, vTC1);
	      if(relayNumber>=1727111828){
		if(eventCondn and iecond==0 and ilink==1) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC16, 3, el, vTC1);
		if(eventCondn and iecond==1 and ilink==1) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC16, 3, el, vTC1);
		if(eventCondn and iecond==2 and ilink==1) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC16, 3, el, vTC1);
	      }else{
		if(eventCondn and iecond==0 and ilink==1) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC4A, 10, el, vTC1);
		if(eventCondn and iecond==1 and ilink==1) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC4A, 6, el, vTC1);
		if(eventCondn and iecond==2 and ilink==1) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC4A, 6, el, vTC1);
	      }
	      if(eventCondn) TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(bx_2, vTC1, up1);
	      //if(eventCondn) vTC1.print();
	      // if(eventCondn) up1.print();
	      if(hasMatched) break;
	    }//bx loop
	    //if(eventCondn) trdata.print();
	    //if(hasMatched) break;
	  }//loop over econt data for a given run
	  if(refbx==-1){
	    //std::cout << "refbx: " << refbx << std::endl ;
	    if(std::find(shiftedBxEvent.begin(),shiftedBxEvent.end(),event) == shiftedBxEvent.end()) shiftedBxEvent.push_back(event);
	    econt_slink_bx = trdata.getSlinkBx();
	    FillHistogram(false, false, dir_diff, relayNumber, event, econt_slink_bx, econTPar[moduleId].getNElinks(), 0x0, elinkemul, ilink, iecond, vTC1, TcRawdata.second, 0x0, 0x0, up1, upemul,
			  unExEmulTC, unExDataTC, elStg1Event);
			  //hrocarray);
	    delete []elinkemul;
	    std::cout<<std::endl<<std::endl<<"=========================================================================="<<std::endl<<"Skipping event: " << event <<std::endl<<std::endl<<std::endl;
	    std::cout << "Skip Event: "<< event
		      << ", ECOND:: (econd_slink_bx: " << econd_slink_bx <<", econd_bx: "<<econd_bx<<", econd econd_bx_4b : " <<  econd_bx_4b
		      << ") ECONT:: (econt_slink_bx: " << trdata.getSlinkBx() <<", econt_slink_bx%8: "<< (trdata.getSlinkBx()%8) <<", TPG bxid range : [" << tpg_m3_bxid << ", " << tpg_p3_bxid << "] "
		      <<"), moduleId : " << moduleId << std::endl;
	    continue;
	  }
	  // if(iecond==0) refbx = 5;
	  const uint32_t *elmid = trdata.getElinks(3);
	  econt_central_bx_4b = (elmid[0]>>28) & 0xF;
	  econt_slink_bx = trdata.getSlinkBx();
	  const uint32_t *eldata = trdata.getElinks(refbx);
	  const uint32_t *unpkWords = trdata.getUnpkWords(refbx);
	  const uint32_t *unpkWords1 = trdata.getUnpkWords(refbx,1);
	  uint32_t bx_data = (eldata[0]>>28) & 0xF;
	  TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TcRawdata.second.type(), TcRawdata.second.size(), eldata, vTC1);	  
	  TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(emul_bx_4b, TcRawdata.second, upemul);
	  TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(bx_data, vTC1, up1);
	  
	  bool hasTcTp1 = false;
	  if(TcRawdata.second.isTcTp1()) hasTcTp1 = true;
	  bool isLargeDiff = false;
	  for(uint32_t iel=0;iel<econTPar[moduleId].getNElinks();iel++){
	    //if( ((eldata[iel]-elinkemul[iel]) != 0) and hasTcTp1==false) isLargeDiff = true;
	    if((eldata[iel]-elinkemul[iel]) != 0) isLargeDiff = true;
	  }
	  bool printCondn = (eventCondn or isLargeDiff);
	  //bool printCondn = (eventCondn or isLargeDiff) and (ilink==1 and iecond==0);
	  //bool printCondn = isLargeDiff;
	  if(isLargeDiff)
	    std::cout<<std::endl<<std::endl<<"=========================================================================="<<std::endl<<"Processing event: " << event <<std::endl<<std::endl<<std::endl;
	  if(printCondn) std::cout << "Elink comparison for  ievent: "<< event << ", moduleId : " << moduleId << std::endl;
	  if(printCondn) std::cout << "Event: "<< event
						  << ", ECOND:: (econd_slink_bx: " << econd_slink_bx <<", econd_bx: "<<econd_bx<<", econd econd_bx_4b : " <<  econd_bx_4b
						  << ") ECONT:: (econt_slink_bx: " << econt_slink_bx <<", econt_slink_bx%8: "<< (econt_slink_bx%8) <<", econt_central_bx_4b: " << econt_central_bx_4b
						  <<"), moduleId : " << moduleId <<", isLargeDiff: " << isLargeDiff << std::endl;
	  //if(printCondn) modTcdata.second.print();
	  if(printCondn) TcRawdata.second.print();
	  if(printCondn) vTC1.print();
          // if(printCondn) up1.print();
	  // if(printCondn) upemul.print();
	  // if(printCondn) trdata.print();
	  // 
	  for(uint32_t iel=0;iel<econTPar[moduleId].getNElinks();iel++){
	    if(printCondn) std::cout<<"Elink emul : 0x" << std::hex << std::setfill('0') << std::setw(8) << elinkemul[iel] << std::dec ;
	    if(printCondn) std::cout<<"\t Elink data : 0x" << std::hex << std::setfill('0') << std::setw(8) << eldata[iel] << std::dec ;//<< std::endl;
	    if(printCondn) std::cout<<"\t Diff: " << std::setfill(' ') << std::setw(10) << (eldata[iel]-elinkemul[iel])  << ", XOR: " << std::bitset<32>{(eldata[iel] ^ elinkemul[iel])} << std::endl;
	    if( ((eldata[iel]-elinkemul[iel]) != 0) and (std::find(nonZeroEvents.begin(),nonZeroEvents.end(),event) == nonZeroEvents.end())) nonZeroEvents.push_back(event);
	    if( ((eldata[iel]-elinkemul[iel]) != 0) and (std::find(nonZeroTcTp0Events.begin(),nonZeroTcTp0Events.end(),event) == nonZeroTcTp0Events.end()) and  !(TcRawdata.second.isTcTp1()) and !(TcRawdata.second.isTcTp2()) and !(TcRawdata.second.isTcTp3())) nonZeroTcTp0Events.push_back(event);
	    if( ((eldata[iel]-elinkemul[iel]) != 0) and (std::find(nonZeroTcTp1Events.begin(),nonZeroTcTp1Events.end(),event) == nonZeroTcTp1Events.end()) and TcRawdata.second.isTcTp1() ) nonZeroTcTp1Events.push_back(event);
	    if( ((eldata[iel]-elinkemul[iel]) != 0) and (std::find(nonZeroTcTp2Events.begin(),nonZeroTcTp2Events.end(),event) == nonZeroTcTp2Events.end()) and TcRawdata.second.isTcTp2() ) nonZeroTcTp2Events.push_back(event);
	    if( ((eldata[iel]-elinkemul[iel]) != 0) and (std::find(nonZeroTcTp3Events.begin(),nonZeroTcTp3Events.end(),event) == nonZeroTcTp3Events.end()) and TcRawdata.second.isTcTp3() ) nonZeroTcTp3Events.push_back(event);
	  }
	  // uint16_t *unpkMsTc = new uint16_t[TcRawdata.second.size()+1];
	  // uint32_t strm = 0;
	  // for(uint32_t iw=0;iw<=TcRawdata.second.size();iw++){
	  //   unpkMsTc[iw] = (iw==0)? upemul.getMsData(strm) : upemul.getTcData(strm, iw-1);
	  //   bool diffcondn = (iw==0 and (TcRawdata.second.type()==TPGFEDataformat::STC4A or TcRawdata.second.type()==TPGFEDataformat::CTC4A or TcRawdata.second.type()==TPGFEDataformat::STC16));
	  //   uint32_t diff = (diffcondn)? ((unpkWords[iw]&0xf) - (unpkMsTc[iw]&0xf)) : (unpkWords[iw] - unpkMsTc[iw]) ;
	  //   uint32_t XOR = (diffcondn)? ((unpkWords[iw]&0xf) xor (unpkMsTc[iw]&0xf)) : (unpkWords[iw] xor unpkMsTc[iw]) ;
	  //   if(printCondn) std::cout<<"Stage1 unpacker Words emul : 0x" << std::hex << ::std::setfill('0') << std::setw(4) << unpkWords[iw] << std::dec ;
	  //   if(printCondn) std::cout<<"\t  data : 0x" << std::hex << ::std::setfill('0') << std::setw(4) << unpkMsTc[iw] << std::dec ;
	  //   if(printCondn) std::cout<<"\t Diff: " << std::setfill(' ') << std::setw(10) << diff  << ", XOR: " << std::bitset<32>{XOR} << std::dec << std::endl;
	  //   }	    
	  FillHistogram(true, isLargeDiff, dir_diff, relayNumber, event, econt_slink_bx, econTPar[moduleId].getNElinks(), eldata, elinkemul, ilink, iecond, vTC1, TcRawdata.second, unpkWords, unpkWords1, up1, upemul,
			unExEmulTC, unExDataTC, elStg1Event);//, hrocarray);	  
	  delete []elinkemul;
	  // delete []unpkMsTc;
	}//econd loop
      }//ilink loop
      processed_events++;
    }//event loop
  }//ieloop
  //===============================================================================================================================
  // uint32_t relayNumber(0);
  // uint32_t runNumber(0);
  // uint32_t linkNumber(0);
  // const uint32_t noflpGBTs(2);
  // bool isTrimming(0);
  // uint32_t density(0);
  // uint32_t droplsb(1);
  // long double maxEvent = 528447 ;         
  
  std::cerr << " RelayNo. " << "| "
	    << " RunN. " << "| "
	    << " maxEvent " << "| "
	    << " processed " << "| "
	    << " TcTp1_ROC/TcTp1_TPG | "
    	    << " TcTp2_ROC/TcTp2_TPG | "
    	    << " TcTp3_ROC/TcTp3_TPG | "
	    << " NCorrp " << "| "
	    << " NSftBx " << "| "
	    << " NFEEmulMM " << "| "
    	    << " NFEEmulTcTp0MM " << "| "
	    << " NFEEmulTcTp1MM " << "| "
	    << " NFEEmulTcTp2MM " << "| "
	    << " NFEEmulTcTp3MM " << "| "
	    << " NElStg1MM " << "| "
	    << std::endl;
  
  std::cerr << relayNumber << "| "
	    << runNumber << "| "
	    << (maxEvent-1) << "| "
	    << processed_events << "| "
	    << TcTp1Events.size() << "/" << TcTp1TrigEvents.size() << "| "
    	    << TcTp2Events.size() << "/" << TcTp2TrigEvents.size() << "| "
    	    << TcTp3Events.size() << "/" << TcTp3TrigEvents.size() << "| "
	    << corruptEvent.size() << "| "
	    << shiftedBxEvent.size() << "| "
	    << nonZeroEvents.size() << "| "
    	    << nonZeroTcTp0Events.size() << "| "
	    << nonZeroTcTp1Events.size() << "| "
	    << nonZeroTcTp2Events.size() << "| "
	    << nonZeroTcTp3Events.size() << "| "
	    << elStg1Event.size() << "| "
	    << std::endl;

  // if(TcTp1Events.size()>0) std::cerr<< "/*TcTp1 events */ uint64_t refEvt["<< TcTp1Events.size() <<"] = {";
  // for(const uint64_t& totEvt : TcTp1Events) std::cerr<<totEvt << ", ";
  // if(TcTp1Events.size()>0) std::cerr<< "};" << std::endl;

  // if(TcTp2Events.size()>0) std::cerr<< "/*TcTp2 events */ uint64_t refEvt["<< TcTp2Events.size() <<"] = {";
  // for(const uint64_t& totEvt : TcTp2Events) std::cerr<<totEvt << ", ";
  // if(TcTp2Events.size()>0) std::cerr<< "};" << std::endl;

  // if(TcTp3Events.size()>0) std::cerr<< "/*TcTp3 events */ uint64_t refEvt["<< TcTp3Events.size() <<"] = {";
  // for(const uint64_t& totEvt : TcTp3Events) std::cerr<<totEvt << ", ";
  // if(TcTp3Events.size()>0) std::cerr<< "};" << std::endl;

  // if(TcTp1TrigEvents.size()>0) std::cerr<< "/*TcTp1 Trig events */ uint64_t refEvt["<< TcTp1TrigEvents.size() <<"] = {";
  // for(const uint64_t& totEvt : TcTp1TrigEvents) std::cerr<<totEvt << ", ";
  // if(TcTp1TrigEvents.size()>0) std::cerr<< "};" << std::endl;

  // if(TcTp2TrigEvents.size()>0) std::cerr<< "/*TcTp2 Trig events */ uint64_t refEvt["<< TcTp2TrigEvents.size() <<"] = {";
  // for(const uint64_t& totEvt : TcTp2TrigEvents) std::cerr<<totEvt << ", ";
  // if(TcTp2TrigEvents.size()>0) std::cerr<< "};" << std::endl;

  // if(TcTp3TrigEvents.size()>0) std::cerr<< "/*TcTp3 Trig events */ uint64_t refEvt["<< TcTp3TrigEvents.size() <<"] = {";
  // for(const uint64_t& totEvt : TcTp3TrigEvents) std::cerr<<totEvt << ", ";
  // if(TcTp3TrigEvents.size()>0) std::cerr<< "};" << std::endl;

  // if(nonZeroEvents.size()>0) std::cerr<< "/*Nonzero diff events*/ uint64_t refEvt["<< nonZeroEvents.size() <<"] = {";
  // for(const uint64_t& totEvt : nonZeroEvents) std::cerr<<totEvt << ", ";
  // if(nonZeroEvents.size()>0) std::cerr<< "};" << std::endl;

  // if(shiftedBxEvent.size()>0) std::cerr<< "/*Shifted Bx/events */ uint64_t refEvt["<< shiftedBxEvent.size() <<"] = {";
  // for(const uint64_t& totEvt : shiftedBxEvent) std::cerr<<totEvt << ", ";
  // if(shiftedBxEvent.size()>0) std::cerr<< "};" << std::endl;

  // if(corruptEvent.size()>0) std::cerr<< "/*Possible corruped events */ uint64_t refEvt["<< corruptEvent.size() <<"] = {";
  // for(const uint64_t& totEvt : corruptEvent) std::cerr<<totEvt << ", ";
  // if(corruptEvent.size()>0) std::cerr<< "};" << std::endl;

  // if(unExDataTC.size()>0) std::cerr<< "/*Unexpect TC in Data in event*/ uint64_t refEvt["<< unExDataTC.size() <<"] = {";
  // for(const uint64_t& totEvt : unExDataTC) std::cerr<<totEvt << ", ";
  // if(unExDataTC.size()>0) std::cerr<< "};" << std::endl;

  // if(unExEmulTC.size()>0) std::cerr<< "/*Unexpect TC in Emul in event*/ uint64_t refEvt["<< unExEmulTC.size() <<"] = {";
  // for(const uint64_t& totEvt : unExEmulTC) std::cerr<<totEvt << ", ";
  // if(unExEmulTC.size()>0) std::cerr<< "};" << std::endl;

  // if(elStg1Event.size()>0) std::cerr<< "/*Difference in elink and Stage1 output in event*/ uint64_t refEvt["<< elStg1Event.size() <<"] = {";
  // for(const uint64_t& totEvt : elStg1Event) std::cerr<<totEvt << ", ";
  // if(elStg1Event.size()>0) std::cerr<< "};" << std::endl;
  
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

  TH1D *hElinkDiff_2 = new TH1D("hElinkDiff_2", Form("Elink diff for Relay: %u, for tctp case : 2",relay), 1000, -10., 10.);
  hElinkDiff_2->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hElinkDiff_2->GetYaxis()->SetTitle("Entries");
  hElinkDiff_2->SetDirectory(dir_diff);

  TH1D *hElinkDiff_3 = new TH1D("hElinkDiff_3", Form("Elink diff for Relay: %u, for tctp case : 3",relay), 1000, -10., 10.);
  hElinkDiff_3->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hElinkDiff_3->GetYaxis()->SetTitle("Entries");
  hElinkDiff_3->SetDirectory(dir_diff);
  
  TH1D *hUWord = new TH1D("hUWord", Form("Unpacker word diff for Relay: %u",relay), 1000, -10., 10.);
  hUWord->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hUWord->GetYaxis()->SetTitle("Entries");
  hUWord->SetDirectory(dir_diff);

  TH1D *hUWord_0 = new TH1D("hUWord_0", Form("Unpacker word diff for Relay: %u, for tctp case : 0",relay), 1000, -10., 10.);
  hUWord_0->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hUWord_0->GetYaxis()->SetTitle("Entries");
  hUWord_0->SetDirectory(dir_diff);

  TH1D *hUWord_1 = new TH1D("hUWord_1", Form("Unpacker word diff for Relay: %u, for tctp case : 1",relay), 1000, -10., 10.);
  hUWord_1->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hUWord_1->GetYaxis()->SetTitle("Entries");
  hUWord_1->SetDirectory(dir_diff);

  TH1D *hUWord_2 = new TH1D("hUWord_2", Form("Unpacker word diff for Relay: %u, for tctp case : 2",relay), 1000, -10., 10.);
  hUWord_2->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hUWord_2->GetYaxis()->SetTitle("Entries");
  hUWord_2->SetDirectory(dir_diff);

  TH1D *hUWord_3 = new TH1D("hUWord_3", Form("Unpacker word diff for Relay: %u, for tctp case : 1",relay), 1000, -10., 10.);
  hUWord_3->GetXaxis()->SetTitle("Difference in (ECONT - Emulator)");
  hUWord_3->GetYaxis()->SetTitle("Entries");
  hUWord_3->SetDirectory(dir_diff);

  TH1D *hElDiff[4][2][3]; //2 modes(tctp==any or 1), 2 lpGBTs, 3 modules in each lpGBT
  TH1D *hUnpkWordDiff[4][2][3][11]; //2 modes(tctp==any or 1), 2 lpGBTs, 3 modules in each lpGBT 8 words max
  TH1D *hTCEDiff[4][2][3][10]; //2 modes(tctp==any or 1), 2 lpGBTs, 3 modules in each lpGBT, 8 TC energies 
  TH1D *hTCADiff[4][2][3][10]; //2 modes(tctp==any or 1), 2 lpGBTs, 3 modules in each lpGBT, 8 TC addresses
  
  for(int imode=0;imode<=3;imode++)
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
  
  
  for(int imode=0;imode<=3;imode++)
    for(int ilp=0;ilp<2;ilp++)
      for(int imdl=0;imdl<3;imdl++)
	for(int itc=0;itc<10;itc++){
	  
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
      

  for(int imode=0;imode<=3;imode++)
    for(int ilp=0;ilp<2;ilp++)
      for(int imdl=0;imdl<3;imdl++)
	for(int iw=0;iw<11;iw++){
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
  
  TH2D *hNZEmulModules = new TH2D("hNZEmulModules", Form("Emul:Non-zero modules for Relay: %u",relay), 2, -0.5, 1.5, 3, -0.5, 2.5);
  hNZEmulModules->GetXaxis()->SetTitle("lpGBT");
  hNZEmulModules->GetYaxis()->SetTitle("Module");
  hNZEmulModules->SetDirectory(dir_diff);

  TH2D *hNZDataModules = new TH2D("hNZDataModules", Form("Data:Non-zero modules for Relay: %u",relay), 2, -0.5, 1.5, 3, -0.5, 2.5);
  hNZDataModules->GetXaxis()->SetTitle("lpGBT");
  hNZDataModules->GetYaxis()->SetTitle("Module");
  hNZDataModules->SetDirectory(dir_diff);

  TH2D *hNZModules = new TH2D("hNZModules", Form("Non-zero modules for Relay: %u",relay), 2, -0.5, 1.5, 3, -0.5, 2.5);
  hNZModules->GetXaxis()->SetTitle("lpGBT");
  hNZModules->GetYaxis()->SetTitle("Module");
  hNZModules->SetDirectory(dir_diff);
  
  TH2D *hBCEnergyData[3], *hBCEnergyEmul[3] ;
  TH2D *hSTCEnergyData[3], *hSTCEnergyEmul[3] ;
  for(int imdl=0;imdl<3;imdl++){
    hBCEnergyData[imdl] = new TH2D(Form("hBCEnergyData_%d",imdl), Form("Data:Expected vs Obtained Energy for relay %u",relay), 70, -0.5, 69.5, 70, -0.5, 69.5);
    hBCEnergyData[imdl]->GetXaxis()->SetTitle("expected Energy");
    hBCEnergyData[imdl]->GetYaxis()->SetTitle("observed Energy");
    hBCEnergyData[imdl]->SetDirectory(dir_diff);
    
    hBCEnergyEmul[imdl] = new TH2D(Form("hBCEnergyEmul_%d",imdl), Form("Emul:Expected vs Obtained Energy for relay %u",relay), 70, -0.5, 69.5, 70, -0.5, 69.5);
    hBCEnergyEmul[imdl]->GetXaxis()->SetTitle("expected Energy");
    hBCEnergyEmul[imdl]->GetYaxis()->SetTitle("observed Energy");
    hBCEnergyEmul[imdl]->SetDirectory(dir_diff);
    
    hSTCEnergyData[imdl] = new TH2D(Form("hSTCEnergyData_%d",imdl), Form("Data:Expected vs Obtained Energy for relay %u",relay), 70, -0.5, 69.5, 70, -0.5, 69.5);
    hSTCEnergyData[imdl]->GetXaxis()->SetTitle("expected Energy");
    hSTCEnergyData[imdl]->GetYaxis()->SetTitle("observed Energy");
    hSTCEnergyData[imdl]->SetDirectory(dir_diff);

    hSTCEnergyEmul[imdl] = new TH2D(Form("hSTCEnergyEmul_%d",imdl), Form("Emul:Expected vs Obtained Energy for relay %u",relay), 70, -0.5, 69.5, 70, -0.5, 69.5);
    hSTCEnergyEmul[imdl]->GetXaxis()->SetTitle("expected Energy");
    hSTCEnergyEmul[imdl]->GetYaxis()->SetTitle("observed Energy");
    hSTCEnergyEmul[imdl]->SetDirectory(dir_diff);
  }

  TH2D *hBCPosData[3], *hBCPosEmul[3] ;
  TH2D *hSTCPosData[3], *hSTCPosEmul[3] ;
  for(int imdl=0;imdl<3;imdl++){
    hBCPosData[imdl] = new TH2D(Form("hBCPosData_%d",imdl), Form("Data:Expected vs Obtained Address for relay %u",relay), 48, -0.5, 47.5, 48, -0.5, 47.5);
    hBCPosData[imdl]->GetXaxis()->SetTitle("expected TC");
    hBCPosData[imdl]->GetYaxis()->SetTitle("observed TC");
    hBCPosData[imdl]->SetDirectory(dir_diff);

    hBCPosEmul[imdl] = new TH2D(Form("hBCPosEmul_%d",imdl), Form("Emul:Expected vs Obtained Address for relay %u",relay), 48, -0.5, 47.5, 48, -0.5, 47.5);
    hBCPosEmul[imdl]->GetXaxis()->SetTitle("expected TC");
    hBCPosEmul[imdl]->GetYaxis()->SetTitle("observed TC");
    hBCPosEmul[imdl]->SetDirectory(dir_diff);

    hSTCPosData[imdl] = new TH2D(Form("hSTCPosData_%d",imdl), Form("Data:Expected vs Obtained Address for relay %u",relay), 48, -0.5, 47.5, 48, -0.5, 47.5);
    hSTCPosData[imdl]->GetXaxis()->SetTitle("expected TC");
    hSTCPosData[imdl]->GetYaxis()->SetTitle("observed TC");
    hSTCPosData[imdl]->SetDirectory(dir_diff);

    hSTCPosEmul[imdl] = new TH2D(Form("hSTCPosEmul_%d",imdl), Form("Emul:Expected vs Obtained Address for relay %u",relay), 48, -0.5, 47.5, 48, -0.5, 47.5);
    hSTCPosEmul[imdl]->GetXaxis()->SetTitle("expected TC");
    hSTCPosEmul[imdl]->GetYaxis()->SetTitle("observed TC");
    hSTCPosEmul[imdl]->SetDirectory(dir_diff);
  }

  TH2D *hBCUnExTCData = new TH2D("hBCUnExTCData", Form("Data:UnExpected TC in data for relay %u",relay), 48, -0.5, 47.5, 3, -0.5, 2.5);
  hBCUnExTCData->GetXaxis()->SetTitle("unexpected TC");
  hBCUnExTCData->GetYaxis()->SetTitle("module");
  hBCUnExTCData->SetDirectory(dir_diff);

  TH2D *hBCUnExTCEmul = new TH2D("hBCUnExTCEmul", Form("Emul:UnExpected TC in emul for relay %u",relay), 48, -0.5, 47.5, 3, -0.5, 2.5);
  hBCUnExTCEmul->GetXaxis()->SetTitle("unexpected TC");
  hBCUnExTCEmul->GetYaxis()->SetTitle("module");
  hBCUnExTCEmul->SetDirectory(dir_diff);

  TH2D *hSTCUnExTCData = new TH2D("hSTCUnExTCData", Form("Data:UnExpected TC in data for relay %u",relay), 48, -0.5, 47.5, 3, -0.5, 2.5);
  hSTCUnExTCData->GetXaxis()->SetTitle("unexpected TC");
  hSTCUnExTCData->GetYaxis()->SetTitle("module");
  hSTCUnExTCData->SetDirectory(dir_diff);

  TH2D *hSTCUnExTCEmul = new TH2D("hSTCUnExTCEmul", Form("Emul:UnExpected TC in emul for relay %u",relay), 48, -0.5, 47.5, 3, -0.5, 2.5);
  hSTCUnExTCEmul->GetXaxis()->SetTitle("unexpected TC");
  hSTCUnExTCEmul->GetYaxis()->SetTitle("module");
  hSTCUnExTCEmul->SetDirectory(dir_diff);

  const char *cutnames[5] = {"", "TcTp==1", "TcTp==2", "TcTp==3", ""};
  TH1D *hTOTCounts = new TH1D("hTOTCounts", Form("TOT counts in trig data for relay : %u",relay), 5, -0.5, 4.5);
  hTOTCounts->GetYaxis()->SetTitle("Entries");
  for (int ibin=1;ibin<hTOTCounts->GetNbinsX();ibin++){
   hTOTCounts->GetXaxis()->SetBinLabel(ibin, cutnames[ibin-1]);
  }
  hTOTCounts->SetDirectory(dir_diff);

}

void FillHistogram(bool matchFound, bool isLargeDiff, TDirectory*& dir_diff, uint32_t relayNumber, uint64_t event, uint16_t inputbxId,
		   uint32_t nelinks,  const uint32_t *eldata, const uint32_t *elemul,
		   uint32_t ilp, uint32_t imdl,
		   TPGFEDataformat::TcRawDataPacket& tcdata, TPGFEDataformat::TcRawDataPacket& tcemul,
		   const uint32_t *unpkWords, const uint32_t *unpkWords1,
		   TPGBEDataformat::UnpackerOutputStreamPair& updata, TPGBEDataformat::UnpackerOutputStreamPair& upemul,
		   std::vector<uint64_t>& unExEmulTC, std::vector<uint64_t>& unExDataTC, std::vector<uint64_t>& elStg1Event){
  //std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>>>& hrocarray){
  
  // uint32_t IdealE[2][3][10] = { //nof lpGBTs, nofModules, nofTC/STCs
  //   {
  //     {48, 48, 48, 48, 48, 49, 00, 00, 00, 00},
  //     {43, 43, 43, 44, 00, 00, 00, 00, 00, 00},
  //     {35, 35, 35, 36, 00, 00, 00, 00, 00, 00}
  //   },
  //   {
  //     {60, 60, 61, 61, 62, 62, 63, 63, 64, 64},
  //     {52, 53, 54, 55, 56, 56, 00, 00, 00, 00},
  //     {02, 27, 35, 39, 41, 43, 00, 00, 00, 00}
  //   }
  // };
  
  // uint32_t IdealA[2][3][10] = { //nof lpGBTs, nofModules, nofTC/STCs
  //   {
  //     {32, 33, 34, 36, 37, 47, 99, 99, 99, 99},
  //     {40, 41, 42, 47, 99, 99, 99, 99, 99, 99},
  //     {44, 45, 46, 47, 99, 99, 99, 99, 99, 99}
  //   },
  //   {
  //     {0, 3, 0, 3, 0, 3, 0, 3, 0, 0},
  //     {3, 3, 3, 3, 0, 3, 9, 9, 9, 9},
  //     {0, 3, 3, 3, 3, 3, 9, 9, 9, 9}
  //   }
  // };
  
  // uint32_t IdealL[2][3][3] = { //nof lpGBTs, nofModules, maxElinks
  //   {
  //     {0x05b8218a, 0x496f60c1, 0x83060c40},
  //     {0x055a29aa, 0xf56ad5ac, 0xffffffff},
  //     {0x049b2dba, 0xf468d1a4, 0xffffffff}
  //   },
  //   {
  //     {0x03333078, 0xf1ebd7cf, 0x9fbf8100},
  //     {0x0ff368d5, 0xb3770e00, 0xffffffff},
  //     {0x03ff046d, 0x1a752ac0, 0xffffffff}
  //   }
  // };

  int imode = 0;
  if(tcemul.isTcTp1()) imode = 1;
  if(tcemul.isTcTp2()) imode = 2;
  if(tcemul.isTcTp3()) imode = 3;

  TList *list = (TList *)dir_diff->GetList();

  // std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>> hrocvec =  hrocarray.at(event);	  
  // for(const auto& data : hrocvec){
  //   const TPGFEDataformat::HalfHgcrocData& hrocdata = data.second ;
  //   if(hrocdata.hasTcTp(1) or hrocdata.hasTcTp(12 or hrocdata.hasTcTp(3)) ((TH1D *) list->FindObject("hTOT"))->Fill( hrocdata.getTot() );
  // }

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
    uint32_t emullink, datalink;
    bool hasDiffEmul = false;
    bool hasDiffData = false;
    for(uint32_t iel=0;iel<nelinks;iel++){
      ((TH1D *) list->FindObject("hElinkDiff"))->Fill( (eldata[iel] - elemul[iel]) );
      if(imode==0) ((TH1D *) list->FindObject("hElinkDiff_0"))->Fill( (eldata[iel] - elemul[iel]) );
      if(imode==1) ((TH1D *) list->FindObject("hElinkDiff_1"))->Fill( (eldata[iel] - elemul[iel]) );
      if(imode==2) ((TH1D *) list->FindObject("hElinkDiff_2"))->Fill( (eldata[iel] - elemul[iel]) );
      if(imode==3) ((TH1D *) list->FindObject("hElinkDiff_3"))->Fill( (eldata[iel] - elemul[iel]) );
	
      ((TH1D *) list->FindObject(Form("hElDiff_%d_%d_%d",imode,ilp,imdl)))->Fill( (eldata[iel] - elemul[iel]) );
      if( ((eldata[iel] - elemul[iel]) != 0) and imode==0) hasDiff = true;
      emullink = (iel==0) ? (elemul[iel] & 0x0fffffff) : elemul[iel] ;
      datalink = (iel==0) ? (eldata[iel] & 0x0fffffff) : eldata[iel] ;
      // if( ((IdealL[ilp][imdl][iel] - emullink) != 0)) hasDiffEmul = true;
      // if( ((IdealL[ilp][imdl][iel] - datalink) != 0)) hasDiffData = true;
    }
    if(hasDiff) ((TH2D *) list->FindObject("hNZModules"))->Fill( ilp, imdl );
    // if(hasDiffData) {
    //   ((TH2D *) list->FindObject("hNZDataModules"))->Fill( ilp, imdl );
    //   if(std::find(unExDataTC.begin(),unExDataTC.end(),event) == unExDataTC.end() and relayNumber==1726581356) unExDataTC.push_back(event);
    // }
    // if(hasDiffEmul) {
    //   ((TH2D *) list->FindObject("hNZEmulModules"))->Fill( ilp, imdl );
    //   if(std::find(unExEmulTC.begin(),unExEmulTC.end(),event) == unExEmulTC.end() and relayNumber==1726581356) unExEmulTC.push_back(event);
    // }
    
    if(imode==0 and imdl==0 and ilp==0){
      if(hasDiff)
	((TH1D *) list->FindObject("hSlinkBx8NZ"))->Fill( bxIdmod8 );
      else
	((TH1D *) list->FindObject("hSlinkBx8Z"))->Fill( bxIdmod8 );
      ((TH1D *) list->FindObject("hSlinkBx8Matched"))->Fill( bxIdmod8 );
    }
  
    std::vector<int> emulist,econtlist; //ideallist, 
    for(uint32_t itc=0;itc<tcdata.size();itc++){
      //int imodeloc = (tcemul.getTcData().at(itc).isTcTp1()) ? 1 : 0 ;
      int imodeloc = 0;
      if(tcemul.getTcData().at(itc).isTcTp1()) imodeloc = 1;
      if(tcemul.getTcData().at(itc).isTcTp2()) imodeloc = 2;
      if(tcemul.getTcData().at(itc).isTcTp3()) imodeloc = 3;

      ((TH1D *) list->FindObject(Form("hTCEDiff_%d_%d_%d_%d",imodeloc,ilp,imdl,itc)))->Fill( (tcdata.getTcData().at(itc).energy() - tcemul.getTcData().at(itc).energy()) );
      ((TH1D *) list->FindObject(Form("hTCADiff_%d_%d_%d_%d",imodeloc,ilp,imdl,itc)))->Fill( (tcdata.getTcData().at(itc).address() - tcemul.getTcData().at(itc).address()) );

      if(tcemul.getTcData().at(itc).isTcTp1()) ((TH1D *) list->FindObject("hTOTCounts"))->Fill(1);
      if(tcemul.getTcData().at(itc).isTcTp2()) ((TH1D *) list->FindObject("hTOTCounts"))->Fill(2);
      if(tcemul.getTcData().at(itc).isTcTp3()) ((TH1D *) list->FindObject("hTOTCounts"))->Fill(3);
      
      //uint32_t ideal_stc = 4*itc + IdealA[ilp][imdl][itc];
      uint32_t data_stc = 4*itc + tcdata.getTcData().at(itc).address();
      uint32_t emul_stc = 4*itc + tcemul.getTcData().at(itc).address();
      // if(isLargeDiff){
      // 	// if(ilp==0){
      // 	//   ((TH2D *) list->FindObject(Form("hBCEnergyData_%d",imdl)))->Fill(IdealE[ilp][imdl][itc],tcdata.getTcData().at(itc).energy());
      // 	//   ((TH2D *) list->FindObject(Form("hBCEnergyEmul_%d",imdl)))->Fill(IdealE[ilp][imdl][itc],tcemul.getTcData().at(itc).energy());
      // 	//   ((TH2D *) list->FindObject(Form("hBCPosData_%d",imdl)))->Fill(IdealA[ilp][imdl][itc],tcdata.getTcData().at(itc).address());
      // 	//   ((TH2D *) list->FindObject(Form("hBCPosEmul_%d",imdl)))->Fill(IdealA[ilp][imdl][itc],tcemul.getTcData().at(itc).address());
      // 	// }else{
      // 	//   ((TH2D *) list->FindObject(Form("hSTCEnergyData_%d",imdl)))->Fill(IdealE[ilp][imdl][itc],tcdata.getTcData().at(itc).energy());
      // 	//   ((TH2D *) list->FindObject(Form("hSTCEnergyEmul_%d",imdl)))->Fill(IdealE[ilp][imdl][itc],tcemul.getTcData().at(itc).energy());
      // 	//   ((TH2D *) list->FindObject(Form("hSTCPosData_%d",imdl)))->Fill(ideal_stc, data_stc);
      // 	//   ((TH2D *) list->FindObject(Form("hSTCPosEmul_%d",imdl)))->Fill(ideal_stc, emul_stc);
      // 	// }
      // }//only for problematic module with elink mismatch
      
      if(imdl==0){
	((TH1D *) list->FindObject("hOccuTCData"))->Fill( tcdata.getTcData().at(itc).address() );
	((TH1D *) list->FindObject("hOccuTCEmul"))->Fill( tcemul.getTcData().at(itc).address() );
      }
      if(ilp==0){
	//ideallist.push_back(IdealA[ilp][imdl][itc]);
	emulist.push_back(tcemul.getTcData().at(itc).address());
	econtlist.push_back(tcdata.getTcData().at(itc).address());
      }else{
	//ideallist.push_back( ideal_stc );
	emulist.push_back( emul_stc );
	econtlist.push_back( data_stc );
      }
      
    }
    // if(isLargeDiff){
    //   int *missingEmulTC = new int[tcdata.size()];
    //   int nofemulmismatches = 0;
    //   for(const auto& itcemul : emulist)
    // 	if(std::find(ideallist.begin(), ideallist.end(), itcemul) == ideallist.end())
    // 	  missingEmulTC[nofemulmismatches++] = itcemul;
      
    //   int *missingDataTC = new int[tcdata.size()];;
    //   int nofdatamismatches = 0;
    //   for(const auto& itcdata : econtlist)
    // 	if(std::find(ideallist.begin(), ideallist.end(), itcdata) == ideallist.end())
    // 	  missingDataTC[nofdatamismatches++] = itcdata;
      
    //   if(nofemulmismatches==1 and nofdatamismatches==1 and imdl==0) {
    // 	((TH2D *) list->FindObject("hMissedTCs"))->Fill( missingEmulTC[0], missingDataTC[0] );
    // 	//std::cout<<"Event with single mismatch : " << event << std::endl;
    //   }

    //   for(int idatamm = 0 ; idatamm < nofdatamismatches ; idatamm++){
    // 	if(ilp==0){
    // 	  ((TH2D *) list->FindObject("hBCUnExTCData"))->Fill( missingDataTC[idatamm], imdl );
    // 	}else{
    // 	  ((TH2D *) list->FindObject("hSTCUnExTCData"))->Fill( missingDataTC[idatamm], imdl );
    // 	}
    //   }//if TC is not expected in data
    //   for(int iemulmm = 0 ; iemulmm < nofemulmismatches ; iemulmm++){
    // 	if(ilp==0){
    // 	  ((TH2D *) list->FindObject("hBCUnExTCEmul"))->Fill( missingEmulTC[iemulmm], imdl );
    // 	}else{
    // 	  ((TH2D *) list->FindObject("hSTCUnExTCEmul"))->Fill( missingEmulTC[iemulmm], imdl );
    // 	}
    //   }//if one missing TC is absent
    //   delete []missingEmulTC;
    //   delete []missingDataTC;
    // }
    
    uint16_t *unpkMsTc = new uint16_t[tcdata.size()+1];
    uint32_t strm = 0;
    if(tcdata.size()<=7){
      for(uint32_t iw=0;iw<=tcdata.size();iw++){
	unpkMsTc[iw] = (iw==0)? updata.getMsData(strm) : updata.getTcData(strm, iw-1);
	int diff = (iw==0 or tcdata.type()!=TPGFEDataformat::BestC)? ((unpkWords[iw]&0xf) - (unpkMsTc[iw]&0xf)) : (unpkWords[iw] - unpkMsTc[iw]) ;
	uint32_t XOR = (iw==0 or tcdata.type()!=TPGFEDataformat::BestC)? ((unpkWords[iw]&0xf) xor (unpkMsTc[iw]&0xf)) : (unpkWords[iw] xor unpkMsTc[iw]) ;
      
	int imodeloc = 0;
	if((iw==0 and tcemul.isTcTp1()) or (iw>0 and tcemul.getTcData().at(iw-1).isTcTp1())) imodeloc = 1;
	if((iw==0 and tcemul.isTcTp2()) or (iw>0 and tcemul.getTcData().at(iw-1).isTcTp2())) imodeloc = 2;
	if((iw==0 and tcemul.isTcTp3()) or (iw>0 and tcemul.getTcData().at(iw-1).isTcTp3())) imodeloc = 3;

	((TH1D *) list->FindObject("hUWord"))->Fill( diff );
	((TH1D *) list->FindObject(Form("hUWord_%d",imodeloc)))->Fill( diff );
	if(iw>0) ((TH1D *) list->FindObject( Form("hUnpkWordDiff_%d_%d_%d_%d",imodeloc,ilp,imdl,iw-1) ))->Fill( diff );
	if(iw>0 and diff!=0 and std::find(elStg1Event.begin(),elStg1Event.end(),event) == elStg1Event.end()) {
	  elStg1Event.push_back(event);
	  std::cout << "Error:: iw: " << iw << ", unpkMsTc[iw]: 0x"
		    << std::hex << std::setfill('0') << std::setw(4) << unpkMsTc[iw] << ", word: 0x" << unpkWords[iw] 
		    << std::dec << std::setfill(' ')
		    << ", diff: " << diff
		    <<", TC energy : " << tcdata.getTcData().at(iw-1).energy() <<", TC address : " << tcdata.getTcData().at(iw-1).address()
		    << std::endl;
	}
      }
    }else if (tcdata.size()>7 and tcdata.size()<=14) {
      int diff = 99;
      uint32_t word = 99;
      for(uint32_t iw=0;iw<=tcdata.size();iw++){
	if(iw==0){
	  unpkMsTc[iw] =  updata.getMsData(0);
	  diff = unpkWords[iw] - unpkMsTc[iw];
	}else {
	  unpkMsTc[iw] =  (iw%2==1)? updata.getTcData(0, (iw-1)/2) : updata.getTcData(1, (iw-2)/2) ;
	  word = (iw%2==1)? unpkWords[iw/2+1] : unpkWords1[iw/2] ;
	  diff = word - unpkMsTc[iw];
	  //std::cout << "iw: " << iw << ", unpkMsTc[iw]: " << unpkMsTc[iw] << ", word: " << word << ", diff: " << diff << std::endl;
	}
	
	int imodeloc = 0;
	if((iw==0 and tcemul.isTcTp1()) or (iw>0 and tcemul.getTcData().at(iw-1).isTcTp1())) imodeloc = 1;
	if((iw==0 and tcemul.isTcTp2()) or (iw>0 and tcemul.getTcData().at(iw-1).isTcTp2())) imodeloc = 2;
	if((iw==0 and tcemul.isTcTp3()) or (iw>0 and tcemul.getTcData().at(iw-1).isTcTp3())) imodeloc = 3;
	
	((TH1D *) list->FindObject("hUWord"))->Fill( diff );
	((TH1D *) list->FindObject(Form("hUWord_%d",imodeloc)))->Fill( diff );
	if(iw>0) ((TH1D *) list->FindObject( Form("hUnpkWordDiff_%d_%d_%d_%d",imodeloc,ilp,imdl,iw-1) ))->Fill( diff );
	if(iw>0 and diff!=0 and std::find(elStg1Event.begin(),elStg1Event.end(),event) == elStg1Event.end()) {
	  elStg1Event.push_back(event);
	  std::cout << "Error1:: iw: " << iw << ", unpkMsTc[iw]: 0x"
		    << std::hex << std::setfill('0') << std::setw(4) << unpkMsTc[iw] << ", word: 0x" << word
		    << std::dec << std::setfill(' ')
		    << ", diff: " << diff
		    <<", TC energy : " << tcdata.getTcData().at(iw-1).energy() <<", TC address : " << tcdata.getTcData().at(iw-1).address()
		    << std::endl;
	}
      }
      ;
    }
    delete []unpkMsTc;
    
  }//if matched
  
}

