#ifndef TPGFEDataformat_h
#define TPGFEDataformat_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <vector>

namespace TPGFEDataformat{

  typedef std::array<uint32_t,14> OrderedElinkPacket;
  
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////// Data Formats by Paul Dauncey /////////////////////////////////
  ///provision for tctp is added by Indra (without altering the existing functionalitites)
  
  class HalfHgcrocChannelData {
  public:
    HalfHgcrocChannelData() {
      setZero();
    }

    void setZero() {
      _data=0;
    }
    
    bool isTot() const {
      return _data>=0x8000;
    }

    uint16_t getTcTp() const {
      return (_data>>12)&0x3;
    }

    uint16_t getAdc() const {
      if(isTot()) return 0;
      return _data&0x3ff;
    }

    uint16_t getTot() const {
      if(!isTot()) return 0;
      return _data&0xfff;
    }

    void setAdc(uint16_t a, uint16_t tctp=0x0) {
      assert(a<0x400 and tctp<0x4);
      _data=tctp<<12|a;
    }
    
    void setTot(uint16_t a, uint16_t tctp=0x3) {
      assert(a<0x1000 and tctp<0x4);
      _data=tctp<<12|a|0x8000;
    }

    void print() const {
      std::cout << "HalfHgcrocChannelData(" << this << ")::print()" << std::endl;

      std::cout << std::dec << ::std::setfill(' ') << ", "
		<< "TcTp: " << getTcTp() << ", "
		<< (isTot()?"TOT = ":"ADC = ") << std::setw(4)
		<< (isTot()?getTot():getAdc())
		<< std::endl;

    }

  private:
    uint16_t _data;
  };
  
  class HalfHgcrocData {
  public:
    enum {
      NumberOfChannels=36
    };
  
    HalfHgcrocData() {
      setZero();
    }

    void setZero() {
      std::memset(_data,0,sizeof(HalfHgcrocChannelData)*NumberOfChannels);
    }
  
    const HalfHgcrocChannelData* getChannels() const {
      return _data;
    }

    /////////////// Following are the modification/addition for emulation ///////////////////
    // HalfHgcrocChannelData* setChannels() {
    //   return _data;
    // }
    void setChannels(const HalfHgcrocChannelData* data) {
      for(unsigned i(0);i<=NumberOfChannels;i++)
	_data[i] = data[i];
    }
    const HalfHgcrocChannelData& getChannelData(uint32_t i) const {
      return _data[i];
    }
    ///////////////////////////////////////////////////////////////////////////////////////
  
    void print() const {
      std::cout << "HalfHgcrocData(" << this << ")::print()" << std::endl;

      const uint16_t *p((const uint16_t*)_data);

      for(unsigned i(0);i<NumberOfChannels;i++) {
	std::cout << " Channel " << std::setw(2) << i << " = 0x"
		  << std::hex << ::std::setfill('0')
		  << std::setw(4) << p[i]
		  << std::dec << ::std::setfill(' ') << ", "
		  << "TcTp: " << _data[i].getTcTp() << ", "
		  << (_data[i].isTot()?"TOT = ":"ADC = ") << std::setw(4)
		  << (_data[i].isTot()?_data[i].getTot():_data[i].getAdc())
		  << std::endl;
      }    
    }

  private:
    HalfHgcrocChannelData _data[36];
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////// Data Formats by Paul and Indra ///////////////////////////////////////////////
  enum Type {
      //select : 0 = Threshold Sum (TS), 1 = Super Trigger Cell (STC), 2 = Best Choice (BC), 3 = Repeater, 4 = Autoencoder (AE) Settings of [5,7] revert to the Repeater algorithm
      //stc_type : 0 = STC4B(5E+4M), 1 = STC16(5E+4M), 2 = CTC4A(4E+3M), 3 = STC4A(4E+3M), 4 = CTC4B(5E+3M)
      BestC, // TC energy format = 4E3M
      STC4A, // 4E3M
      STC4B, // 5E4M
      STC16, // 5E4M
      CTC4A, // 4E3M
      CTC4B, // 5E4M
      TS,    // 4E3M, placeholder
      RA,    // 4E3M, Repeater algorithm is intended for testing and debugging of ECONT
      AE,    // blocks of 16b, uses convolutional neural network
      Unknown
  };

  class TcRawData {
  public:
  
    TcRawData() : _data(0) {
    }
    
    TcRawData(TPGFEDataformat::Type t, uint8_t a, uint16_t e) {
      setTriggerCell(t,a,e);
    }
    
    uint8_t address() const {
      return (_data & 0x3f);
    }
    
    uint16_t energy() const {
      return ((_data >> 6 ) & 0x1ff);
    }
    
    void setTriggerCell(TPGFEDataformat::Type t, uint8_t a, uint16_t e) {
      switch(t){
      case TPGFEDataformat::BestC:
	assert(a<=47);
	assert(e<=0x7f);
	break;
      case TPGFEDataformat::STC4A:
	assert(a<=3);
	assert(e<=0x7f);
	break;
      case TPGFEDataformat::STC4B:
	assert(a<=3);
	assert(e<=0x1ff);
	break;
      case TPGFEDataformat::STC16:
	assert(a<=15);
	assert(e<=0x1ff);
	break;
      case TPGFEDataformat::CTC4A:
	assert(e<=0x7f);
	//a = 0;
	break;
      case TPGFEDataformat::CTC4B:
	assert(e<=0x1ff);
	//a = 0;
	break;
      default:
	;
      }
      _data = (e<<6|a);
    }
    
