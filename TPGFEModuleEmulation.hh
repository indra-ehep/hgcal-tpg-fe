#ifndef TPGFEModuleEmulation_h
#define TPGFEModuleEmulation_h

#include <iostream>
#include <cstring>
#include <vector>
#include <cassert>
#include <cstddef>

#include "TPGFEConfiguration.hh"

namespace TPGFEModuleEmulation{
  
  //The following emulation work per event per module
  class HGCROCTPGEmulation{
  public:
    HGCROCTPGEmulation(TPGFEConfiguration::Configuration& cfgs) : configs(cfgs) {}
    //The following emulation function performs 1) pedestal subtraction, 2) linearization, 3) compression
    void Emulate(bool isSim, uint64_t ievent, uint32_t& moduleId, const std::map<uint32_t,TPGFEDataformat::HalfHgcrocData>&, std::pair<uint32_t,TPGFEDataformat::ModuleTcData>&, uint64_t);

    uint16_t CompressHgroc(uint32_t val, bool isldm){
      
      //////The configuration of half of ROC based on HGCROC3a [doc. no. v2.0] (Read subsection 1.3.3 of Page-39)
      //////EDMS ROCv3a: https://edms.cern.ch/ui/#!master/navigator/document?D:100570166:100570166:subDocs
      //4E+3M
      //Ref:https://graphics.stanford.edu/%7Eseander/bithacks.html
      
      uint32_t maxval = 0x3FFFF ;      //18b
      uint32_t maxval_ldm = 0x7FFFF ;  //19b
      uint32_t maxval_hdm = 0x1FFFFF ; //21b
      
      if(isldm){
	if(val>maxval_ldm) val = maxval_ldm;
	if(val>maxval and val<=maxval_ldm) val = val>>1;
      }else{
	if(val>maxval_hdm) val = maxval_hdm;
	if(val>0xFFFFF and val<=maxval_ldm) val = val>>3;	
	if(val>0x7FFFF and val<=0xFFFFF) val = val>>2;
	if(val>maxval and val<=0x7FFFF) val = val>>1;
      }
      
      
      uint32_t r = 0; // r will be lg(v)
      uint32_t sub ;
      uint32_t shift ;
      uint32_t mant ;
      
      if(val>7){
	uint32_t v = val; 
	r = 0; 
	while (v >>= 1) r++;
	sub = r - 2;
	shift = r - 3;
	if(sub<=0xF){
	  mant = (val>>shift) & 0x7;
	}else{
	  sub = 0xF;
	  mant = 0x7;
	}
      }else{
	r = 0;
	sub = 0;
	shift = 0;
	mant = val & 0x7;
      }
  
      uint16_t cdata = (sub<<3) | mant;
  
      return cdata;
    }
    
  private:    
    TPGFEConfiguration::Configuration& configs;
    TPGFEConfiguration::TPGFEIdPacking pck;
  };
  
