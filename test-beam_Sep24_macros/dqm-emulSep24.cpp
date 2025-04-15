/**********************************************************************
 Created on : 16/09/2024
 Purpose    : Emulation for test-beam of September 2024
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
#include "TPGFEReader3TSep2024.hh"
#include "Stage1IO.hh"

int main(int argc, char** argv)
{
  //===============================================================================================================================
  // make
  // ./emul_Jul24.exe $Relay $rname $link_number $istrimming
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
  uint32_t linkNumber(1);
  const uint32_t noflpGBTs(2);
  bool isTrimming(1);
  long double maxEvent = 100;
  std::istringstream issRelay(argv[1]);
  issRelay >> relayNumber;
  //Notes:
  // 1. firmware test for third train 1727007636 (old ECON-T configuration so should not be analyzed)
  // 2. first pedestal with 3rd layer in configuration 1727033054 and installed 1727099251 (with correct ECON-T configuration)
  // 3. first technical with third layer : 1727099443 //https://cmsonline.cern.ch/webcenter/portal/cmsonline/Common/Elog?__adfpwp_action_portlet=683379043&__adfpwp_backurl=https%3A%2F%2Fcmsonline.cern.ch%3A443%2Fwebcenter%2Fportal%2Fcmsonline%2FCommon%2FElog%3F_afrRedirect%3D23225796808764440&_piref683379043.strutsAction=%2FviewMessageDetails.do%3FmsgId%3D1237840
  // 4. swap back roc2 config of mod16 for relays after 1727116899 (not necessary for this code, kept as bookkeeping)
  // 5. List of run flagged as green by Paul for TC Processor are Relay1727219172 to Run1727219175 and probably Relay1727211141
  // 6. Sampling phase issue has been solved for relays between 1727123194 to 1727153189
  // 7. 14 0xcafecafe separators from Relay 1727191081 onwards
  if(relayNumber<1727099251){
    std::cerr << "Third layer is not properly configured" << std::endl;
    return false;
  }

  std::istringstream issRun(argv[2]);
  issRun >> runNumber;
  std::istringstream issEvents(argv[3]);
  issEvents >> maxEvent;
  // std::istringstream issLink(argv[3]);
  // issLink >> linkNumber;
  if(linkNumber!=1 and linkNumber!=2){
    std::cerr << "Link number "<< argv[3] <<"is out of bound (use: 1 or 2)" << std::endl;
    return false;
  }
  uint32_t trig_linkNumber = TMath::FloorNint((linkNumber-1)/2);
  // std::istringstream issTrimming(argv[4]);
  // issTrimming >> isTrimming;
  
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
  
  TFile *fout = new TFile(Form("Diff_3T_Relay-%u.root",relayNumber), "recreate");
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
      {"320MLF3WXIH0016_roc0_e2.yaml", "320MLF3WXIH0016_roc1_e2.yaml", "320MLF3WXIH0016_roc2_e2.yaml"},
      {"320MLF3WXIH0017_roc0_e1.yaml", "320MLF3WXIH0017_roc1_e1.yaml", "320MLF3WXIH0017_roc2_e1.yaml"},
      {"320MLF3WXIH0014_roc0_e0.yaml", "320MLF3WXIH0014_roc1_e0.yaml", "320MLF3WXIH0014_roc2_e0.yaml"}
    }
  };
  
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

      cfgs.setEconTFile("cfgmap/init_econt_mux_test1.yaml");
      cfgs.readEconTConfigYaml();

      cfgs.setEconDFile("cfgmap/init_econd.yaml");
      cfgs.readEconDConfigYaml();      
      
      for(uint32_t iroc=0;iroc<3;iroc++){
	std::cout<<"idx: "<<idx<<", ilink: " << ilink << ", iecond: "<<iecond<<", iroc: "<<iroc << ", fname : " << cfgrocname[ilink][iecond][iroc] << std::endl;
	uint32_t rocid_0 = pck.packRocId(zside, sector, ilink, det, iecond, selTC4, module, iroc, 0);
	uint32_t rocid_1 = pck.packRocId(zside, sector, ilink, det, iecond, selTC4, module, iroc, 1);
	cfgs.setRocFile(Form("cfgmap/configs_v3b_full/%s",cfgrocname[ilink][iecond][iroc].c_str()));
	cfgs.readRocConfigYaml(rocid_0, rocid_1);	
      }//roc loop
    }//econd loop
  }//lpGBT loop
  if(!isTrimming){
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
  link = 0; econt = 0; uint32_t lp0_bc0 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 0; econt = 1; uint32_t lp0_bc1 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 0; econt = 2; uint32_t lp0_bc2 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 1; econt = 0; uint32_t lp1_stc160 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 1; econt = 1; uint32_t lp1_stc161 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 1; econt = 2; uint32_t lp1_stc162 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 2; econt = 0; uint32_t lp2_stc4a0 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 2; econt = 1; uint32_t lp2_stc4a1 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 3; econt = 0; uint32_t lp3_stc4a0 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 3; econt = 1; uint32_t lp3_stc4a1 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  link = 4; econt = 0; uint32_t lp4_ext_trigger = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  
  uint32_t density(0);
  uint32_t droplsb(1);
  std::map<uint32_t,TPGFEConfiguration::ConfigEconT>& econTPar =  cfgs.getEconTPara();
  for(const auto& it : econTPar){
    econTPar[it.first].setDensity(density);
    econTPar[it.first].setDropLSB(droplsb);
    
    if(it.first==lp0_bc0){              //// starting 1st train
      econTPar[it.first].setSelect(2);
      econTPar[it.first].setNElinks(3);      
    }else if (it.first==lp0_bc1){
      econTPar[it.first].setSelect(2);
      econTPar[it.first].setNElinks(2);     
    }else if (it.first==lp0_bc2){
      econTPar[it.first].setSelect(2);
      econTPar[it.first].setNElinks(2);
    }else if (it.first==lp1_stc160){    //// starting 2nd train
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(1);
    }else if (it.first==lp1_stc161){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(1);
    }else if (it.first==lp1_stc162){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(1);
    }else if (it.first==lp2_stc4a0){    //// starting 3rd train
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(4);
      econTPar[it.first].setSTCType(3);
    }else if (it.first==lp2_stc4a1){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(2);
      econTPar[it.first].setSTCType(3);
    }else if (it.first==lp3_stc4a0){    //// starting motherboard
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(4);
      econTPar[it.first].setSTCType(3);
    }else if (it.first==lp3_stc4a1){
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setNElinks(3);
      econTPar[it.first].setSTCType(3);
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
  // for(auto const& hroc : hroccfg){
  //   if(hroc.first == rocid_0) {
  //     hroccfg[hroc.first].setTotP(2, 13);
  //     hroccfg[hroc.first].setTotP(3, 0);
  //     hroccfg[hroc.first].setTotTH(2, 107);
  //   }
  // }
  // ===============================================================================================================================
  
  // ===============================================================================================================================
  // Set and Initialize the ECONT reader
  // ===============================================================================================================================
  TPGFEReader::ECONTReader econTReader(cfgs);
  //output array
  //econTReader.checkEvent(1);
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGBEDataformat::Trig24Data>>> econtarray; //event,moduleId (from link)
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGBEDataformat::TrigTCProcData>>> tcprocarray; //event,moduleId (from link)
  // ===============================================================================================================================

  int FindBxShift(TDirectory*& dir_diff, std::vector<uint64_t>& eventList, uint32_t maxTestEvent,
		  TPGFEConfiguration::TPGFEIdPacking& pck,
		  std::map<uint32_t,TPGFEConfiguration::ConfigEconT>& econTPar,
		  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>>>& hrocarray,
		  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGBEDataformat::Trig24Data>>> & econtarray,
		  TPGFEModuleEmulation::HGCROCTPGEmulation& rocTPGEmul,
		  TPGFEModuleEmulation::ECONTEmulation& econtEmul
		  );
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
  //uint64_t refEvt[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  //uint64_t refEvt[7] = {473, 1380, 1623, 2149, 5840, 5987, 7677};
  /*Nonzero diff events*/ //uint64_t refEvt[854] = {165, 167, 486, 884, 1241, 1612, 1628, 2681, 2689, 2734, 3532, 3884, 4356, 4829, 5137, 6197, 6293, 6398, 6870, 7261, 7937, 8181, 8575, 8650, 9060, 9797, 10213, 10245, 10580, 10663, 10972, 11215, 12659, 13254, 14181, 14624, 14996, 15439, 15629, 17571, 18439, 19051, 20569, 20761, 21218, 21685, 22132, 23156, 23336, 24075, 24113, 24344, 24648, 25499, 25566, 25928, 27294, 27687, 28011, 28535, 28592, 28797, 29151, 29562, 30391, 31936, 32176, 32973, 33123, 34601, 36668, 36762, 36823, 37957, 38446, 38782, 39670, 40118, 40767, 42750, 42817, 43090, 44124, 47109, 47651, 47798, 48204, 48603, 48877, 51132, 52828, 53909, 54251, 54947, 55518, 55959, 56821, 58291, 59149, 60599, 60848, 60864, 60883, 61326, 61777, 62098, 62502, 63108, 63216, 63718, 64564, 65012, 66055, 66078, 66188, 67353, 67503, 67621, 70299, 70648, 70954, 71167, 71332, 74175, 74459, 75421, 75459, 77467, 77522, 77757, 78147, 78851, 79332, 79495, 79890, 80535, 80542, 81178, 81302, 81449, 81795, 82012, 83949, 84219, 84898, 85561, 85706, 86370, 87012, 87015, 87077, 87402, 87568, 87831, 88026, 88406, 89367, 89399, 89808, 89920, 92595, 93124, 94231, 94705, 95074, 96256, 97078, 97366, 97599, 98039, 99663, 99696, 99718, 100246, 100250, 100545, 101571, 101813, 102084, 102917, 103271, 103514, 104166, 104352, 104366, 105126, 105183, 105478, 105903, 105998, 107342, 107758, 109096, 109124, 110264, 110489, 110969, 111252, 112262, 112749, 112847, 113214, 113216, 113920, 114747, 114807, 116349, 117400, 117558, 117878, 118634, 119906, 120872, 120899, 121308, 121775, 123017, 123239, 123462, 123650, 123656, 123877, 126252, 126568, 126680, 127341, 127724, 128295, 128430, 129276, 129710, 130195, 130723, 131783, 132300, 132807, 132904, 132906, 132914, 132951, 133007, 133927, 134096, 135264, 135274, 136167, 137685, 137970, 138295, 138913, 139349, 139456, 139669, 139902, 140258, 140803, 141215, 141508, 141632, 142027, 142068, 142119, 142215, 142754, 142834, 143056, 144202, 144207, 145951, 146150, 146177, 146847, 147825, 147840, 148326, 149467, 151185, 152006, 152843, 152845, 152893, 153555, 154124, 154968, 154970, 155701, 155729, 155780, 155803, 156875, 157106, 157546, 157975, 158930, 158972, 159096, 159798, 161327, 161404, 161613, 162044, 162205, 163450, 164170, 164682, 166194, 166609, 167027, 167038, 169760, 170146, 170911, 171009, 172965, 173470, 173479, 174169, 174800, 176448, 177673, 177848, 178556, 178581, 178938, 179665, 182405, 182409, 185531, 187019, 187231, 187669, 187833, 189011, 189439, 189721, 190641, 191396, 191976, 192416, 192721, 193052, 193109, 194305, 194398, 195129, 195140, 195463, 196998, 197476, 197542, 197814, 198869, 199321, 200068, 200132, 201033, 201291, 202130, 202686, 202972, 203177, 203276, 203969, 204161, 206199, 206387, 206589, 207013, 207687, 208374, 209363, 209464, 209616, 210374, 210510, 210580, 211308, 211379, 213122, 213244, 213983, 214120, 214587, 214684, 215357, 215647, 215977, 215984, 216607, 218182, 218434, 218878, 219110, 219592, 219628, 220795, 220802, 221187, 222779, 223175, 225083, 225806, 225822, 225906, 226706, 228119, 228170, 230328, 230478, 231132, 231173, 232126, 232458, 232487, 232928, 233486, 234135, 234311, 236132, 238047, 238703, 239427, 239472, 239799, 239945, 239975, 240742, 241820, 243215, 244034, 244755, 245391, 245414, 245718, 245929, 246298, 246654, 246792, 246809, 247154, 247282, 247639, 247892, 248311, 248461, 248562, 250141, 252226, 253652, 253674, 253836, 254786, 255561, 255867, 257223, 257803, 258056, 258185, 258499, 259653, 261755, 262628, 263125, 264905, 264951, 265013, 266720, 266891, 270174, 271881, 272121, 273547, 273625, 274415, 277592, 277797, 279318, 280033, 281885, 282284, 282529, 284056, 284143, 284218, 284596, 285399, 286088, 286342, 286684, 287262, 287455, 289142, 289206, 289301, 290192, 290548, 292823, 293250, 293420, 294384, 294464, 294689, 295287, 296811, 297979, 299039, 299680, 301433, 301844, 302068, 302799, 303175, 303731, 304098, 304284, 304534, 305597, 306646, 307579, 307726, 308956, 312404, 312804, 313206, 313220, 313790, 313927, 314216, 315420, 316440, 316647, 316653, 317449, 317563, 318549, 318896, 319406, 319525, 319886, 320264, 320342, 321260, 324926, 325459, 326167, 326227, 326774, 327977, 328801, 329321, 329385, 330335, 331386, 332443, 333055, 333448, 333749, 333984, 334314, 335616, 336572, 336613, 339976, 340293, 340298, 340878, 341630, 344584, 344918, 344950, 344987, 345291, 346761, 347017, 347512, 347668, 348216, 348447, 348969, 349031, 349437, 350176, 350407, 350588, 351150, 351262, 352498, 352698, 353641, 354095, 355083, 355107, 355351, 355758, 355847, 356266, 356869, 357341, 357543, 357829, 357889, 358147, 358344, 358928, 359355, 359488, 360886, 360973, 361707, 361762, 361865, 362978, 363498, 364439, 364987, 365095, 366177, 367193, 367630, 369046, 369319, 370042, 370873, 371134, 371345, 374173, 374460, 375113, 375141, 376465, 376506, 377362, 377944, 378972, 379597, 380614, 381546, 381671, 381837, 382197, 382320, 383104, 384101, 384831, 385447, 385942, 386123, 386969, 388744, 388750, 389458, 391378, 392981, 393054, 393883, 394548, 395152, 395221, 395308, 395945, 396303, 396799, 397025, 398114, 399429, 400342, 400656, 400693, 400744, 401125, 402234, 403283, 403449, 404010, 404561, 405505, 405635, 406024, 406305, 406407, 406736, 407358, 407665, 407758, 407908, 408007, 408298, 409074, 409950, 410965, 411721, 412364, 413852, 414049, 414136, 414257, 414374, 416032, 416258, 416631, 416804, 416838, 417582, 417726, 418228, 418417, 418960, 419975, 420852, 423096, 423407, 423871, 426613, 427102, 427419, 428374, 428867, 430176, 432004, 432374, 434635, 435479, 435609, 435762, 436302, 436520, 436534, 438166, 438168, 439910, 441927, 442022, 443363, 444012, 445738, 445965, 446024, 446468, 446533, 446668, 447602, 448051, 448603, 449220, 449926, 451475, 451800, 451922, 451925, 452759, 453213, 453216, 453832, 453874, 453888, 455624, 455950, 457978, 459689, 459844, 461178, 461777, 462261, 462617, 462757, 462855, 463268, 463275, 463951, 465283, 465803, 466753, 466832, 467765, 467915, 467989, 468023, 468572, 468921, 469037, 469063, 469196, 469601, 469637, 470698, 470956, 471669, 471994, 472484, 473678, 473857, 475385, 475937, 476178, 476350, 477198, 477425, 477703, 478235, 478641, 479331, 479906, 481550, 482425, 483226, 483983, 484472, 484689, 486213, 486317, 487509, 487587, 487949, 488337, 489301, 489996, 490460, 491167, 492176, 492599, 492957, 493018, 493895, 493953, 494419, 495833, 495963, 495992, 496169, 496475, 496571, 496876, 496950, 497397, 497591, 497776, 499250, 499895, 501364, 501413, 501909, 502054, 502227, 502953, 503034, 503401, 503595, 503667, 505151, 505195, 505251, 505321, 505911, 507170};

  /*Nonzero diff events*/ uint64_t refEvt[567] = {364, 1705, 1751, 3621, 4922, 5690, 5737, 5981, 6477, 7134, 7229, 7407, 8079, 8305, 8451, 9013, 9280, 9439, 11128, 11201, 11847, 12233, 12327, 12822, 14325, 14833, 15263, 15489, 15563, 18121, 18626, 18678, 18962, 19514, 20682, 20925, 21664, 21925, 22067, 22070, 24084, 24269, 24698, 25617, 25894, 26117, 26805, 27061, 27174, 28299, 28714, 29134, 29389, 29740, 30069, 31733, 32239, 33539, 34007, 34416, 35300, 37399, 37565, 37787, 38790, 38948, 39936, 40015, 41218, 41312, 42583, 42796, 43107, 43416, 44227, 45005, 45027, 45859, 46169, 46659, 47004, 48419, 49109, 49508, 50118, 51593, 52031, 53256, 53406, 53578, 53920, 53941, 54714, 56534, 56784, 57398, 57554, 57643, 58655, 59478, 59713, 59756, 59942, 60564, 61280, 61595, 61888, 62519, 63158, 63181, 65281, 65858, 66909, 67627, 67732, 67959, 67960, 69836, 70045, 70695, 71348, 71497, 71667, 72279, 72818, 73558, 73685, 74193, 74292, 74611, 74717, 74965, 76228, 76826, 77033, 77037, 78873, 79866, 80902, 83424, 84345, 85694, 86415, 86450, 86907, 87063, 87773, 87827, 88017, 88402, 88908, 89248, 90525, 90749, 90787, 94255, 94728, 95345, 95401, 95724, 96393, 96742, 96752, 96817, 97128, 97372, 98513, 98826, 99377, 100000, 100169, 100819, 100835, 101661, 102155, 102843, 103649, 104336, 104478, 104957, 105971, 107044, 107709, 108737, 108755, 110480, 110712, 111883, 111915, 112988, 113187, 113952, 114236, 116739, 116838, 116901, 118078, 119649, 119889, 120401, 120482, 120753, 121302, 121758, 122664, 122916, 123443, 123602, 123619, 124335, 124352, 124622, 125407, 127389, 127635, 127808, 127877, 128499, 129930, 130673, 131769, 133760, 134278, 134574, 134730, 135976, 136017, 136185, 136880, 138222, 138369, 138834, 139959, 140131, 141159, 141168, 141447, 141594, 142702, 142715, 143012, 144454, 145145, 145452, 145476, 145834, 145871, 146018, 146538, 147417, 147521, 147716, 147814, 148315, 148568, 148762, 150032, 151220, 151246, 152135, 152672, 152769, 153867, 153958, 154026, 154187, 154234, 154441, 156160, 156874, 157298, 157382, 157614, 157838, 158804, 158890, 159039, 160843, 160968, 161212, 161233, 162234, 162336, 164395, 164953, 166333, 166483, 166527, 166928, 167573, 167991, 168683, 169326, 169393, 169801, 169841, 171054, 171468, 172240, 173685, 175540, 176255, 176887, 177462, 178558, 179646, 179766, 180213, 180603, 181823, 182230, 182280, 182858, 183760, 184493, 185448, 185604, 186655, 187742, 187805, 188264, 188495, 190158, 190297, 191347, 191357, 193108, 193343, 193388, 193457, 193794, 193888, 194662, 194841, 194905, 195237, 195580, 195592, 196372, 196535, 196608, 197303, 198118, 198133, 198394, 198416, 198857, 199164, 199342, 200094, 200972, 201037, 201784, 202933, 203108, 203963, 204958, 205248, 205494, 205771, 206871, 206945, 207306, 208474, 209094, 209268, 209970, 211126, 211253, 212361, 212541, 212707, 214566, 214776, 216336, 216985, 217184, 218029, 220226, 220374, 220778, 220876, 221102, 222843, 222968, 224411, 226343, 228397, 228675, 229387, 229739, 230165, 231117, 232503, 232710, 232925, 233927, 234570, 235156, 237403, 237626, 237799, 237812, 238128, 238509, 238741, 238849, 240464, 241347, 241397, 241429, 241781, 242149, 243092, 243924, 244226, 245492, 246980, 249508, 249727, 249830, 250917, 252002, 252498, 252806, 253349, 253608, 253716, 254322, 254500, 254992, 256265, 256388, 256627, 257089, 258924, 259709, 259732, 263166, 263888, 264067, 264591, 265007, 265387, 265504, 265568, 265879, 266832, 267425, 268684, 268963, 269095, 271383, 272569, 273332, 274773, 275045, 276774, 277667, 278481, 279665, 280354, 280844, 281969, 282199, 282373, 282869, 282920, 282949, 283680, 283915, 284189, 284197, 285133, 285403, 285900, 286089, 286495, 287038, 288524, 288581, 288924, 289562, 290502, 290575, 290722, 291838, 292318, 292688, 294006, 294644, 295188, 295355, 295665, 297131, 297454, 297815, 299180, 299189, 299863, 300321, 300699, 300793, 302164, 305758, 305945, 307682, 311078, 311550, 311766, 312153, 312836, 313502, 313726, 313876, 313967, 315997, 317330, 319012, 319809, 319959, 320124, 320602, 320921, 321467, 322558, 322579, 322993, 323373, 323462, 324269, 324273, 325009, 325282, 325505, 326974, 327923, 328978, 331016, 331612, 331647, 331926, 332192, 334152, 334648, 334700, 334927, 335063, 335611, 335675, 336431, 337491, 339726, 340867, 341044, 341059, 341606, 342213, 342508, 342722, 345228, 346220, 346521, 347655, 348035, 348861, 349201, };
  /*Possible corruped events */ //uint64_t refEvt[3] = {131371, 145962, 173350, };

  std::vector<uint64_t> refEvents;
  for(int ievent=0;ievent<26;ievent++) refEvents.push_back(refEvt[ievent]);
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
  uint32_t link1_bx_diff(1), link2_bx_diff(0);
  
  uint64_t minEventDAQ, maxEventDAQ; 
  long double nloopEvent =  100000;
  int nloop = TMath::CeilNint(maxEvent/nloopEvent) ;
  if(refEvents.size()>0) nloop = 1;
  std::cout<<"nloop: "<<nloop<<std::endl;
  for(int ieloop=0;ieloop<nloop;ieloop++){
    
    minEventDAQ = ieloop*nloopEvent ;
    maxEventDAQ = (ieloop+1)*nloopEvent;
    maxEventDAQ = (maxEventDAQ>uint64_t(maxEvent)) ? uint64_t(maxEvent) : maxEventDAQ ;
    
    printf("iloop : %d, minEventDAQ = %lu, maxEventDAQ = %lu\n",ieloop,minEventDAQ, maxEventDAQ);
    std::cerr<<"iloop : "<< ieloop <<", minEventDAQ = "<< minEventDAQ <<", maxEventDAQ = " << maxEventDAQ << std::endl;
    
    hrocarray.clear();
    eventList.clear();
    econtarray.clear();
    tcprocarray.clear();
    
    econTReader.init(relayNumber,runNumber,0);
    econTReader.setNofCAFESep(11);
    if(relayNumber>=1727191081) econTReader.setNofCAFESep(14);
    std::cout<<"TRIG Before Link "<<trig_linkNumber<<", size : " << econtarray.size() <<std::endl;
    econTReader.getEvents(refEvents, minEventDAQ, maxEventDAQ, econtarray, tcprocarray);
    std::cout<<"TRIG After Link "<<trig_linkNumber<<", size : " << econtarray.size() <<std::endl;
    econTReader.terminate();
    
    econDReader.init(relayNumber,runNumber,linkNumber);
    std::cout<<"DAQ Before Link "<<linkNumber<<", size : " << hrocarray.size() <<std::endl;
    econDReader.getEvents(refEvents, minEventDAQ, maxEventDAQ, hrocarray, eventList); //Input <---> output
    std::cout<<"DAQ After Link "<<linkNumber<<", size : " << hrocarray.size() <<std::endl;
    econDReader.terminate();
    
    // for(uint64_t ievt = 0 ; ievt < eventList.size(); ievt++ ){
    //   uint64_t ievent = eventList[ievt] ;
    //   std::cout << "Event: " << ievent << ", ECONT: " << econtarray.at(ievent).size() << ", ROC: "<< hrocarray.at(ievent).size() << ", tcprocarray: " << tcprocarray.size() << std::endl;
    //   std::cout << "lp4 ext trigger  ID " << lp4_ext_trigger << std::endl;
    //   std::vector<std::pair<uint32_t,TPGBEDataformat::Trig24Data>> econtdata =  econtarray[ievent];
    //   for(uint32_t iecon=0;iecon<econtarray.at(ievent).size();iecon++){
    // 	std::cout<<"\tecon numbers : " << econtdata.at(iecon).first << std::endl;
    //   }
    // }
    // continue;
    
    for(uint64_t ievt = 0 ; ievt < eventList.size(); ievt++ ){
      uint64_t ievent = eventList[ievt] ;
      if(econtarray.at(ievent).size()!=11 or hrocarray.at(ievent).size()!=36) continue;
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
    
    uint32_t maxTestEvent = (refEvents.size()>0)?refEvents.size():nloopEvent-1;
    FindBxShift(dir_diff, eventList, maxTestEvent, pck, econTPar, hrocarray, econtarray, rocTPGEmul, econtEmul);
    if(ieloop==0 and refEvents.size()==0){
      TList *list = (TList *)dir_diff->GetList();
      TH1D *projX_link1[7];
      for(int ibin=0;ibin<7;ibin++){
	projX_link1[ibin] = ((TH2D *)list->FindObject(Form("hBxCorr_0")))->ProjectionX(Form("projX_link1_%d",ibin),ibin+1,ibin+1);
	if(projX_link1[ibin]->GetEntries()>0){
	  link1_bx_diff = projX_link1[ibin]->GetXaxis()->GetBinCenter(projX_link1[ibin]->GetMaximumBin()) - float(ibin) ;
	  break;
	}
      }//bin loop
      TH1D *projX_link2[7];
      for(int ibin=0;ibin<7;ibin++){
	projX_link2[ibin] = ((TH2D *)list->FindObject(Form("hBxCorr_1")))->ProjectionX(Form("projX_link2_%d",ibin),ibin+1,ibin+1);
	if(projX_link2[ibin]->GetEntries()>0){
	  link2_bx_diff = projX_link2[ibin]->GetXaxis()->GetBinCenter(projX_link2[ibin]->GetMaximumBin()) - float(ibin) ;
	  break;
	}
      }//bin loop
      std::cout << "link1_bx_diff: " << link1_bx_diff << ", link2_bx_diff: " << link2_bx_diff << std::endl;
    }//for first iteration only

    //===============================================================================================================================
    // Emulate
    //===============================================================================================================================
    std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::ModuleTcData>>> modarray; //event,moduleId
    modarray.clear();
    /////////////////////////////////////////////////////////////
    //// The following part should be in a loop over modules
    /////////////////////////////////////////////////////////////
    std::cout<<"eventList size: " << eventList.size() <<std::endl;
    
    for(uint64_t ievt = 0 ; ievt < eventList.size(); ievt++ ){
      uint64_t event = eventList[ievt] ;
      
      //first check that both econd and econt data has the same eventid otherwise skip the event
      if(econtarray.find(event) == econtarray.end() or hrocarray.find(event) == hrocarray.end() or econtarray.at(event).size()!=11 or hrocarray.at(event).size()!=36) {
	if(std::find(corruptEvent.begin(),corruptEvent.end(),event) == corruptEvent.end()) corruptEvent.push_back(event);
	continue;
      }
      
      //bool eventCondn = (event<1000);
      bool eventCondn = (refEvents.size()==0) ? (ievt<10) : (ievt<refEvents.size());
      if( eventCondn or event%100000==0)
	std::cout<<std::endl<<std::endl<<"=========================================================================="<<std::endl<<"Processing event: " << event <<std::endl<<std::endl<<std::endl;
      
      for(int ilink=0;ilink<2;ilink++){
	for(int iecond=0;iecond<3;iecond++){
	  uint32_t moduleId = pck.packModId(zside, sector, ilink, det, iecond, selTC4, module); //we assume same ECONT and ECOND number for a given module
	  std::map<uint32_t,TPGFEDataformat::HalfHgcrocData> rocdata;
	  std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;   
	  std::map<uint32_t,TPGFEDataformat::ModuleTcData> moddata;
	  
	  int emul_bx;
	  uint32_t econd_bx, econd_bx_4b, emul_bx_4b;
	  uint32_t econt_central_bx_4b;
	  uint16_t econt_slink_bx, econd_slink_bx; 
	  std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>> hrocvec ;
	  hrocvec =  hrocarray.at(event);	  
	  for(const auto& data : hrocvec){
	    rocdata[data.first] = data.second ;
	    const TPGFEDataformat::HalfHgcrocData& hrocdata = data.second ;
	    //if(moduleId==pck.getModIdFromRocId(uint32_t(data.first)))	    
	    if(data.first==moduleId){
	      econd_bx = rocdata[data.first].getBx();
	      econd_slink_bx = rocdata[data.first].getSlinkBx();
	      econd_bx_4b = (econd_bx==3564) ? 0xF : (econd_bx%8);
	      emul_bx_4b = econd_bx_4b ;
	      //tune the bx here to match the TPG
	      if((link1_bx_diff==1 and ilink==0) or (link2_bx_diff==1 and ilink==1)){ //note these changes only work for cases with bxdiff = +1
		if(rocdata[data.first].getBx()==3564)
		  emul_bx_4b = 1;
		else if(rocdata[data.first].getBx()==3563)
		  emul_bx_4b = 0xF;
		else if(emul_bx_4b==7)
		  emul_bx_4b = 0x0;
		else
		  emul_bx_4b += 1;
	      }//link1 or link2
	    }//match moduleId
	  }//loop halfHGCROCs

	  
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
	  TPGFEDataformat::TcRawDataPacket vTCel;	
    	  int refbx[2]; refbx[0] = -1; refbx[1] = -1;
	  int tpg_m3_bxid = -1, tpg_p3_bxid = -1; 
	  TPGBEDataformat::Trig24Data trdata;
	  uint32_t nofMatches = 0;
	  for(const auto& econtit : econtdata){
	    if(econtit.first!=moduleId) continue;
	    trdata = econtit.second ;
	    //if(eventCondn) std::cout << "Dataloop:: event: " << event << ", moduleId : " << econtit.first << std::endl;
	    for(int ibx=0;ibx<7;ibx++){
	      const uint32_t *el = trdata.getElinks(ibx); 
	      uint32_t bx_2 = (el[0]>>28) & 0xF;
	      if(ibx==0) tpg_m3_bxid = bx_2;
	      if(ibx==6) tpg_p3_bxid = bx_2;
	      if(bx_2==emul_bx_4b and nofMatches<=1) {
		refbx[nofMatches] = ibx;
		nofMatches++;
	      }//match found;
	      if(ilink==0) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::BestC, econTPar[moduleId].getNofTCs(), el, vTCel);
	      if(ilink==1) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC16, econTPar[moduleId].getNofSTCs(), el, vTCel);
	      if(ilink==2 or ilink==3) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC4A, econTPar[moduleId].getNofSTCs(), el, vTCel);
	      if(eventCondn) TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(bx_2, vTCel, up1);
	      //if(eventCondn) vTCel.print();
	      // if(eventCondn) up1.print();
	      //if(hasMatched) break;
	    }//bx loop
	    //if(eventCondn) trdata.print();
	    //if(hasMatched) break;
	  }//loop over econt data for a given run
	  if(refbx[0]==-1 and refbx[1]==-1){
	    //std::cout << "refbx: " << refbx << std::endl ;
	    if(std::find(shiftedBxEvent.begin(),shiftedBxEvent.end(),event) == shiftedBxEvent.end()) shiftedBxEvent.push_back(event);
	    econt_slink_bx = trdata.getSlinkBx();
	    FillHistogram(false, false, dir_diff, relayNumber, event, econt_slink_bx, econTPar[moduleId].getNElinks(), 0x0, elinkemul, ilink, iecond, vTCel, TcRawdata.second, 0x0, 0x0, up1, upemul,
			  unExEmulTC, unExDataTC, elStg1Event);
			  //hrocarray);
	    delete []elinkemul;
	    std::cout<<std::endl<<std::endl<<"=========================================================================="<<std::endl<<"Skipping event: " << event <<std::endl<<std::endl<<std::endl;
	    std::cout << "Skip Event: "<< event
		      << ", ECOND:: (econd_slink_bx: " << econd_slink_bx <<", econd_bx: "<<econd_bx<<", econd econd_bx_4b : " <<  econd_bx_4b
		      << ") ECONT:: (econt_slink_bx: " << trdata.getSlinkBx() <<", econt_slink_bx%8: "<< (trdata.getSlinkBx()%8) <<", TPG bxid range : [" << tpg_m3_bxid << ", " << tpg_p3_bxid << "] "
		      <<"), moduleId : " << moduleId << std::endl;
	    TcRawdata.second.print();
	    continue;
	  }
	  const uint32_t *elmid = trdata.getElinks(3);
	  econt_central_bx_4b = (elmid[0]>>28) & 0xF;
	  econt_slink_bx = trdata.getSlinkBx();
	  int selbx = -1;
	  if(nofMatches==1)
	    selbx = refbx[0];
	  else
	    selbx = (TMath::Abs(refbx[0]-3)<TMath::Abs(refbx[1]-3))?refbx[0]:refbx[1];
	  const uint32_t *eldata = trdata.getElinks(selbx);
	  const uint32_t *unpkWords = trdata.getUnpkWords(selbx);
	  const uint32_t *unpkWords1 = trdata.getUnpkWords(selbx,1);
	  uint32_t bx_data = (eldata[0]>>28) & 0xF;
	  TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TcRawdata.second.type(), TcRawdata.second.size(), eldata, vTCel);	  
	  TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(emul_bx_4b, TcRawdata.second, upemul);
	  TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(bx_data, vTCel, up1);

	  
	  bool hasTcTp1 = false;
	  if(TcRawdata.second.isTcTp1()) hasTcTp1 = true;
	  bool isLargeDiff = false;
	  for(uint32_t iel=0;iel<econTPar[moduleId].getNElinks();iel++) if((eldata[iel]-elinkemul[iel]) != 0) isLargeDiff = true;
	  
	  bool printCondn = (eventCondn or isLargeDiff);
	  //bool printCondn = eventCondn;
	  
	  if(isLargeDiff)
	    std::cout<<std::endl<<std::endl<<"=========================================================================="<<std::endl<<"Processing event: " << event <<std::endl<<std::endl<<std::endl;
	  if(printCondn) std::cout << "Elink comparison for  ievent: "<< event << ", moduleId : " << moduleId << std::endl;
	  if(printCondn) std::cout << "Event: "<< event
						  << ", ECOND:: (econd_slink_bx: " << econd_slink_bx <<", econd_bx: "<<econd_bx<<", econd econd_bx_4b : " <<  econd_bx_4b
						  << ") ECONT:: (econt_slink_bx: " << econt_slink_bx <<", econt_slink_bx%8: "<< (econt_slink_bx%8) <<", econt_central_bx_4b: " << econt_central_bx_4b
						  <<"), moduleId : " << moduleId <<", isLargeDiff: " << isLargeDiff << std::endl;
	  //if(printCondn) modTcdata.second.print();
	  if(printCondn) TcRawdata.second.print();
	  if(printCondn) vTCel.print();
          // if(printCondn) up1.print();
	  // if(printCondn) upemul.print();
	  // if(printCondn) trdata.print();
	  // 
	  for(uint32_t iel=0;iel<econTPar[moduleId].getNElinks();iel++){
	    if(printCondn) std::cout<<"Elink emul : 0x" << std::hex << std::setfill('0') << std::setw(8) << elinkemul[iel] << std::dec ;
	    if(printCondn) std::cout<<"\t Elink data : 0x" << std::hex << std::setfill('0') << std::setw(8) << eldata[iel] << std::dec ;//<< std::endl;
	    if(printCondn) std::cout<<"\t Diff: " << std::setfill(' ') << std::setw(10) << (eldata[iel]-elinkemul[iel])  << ", XOR: " << std::bitset<32>{(eldata[iel] ^ elinkemul[iel])} << std::endl;
	    if( ((eldata[iel]-elinkemul[iel]) != 0) and (std::find(nonZeroEvents.begin(),nonZeroEvents.end(),event) == nonZeroEvents.end())) nonZeroEvents.push_back(event);
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
	  FillHistogram(true, isLargeDiff, dir_diff, relayNumber, event, econt_slink_bx, econTPar[moduleId].getNElinks(), eldata, elinkemul, ilink, iecond, vTCel, TcRawdata.second, unpkWords, unpkWords1, up1, upemul,
			unExEmulTC, unExDataTC, elStg1Event);//, hrocarray);	  
	  delete []elinkemul;
	  // delete []unpkMsTc;
	}//econd loop
      }//ilink loop    
    }//event loop
  }//ieloop
  //===============================================================================================================================
  if(TcTp1Events.size()>0) std::cerr<< "/*TcTp1 events */ uint64_t refEvt["<< TcTp1Events.size() <<"] = {";
  for(const uint64_t& totEvt : TcTp1Events) std::cerr<<totEvt << ", ";
  if(TcTp1Events.size()>0) std::cerr<< "};" << std::endl;
  
  if(TcTp2Events.size()>0) std::cerr<< "/*TcTp2 events */ uint64_t refEvt["<< TcTp2Events.size() <<"] = {";
  for(const uint64_t& totEvt : TcTp2Events) std::cerr<<totEvt << ", ";
  if(TcTp2Events.size()>0) std::cerr<< "};" << std::endl;

  if(TcTp3Events.size()>0) std::cerr<< "/*TcTp3 events */ uint64_t refEvt["<< TcTp3Events.size() <<"] = {";
  for(const uint64_t& totEvt : TcTp3Events) std::cerr<<totEvt << ", ";
  if(TcTp3Events.size()>0) std::cerr<< "};" << std::endl;

  if(TcTp1TrigEvents.size()>0) std::cerr<< "/*TcTp1 Trig events */ uint64_t refEvt["<< TcTp1TrigEvents.size() <<"] = {";
  for(const uint64_t& totEvt : TcTp1TrigEvents) std::cerr<<totEvt << ", ";
  if(TcTp1TrigEvents.size()>0) std::cerr<< "};" << std::endl;

  if(TcTp2TrigEvents.size()>0) std::cerr<< "/*TcTp2 Trig events */ uint64_t refEvt["<< TcTp2TrigEvents.size() <<"] = {";
  for(const uint64_t& totEvt : TcTp2TrigEvents) std::cerr<<totEvt << ", ";
  if(TcTp2TrigEvents.size()>0) std::cerr<< "};" << std::endl;

  if(TcTp3TrigEvents.size()>0) std::cerr<< "/*TcTp3 Trig events */ uint64_t refEvt["<< TcTp3TrigEvents.size() <<"] = {";
  for(const uint64_t& totEvt : TcTp3TrigEvents) std::cerr<<totEvt << ", ";
  if(TcTp3TrigEvents.size()>0) std::cerr<< "};" << std::endl;

  if(nonZeroEvents.size()>0) std::cerr<< "/*Nonzero diff events*/ uint64_t refEvt["<< nonZeroEvents.size() <<"] = {";
  for(const uint64_t& totEvt : nonZeroEvents) std::cerr<<totEvt << ", ";
  if(nonZeroEvents.size()>0) std::cerr<< "};" << std::endl;

  if(shiftedBxEvent.size()>0) std::cerr<< "/*Shifted Bx/events */ uint64_t refEvt["<< shiftedBxEvent.size() <<"] = {";
  for(const uint64_t& totEvt : shiftedBxEvent) std::cerr<<totEvt << ", ";
  if(shiftedBxEvent.size()>0) std::cerr<< "};" << std::endl;

  if(corruptEvent.size()>0) std::cerr<< "/*Possible corruped events */ uint64_t refEvt["<< corruptEvent.size() <<"] = {";
  for(const uint64_t& totEvt : corruptEvent) std::cerr<<totEvt << ", ";
  if(corruptEvent.size()>0) std::cerr<< "};" << std::endl;

  // if(unExDataTC.size()>0) std::cerr<< "/*Unexpect TC in Data in event*/ uint64_t refEvt["<< unExDataTC.size() <<"] = {";
  // for(const uint64_t& totEvt : unExDataTC) std::cerr<<totEvt << ", ";
  // if(unExDataTC.size()>0) std::cerr<< "};" << std::endl;

  // if(unExEmulTC.size()>0) std::cerr<< "/*Unexpect TC in Emul in event*/ uint64_t refEvt["<< unExEmulTC.size() <<"] = {";
  // for(const uint64_t& totEvt : unExEmulTC) std::cerr<<totEvt << ", ";
  // if(unExEmulTC.size()>0) std::cerr<< "};" << std::endl;

  if(unExDataTC.size()>0) std::cerr<< "/*Check STC in event*/ uint64_t refEvt["<< unExDataTC.size() <<"] = {";
  for(const uint64_t& totEvt : unExDataTC) std::cerr<<totEvt << ", ";
  if(unExDataTC.size()>0) std::cerr<< "};" << std::endl;

  if(unExEmulTC.size()>0) std::cerr<< "/*Check BC in event*/ uint64_t refEvt["<< unExEmulTC.size() <<"] = {";
  for(const uint64_t& totEvt : unExEmulTC) std::cerr<<totEvt << ", ";
  if(unExEmulTC.size()>0) std::cerr<< "};" << std::endl;

  // if(elStg1Event.size()>0) std::cerr<< "/*Difference in elink and Stage1 output in event*/ uint64_t refEvt["<< elStg1Event.size() <<"] = {";
  // for(const uint64_t& totEvt : elStg1Event) std::cerr<<totEvt << ", ";
  // if(elStg1Event.size()>0) std::cerr<< "};" << std::endl;
  
  fout->cd();
  dir_diff->Write();
  TList *list = (TList *)dir_diff->GetList();
  TH1D *projX_link1_bin1 = ((TH2D *)list->FindObject(Form("hBxCorr_0")))->ProjectionX("projX_link1_bin1",1,1);
  TH1D *projX_link1_bin2 = ((TH2D *)list->FindObject(Form("hBxCorr_0")))->ProjectionX("projX_link1_bin2",2,2);
  TH1D *projX_link1_bin3 = ((TH2D *)list->FindObject(Form("hBxCorr_0")))->ProjectionX("projX_link1_bin3",3,3);
  TH1D *projX_link1_bin4 = ((TH2D *)list->FindObject(Form("hBxCorr_0")))->ProjectionX("projX_link1_bin4",4,4);
  TH1D *projX_link1_bin5 = ((TH2D *)list->FindObject(Form("hBxCorr_0")))->ProjectionX("projX_link1_bin5",5,5);
  TH1D *projX_link2_bin1 = ((TH2D *)list->FindObject(Form("hBxCorr_1")))->ProjectionX("projX_link2_bin1",1,1);
  TH1D *projX_link2_bin2 = ((TH2D *)list->FindObject(Form("hBxCorr_1")))->ProjectionX("projX_link2_bin2",2,2);
  TH1D *projX_link2_bin3 = ((TH2D *)list->FindObject(Form("hBxCorr_1")))->ProjectionX("projX_link2_bin3",3,3);
  TH1D *projX_link2_bin4 = ((TH2D *)list->FindObject(Form("hBxCorr_1")))->ProjectionX("projX_link2_bin4",4,4);
  TH1D *projX_link2_bin5 = ((TH2D *)list->FindObject(Form("hBxCorr_1")))->ProjectionX("projX_link2_bin5",5,5);
  projX_link1_bin1->Write();
  projX_link1_bin2->Write();
  projX_link1_bin3->Write();
  projX_link1_bin4->Write();
  projX_link1_bin5->Write();
  projX_link2_bin1->Write();
  projX_link2_bin2->Write();
  projX_link2_bin3->Write();
  projX_link2_bin4->Write();
  projX_link2_bin5->Write();
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

  TH2D *hBxCorr_0 = new TH2D("hBxCorr_0", Form("Bx correlation link-0 for relay %u",relay), 20, -0.5, 19.5, 20, -0.5, 19.5);
  hBxCorr_0->GetXaxis()->SetTitle("bx data");
  hBxCorr_0->GetYaxis()->SetTitle("bx emul");
  hBxCorr_0->SetDirectory(dir_diff);

  TH2D *hBxCorr_1 = new TH2D("hBxCorr_1", Form("Bx correlation link-1 for relay %u",relay), 20, -0.5, 19.5, 20, -0.5, 19.5);
  hBxCorr_1->GetXaxis()->SetTitle("bx data");
  hBxCorr_1->GetYaxis()->SetTitle("bx emul");
  hBxCorr_1->SetDirectory(dir_diff);

  TH2D *hBCEnergyDataVsEmul[3], *hBCEnergyDataVsEmulNoTcTp[3] ;
  TH2D *hSTCEnergyDataVsEmul[3], *hSTCEnergyDataVsEmulNoTcTp[3] ;
  for(int imdl=0;imdl<3;imdl++){
    hBCEnergyDataVsEmul[imdl] = new TH2D(Form("hBCEnergyDataVsEmul_%d",imdl), Form("BestChoice: Emulated vs Trigger energy for relay %u",relay), 200, 0.0, 20000.0, 200, 0.0, 20000.0);
    hBCEnergyDataVsEmul[imdl]->GetXaxis()->SetTitle("Emulated energy using DAQ readout (raw unpacked)");
    hBCEnergyDataVsEmul[imdl]->GetYaxis()->SetTitle("Energy in trigger readout (raw unpacked)");
    hBCEnergyDataVsEmul[imdl]->SetDirectory(dir_diff);
    
    hBCEnergyDataVsEmulNoTcTp[imdl] = new TH2D(Form("hBCEnergyDataVsEmulNoTcTp_%d",imdl), Form("BestChoice: Emulated vs Trigger energy for relay %u",relay), 200, 0.0, 20000.0, 200, 0.0, 20000.0);
    hBCEnergyDataVsEmulNoTcTp[imdl]->GetXaxis()->SetTitle("Emulated energy using DAQ readout (raw unpacked)");
    hBCEnergyDataVsEmulNoTcTp[imdl]->GetYaxis()->SetTitle("Energy in trigger readout (raw unpacked)");
    hBCEnergyDataVsEmulNoTcTp[imdl]->SetDirectory(dir_diff);
    
    hSTCEnergyDataVsEmul[imdl] = new TH2D(Form("hSTCEnergyDataVsEmul_%d",imdl), Form("STC16: Emulated vs Trigger energy for relay %u",relay), 200, 0.0, 20000.0, 200, 0.0, 20000.0);
    hSTCEnergyDataVsEmul[imdl]->GetXaxis()->SetTitle("Emulated energy using DAQ readout (raw unpacked)");
    hSTCEnergyDataVsEmul[imdl]->GetYaxis()->SetTitle("Energy in trigger readout (raw unpacked)");
    hSTCEnergyDataVsEmul[imdl]->SetDirectory(dir_diff);
    
    hSTCEnergyDataVsEmulNoTcTp[imdl] = new TH2D(Form("hSTCEnergyDataVsEmulNoTcTp_%d",imdl), Form("STC16: Emulated vs Trigger energy for relay %u",relay), 200, 0.0, 20000.0, 200, 0.0, 20000.0);
    hSTCEnergyDataVsEmulNoTcTp[imdl]->GetXaxis()->SetTitle("Emulated energy using DAQ readout (raw unpacked)");
    hSTCEnergyDataVsEmulNoTcTp[imdl]->GetYaxis()->SetTitle("Energy in trigger readout (raw unpacked)");
    hSTCEnergyDataVsEmulNoTcTp[imdl]->SetDirectory(dir_diff);
  }
  
  TH1D *hBCTCEDataNoTcTp[3], *hBCTCEEmulNoTcTp[3] ;
  TH1D *hSTCEDataNoTcTp[3], *hSTCEEmulNoTcTp[3] ;
  for(int imdl=0;imdl<3;imdl++){
    hBCTCEDataNoTcTp[imdl] = new TH1D(Form("hBCTCEDataNoTcTp_%d",imdl), Form("BestChoice: Energy in trigger readout for relay %u",relay), 1000, 0.0, 20000.0);
    hBCTCEDataNoTcTp[imdl]->GetXaxis()->SetTitle("Energy in trigger readout (raw unpacked)");
    hBCTCEDataNoTcTp[imdl]->SetDirectory(dir_diff);
    
    hBCTCEEmulNoTcTp[imdl] = new TH1D(Form("hBCTCEEmulNoTcTp_%d",imdl), Form("BestChoice: Emulated energy using DAQ readout for relay %u",relay), 1000, 0.0, 20000.0);
    hBCTCEEmulNoTcTp[imdl]->GetXaxis()->SetTitle("Emulated energy using DAQ readout (raw unpacked)");
    hBCTCEEmulNoTcTp[imdl]->SetDirectory(dir_diff);
    
    hSTCEDataNoTcTp[imdl] = new TH1D(Form("hSTCEDataNoTcTp_%d",imdl), Form("STC16: Energy for relay %u",relay), 1000, 0.0, 20000.0);
    hSTCEDataNoTcTp[imdl]->GetXaxis()->SetTitle("Energy in trigger readout (raw unpacked)");
    hSTCEDataNoTcTp[imdl]->SetDirectory(dir_diff);
    
    hSTCEEmulNoTcTp[imdl] = new TH1D(Form("hSTCEEmulNoTcTp_%d",imdl), Form("STC16: Emulated energy using DAQ readout for relay %u",relay), 1000, 0.0, 20000.0);
    hSTCEEmulNoTcTp[imdl]->GetXaxis()->SetTitle("Emulated energy using DAQ readout (raw unpacked)");
    hSTCEEmulNoTcTp[imdl]->SetDirectory(dir_diff);
  }

  
}

