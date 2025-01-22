/**********************************************************************
 Created on : 05/01/2025
 Purpose    : Tower emulation test for stage1 PRR
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
#include "TowerSums.h"
#include "Utilities.h"

#include <filesystem>
#include <iostream>
#include <fstream>

#include "L1Trigger/DemonstratorTools/interface/utilities.h"

#include "TPGFEConfiguration.hh"
#include "TPGFEDataformat.hh"
#include "TPGBEDataformat.hh"
#include "Stage1IO.hh"

#include "yaml-cpp/yaml.h"

using namespace std;

int main(int argc, char** argv)
{

  std::string inputFileName = "./input/stage1-PRR/EMPSample/rx_summary.txt";
  l1t::demo::BoardData inputs = l1t::demo::read( inputFileName, l1t::demo::FileFormat::EMPv2 );
  auto nChannels = inputs.size();
  std::cout << "Board name : " << inputs.name() << std::endl;
  std::cout << "Number of lpGBTs : " << inputs.size() << std::endl;
  std::cout << "Number of lpGBT pairs : " << inputs.size()/2 << std::endl;

  uint32_t zside = 0, sector = 0, link = 0, det = 0;
  uint32_t econt = 0, selTC4 = 1, module = 0;
  std::map<uint32_t,TPGFEConfiguration::ConfigEconT> econTPar;
  TPGFEConfiguration::ConfigEconT econTConf[5];
  TPGFEDataformat::TcRawDataPacket vTCel[5];
  TPGBEDataformat::UnpackerOutputStreamPair up1[5];
  std::vector<uint64_t> bcElist;
  int lpGBTpairs = inputs.size()/2;
  int stcydim = 3*lpGBTpairs ; //nof ECONTs with STC is 3
  std::vector<std::vector<uint64_t>> stcElist(12, std::vector<uint64_t>(stcydim, 0)); //12 corresponds to the maximum STC among the three ECONT types
  
  uint32_t Elinks[14], iel;
  std::map<uint32_t,uint32_t> lpGBTPair;
  std::map<size_t, l1t::demo::BoardData::Channel>::const_iterator itrlp;
  TPGFEConfiguration::TPGFEIdPacking pck;
  int ilppair = 0;
  for (itrlp = inputs.begin() ;  itrlp != inputs.end() ; itrlp++ ) { //loop on lpGBT pairs
    uint32_t lpGBT0 = itrlp->first;
    std::cout << "lpGBT0 : " << lpGBT0 << std::endl;
    l1t::demo::BoardData::Channel lp0 = itrlp->second;
    unsigned int iFrame0 = 0;
    unsigned int iFrame1 = 0;
    iel = 0;
    for (unsigned int ifr0 = 0 ;  ifr0 < lp0.size() ; ifr0++ ) { //loop on lpGBT pairs
      if(iFrame0>6) continue;
      l1t::demo::Frame frame0 = lp0.at(ifr0);
      Elinks[iel++] = frame0.data;
      std::cout << frame0.startOfOrbit << frame0.startOfPacket << frame0.endOfPacket << frame0.valid << " " << std::hex << frame0.data << std::dec << std::endl;
      iFrame0++;
    }
    itrlp++;
    uint32_t lpGBT1 = itrlp->first;
    std::cout << "lpGBT1 : " << lpGBT1 << std::endl;
    l1t::demo::BoardData::Channel lp1 = itrlp->second;
    for (unsigned int ifr1 = 0 ;  ifr1 < lp0.size() ; ifr1++ ) { //loop on lpGBT pairs
      if(iFrame1>6) continue;
      l1t::demo::Frame frame1 = lp1.at(ifr1);
      Elinks[iel++] = frame1.data;
      std::cout << frame1.startOfOrbit << frame1.startOfPacket << frame1.endOfPacket << frame1.valid << " " << std::hex << frame1.data << std::dec << std::endl;
      iFrame1++;
    }
    std::cout << std::endl;
    for(uint32_t iel1=0;iel1<iel;iel1++)  std::cout << std::setfill('0') << std::setw(2) << iel1 << ": 0x" << std::hex << std::setfill('0') << setw(8) << Elinks[iel1] << std::dec << ", "<< std::endl;
    std::cout << std::endl;
    
    lpGBTPair[lpGBT0] = lpGBT1;
    link = lpGBT0;
    econt = 0; uint32_t bc0 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
    econt = 1; uint32_t bc1 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
    econt = 2; uint32_t stc4a = pck.packModId(zside, sector, link, det, econt, selTC4, module);
    econt = 3; uint32_t stc160 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
    econt = 4; uint32_t stc161 = pck.packModId(zside, sector, link, det, econt, selTC4, module);
    econTConf[0].setSelect(2);
    econTConf[0].setNElinks(4);
    econTPar[bc0] = econTConf[0];
    econTConf[1].setSelect(2);
    econTConf[1].setNElinks(2);
    econTPar[bc1] = econTConf[1];
    econTConf[2].setSelect(1);
    econTConf[2].setNElinks(4);
    econTConf[2].setSTCType(3);
    econTPar[stc4a] = econTConf[2];
    econTConf[3].setSelect(1);
    econTConf[3].setNElinks(2);
    econTConf[3].setSTCType(1);
    econTPar[stc160] = econTConf[3];
    econTConf[4].setSelect(1);
    econTConf[4].setNElinks(2);
    econTConf[4].setSTCType(1);
    econTPar[stc161] = econTConf[4];
    
    for(int iecon=0;iecon<2;iecon++) std::cout << "EconT : " << iecon << ", nof TCs : " << econTConf[iecon].getNofTCs() << std::endl;
    for(int iecon=2;iecon<5;iecon++) std::cout << "EconT : " << iecon << ", nof STCs : " << econTConf[iecon].getNofSTCs() << std::endl;
    
    TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::BestC, econTConf[0].getNofTCs(), &Elinks[0], vTCel[0]);
    TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::BestC, econTConf[1].getNofTCs(), &Elinks[4], vTCel[1]);
    TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC4A, econTConf[2].getNofSTCs(), &Elinks[6], vTCel[2]);
    TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC16, econTConf[3].getNofSTCs(), &Elinks[10], vTCel[3]);
    TPGStage1Emulation::Stage1IO::convertElinksToTcRawData(TPGFEDataformat::STC16, econTConf[3].getNofSTCs(), &Elinks[12], vTCel[4]);
    for(int iecon=0;iecon<5;iecon++) vTCel[iecon].print();
    
    for(int iecon=0;iecon<5;iecon++) TPGStage1Emulation::Stage1IO::convertTcRawDataToUnpackerOutputStreamPair(vTCel[iecon].bx(), vTCel[iecon], up1[iecon]);
    for(int iecon=0;iecon<5;iecon++) up1[iecon].print();
    

    for(int iecon=0;iecon<2;iecon++) bcElist.push_back(up1[iecon].moduleSum(0));
    for(int iecon=2;iecon<5;iecon++){
      int ix = ilppair*3 + (iecon-2);
      int iy = 0;
      for(int is = 0 ; is < 2 ; is++){
    	for(int itc = 0 ; itc<TPGBEDataformat::UnpackerOutputStreamPair::NumberOfTCs ; itc++){
    	  if(up1[iecon].isValidChannel(is, itc)) stcElist[iy++][ix] = up1[iecon].channelEnergy(is,itc);
	}//tc loop
      }//stream loop
    }//econ loop
    ilppair++;
    
  }

  std::cout<<"nof BC Module Sums: " << bcElist.size() << std::endl;
  std::cout<<"nof STCs: " << stcElist.size() << std::endl;
  
  string inputFolderArch     = "./input/stage1-PRR/Verilog";
  string fileName_CE_E_arch = inputFolderArch + "/CE_E_dummy_20x30_241217.vh";
  string fileName_CE_H_arch = inputFolderArch + "/CE_H_dummy_20x30_241217.vh";
  
  TowerSums *ts = new TowerSums();
  vector<vector<int>> matVariable_CE_E;
  vector<vector<vector<int>>> matVariable_CE_H;
  pair<int, int> inputOutputE, inputOutputH;

  inputOutputE = ts->getParametersFromVhFile(fileName_CE_E_arch);
  inputOutputH = ts->getParametersFromVhFile(fileName_CE_H_arch);
  // Input to output mapping is fetched
  matVariable_CE_E = ts->vhArchInputToArray_CE_E( fileName_CE_E_arch, inputOutputE.second, inputOutputE.first );
  string STC_architecture_0 = "STC4"; // STC4, STC16 or oneSize
  string STC_architecture_1 = "STC16"; // STC4, STC16 or oneSize
  string STC_architecture_2 = "oneSize"; // STC4, STC16 or oneSize
  matVariable_CE_H = ts->vhArchInputToArray_CE_H( fileName_CE_H_arch, inputOutputH.second, inputOutputH.first, STC_architecture_2 );
  // Begin test
  cout << "Number of CE_E inputs: "  << inputOutputE.first  << endl;
  cout << "Number of CE_E outputs: " << inputOutputE.second << endl;
  cout << "Number of CE_H inputs: "  << inputOutputH.first  << endl;
  cout << "Number of CE_H outputs: " << inputOutputH.second << endl;
  int x_dim = matVariable_CE_H.size();
  int y_dim = matVariable_CE_H[0].size();
  cout << "matVariable_CE_H x_dim: "  << x_dim  << endl;
  cout << "matVariable_CE_H y_dim: " << y_dim << endl;

  ///////////////////////////////// CE_E ///////////////////////////////////////////
  auto inputArray_CE_E = bcElist;//ts->readInputEnergies_CE_E(fileName_CE_E_energies);
  std::cout<<"nof bcs: " << inputArray_CE_E.size() << std::endl;
  // Test: print energies
  cout << endl
       << "======================================" << endl
       << "           inputArray_CE_E            " << endl
       << "======================================" << endl;
  for (int energy : inputArray_CE_E)
    {
      cout << energy << ", ";
    }
  cout << endl;
  // End test
  
  // Integer unpacking.
  auto decodedInputArray_CE_E = ts->unpackInteger3m(inputArray_CE_E);
            
  // Test: print decoded energies
  cout << endl
       << "=============================================" << endl
       << "           decodedInputArray_CE_E            " << endl
       << "=============================================" << endl;
  for (auto energy : decodedInputArray_CE_E)
    {
      cout << energy << ", ";
    }
  cout << endl;
  // End test
            
            
            
  // Performing summation on unpacked data (integers). Parameter 1 or 2 indicates the mode of summation (for study purposes, different summation techniques have been implemented). 
  // If version 2 is selected, the subsequent false parameter is disregarded (i.e., it is valid only for version 1 of the summator).
  auto summedValues_CE_E = ts->summation(decodedInputArray_CE_E, matVariable_CE_E, 2, false);
            
  // Test: print summed values
  cout << endl << "summedValues_CE_E" << endl;
  cout << endl << "summedValues_CE_E size: " << summedValues_CE_E.size() << endl;
  for (auto value : summedValues_CE_E)
    {
      cout << value << ", ";
    }
  cout << endl;
  // End test
            
            
            
  // Checking for summation overflow (which can not be correctly coded with 5E3M format).
  // Second parameter is maximum number of allowed bits, 34 bits in case of 5E3M.
  auto checkedOutputValues_CE_E = ts->overflowChecker(summedValues_CE_E, 34);

  // Test: print checked values
  cout << endl << "checkedOutputValues_CE_E" << endl;
  for (auto value : checkedOutputValues_CE_E)
    {
      cout << value << ", ";
    }
  cout << endl;
  // End test            
            
  // Trimming to 19 bits (4E4M) - taking the first 19 MSBs (required parameters: targetNumberBits, maxNumberBits).
  auto trimmedOutputValues_CE_E = ts->trimming(checkedOutputValues_CE_E, 19, 34);
                  
  // Test: print trimmed values
  cout << endl << "trimmedOutputValues_CE_E" << endl;
  for (auto value : trimmedOutputValues_CE_E)
    {
      cout << value << ", ";
    }
  cout << endl;
  // End test
  
  // Converting the sums from pure integer format to 4E4M format.
  auto outputValues_CE_E = ts->packInteger4e4m(trimmedOutputValues_CE_E);
            
  // Test: print output values
  cout << endl
       << "========================================" << endl
       << "           outputValues_CE_E            " << endl
       << "========================================" << endl;
  cout << "[";
  for (auto value : outputValues_CE_E)
    {
      cout << value << ", ";
    }
  cout << "]" << endl;
  // End test          
            
  // // Write energies in hex format to file
  // ts->writeToFile(outputValues_CE_E, iSector, iBoard, "CE_E");
  ///////////////////////////////// CE_E ///////////////////////////////////////////
  
  ///////////////////////////////// CE_H ///////////////////////////////////////////
  std::cout << "stcydim : "  << stcydim << std::endl;
  for(int ix = 0 ; ix < stcydim ; ix++){
    for(int iy = 0 ; iy < 12 ; iy++){
      std::cout << stcElist[iy][ix] << ", ";
    }
    std::cout << std::endl;
  }

  // Read input energies
  auto inputArray_H = stcElist;//ts->readInputEnergies_CE_H(fileName_CE_H_energies, inputOutputH.first);
      
  // Test: print energies
  cout << endl << "inputArray_H" << endl;
  for (const auto& row : inputArray_H)
    {
      for (const auto& energy : row)
	{
	  cout << energy << ", ";
	}
    }         
  cout << endl ;
  
  // Integer unpacking
  auto decodedInputArrayH = ts->unpackInteger4m(inputArray_H);
      
  // Test: print decoded energies
  cout << "decodedInputArrayH" << endl;
  for(int ix = 0 ; ix < stcydim ; ix++){
    for(int iy = 0 ; iy < 12 ; iy++){
      std::cout << decodedInputArrayH[iy][ix] << ", ";
    }
    std::cout << std::endl;
  }

  for (const auto& row : decodedInputArrayH)
    {
      for (const auto& energy : row)
  	{
  	  cout << energy << ", ";
  	}
    }
  cout << endl;
   
  // Summation
  vector<uint64_t> summedValues_H;
  // int x_dim = matVariable_CE_H.size();
  // int y_dim = matVariable_CE_H[0].size();
            
  for (int k = 0; k < 12; ++k)
    {
               
      int z_slice = k; // Fixed depth index for the z dimension (equivalent to array[:, :, k] in Python)
       
      // Extract a 2D slice at z = z_slice
      vector<vector<int>> slice2D(x_dim, vector<int>(y_dim));
      for (int i = 0; i < x_dim; ++i)
  	{
  	  for (int j = 0; j < y_dim; ++j)
  	    {
  	      slice2D[i][j] = matVariable_CE_H[i][j][z_slice];
  	    }
  	}
               
               
      vector<uint64_t> temp_sum = ts->summation(decodedInputArrayH[k], slice2D, 2, false);

      if (k == 0)
  	{
  	  summedValues_H = temp_sum; // Initialize summedValues_H with the first summation result
  	}
      else
  	{
  	  summedValues_H = ts->addVectors(summedValues_H, temp_sum); // Add element-wise with summedValues_H
  	}
    }

  // Test: print summed values
  cout << "summedValues_H" << endl;
  for (auto value : summedValues_H)
    {
      cout << value << ", ";
    }
  cout << endl;                          
  
  // Second parameter is maximum number of allowed bits, 34 bits in case of 5E3M
  auto checkedOutputValues_H = ts->overflowChecker(summedValues_H, 35);
 
  // Test: print checked values
  cout << "checkedOutputValues_H" << endl;
  for (auto value : checkedOutputValues_H)
    {
      cout << value << ", ";
    }
  cout << endl;
            
            
            
  // Trimming to 19 bit (4E4M) - taking first 19 MSBs  (requied parameters> targetNumberBits, maxNzmberBits)
  auto trimmedOutputValues_H = ts->trimming(checkedOutputValues_H, 19, 35);
            
  // Test: print trimmed values
  cout << endl << "trimmedOutputValues_H" << endl;
  for (auto value : trimmedOutputValues_H)
    {
      cout << value << ", ";
    }
  cout << endl;
            
            
            
  // Converting the summs from pure integer number format to 4E4M format
  auto outputValues_H = ts->packInteger4e4m(trimmedOutputValues_H);
      
  // Test: print output values
  cout << endl << "outputValues_H" << endl;
  cout << "[";
  for (auto value : outputValues_H)
    {
      cout << value << ", ";
    }
  cout << "]" << endl;
  // End test

            
  // // Write energies in hex format to file
  // ts->writeToFile(outputValues_H, iSector, iBoard, "CE_H");

  ///////////////////////////////// CE_H ///////////////////////////////////////////
  
  delete ts;


  return true;
}