  void HGCROCTPGEmulation::Emulate(bool isSim, uint64_t ievent, uint32_t& moduleId, const std::map<uint32_t,TPGFEDataformat::HalfHgcrocData>& rocdata, std::pair<uint32_t,TPGFEDataformat::ModuleTcData>& moddata, uint64_t refEvent = 0xFFFFFFFFFFFFFFFF){

    pck.setModId(moduleId);    
    const std::map<std::tuple<uint32_t,uint32_t,uint32_t>,std::string>& modNameMap = configs.getModIdxToName();
    const std::string& modName = modNameMap.at(std::make_tuple(pck.getDetType(),pck.getSelTC4(),pck.getModule()));
    const std::map<std::string,std::vector<uint32_t>>& modTClist = (pck.getDetType()==0)?configs.getSiModTClist():configs.getSciModTClist();
    const std::vector<uint32_t>& tclist = modTClist.at(modName) ;
    const std::map<std::pair<std::string,uint32_t>,std::vector<uint32_t>>& tcPinMap = (pck.getDetType()==0)?configs.getSiTCToROCpin():configs.getSciTCToROCpin();

    uint16_t bx = 0xFFFF;
    const uint32_t nTCs = tclist.size();
    TPGFEDataformat::ModuleTcData mdata;
    mdata.setNofTCs(nTCs);
    TPGFEDataformat::HgcrocTcData hrtcdata[nTCs];
    
    for(const auto& itc : tclist){
      const std::vector<uint32_t>& pinlist = tcPinMap.at(std::make_pair(modName,itc)) ;
      uint32_t totadc = 0;
      bool isTot = false;
      for(const auto& tcch : pinlist){
	uint32_t rocpin = tcch%36 ;
	uint32_t rocn = TMath::Floor(tcch/72);
	uint32_t half = (int(TMath::Floor(tcch/36))%2==0)?0:1;
	uint32_t rocid = pck.getRocIdFromModId(moduleId,rocn,half);
	if(rocdata.find(rocid)==rocdata.end()){
	  std::cerr << "HalfRoc not found in data for Event "<< ievent <<", Tcch: "<< tcch << ", rocn: " << rocn << ", half: " << half << ", rocpin: " << rocpin << std::endl;
	  continue ; 
	}
	if(ievent==refEvent) std::cout<<"TC : " << itc << ", tcch: " << tcch <<", rocpin : "<<rocpin<<", rocid: "<<rocid<<", rocn: "<<rocn<<", half: "<<half<<std::endl;
	const TPGFEDataformat::HalfHgcrocChannelData& chdata = rocdata.at(rocid).getChannelData(rocpin);
	bx = rocdata.at(rocid).getBx();
	if(ievent==refEvent) chdata.print();
	if(!isSim){ //if not simulation, assumed beamtest data
	  const TPGFEConfiguration::ConfigHfROC& rocpara = configs.getRocPara().at(rocid);
	  uint32_t ped = configs.getChPara().at(pck.packChId(rocid,rocpin)).getAdcpedestal();
	  if(ievent==refEvent) rocpara.print();
	  if(!chdata.isTot()){
	    unsigned thr = rocpara.getAdcTH();
	    uint32_t adc = chdata.getAdc();
	    adc = (adc>(ped+thr) and !(rocpara.isChMasked(rocpin)) and (ped<0xFF)) ? adc-ped : 0 ;
	    totadc += adc;
	    if(ievent==refEvent) std::cout<<"\t ped: " << ped << ", thr: " << thr <<", adc : "<<adc<<", rocpara.isChMasked(rocpin): "<< rocpara.isChMasked(rocpin) <<std::endl;
	  }else{
	    uint32_t tot1 = (chdata.getTot()>rocpara.getTotTH(rocpin)) ? (chdata.getTot()-rocpara.getTotP(rocpin)) : (rocpara.getTotTH(rocpin)-rocpara.getTotP(rocpin)) ;
	    //ideally ped<255 condition should only be restricted for ADC case, however TOT signal triggers in emulation for event 146245 of relay 1695829026 and link 1, which is not that seen by ECONT data
	    uint32_t totlin = (!(rocpara.isChMasked(rocpin)) and (ped<0xFF))?tot1*rocpara.getMultFactor():0; 
	    totadc += totlin;
	    isTot = true;
	    if(ievent==refEvent) std::cout<<"\t tot1: " << tot1 << ", tot: " << chdata.getTot() <<", thr : "<< rocpara.getTotTH(rocpin) <<", ped: "<< rocpara.getTotP(rocpin) << ", totlin : "<< totlin <<std::endl;
	  }//istot or adc
	}else{
	  //check CMSSW for details should be 
	  if(!chdata.isTot())
	    totadc += chdata.getAdc(); 
	  else
	    totadc += chdata.getTot(); 
	}//isCMSSW simulation of beam-test analysis
      }//loop of sensor channels
      hrtcdata[itc].setCharge(totadc);
      hrtcdata[itc].setTot(isTot);
      if(!isSim){
	pck.setModId(moduleId);    
	//hrtcdata[itc].setCdata(CompressHgroc(totadc, configs.getEconTPara().at(moduleId).getDensity()));
	hrtcdata[itc].setCdata(CompressHgroc(totadc, pck.getSelTC4()));
      }else
	hrtcdata[itc].setCdata(CompressHgroc(totadc,1));
    }//loop over TCs
    mdata.setTCs(hrtcdata);
    mdata.setBx(bx);
    moddata = std::make_pair(moduleId,mdata);
  }

  class ECONTEmulation{
  public:
    ECONTEmulation(TPGFEConfiguration::Configuration& cfgs) : configs(cfgs) {}
    //~ECONTEmulation() {tcrawlist.clear();} Not needed
    ~ECONTEmulation() {}
    