void FillHistogram(bool matchFound, bool isLargeDiff, TDirectory*& dir_diff, uint32_t relayNumber, uint64_t event, uint16_t inputbxId,
		   uint32_t nelinks,  const uint32_t *eldata, const uint32_t *elemul,
		   uint32_t ilp, uint32_t imdl,
		   TPGFEDataformat::TcRawDataPacket& tcdata, TPGFEDataformat::TcRawDataPacket& tcemul,
		   const uint32_t *unpkWords, const uint32_t *unpkWords1,
		   TPGBEDataformat::UnpackerOutputStreamPair& updata, TPGBEDataformat::UnpackerOutputStreamPair& upemul,
		   std::vector<uint64_t>& unExEmulTC, std::vector<uint64_t>& unExDataTC, std::vector<uint64_t>& elStg1Event){
  //std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>>>& hrocarray){
  
  uint32_t IdealE[2][3][10] = { //nof lpGBTs, nofModules, nofTC/STCs
    {
      {48, 48, 48, 48, 48, 49, 00, 00, 00, 00},
      {43, 43, 43, 44, 00, 00, 00, 00, 00, 00},
      {35, 35, 35, 36, 00, 00, 00, 00, 00, 00}
    },
    {
      {60, 60, 61, 61, 62, 62, 63, 63, 64, 64},
      {52, 53, 54, 55, 56, 56, 00, 00, 00, 00},
      {02, 27, 35, 39, 41, 43, 00, 00, 00, 00}
    }
  };
  
  uint32_t IdealA[2][3][10] = { //nof lpGBTs, nofModules, nofTC/STCs
    {
      {32, 33, 34, 36, 37, 47, 99, 99, 99, 99},
      {40, 41, 42, 47, 99, 99, 99, 99, 99, 99},
      {44, 45, 46, 47, 99, 99, 99, 99, 99, 99}
    },
    {
      {0, 3, 0, 3, 0, 3, 0, 3, 0, 0},
      {3, 3, 3, 3, 0, 3, 9, 9, 9, 9},
      {0, 3, 3, 3, 3, 3, 9, 9, 9, 9}
    }
  };
  
  uint32_t IdealL[2][3][3] = { //nof lpGBTs, nofModules, maxElinks
    {
      {0x05b8218a, 0x496f60c1, 0x83060c40},
      {0x055a29aa, 0xf56ad5ac, 0xffffffff},
      {0x049b2dba, 0xf468d1a4, 0xffffffff}
    },
    {
      {0x03333078, 0xf1ebd7cf, 0x9fbf8100},
      {0x0ff368d5, 0xb3770e00, 0xffffffff},
      {0x03ff046d, 0x1a752ac0, 0xffffffff}
    }
  };

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
      if( ((IdealL[ilp][imdl][iel] - emullink) != 0)) hasDiffEmul = true;
      if( ((IdealL[ilp][imdl][iel] - datalink) != 0)) hasDiffData = true;
    }
    if(hasDiff) ((TH2D *) list->FindObject("hNZModules"))->Fill( ilp, imdl );
    if(hasDiffData) {
      ((TH2D *) list->FindObject("hNZDataModules"))->Fill( ilp, imdl );
      //if(std::find(unExDataTC.begin(),unExDataTC.end(),event) == unExDataTC.end() and relayNumber==1726581356) unExDataTC.push_back(event);
    }
    if(hasDiffEmul) {
      ((TH2D *) list->FindObject("hNZEmulModules"))->Fill( ilp, imdl );
      //if(std::find(unExEmulTC.begin(),unExEmulTC.end(),event) == unExEmulTC.end() and relayNumber==1726581356) unExEmulTC.push_back(event);
    }
    
    if(imode==0 and imdl==0 and ilp==0){
      if(hasDiff)
	((TH1D *) list->FindObject("hSlinkBx8NZ"))->Fill( bxIdmod8 );
      else
	((TH1D *) list->FindObject("hSlinkBx8Z"))->Fill( bxIdmod8 );
      ((TH1D *) list->FindObject("hSlinkBx8Matched"))->Fill( bxIdmod8 );
    }
  
    std::vector<int> ideallist, emulist,econtlist;
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
      
      uint32_t ideal_stc = 4*itc + IdealA[ilp][imdl][itc];
      uint32_t data_stc = 4*itc + tcdata.getTcData().at(itc).address();
      uint32_t emul_stc = 4*itc + tcemul.getTcData().at(itc).address();
      if(isLargeDiff){
	if(ilp==0){
	  ((TH2D *) list->FindObject(Form("hBCEnergyData_%d",imdl)))->Fill(IdealE[ilp][imdl][itc],tcdata.getTcData().at(itc).energy());
	  ((TH2D *) list->FindObject(Form("hBCEnergyEmul_%d",imdl)))->Fill(IdealE[ilp][imdl][itc],tcemul.getTcData().at(itc).energy());
	  ((TH2D *) list->FindObject(Form("hBCPosData_%d",imdl)))->Fill(IdealA[ilp][imdl][itc],tcdata.getTcData().at(itc).address());
	  ((TH2D *) list->FindObject(Form("hBCPosEmul_%d",imdl)))->Fill(IdealA[ilp][imdl][itc],tcemul.getTcData().at(itc).address());
	}else{
	  ((TH2D *) list->FindObject(Form("hSTCEnergyData_%d",imdl)))->Fill(IdealE[ilp][imdl][itc],tcdata.getTcData().at(itc).energy());
	  ((TH2D *) list->FindObject(Form("hSTCEnergyEmul_%d",imdl)))->Fill(IdealE[ilp][imdl][itc],tcemul.getTcData().at(itc).energy());
	  ((TH2D *) list->FindObject(Form("hSTCPosData_%d",imdl)))->Fill(ideal_stc, data_stc);
	  ((TH2D *) list->FindObject(Form("hSTCPosEmul_%d",imdl)))->Fill(ideal_stc, emul_stc);
	}
      }//only for problematic module with elink mismatch
      // TH2D *hBCEnergyDataVsEmul[3], *hBCEnergyDataVsEmulNoTcTp[3] ;
      // TH2D *hSTCEnergyDataVsEmul[3], *hSTCEnergyDataVsEmulNoTcTp[3] ;
      // TH1D *hBCTCEDataNoTcTp[3], *hBCTCEEmulNoTcTp[3] ;
      // TH1D *hSTCEDataNoTcTp[3], *hSTCEEmulNoTcTp[3] ;
      if(ilp==0){
	uint32_t emulE = tcemul.getTcData().at(itc).decodedE(TPGFEDataformat::BestC) ;
	uint32_t dataE = tcdata.getTcData().at(itc).decodedE(TPGFEDataformat::BestC) ;
	
	((TH2D *) list->FindObject(Form("hBCEnergyDataVsEmul_%d",imdl)))->Fill( emulE, dataE );
	if(imode!=1 and imode!=2) { //In case of BestChoice TcTp condition to be checked for full module as ordering changes
	  ((TH2D *) list->FindObject(Form("hBCEnergyDataVsEmulNoTcTp_%d",imdl)))->Fill(  emulE, dataE  );
	  ((TH1D *) list->FindObject(Form("hBCTCEDataNoTcTp_%d",imdl)))->Fill( dataE );
	  ((TH1D *) list->FindObject(Form("hBCTCEEmulNoTcTp_%d",imdl)))->Fill( emulE );
	  if(std::find(unExEmulTC.begin(),unExEmulTC.end(),event) == unExEmulTC.end() and (emulE-dataE)!=0) unExEmulTC.push_back(event);
	}
      }else if(ilp==1) {
	uint32_t emulE = tcemul.getTcData().at(itc).decodedE(TPGFEDataformat::STC16) ;
	uint32_t dataE = tcdata.getTcData().at(itc).decodedE(TPGFEDataformat::STC16) ;

	((TH2D *) list->FindObject(Form("hSTCEnergyDataVsEmul_%d",imdl)))->Fill( emulE, dataE );
	if(imode!=1 and imode!=2) {
	  ((TH2D *) list->FindObject(Form("hSTCEnergyDataVsEmulNoTcTp_%d",imdl)))->Fill( emulE, dataE );
	  ((TH1D *) list->FindObject(Form("hSTCEDataNoTcTp_%d",imdl)))->Fill( dataE );
	  ((TH1D *) list->FindObject(Form("hSTCEEmulNoTcTp_%d",imdl)))->Fill( emulE );
	  if(std::find(unExDataTC.begin(),unExDataTC.end(),event) == unExDataTC.end() and (emulE-dataE)!=0) unExDataTC.push_back(event);
	}
      }

      
      if(imdl==0){
	((TH1D *) list->FindObject("hOccuTCData"))->Fill( tcdata.getTcData().at(itc).address() );
	((TH1D *) list->FindObject("hOccuTCEmul"))->Fill( tcemul.getTcData().at(itc).address() );
      }
      if(ilp==0){
	ideallist.push_back(IdealA[ilp][imdl][itc]);
	emulist.push_back(tcemul.getTcData().at(itc).address());
	econtlist.push_back(tcdata.getTcData().at(itc).address());
      }else{
	ideallist.push_back( ideal_stc );
	emulist.push_back( emul_stc );
	econtlist.push_back( data_stc );
      }
      
    }//TC loop
    if(isLargeDiff){
      int *missingEmulTC = new int[tcdata.size()];
      int nofemulmismatches = 0;
      for(const auto& itcemul : emulist)
	if(std::find(ideallist.begin(), ideallist.end(), itcemul) == ideallist.end())
	  missingEmulTC[nofemulmismatches++] = itcemul;
      
      int *missingDataTC = new int[tcdata.size()];;
      int nofdatamismatches = 0;
      for(const auto& itcdata : econtlist)
	if(std::find(ideallist.begin(), ideallist.end(), itcdata) == ideallist.end())
	  missingDataTC[nofdatamismatches++] = itcdata;
      
      if(nofemulmismatches==1 and nofdatamismatches==1 and imdl==0) {
	((TH2D *) list->FindObject("hMissedTCs"))->Fill( missingEmulTC[0], missingDataTC[0] );
	//std::cout<<"Event with single mismatch : " << event << std::endl;
      }

      for(int idatamm = 0 ; idatamm < nofdatamismatches ; idatamm++){
	if(ilp==0){
	  ((TH2D *) list->FindObject("hBCUnExTCData"))->Fill( missingDataTC[idatamm], imdl );
	}else{
	  ((TH2D *) list->FindObject("hSTCUnExTCData"))->Fill( missingDataTC[idatamm], imdl );
	}
      }//if TC is not expected in data
      for(int iemulmm = 0 ; iemulmm < nofemulmismatches ; iemulmm++){
	if(ilp==0){
	  ((TH2D *) list->FindObject("hBCUnExTCEmul"))->Fill( missingEmulTC[iemulmm], imdl );
	}else{
	  ((TH2D *) list->FindObject("hSTCUnExTCEmul"))->Fill( missingEmulTC[iemulmm], imdl );
	}
      }//if one missing TC is absent
      delete []missingEmulTC;
      delete []missingDataTC;
    }
    
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

