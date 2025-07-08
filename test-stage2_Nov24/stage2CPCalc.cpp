/**********************************************************************
 Created on : 03/03/2025
 Purpose    : Calculate Stage2 cluster properties 
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
#include <iostream>
#include "L1Trigger/DemonstratorTools/interface/utilities.h"
#include "DataFormats/L1THGCal/interface/HGCalCluster_HW.h"

#include "TPGBEDataformat.hh"
#include "Stage2.hh"

#include "TPGStage2Configuration.hh"
#include "TPGClusterProperties.hh"
#include "TPGLSBScales.hh"

int main(int argc, char** argv)
{
  
  TPGBEDataformat::TcAccumulatorFW accmulInput(3);
  l1thgcfirmware::HGCalCluster_HW L1TOutputEmul;
  l1thgcfirmware::HGCalCluster_HW L1TOutputFW;
  //TPGStage2Emulation::Stage2 s2Clustering;
  TPGClusterProperties s2Clustering;
  
  std::cout <<"size of TPGStage2Emulation::TcAccumulator : " << sizeof(TPGStage2Emulation::TcAccumulator) << std::endl;
  std::cout <<"size of TPGBEDataformat::TcAccumulatorFW : " << sizeof(TPGBEDataformat::TcAccumulatorFW) << std::endl;
  std::cout <<"size of l1thgcfirmware::HGCalCluster_HW : " << sizeof(l1thgcfirmware::HGCalCluster_HW) << std::endl;
  
  //In following order as from https://gitlab.cern.ch/hgcal-tpg/Stage2/-/blob/master/firmware/hdl/types/PkgCluster.vhd#L59-61
  // TYPE tFieldType IS                  ( N_TC , E  , E_EM , E_EM_core , E_H_early , W   , N_TC_W , W2  , Wz  , Wroz , Wphi , Wz2 , Wroz2 , Wphi2 , LayerBits , Sat_TC , ShapeQ );
  // CONSTANT FieldWidth : tSizeArray := ( 10   , 22 , 22   , 22        , 22        , 16  , 10     , 32  , 29  , 28   , 28   , 42  , 40    , 40    , 34        , 1      , 1      );
  // CONSTANT FieldOps   : tCharArray := ( '+' , '+' , '+'  , '+'       , '+'       , '+' , '+'    , '+' , '+' , '+'  , '+'  , '+' , '+'   , '+'   , '|'       , '|'    , '|'    );

  //FW Input (in 64-bit hex)
  //8000000000000f10  0000000000200000  0000000000140000  0000000000080000  0000000000180000  0000000000000100  0000000000000000 0000000000000000  000000000000ffff  0000000000000000  0000000000001234 0000000000005678  0000000000009abc 000000000000def0  000000007fffeff8 0000000000000001  0000000000000001

  //FW Output (in 64-bit hex)
  //04c066a0d5002000  0000ff10080402af 0004858000038f80
  // accmulInput.setNumberOfTcs(0xf10);
  // accmulInput.setTotE(0x200000);
  // accmulInput.setCeeE(0x140000);
  // accmulInput.setCeeECore(0x80000);
  // accmulInput.setCeHEarly(0x180000);
  // accmulInput.setSumW(0x100);
  // accmulInput.setNumberOfTcsW(0);
  // accmulInput.setSumW2(0);
  // accmulInput.setSumWZ(0xffff);
  // accmulInput.setSumWRoZ(0);
  // accmulInput.setSumWPhi(0x1234);
  // accmulInput.setSumWZ2(0x5678);
  // accmulInput.setSumWRoZ2(0x9abc);
  // accmulInput.setSumWPhi2(0xdef0);
  // accmulInput.setLayerBits(0x7fffeff8);
  // accmulInput.setsatTC(1);
  // accmulInput.setshapeQ(1);


  //FW Input
  //N_TC , E      , E_EM  , E_EM_core , E_H_early ,
  //W    , N_TC_W , W2    , Wz        , Wroz ,
  //Wphi , Wz2    , Wroz2 , Wphi2     , LayerBits ,
  //Sat_TC , ShapeQ;
  //00000000000000d3,000000000000e025,000000000000a2c6,00000000000065f4,00000000000025c9
  //0000000000001bbe,00000000000000d3,00000000000b293c,000000000049aca0,00000000008a6c48
  //00000000017911f5,0000000146632bbc,00000002b3487988,0000001409642eab,00000003fff7f7c0,
  //0000000000000000,0000000000000001
  //FW Output
  //012ba0bab028c0e0  000178d3154689e5 4a53912e00038e00 00000003fff7f7c0

  
    
  //N_TC,E,E_EM,E_EM_core,E_H_early,
  ///W,N_TC_W,W2,Wz,Wphi
  //Wroz, Wz2,Wphi2,Wroz2,LayerBits,Sat_TC,ShapeQ
  //00000000000000d3,000000000000e025,000000000000a2c6,00000000000065f4,00000000000025c9,
  //0000000000001bbe,00000000000000d3,00000000000b293c,000000000049aca0,00000000008a6c48,
  //00000000017911f5,0000000146632bbc, 00000002b3487988,0000001409642eab,00000003fff7f7c0
  //0000000000000000,0000000000000001
  
  //k==3, only 28 bit space is available for 
  //muZ saturation W=0x64 (100), Wz=0x63FFF9C (104857500) , muZ = 1048575 (saturation)
  //muZ saturation W=0x64 (100), Wz=0x6400000 (104857600) , muZ = 1048576 (saturation+1)
  //muZ saturation W=0x64 (100), Wz=0x63FFFCD (104857549) , muZ = 1048575.49 (saturation+rounding)
  //muZ saturation W=0x64 (100), Wz=0x6400031 (104857649) , muZ = 1048576 (saturation+1+rounding)
  
  // accmulInput.setNumberOfTcs(0x00000000000000d3);
  // accmulInput.setTotE(0x000000000000e025);
  // accmulInput.setCeeE(0x000000000000a2c6);
  // accmulInput.setCeeECore(0x00000000000065f4);
  // accmulInput.setCeHEarly(0x00000000000025c9);
  
  // accmulInput.setSumW(0x0000000000001bbe);
  // accmulInput.setNumberOfTcsW(0x00000000000000d3);
  // accmulInput.setSumW2(0x00000000000b293c);
  // accmulInput.setSumWZ(0x000000000049aca0);
  // accmulInput.setSumWRoZ(0x00000000017911f5);
  
  // accmulInput.setSumWPhi(0x00000000008a6c48);
  // accmulInput.setSumWZ2(0x0000000146632bbc);
  // accmulInput.setSumWRoZ2(0x0000001409642eab);  
  // accmulInput.setSumWPhi2(0x00000002b3487988);
  // accmulInput.setLayerBits(0x00000003fff7f7c0);
  
  // accmulInput.setsatTC(0x0000000000000000);
  // accmulInput.setshapeQ(0x0000000000000001);

  /////////////////// input 1 //////////////////
  //Frame 0000    8000000000000029  8000000000000003 800000000000000b 8000000000000013 8000000000000007 8000000000000016 8000000000000011 80000000000000c0 800000000000000f 8000000000000456 800000000000000e 8000000000000452 8000000000000005 8000005800091c1f 8000000000000409 8000000000000000 8000000000000000
  accmulInput.setNumberOfTcs(0x0000000000000029);
  accmulInput.setTotE(0x0000000000000003);
  accmulInput.setCeeE(0x000000000000000b);
  accmulInput.setCeeECore(0x0000000000000013);
  accmulInput.setCeHEarly(0x0000000000000007);
  
  accmulInput.setSumW(0x0000000000000016);
  accmulInput.setNumberOfTcsW(0x0000000000000011);
  accmulInput.setSumW2(0x00000000000000c0);
  accmulInput.setSumWZ(0x000000000000000f);
  accmulInput.setSumWRoZ(0x0000000000000456);
  
  accmulInput.setSumWPhi(0x000000000000000e);
  accmulInput.setSumWZ2(0x0000000000000452);
  accmulInput.setSumWRoZ2(0x0000000000000005);  
  accmulInput.setSumWPhi2(0x0000005800091c1f);
  accmulInput.setLayerBits(0x0000000000000409);
  
  accmulInput.setsatTC(0x0000000000000000);
  accmulInput.setshapeQ(0x0000000000000000);
  ///////////////////////////////////////////
  
  // /////////////////// input 2 //////////////////
  // //Frame 0001   8000000000000083 8000000000000022 8000000000000003 8000000000000012 8000000000000016 8000000000000010 8000000000000002 800000000000000f 800000000000003a 8000000000000038 8000000000000097 800000000000062d 8000000000000014 8000000000e30383 80000000000e8002 8000000000000000 8000000000000000
  
  // accmulInput.setNumberOfTcs(0x0000000000000083);
  // accmulInput.setTotE(0x0000000000000022);
  // accmulInput.setCeeE(0x0000000000000003);
  // accmulInput.setCeeECore(0x0000000000000012);
  // accmulInput.setCeHEarly(0x0000000000000016);
  
  // accmulInput.setSumW(0x0000000000000010);
  // accmulInput.setNumberOfTcsW(0x0000000000000002);
  // accmulInput.setSumW2(0x000000000000000f);
  // accmulInput.setSumWZ(0x000000000000003a);
  // accmulInput.setSumWRoZ(0x0000000000000038);
  
  // accmulInput.setSumWPhi(0x0000000000000097);
  // accmulInput.setSumWZ2(0x000000000000062d);
  // accmulInput.setSumWRoZ2(0x0000000000000014);  
  // accmulInput.setSumWPhi2(0x0000000000e30383);
  // accmulInput.setLayerBits(0x00000000000e8002);
  
  // accmulInput.setsatTC(0x0000000000000000);
  // accmulInput.setshapeQ(0x0000000000000000);
  // ///////////////////////////////////////////
  
  // /////////////////// input 3 //////////////////
  // //Frame 0002   800000000000000e 800000000000003a 800000000000002f 8000000000000014 8000000000000032 8000000000000002 8000000000000008 80000000000000cf 80000000000000c8 800000000000f002 8000000000000f04 800000000003e03b 800000000000000b 8000000000000106 8000000000000345 8000000000000000 8000000000000000
  
  // accmulInput.setNumberOfTcs(0x000000000000000e);
  // accmulInput.setTotE(0x000000000000003a);
  // accmulInput.setCeeE(0x000000000000002f);
  // accmulInput.setCeeECore(0x0000000000000014);
  // accmulInput.setCeHEarly(0x0000000000000032);
  
  // accmulInput.setSumW(0x0000000000000002);
  // accmulInput.setNumberOfTcsW(0x0000000000000008);
  // accmulInput.setSumW2(0x00000000000000cf);
  // accmulInput.setSumWZ(0x00000000000000c8);
  // accmulInput.setSumWRoZ(0x000000000000f002);
  
  // accmulInput.setSumWPhi(0x0000000000000f04);
  // accmulInput.setSumWZ2(0x000000000003e03b);
  // accmulInput.setSumWRoZ2(0x000000000000000b);  
  // accmulInput.setSumWPhi2(0x0000000000000106);
  // accmulInput.setLayerBits(0x0000000000000345);
  
  // accmulInput.setsatTC(0x0000000000000000);
  // accmulInput.setshapeQ(0x0000000000000000);
  // ///////////////////////////////////////////

  // /////////////////// input 4 //////////////////
  // //Frame 0003   8000000000000008 8000000000000027 8000000000000127 80000000000000c2 8000000000000026 8000000000000004 8000000000000003 80000000000005f9 8000000000000005 80000000000700ed 8000000000000402 8000000000440604 800000000000019b 8000000000004401 800000000000400c 8000000000000001 8000000000000001
   
  // accmulInput.setNumberOfTcs(0x0000000000000008);
  // accmulInput.setTotE(0x0000000000000027);
  // accmulInput.setCeeE(0x0000000000000127);
  // accmulInput.setCeeECore(0x00000000000000c2);
  // accmulInput.setCeHEarly(0x0000000000000026);
  
  // accmulInput.setSumW(0x0000000000000004);
  // accmulInput.setNumberOfTcsW(0x0000000000000003);
  // accmulInput.setSumW2(0x00000000000005f9);
  // accmulInput.setSumWZ(0x0000000000000005);
  // accmulInput.setSumWRoZ(0x00000000000700ed);
  
  // accmulInput.setSumWPhi(0x0000000000000402);
  // accmulInput.setSumWZ2(0x0000000000440604);
  // accmulInput.setSumWRoZ2(0x000000000000019b);  
  // accmulInput.setSumWPhi2(0x0000000000004401);
  // accmulInput.setLayerBits(0x000000000000400c);
  
  // accmulInput.setsatTC(0x0000000000000001);
  // accmulInput.setshapeQ(0x0000000000000001);
  // ///////////////////////////////////////////

  // /////////////////// input 5 //////////////////
  // //Frame 0004   8000000000000081 8000000000000002 80000000000001ee 8000000000000034 800000000000016f 8000000000000020 800000000000000c 8000000000007005 800000000000000d 8000000000800802 80000000000000fd 800000000002403a 8000000000000285 8000000000000005 80000000000d8007 8000000000000000 8000000000000001
   
  // accmulInput.setNumberOfTcs(0x0000000000000081);
  // accmulInput.setTotE(0x0000000000000002);
  // accmulInput.setCeeE(0x00000000000001ee);
  // accmulInput.setCeeECore(0x0000000000000034);
  // accmulInput.setCeHEarly(0x000000000000016f);
  
  // accmulInput.setSumW(0x0000000000000020);
  // accmulInput.setNumberOfTcsW(0x000000000000000c);
  // accmulInput.setSumW2(0x0000000000007005);
  // accmulInput.setSumWZ(0x000000000000000d);
  // accmulInput.setSumWRoZ(0x0000000000800802);
  
  // accmulInput.setSumWPhi(0x00000000000000fd);
  // accmulInput.setSumWZ2(0x000000000002403a);
  // accmulInput.setSumWRoZ2(0x0000000000000285);  
  // accmulInput.setSumWPhi2(0x0000000000000005);
  // accmulInput.setLayerBits(0x00000000000d8007);
  
  // accmulInput.setsatTC(0x0000000000000000);
  // accmulInput.setshapeQ(0x0000000000000001);
  // ///////////////////////////////////////////
  
  //000000000000003f, 0000000000002cfa, 00000000000022bd, 0000000000000c7e, 0000000000000a3d,
  //0000000000000580, 000000000000003f, 00000000000109e2, 00000000000e80c8, 0000000000103d7f,
  //00000000007b1d49, 000000002cc73f86, 00000000300fc151, 0000000ac5ac785f, 000000013ff60000,
  //0000000000000000, 0000000000000001
    
  
  // accmulInput.setNumberOfTcs(0x000000000000003f);
  // accmulInput.setTotE(0x0000000000002cfa);
  // accmulInput.setCeeE(0x00000000000022bd);
  // accmulInput.setCeeECore(0x0000000000000c7e);
  // accmulInput.setCeHEarly(0x0000000000000a3d);
  
  // accmulInput.setSumW(0x0000000000000580);
  // accmulInput.setNumberOfTcsW(0x000000000000003f);
  // accmulInput.setSumW2(0x00000000000109e2);
  // accmulInput.setSumWZ(0x00000000000e80c8);
  // accmulInput.setSumWRoZ(0x00000000007b1d49); //swapped phi and roz
  
  // accmulInput.setSumWPhi(0x0000000000103d7f);
  // accmulInput.setSumWZ2(0x000000002cc73f86);
  // accmulInput.setSumWRoZ2(0x0000000ac5ac785f);
  // accmulInput.setSumWPhi2(0x00000000300fc151);  
  // accmulInput.setLayerBits(0x000000013ff60000);
  
  // accmulInput.setsatTC(0x0000000000000000);
  // accmulInput.setshapeQ(0x0000000000000001);

  accmulInput.printdetail(0);
  accmulInput.printdetail(1);
  accmulInput.printmaxval();
  
  //Set LUTs
  TPGStage2Configuration::ClusPropLUT cplut;
  cplut.readMuEtaLUT("input/stage2/configuration/mean_eta_LUT.csv");
  cplut.readSigmaEtaLUT("input/stage2/configuration/sigma_eta_LUT.csv");
  L1TOutputEmul.clear();

  //Set LSBScales
  TPGLSBScales::TPGStage2ClusterLSB lsbScales;
  lsbScales.setMaxTCETbits(19);
  lsbScales.setMaxTCRoZbits(13);
  
  //========================================
  //Emulation
  
  s2Clustering.setLSBScales(&lsbScales);
  s2Clustering.setClusPropLUT(&cplut);
  s2Clustering.ClusterProperties(accmulInput, L1TOutputEmul);
  //========================================
  
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_FIRSTWORD> firstw = 0x04c066a0d5002000;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_SECONDWORD> secondw = 0x0000ff10080402af;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_THIRDWORD> thirdw = 0x0004858000038f80;

  // //new EMP
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_FIRSTWORD> firstw = 0x010a9df4b01f8085;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_SECONDWORD> secondw = 0x0001788709b73216;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_THIRDWORD> thirdw = 0x4e73951500028a00;

  //03fffffff0200080  0001 0000fc000003fd40  0001 0000400000022980
  //new Emul
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_FIRSTWORD> firstw = 0x03fffffff0200080;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_SECONDWORD> secondw = 0x0000fc000003fd40;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_THIRDWORD> thirdw = 0x0000400000022980;

  //012ba0bab028c0e0  000178d3154689e5 4a53912e00038e00 00000003fff7f7c0
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_FIRSTWORD> firstw = 0x012ba0bab028c0e0;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_SECONDWORD> secondw = 0x000178d3154689e5 ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_THIRDWORD> thirdw = 0x4a53912e00038e00;

  //================================= Saturation test ======================================
  //Input Frame825, Output Frame 52
  //01392aa39019c0a2 0000f8a70003fd40 0003800000041001
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_FIRSTWORD> firstw = 0x01392aa39019c0a2;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_SECONDWORD> secondw = 0x0000f8a70003fd40 ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_THIRDWORD> thirdw = 0x0003800000041001;
  
  //011911ba1005801e 000178310006b940  0001800000036d80
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_FIRSTWORD> firstw = 0x011911ba1005801e;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_SECONDWORD> secondw = 0x000178310006b940 ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_THIRDWORD> thirdw = 0x0001800000036d80;
  
  //016e66911008803d  0001784b0000b140  0003800000028a00
  //01199cd330034010  000178210002d140  000300000002cb00
  //0125af9530058025  0001 0000f8350003fd40  0001 0001c0000003cf00
  //01227cd11010004f  0001 0000f88400040140  0001 000380000002aa80
  //010484f3b01d807c  0001 0000f8bb00040140  0001 0003800000032c80
  //01047fef9016c061  0001 0000f87a0003fd40  0001 0003400000032c80
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_FIRSTWORD> firstw = 0x01047fef9016c061;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_SECONDWORD> secondw = 0x0000f87a0003fd40 ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_THIRDWORD> thirdw = 0x0003400000032c80;

  //01156fc890164072 000178bb00101140  0003807f00036d80
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_FIRSTWORD> firstw = 0x01156fc890164072 ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_SECONDWORD> secondw = 0x000178bb00101140 ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_THIRDWORD> thirdw = 0x0003807f00036d80 ;

  //01227cd11010004f  0000f88400040140  000380000002aa80
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_FIRSTWORD> firstw = 0x01227cd11010004f ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_SECONDWORD> secondw = 0x0000f88400040140 ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_THIRDWORD> thirdw = 0x000380000002aa80 ;

  //01511e50000d40a9  0001 0001789800012140  0001 000380550003ae81
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_FIRSTWORD> firstw = 0x01511e50000d40a9 ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_SECONDWORD> secondw = 0x0001789800012140 ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_THIRDWORD> thirdw = 0x000380550003ae81 ;
  
  //012a7db1901a809a  0001 0000f8d97ffc0140  0001 0003807f00032c80
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_FIRSTWORD> firstw = 0x012a7db1901a809a ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_SECONDWORD> secondw = 0x0000f8d97ffc0140 ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_THIRDWORD> thirdw = 0x0003807f00032c80 ;
  
  //0139b6c730070024  0000f8357ff40140  0002403600020800
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_FIRSTWORD> firstw = 0x0139b6c730070024 ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_SECONDWORD> secondw = 0x0000f8357ff40140 ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_THIRDWORD> thirdw = 0x0002403600020800 ;

  //Frame 0251    0001 1855ffff30000000  0001 0000b829000c02af  0001 25f0400100017100  
  //Frame 0252    0001 0fa6001740000000  0001 0000b883002402af  0001 c1e0c00100027080  
  //Frame 0253    0001 19dd6dcf50000000  0001 0001380e0326626b  0001 0000801c00015100  
  //Frame 0254    0001 14f9a8ff70004000  0001 00017c08000feaaf  0001 000080570001b000  
  //Frame 0255    0001 0fff1bff50008000  0001 000178810007eeaf  0001 0000c00600029100  
  
  ///////////////////////////////////// Output 1 //////////////////////////////////////////
  ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_FIRSTWORD> firstw = 0x1855ffff30000000 ;
  ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_SECONDWORD> secondw = 0x0000b829000c02af ;
  ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_THIRDWORD> thirdw = 0x25f0400100017100 ;
  /////////////////////////////////////////////////////////////////////////////////////////
  
  // ///////////////////////////////////// Output 2 //////////////////////////////////////////
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_FIRSTWORD> firstw = 0x0fa6001740000000 ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_SECONDWORD> secondw = 0x0000b883002402af ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_THIRDWORD> thirdw = 0xc1e0c00100027080 ;
  // /////////////////////////////////////////////////////////////////////////////////////////

  // ///////////////////////////////////// Output 3 //////////////////////////////////////////
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_FIRSTWORD> firstw = 0x19dd6dcf50000000 ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_SECONDWORD> secondw = 0x0001380e0326626b ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_THIRDWORD> thirdw = 0x0000801c00015100 ;
  // /////////////////////////////////////////////////////////////////////////////////////////

  // ///////////////////////////////////// Output 4 //////////////////////////////////////////
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_FIRSTWORD> firstw = 0x14f9a8ff70004000 ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_SECONDWORD> secondw = 0x00017c08000feaaf ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_THIRDWORD> thirdw = 0x000080570001b000 ;
  // /////////////////////////////////////////////////////////////////////////////////////////

  // ///////////////////////////////////// Output 5 //////////////////////////////////////////
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_FIRSTWORD> firstw = 0x0fff1bff50008000 ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_SECONDWORD> secondw = 0x000178810007eeaf ;
  // ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_THIRDWORD> thirdw = 0x0000c00600029100 ;
  // /////////////////////////////////////////////////////////////////////////////////////////

    
  l1thgcfirmware::HGCalCluster_HW::unpack_firstWord(firstw,L1TOutputFW);
  l1thgcfirmware::HGCalCluster_HW::unpack_secondWord(secondw,L1TOutputFW);
  l1thgcfirmware::HGCalCluster_HW::unpack_thirdWord(thirdw,L1TOutputFW);

  //L1TOutputEmul.sigma_phi = 0x0b;
  //L1TOutputEmul.sigma_roz = 0x0;
  
  L1TOutputEmul.print();
  L1TOutputFW.print();

  // TPGLSBScales::TPGStage2ClusterLSB lsbScales;
  lsbScales.print();  
  return true;

  return true;
}