    //The following emulation function performs 1) decompression, 2) calibration, 3) compression
    void Emulate(bool isSim, uint64_t ievent, uint32_t& moduleId, const std::map<uint32_t,TPGFEDataformat::ModuleTcData>&, TPGFEDataformat::TcModulePacket& );
    void EmulateSTC(bool isSim, uint64_t ievent, uint32_t& moduleId, const std::map<uint32_t,TPGFEDataformat::ModuleTcData>&);
    void EmulateBC(bool isSim, uint64_t ievent, uint32_t& moduleId, const std::map<uint32_t,TPGFEDataformat::ModuleTcData>&);
    
    void Emulate(bool isSim, uint64_t ievent, uint32_t& moduleId, const std::map<uint32_t,TPGFEDataformat::ModuleTcData>&);
    const TPGFEDataformat::TcModulePacket& getTcRawDataPacket() const {return emulOut;}
    TPGFEDataformat::TcModulePacket& accessTcRawDataPacket() {return emulOut;}
    
    // BX here may be absorbed into TcRawDataPacket?
    static void convertToElinkData(unsigned bx, const TPGFEDataformat::TcRawDataPacket &tcrdp, uint32_t *ve);
    void getElinkData(unsigned bx, uint32_t *ptr) const;

    void fillRandomTcRawData(unsigned bx);
    void fillZeroEnergyTcRawData(unsigned bx);
    
    static void generateTcRawData(bool zero, unsigned bx,
				  TPGFEDataformat::Type type,
				  unsigned nTc,
				  TPGFEDataformat::TcRawDataPacket &vtcrp);
    static void generateRandomTcRawData(unsigned bx,
					TPGFEDataformat::Type type,
					unsigned nTc,
					TPGFEDataformat::TcRawDataPacket &vtcrp);
    static void generateZeroEnergyTcRawData(unsigned bx,
					    TPGFEDataformat::Type type,
					    unsigned nTc,
					    TPGFEDataformat::TcRawDataPacket &vtcrp);
    
  private:
    uint32_t DecompressEcont(uint16_t compressed, bool isldm){
      //4E+3M with midpoint correction
      uint32_t mant = compressed & 0x7;
      uint32_t expo = (compressed>>3) & 0xF;
      
      if(expo==0) 
	return (isldm) ? (mant<<1) : (mant<<3) ;
      
      uint32_t shift = expo+3;
      uint32_t decomp = 1<<shift; //Should this shift be controlled by the density parameter of econt config  ?
      uint32_t mpdeco = 1<<(shift-4);
      decomp = decomp | (mant<<(shift-3));
      decomp = decomp | mpdeco;
      decomp = (isldm) ? (decomp<<1) : (decomp<<3) ;
    
      return decomp;
    }

    uint32_t DecompressEcontSTC(uint16_t compressed, bool isldm){
      //4E+3M with midpoint correction
      uint32_t mant = compressed & 0x7;
      uint32_t expo = (compressed>>3) & 0xF;
      
      if(expo==0) 
	return (isldm) ? (mant<<1)+1 : (mant<<3)+4 ;
    
      uint32_t shift = expo+4;
      uint32_t decomp = 1<<shift; //Should this shift be controlled by the density parameter of econt config  ?
      uint32_t mpdeco = 1<<(shift-4);
      decomp = decomp | (mant<<(shift-3));
      decomp = decomp | mpdeco;
      decomp = (isldm) ? (decomp<<0) : (decomp<<2) ;
      
      return decomp;
    }
    
    uint16_t CompressEcontStc4E3M(uint32_t val, bool isldm){
      //4E+3M
      //The following is probably driven by drop_LSB setting, to be tested with a standalone run as no LSB drop is expected for STC
      val = (isldm)?val>>1:val>>3;  
      
      uint32_t r = 0; // r will be lg(v)
      uint32_t sub ;
      uint32_t shift ;
      uint32_t mant ;
      
      if(val>7){
	uint32_t v = val; 
	r = 0; 
	while (v >>= 1) r++;
	sub = r - 2;
	shift = r - 3;
	if(sub<=0xF){
	  mant = (val>>shift) & 0x7;
	}else{
	  sub = 0xF;
	  mant = 0x7;
	}
      }else{
	r = 0;
	sub = 0;
	shift = 0;
	mant = val & 0x7;
      }
      
      sub = sub & 0xF;
      mant = mant & 0x7;

      uint16_t packed = (sub<<3) | mant;
      
      return packed;
    }
    