int FindBxShift(TDirectory*& dir_diff, std::vector<uint64_t>& eventList, uint32_t maxTestEvent,
		TPGFEConfiguration::TPGFEIdPacking& pck,
		std::map<uint32_t,TPGFEConfiguration::ConfigEconT>& econTPar,
		std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>>>& hrocarray,
		std::map<uint64_t,std::vector<std::pair<uint32_t,TPGBEDataformat::Trig24Data>>> & econtarray,
		TPGFEModuleEmulation::HGCROCTPGEmulation& rocTPGEmul,
		TPGFEModuleEmulation::ECONTEmulation& econtEmul)
{
  TList *list = (TList *)dir_diff->GetList();
  uint32_t zside = 0, sector = 0, link = 0, det = 0;
  uint32_t econt = 0, selTC4 = 1, module = 0;
  
  for(uint64_t ievt = 0 ; ievt < eventList.size(); ievt++ ){
    uint64_t event = eventList[ievt] ;
    for(int ilink=0;ilink<2;ilink++){
      for(int iecond=0;iecond<3;iecond++){
	uint32_t moduleId = pck.packModId(zside, sector, ilink, det, iecond, selTC4, module); //we assume same ECONT and ECOND number for a given module
	
	uint32_t emul_bx_4b ;
	std::map<uint32_t,TPGFEDataformat::HalfHgcrocData> rocdata;
	std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>> hrocvec =  hrocarray[event];
	for(const auto& data : hrocvec){
	  rocdata[data.first] = data.second ;
	  if(data.first==moduleId) emul_bx_4b = (rocdata[data.first].getBx()==3564) ? 0xF : (rocdata[data.first].getBx()%8);
	}
	
	std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;
	std::map<uint32_t,TPGFEDataformat::ModuleTcData> moddata;
	std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::ModuleTcData>>> modarray; //event,moduleId
	//================================================
	//HGCROC emulation for a given module
	//================================================
	rocTPGEmul.Emulate(false, event, moduleId, rocdata, modTcdata);
	//================================================
	modarray[event].push_back(modTcdata);	  
	for(const auto& data : modarray.at(event)) if(data.first==moduleId) moddata[data.first] = data.second ;
	//================================================
	//ECONT emulation for a given module
	//================================================
	econtEmul.Emulate(false, event, moduleId, moddata);
	TPGFEDataformat::TcModulePacket& TcRawdata = econtEmul.accessTcRawDataPacket();
	//================================================
	if(TcRawdata.second.type()==TPGFEDataformat::BestC) TcRawdata.second.sortCh();
	TcRawdata.second.setBX(emul_bx_4b);
	//uint32_t tc_energy_emul = TcRawdata.second.getTc(itc).energy();
	
	std::vector<std::pair<uint32_t,TPGBEDataformat::Trig24Data>> econtdata =  econtarray[event];
	TPGFEDataformat::TcRawDataPacket vTCel;	
	TPGBEDataformat::Trig24Data trdata;
	uint32_t econt_bx;
	uint32_t nofmatches = 0;
	for(const auto& econtit : econtdata){
	  if(econtit.first!=moduleId) continue;
	  trdata = econtit.second ;
	  
	  for(int ibx=0;ibx<7;ibx++){
	    const uint32_t *el = trdata.getElinks(ibx); 
	    uint32_t bx_2 = (el[0]>>28) & 0xF;
	    if(ilink==0) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::BestC, econTPar[moduleId].getNofTCs(), el, vTCel);
	    if(ilink==1) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC16, econTPar[moduleId].getNofSTCs(), el, vTCel);
	    if(ilink==2 or ilink==3) TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC4A, econTPar[moduleId].getNofSTCs(), el, vTCel);
	    if(vTCel==TcRawdata.second){
	      // std::cout << "Found a match: event:"<< event << ", ModuleId:" << moduleId << ", ibx: " << ibx << ", bx_2: " << bx_2 <<", emul_bx_4b: " << emul_bx_4b << std::endl;
	      // TcRawdata.second.print();
	      // vTCel.print();
	      // std::cout << "=============" << std::endl;
	      nofmatches++;
	      econt_bx = bx_2;
	    }
	  }//bx loop	  
	}//loop over econt data for a given run
	if(nofmatches==1) ((TH2D *) list->FindObject(Form("hBxCorr_%d",ilink)))->Fill( econt_bx, emul_bx_4b );	
      }//econd loop
    }////ilink loop
  }//event loop

  
  
  return true;
} 