    void print() const {
      std::cout << "TcRawData(" << this << ")::print(): Data = 0x"
		<< std::hex << ::std::setfill('0')
		<< std::setw(4) << _data
		<< std::dec << ::std::setfill(' ')
		<< ", address = " << std::setw(2) << unsigned(address())
		<< ", energy = " << std::setw(3) << energy() << std::endl;
    }
    
  private:
    uint16_t _data;
  };

  static std::string tctypeName[10]={"BestC","STC4A","STC4B","STC16", "CTC4A", "CTC4B", "TS", "RA", "AE", "Unknown"};
  
  class TcRawDataPacket {
  public:    
    TcRawDataPacket() : _t(Unknown), _bx(0), _ms(0) { _tcdata.resize(0);}
    TcRawDataPacket(TPGFEDataformat::Type t, uint8_t bx, uint8_t e) : _t(t), _bx(bx), _ms(e) { _tcdata.resize(0);}
    const std::string& typeName() const { return TPGFEDataformat::tctypeName[type()]; }
    TPGFEDataformat::Type type() const { return _t;}
    uint16_t moduleSum() const { return uint16_t(_ms & 0xff);}
    uint16_t bx() const { return uint16_t(_bx);}
    const std::vector<TPGFEDataformat::TcRawData>& getTcData() const {return _tcdata;}
    bool is4E3M() const { return (_t==BestC or _t==STC4A or _t==CTC4A or _t==TS or _t==RA) ? true : false ; }
    void reset() { _t = TPGFEDataformat::Type::Unknown; _bx = 0; _ms = 0;  _tcdata.resize(0);}
    
    void setTBM(TPGFEDataformat::Type t, uint8_t bx, uint8_t e) { _t = t; _bx = bx; _ms = e;}
    void setType(TPGFEDataformat::Type t) { _t = t;}
    void setModuleSum(uint8_t e) { _ms = e;}
    void setBX(uint8_t bx) { _bx = bx;}
    std::vector<TPGFEDataformat::TcRawData>& setTcData() {return _tcdata;}
    void setTcData(TPGFEDataformat::Type t, uint8_t a, uint16_t e) {
      TPGFEDataformat::TcRawData tc;
      tc.setTriggerCell(t, a, e);
      _tcdata.push_back(tc);
    }
    
    void print() const {
      std::cout << "TcRawDataPacket(" << this << ")::print(): "
		<< "type = " << typeName()
		<< ", bx = " << bx()
		<< ", ms = " << moduleSum()
		<< std::endl;
      for(uint32_t itc(0); itc < _tcdata.size(); itc++) _tcdata.at(itc).print();
    }    
    
  private:
    TPGFEDataformat::Type _t;
    uint8_t _bx;
    uint8_t _ms;
    std::vector<TPGFEDataformat::TcRawData> _tcdata;
  };


  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //typedef std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>> TcRawDataPacket;
  typedef std::pair<uint32_t,TPGFEDataformat::TcRawDataPacket> TcModulePacket;
  
  
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  class HgcrocTcData {
  public:
    HgcrocTcData() {
      setZero();
    }
  
    void setZero() {
      _data=0;
      _cdata=0;
      _isTot=false;
    }

    bool isTot() const {
      return _isTot;
    }
    
    uint16_t getCdata() const {
      return _cdata;
    }
  
    uint32_t getCharge() const {
      return _data;
    }
    
    void setCdata(uint16_t a) {
      assert(a<0x80); //compressed HGCROC TC data is packed into7bits 
      _cdata=a;
    }

    void setCharge(uint32_t a) {
      assert(a<0x1000000); //assuming 21bit for HD module, then bit shift of 3, then +1
      _data=a;
    }
  
    void setTot(bool a) {
      _isTot=a;
    }
  
  private:
    uint32_t _data;  //raw-uncompressed data
    uint16_t _cdata; //compressed 4E+3M ()
    bool _isTot;     //isTOT or ADC
  };

  class ModuleTcData {
  public:
    enum {
      MaxNumberOfTCs = 48
    };

  
    ModuleTcData() {
      setZero();
    }
  
    void setZero() {
      std::memset(_data,0,sizeof(HgcrocTcData)*MaxNumberOfTCs);
      NumberOfTCs=0;
    }
  
    const uint32_t getNofTCs() const {return uint32_t(NumberOfTCs);}
    const HgcrocTcData* getTCs() const {
      return _data;
    }
    const HgcrocTcData& getTC(uint32_t i) const {
      return _data[i];
    }
  
    void setNofTCs(const unsigned nofTCs) {NumberOfTCs = nofTCs;}
    void setTCs(const HgcrocTcData* data) {
      for(uint16_t i(0);i<=NumberOfTCs;i++)
	_data[i] = data[i];
    }
  
    void print() const {
      std::cout << "ModuleTriggerCellData(" << this << ")::print()" << std::endl;
    
      for(uint16_t i(0);i<NumberOfTCs;i++) {
	std::cout << " TC " << std::setw(2) << i << ": compressed = "
		  << std::dec << ::std::setfill(' ')
		  << std::setw(10) << _data[i].getCdata()
		  << std::dec << ::std::setfill(' ')
		  << ", raw-uncompressed "
		  << std::setw(10) << _data[i].getCharge()
		  << ", istot : "
		  << std::setw(10) << _data[i].isTot()
		  << std::endl;
      }    
    }
  
  private:
    HgcrocTcData _data[48];
    uint16_t NumberOfTCs;
  };


}

#endif
