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


int main(int argc, char** argv)
{
  
  TPGBEDataformat::TcAccumulatorFW accmulInput(3);
  l1thgcfirmware::HGCalCluster_HW L1TOutputEmul;
  l1thgcfirmware::HGCalCluster_HW L1TOutputFW;
  TPGStage2Emulation::Stage2 s2Clustering;
  
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
  
  accmulInput.setNumberOfTcs(0x00000000000000d3);
  accmulInput.setTotE(0x000000000000e025);
  accmulInput.setCeeE(0x000000000000a2c6);
  accmulInput.setCeeECore(0x00000000000065f4);
  accmulInput.setCeHEarly(0x00000000000025c9);
  accmulInput.setSumW(0x0000000000001bbe);
  accmulInput.setNumberOfTcsW(0x00000000000000d3);
  accmulInput.setSumW2(0x00000000000b293c);
  accmulInput.setSumWZ(0x000000000049aca0);
  accmulInput.setSumWRoZ(0x00000000017911f5);
  accmulInput.setSumWPhi(0x00000000008a6c48);
  accmulInput.setSumWZ2(0x0000000146632bbc);
  
  accmulInput.setSumWRoZ2(0x0000001409642eab);
  
  accmulInput.setSumWPhi2(0x00000002b3487988);
  
  accmulInput.setLayerBits(0x00000003fff7f7c0);
  accmulInput.setsatTC(0x0000000000000000);
  accmulInput.setshapeQ(0x0000000000000001);
  
  accmulInput.printdetail(0);
  //accmulInput.printmaxval();
  
  //Set LUTs
  TPGStage2Configuration::ClusPropLUT cplut;
  cplut.readMuEtaLUT("input/stage2/configuration/mean_eta_LUT.csv");
  cplut.readSigmaEtaLUT("input/stage2/configuration/sigma_eta_LUT.csv");
  L1TOutputEmul.clear();
  //Emulation
  s2Clustering.setClusPropLUT(&cplut);
  s2Clustering.ClusterProperties(accmulInput, L1TOutputEmul);
  
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
  ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_FIRSTWORD> firstw = 0x012ba0bab028c0e0;
  ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_SECONDWORD> secondw = 0x000178d3154689e5 ;
  ap_uint<l1thgcfirmware::HGCalCluster_HW::BITWIDTH_THIRDWORD> thirdw = 0x4a53912e00038e00;

  l1thgcfirmware::HGCalCluster_HW::unpack_firstWord(firstw,L1TOutputFW);
  l1thgcfirmware::HGCalCluster_HW::unpack_secondWord(secondw,L1TOutputFW);
  l1thgcfirmware::HGCalCluster_HW::unpack_thirdWord(thirdw,L1TOutputFW);

  //L1TOutputEmul.sigma_phi = 0x0b;
  //L1TOutputEmul.sigma_roz = 0x0;
  
  L1TOutputEmul.print();
  L1TOutputFW.print();
  
  return true;
}