    uint16_t CompressEcontStc5E4M(uint32_t val, bool isldm){
      //5E+4M
      //The following is probably driven by drop_LSB setting, to be tested with a standalone run as no LSB drop is expected for STC
      val = (isldm)?val>>1:val>>3;  

      uint32_t r = 0; // r will be lg(v)
      uint32_t sub ;
      uint32_t shift ;
      uint32_t mant ;
      
      if(val>0xF){
	uint32_t v = val; 
	r = 0; 
	while (v >>= 1) r++;
	sub = r - 2;         //// Do we reduce the index by 2 or not ?? For the moment keep it untill tested with any technial run
	shift = r - 4;       //// 4 for 4M 
	if(sub<=0x1F){
	  mant = (val>>shift) & 0xF;
	}else{
	  sub = 0x1F;
	  mant = 0xF;
	}
      }else{
	r = 0;
	sub = 0;
	shift = 0;
	mant = val & 0xF;
      }

      sub = sub & 0x1F;
      mant = mant & 0xF;

      uint16_t packed = (sub<<4) | mant;
      
      return packed;
    }

    uint16_t CompressEcontBc(uint32_t val, bool isldm){
      //4E+3M
      val = (isldm)?val>>1:val>>3;  
      if(val>0x3FFFFF) val = 0x3FFFFF; // maximum energy value is 0x3FFFFF for 22 bit register

      uint32_t r = 0; // r will be lg(v)
      uint32_t sub ;
      uint32_t shift ;
      uint32_t mant ;
  
      if(val>7){
	uint32_t v = val; 
	r = 0; 
	while (v >>= 1) r++;
	sub = r - 2;
	shift = r - 3;
	if(sub<=0xF){
	  mant = (val>>shift) & 0x7;
	}else{
	  sub = 0xF;
	  mant = 0x7;
	}
      }else{
	r = 0;
	sub = 0;
	shift = 0;
	mant = val & 0x7;
      }
      
      sub = sub & 0xF;
      mant = mant & 0x7;

      uint16_t packed = (sub<<3) | mant;
      
      return packed;
    }
    
    uint16_t CompressEcontModsum(uint32_t val, bool isldm){
      //5E+3M
      val = (isldm)?val>>1:val>>3;  
  
      uint32_t r = 0; // r will be lg(v)
      uint32_t sub ;
      uint32_t shift ;
      uint32_t mant ;
  
      if(val>7){
	uint32_t v = val; 
	r = 0; 
	while (v >>= 1) r++;
	sub = r - 2;
	shift = r - 3;
	if(sub<=0x1F){
	  mant = (val>>shift) & 0x7;
	}else{
	  sub = 0x1F;
	  mant = 0x7;
	}
      }else{
	r = 0;
	sub = 0;
	shift = 0;
	mant = val & 0x7;
      }

      sub = sub & 0x1F;
      mant = mant & 0x7;

      uint16_t packed = (sub<<3) | mant;
 
      return packed;
    }
    
    //C++ adaptation of batcher odd-even sorting of https://github.com/dnoonan08/ECONT_Emulator/ASICBlocks/bestchoice.py
    void batcherOEMSort(std::vector<TPGFEDataformat::TcRawData>& tc) {
      //Fixme: Requires 49 Tcs
      TPGFEDataformat::TcRawData dummy(TPGFEDataformat::Unknown,0x3f, 0);
      tc.push_back(dummy);
      
      uint32_t N = uint32_t(tc.size());
      uint32_t t = uint32_t(ceil(log(N)/log(2)));
      uint32_t p = uint32_t(pow(2,(t-1)));
      while(p>=1){
	uint32_t q = uint32_t(pow(2,(t-1)));
	uint32_t r = 0;
	uint32_t d = p;
	while (q>=p){
	  for(uint32_t i=0; i<(N-d) ; i++){
	    if ((i & p) != r) continue;
	    if (tc[i] < tc[i+d]) std::swap(tc[i], tc[i+d]);
	  }
	  d = q - p;
	  q = floor(q/2);
	  r = p;
	}
	p = floor(p/2);
      }
    }

    uint32_t findLastMax(std::vector<TPGFEDataformat::TcRawData>& tc) {
      for(uint32_t itc = 0 ; itc< uint32_t(tc.size()-1); itc++) if(tc[itc+1] < tc[0]) return itc;
      return uint32_t(tc.size()-1);
    }

    TPGFEConfiguration::Configuration& configs;
    TPGFEConfiguration::TPGFEIdPacking pck;
    TPGFEDataformat::TcModulePacket emulOut;
  };

  void ECONTEmulation::Emulate(bool isSim, uint64_t ievent, uint32_t& moduleId, const std::map<uint32_t,TPGFEDataformat::ModuleTcData>& moddata, TPGFEDataformat::TcModulePacket& econtOut){
    Emulate(isSim, ievent, moduleId, moddata);
    econtOut=emulOut;
  }
  
  void ECONTEmulation::Emulate(bool isSim, uint64_t ievent, uint32_t& moduleId, const std::map<uint32_t,TPGFEDataformat::ModuleTcData>& moddata){
    emulOut.first=moduleId;
    emulOut.second.reset();
    
    const TPGFEDataformat::Type& outputType = configs.getEconTPara().at(moduleId).getOutType();
    if(outputType==TPGFEDataformat::BestC)
      EmulateBC(isSim, ievent, moduleId, moddata);
    else if(outputType==TPGFEDataformat::STC4A or outputType==TPGFEDataformat::STC4B or outputType==TPGFEDataformat::STC16 or TPGFEDataformat::CTC4A or TPGFEDataformat::CTC4B)
      EmulateSTC(isSim, ievent, moduleId, moddata);
  }
  
  void ECONTEmulation::EmulateSTC(bool isSim, uint64_t ievent, uint32_t& moduleId, const std::map<uint32_t,TPGFEDataformat::ModuleTcData>& moddata) {
    
    pck.setModId(moduleId);    
    const std::map<std::tuple<uint32_t,uint32_t,uint32_t>,std::string>& modNameMap = configs.getModIdxToName();
    const std::map<uint32_t,uint32_t>& refMuxMap = configs.getMuxMapping() ;
    const std::string& modName = modNameMap.at(std::make_tuple(pck.getDetType(),pck.getSelTC4(),pck.getModule()));
    
    const TPGFEDataformat::Type& outputType = configs.getEconTPara().at(moduleId).getOutType();
   
    if(outputType==TPGFEDataformat::STC4A or outputType==TPGFEDataformat::STC4B or outputType==TPGFEDataformat::CTC4A or outputType==TPGFEDataformat::CTC4B){
      
      const std::map<std::string,std::vector<uint32_t>>& modSTClist = (pck.getDetType()==0)?configs.getSiModSTClist():configs.getSciModSTClist();
      const std::vector<uint32_t>& stclist = modSTClist.at(modName) ;
      const std::map<std::pair<std::string,uint32_t>,std::vector<uint32_t>>& stcTcMap = (pck.getDetType()==0)?configs.getSiSTCToTC():configs.getSciSTCToTC();
      uint16_t bx = 0xffff;
      emulOut.second.reset();
      for(const auto& istc : stclist){
	const std::vector<uint32_t>& tclist = stcTcMap.at(std::make_pair(modName,istc));
	const TPGFEDataformat::ModuleTcData& mdata = moddata.at(moduleId);
	bx = (mdata.getBx()==3564) ? 0xF : mdata.getBx() & 0x7 ; 
	uint32_t decompressedSTC = 0;
	std::vector<TPGFEDataformat::TcRawData> tcrawdatalist;
	TPGFEDataformat::TcRawData tcdata;
	for(const auto& econtc : tclist){
	  uint32_t hgctc = configs.getEconTPara().at(moduleId).getInputMux(econtc) ;
	  bool hasFound = false;
	  uint32_t emultc = 0xffffffff;
	  for (const auto& it : refMuxMap){
	    if (it.second == hgctc)  {
	      hasFound = true;
	      emultc = it.first;
	    }
	  }
	  if(!hasFound){
	    std::cerr << "TPGFEModuleEmulation::ECONTEmulation::EmulateSTC (moduleid="<<moduleId<<") : Mux not set for TC " << econtc << std::endl;
	    continue;
	  }
	  uint32_t decompressed = DecompressEcontSTC(mdata.getTC(emultc).getCdata(),pck.getSelTC4());
	  decompressed *= configs.getEconTPara().at(moduleId).getCalibration(econtc) ;
	  decompressed =  decompressed >> 11;
	  decompressedSTC += decompressed ;
	  uint16_t compressed_bc = CompressEcontBc(decompressed,pck.getSelTC4());
	  tcdata.setTriggerCell(TPGFEDataformat::BestC, econtc, compressed_bc) ;
	  tcrawdatalist.push_back(tcdata);
	}
	batcherOEMSort(tcrawdatalist);
	uint32_t lMaxId = findLastMax(tcrawdatalist);
	tcrawdatalist.resize(lMaxId+1);
	std::sort(tcrawdatalist.begin(),tcrawdatalist.end(),TPGFEDataformat::TcRawDataPacket::customGT);
	TPGFEDataformat::TcRawData lastMax = tcrawdatalist[0] ;
	uint16_t compressed_energy = (outputType==TPGFEDataformat::STC4A or outputType==TPGFEDataformat::CTC4A) ? CompressEcontStc4E3M(decompressedSTC,pck.getSelTC4()) : CompressEcontStc5E4M(decompressedSTC,pck.getSelTC4());
	emulOut.second.setTBM(outputType, bx, 0); 
	if(outputType==TPGFEDataformat::STC4A or outputType==TPGFEDataformat::STC4B)
	  emulOut.second.setTcData(outputType, lastMax.address()%4, compressed_energy);
	else
	  emulOut.second.setTcData(outputType, istc, compressed_energy);
	tcrawdatalist.clear();
      }//stc loop
      
    }
    
    if(outputType==TPGFEDataformat::STC16){
      
      const std::map<std::string,std::vector<uint32_t>>& modSTC16list = (pck.getDetType()==0)?configs.getSiModSTC16list():configs.getSciModSTC16list();
      const std::vector<uint32_t>& stc16list = modSTC16list.at(modName) ;
      const std::map<std::pair<std::string,uint32_t>,std::vector<uint32_t>>& stc16TcMap = (pck.getDetType()==0)?configs.getSiSTC16ToTC():configs.getSciSTC16ToTC();
      uint16_t bx = 0xffff;
      emulOut.second.reset();
      for(const auto& istc16 : stc16list){
	const std::vector<uint32_t>& tclist = stc16TcMap.at(std::make_pair(modName,istc16));
	const TPGFEDataformat::ModuleTcData& mdata = moddata.at(moduleId);
	bx = (mdata.getBx()==3564) ? 0xF : mdata.getBx() & 0x7 ; 
	uint32_t decompressedSTC16 = 0;
	std::vector<TPGFEDataformat::TcRawData> tcrawdatalist;
	TPGFEDataformat::TcRawData tcdata;
	for(const auto& econtc : tclist){
	  uint32_t hgctc = configs.getEconTPara().at(moduleId).getInputMux(econtc) ;
	  bool hasFound = false;
	  uint32_t emultc = 0xffffffff;
	  for (const auto& it : refMuxMap){
	    if (it.second == hgctc)  {
	      hasFound = true;
	      emultc = it.first;
	    }
	  }
	  if(!hasFound){
	    std::cerr << "TPGFEModuleEmulation::ECONTEmulation::EmulateBC (moduleid="<<moduleId<<") : Mux not set for TC " << econtc << std::endl;
	    continue;
	  }
	  uint32_t decompressed = DecompressEcont(mdata.getTC(emultc).getCdata(),pck.getSelTC4());
	  decompressed *= configs.getEconTPara().at(moduleId).getCalibration(econtc) ;
	  decompressed =  decompressed >> 11;
	  decompressedSTC16 += decompressed ;
	  uint16_t compressed_bc = CompressEcontBc(decompressed,pck.getSelTC4());
	  tcdata.setTriggerCell(TPGFEDataformat::BestC, econtc, compressed_bc) ;
	  tcrawdatalist.push_back(tcdata);
	}
	batcherOEMSort(tcrawdatalist);
	uint32_t lMaxId = findLastMax(tcrawdatalist);
	tcrawdatalist.resize(lMaxId+1);
	std::sort(tcrawdatalist.begin(),tcrawdatalist.end(),TPGFEDataformat::TcRawDataPacket::customGT);
	TPGFEDataformat::TcRawData lastMax = tcrawdatalist[0] ;
	uint16_t compressed_energy = CompressEcontStc5E4M(decompressedSTC16,pck.getSelTC4());
	emulOut.second.setTBM(outputType, bx, 0); 
	emulOut.second.setTcData(outputType, (lastMax.address())%16, compressed_energy);
	tcrawdatalist.clear();
      }//stc16 loop
    }//STC16 select condition
    
  }//Emulate STC
  
  void ECONTEmulation::EmulateBC(bool isSim, uint64_t ievent, uint32_t& moduleId, const std::map<uint32_t,TPGFEDataformat::ModuleTcData>& moddata) {
    pck.setModId(moduleId);    
    const std::map<std::tuple<uint32_t,uint32_t,uint32_t>,std::string>& modNameMap = configs.getModIdxToName();
    const std::map<uint32_t,uint32_t>& refMuxMap = configs.getMuxMapping() ;
    const std::string& modName = modNameMap.at(std::make_tuple(pck.getDetType(),pck.getSelTC4(),pck.getModule()));
    const std::map<std::string,std::vector<uint32_t>>& modTClist = (pck.getDetType()==0)?configs.getSiModTClist():configs.getSciModTClist();
    const std::vector<uint32_t>& tclist = modTClist.at(modName) ;
    
    const TPGFEDataformat::Type& outputType = configs.getEconTPara().at(moduleId).getOutType();
    const TPGFEDataformat::ModuleTcData& mdata = moddata.at(moduleId);

    uint16_t bx = (mdata.getBx()==3564) ? 0xF : mdata.getBx() & 0x7 ; //make 8 modulo
    uint32_t decompressedMS = 0;
    std::vector<TPGFEDataformat::TcRawData> tcrawdatalist;
    TPGFEDataformat::TcRawData tcdata;
    for(const auto& econtc : tclist){
      uint32_t hgctc = configs.getEconTPara().at(moduleId).getInputMux(econtc) ;
      bool hasFound = false;
      uint32_t emultc = 0xffff;
      for (const auto& it : refMuxMap){
	if (it.second == hgctc)  {
	  hasFound = true;
	  emultc = it.first;
	}
      }
      if(!hasFound){
	std::cerr << "TPGFEModuleEmulation::ECONTEmulation::EmulateBC (moduleid="<<moduleId<<") : Mux not set for TC " << econtc << std::endl;
	continue;
      }
      uint32_t decompressed = DecompressEcont(mdata.getTC(emultc).getCdata(),pck.getSelTC4());
      decompressed *= configs.getEconTPara().at(moduleId).getCalibration(econtc) ;
      decompressed =  decompressed >> 11;
      decompressedMS += decompressed ;
      uint16_t compressed_bc = CompressEcontBc(decompressed,pck.getSelTC4());
      tcdata.setTriggerCell(outputType, econtc, compressed_bc) ;
      tcrawdatalist.push_back(tcdata);
    }
    batcherOEMSort(tcrawdatalist);
    
    uint16_t compressed_modsum = CompressEcontModsum(decompressedMS,pck.getSelTC4());
    emulOut.second.reset();
    emulOut.second.setTBM(outputType, bx, compressed_modsum); 
    uint32_t nofBCTcs = configs.getEconTPara().at(moduleId).getBCType();
    for(uint32_t itc = 0 ; itc<nofBCTcs ; itc++)
      emulOut.second.setTcData(outputType, tcrawdatalist[itc].address(), tcrawdatalist[itc].energy());
    
  }//Emulate BC
  
  void ECONTEmulation::convertToElinkData(unsigned bx, const TPGFEDataformat::TcRawDataPacket &tcrdp, uint32_t *ve) {
    
    const std::vector<TPGFEDataformat::TcRawData>& vtc = tcrdp.getTcData();
    
    unsigned nVe(0);
    
    assert(vtc.size()>0);
    
    uint64_t data(uint64_t(bx)<<60);
    unsigned last(60);
    unsigned w(0);
    
    TPGFEDataformat::Type tcType(tcrdp.type());
    bool bcMap(false);
    uint64_t bcBits(0);
    
    if(tcType==TPGFEDataformat::BestC) {
      //assert(vtc[0].isModuleSum());
      data|=(uint64_t(tcrdp.moduleSum())<<52);
      last-=8;
      w++;
      
      bcMap=(vtc.size()>7); //High occupancy if nTC>=8
      
      ////////////// construct map for high occupancy ///////////////
      if(bcMap) {
        for(unsigned i(0);i<vtc.size();i++) {
          data|=uint64_t(1)<<(51-vtc[i].address());
        }
        last-=48;
      }
      ////////////////////////////////////////////
    }
    
    if(!bcMap) {
      for(w = 0;w<vtc.size() && !bcMap;w++) {
        if(last<32) {
          ve[nVe++]=data>>32;
          data=(data<<32);
          last+=32;
        }
        
        if     (tcType==TPGFEDataformat::BestC) last-=6;
        else if(tcType==TPGFEDataformat::STC4A) last-=2;
        else if(tcType==TPGFEDataformat::STC4B) last-=2;
        else if(tcType==TPGFEDataformat::CTC4A) last-=0;
        else if(tcType==TPGFEDataformat::CTC4B) last-=0;
        else if(tcType==TPGFEDataformat::STC16) last-=4;
        else assert(false);

	if(tcType!=TPGFEDataformat::CTC4A and tcType!=TPGFEDataformat::CTC4B)
	  data|=(uint64_t(vtc[w].address())<<last);
      }
    }
    
    //unsigned wLo(tcType==TPGFEDataformat::BestC?1:0);
    for(unsigned w(0);w<vtc.size();w++) {
      if(last<32) {
        ve[nVe++]=data>>32;
        data=(data<<32);
        last+=32;
      }

      if     (tcType==TPGFEDataformat::BestC) last-=7;
      else if(tcType==TPGFEDataformat::STC4A) last-=7;
      else if(tcType==TPGFEDataformat::STC4B) last-=9;
      else if(tcType==TPGFEDataformat::CTC4A) last-=7;
      else if(tcType==TPGFEDataformat::CTC4B) last-=9;
      else if(tcType==TPGFEDataformat::STC16) last-=9;
      else assert(false);

      data|=(uint64_t(vtc[w].energy())<<last);
    }
    
    // Catch last words
    if(last<32) {
      ve[nVe++]=data>>32;
      data=(data<<32);
      last+=32;
    }
    
    ve[nVe++]=data>>32;
  }
  
  void ECONTEmulation::getElinkData(unsigned bx, uint32_t *ptr) const {
    convertToElinkData(bx,emulOut.second,ptr);
  }

  void ECONTEmulation::fillRandomTcRawData(unsigned bx) {
    // FIXME - HOW TO GET TYPE AND NUMBER OF TCS?
    //generateRandomTcRawData(bx,configs.getEconTPara().at(moduleId).getOutType(),9,emulOut);
  }

  void ECONTEmulation::fillZeroEnergyTcRawData(unsigned bx) {
    // FIXME - HOW TO GET TYPE AND NUMBER OF TCS?
    //generateZeroEnergyTcRawData(bx,configs.getEconTPara().at(moduleId).getOutType(),9,emulOut);
  }

  void ECONTEmulation::generateRandomTcRawData(unsigned bx,
					       TPGFEDataformat::Type type,
					       unsigned nTc,
					       TPGFEDataformat::TcRawDataPacket &vtcrp) {
    generateTcRawData(false,bx,type,nTc,vtcrp);
  }
  
  void ECONTEmulation::generateZeroEnergyTcRawData(unsigned bx,
						   TPGFEDataformat::Type type,
						   unsigned nTc,
						   TPGFEDataformat::TcRawDataPacket &vtcrp) {
    generateTcRawData(true,bx,type,nTc,vtcrp);
  }
  
  void ECONTEmulation::generateTcRawData(bool zero, unsigned bx,
					 TPGFEDataformat::Type type,
					 unsigned nTc,
					 TPGFEDataformat::TcRawDataPacket &vtcrp) {
    
    //std::vector<TPGFEDataformat::TcRawData> &vtc(vtcrp.second);
    std::vector<TPGFEDataformat::TcRawData> &vtc(vtcrp.setTcData());
  
    if(type==TPGFEDataformat::BestC) {
      // vtc.resize(nTc+1);
      // vtc[0].setModuleSum(zero?0:rand()&0xff);
    
      unsigned step(48/nTc);
      //for(unsigned i(1);i<vtc.size();i++) {
      for(unsigned i(0);i<vtc.size();i++) {
	vtc[i].setTriggerCell(type,step*(i-1)+(rand()%step),zero?0:rand()&0x7f);
      }
    
    } else if(type==TPGFEDataformat::STC4A) {
      vtc.resize(nTc);
      for(unsigned i(0);i<vtc.size();i++) {
	vtc[i].setTriggerCell(type,rand()&0x3,zero?0:rand()&0x7f);
      }
    
    } else if(type==TPGFEDataformat::STC4B) {
      vtc.resize(nTc);
      for(unsigned i(0);i<vtc.size();i++) {
	vtc[i].setTriggerCell(type,rand()&0x3,zero?0:rand()&0x1ff);
      }
    
    } else if(type==TPGFEDataformat::STC16) {
      vtc.resize(nTc);
      for(unsigned i(0);i<vtc.size();i++) {
	vtc[i].setTriggerCell(type,rand()&0xf,zero?0:rand()&0x1ff);
      }
    
    } else {
      assert(false);
    }
  }

  
}//end of namespace

#endif
