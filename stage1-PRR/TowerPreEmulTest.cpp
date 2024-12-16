#include "TowerSums.h"
#include "Utilities.h"

#include <filesystem>
#include <iostream>


using namespace std;


int main()
{
     
//   bool debugMode{false};
//   bool filesCreatedFlag{false};
   
   int numOfSectors{3}; // 3 120 degrees HGCAL sectors
   int numOfBoards{14};  // 14 Phase-1 TPG Serenity boards per HGCAL sector
   

   for (int iSector = 0; iSector < numOfSectors; ++iSector)
   {
      for (int iBoard = 0; iBoard < numOfBoards; ++iBoard)
      {
   
         // Define everything needed to read mapping and architecture.
         string inputFolderArch     = "./input/arhitecture/v2";
         string inputFolderEnergies = "./input/energies/stage_1_unpacker/";
           
         string fileName_CE_E_arch = inputFolderArch + "/CE_E_" + to_string(iBoard) + "_v2.vh";
         string fileName_CE_H_arch = inputFolderArch + "/CE_H_" + to_string(iBoard) + "_v2.vh";
          
         string fileName_CE_E_energies = inputFolderEnergies + "/SinglePhotonPU0V16/CE_E/Sector_" + to_string(iSector) + "_Board_" + to_string(iBoard) + ".txt";
         string fileName_CE_H_energies = inputFolderEnergies + "/SinglePionPU0V16/CE_H/Sector_" + to_string(iSector) + "_Board_" + to_string(iBoard) + ".txt";
           
         string STC_architecture = "oneSize"; // STC4, STC16 or oneSize
           
         
         TowerSums *ts = new TowerSums();
           
         vector<vector<int>> matVariable_CE_E;
         vector<vector<vector<int>>> matVariable_CE_H;
         pair<int, int> inputOutputE, inputOutputH;
   
   

// ******************************************
// Read CE_E VH files and architecture matrix
// ******************************************
   
         filesystem::path filePath_CE_E_arch(fileName_CE_E_arch);
         if (filesystem::exists(filePath_CE_E_arch) && filesystem::is_regular_file(filePath_CE_E_arch))
         {
            cout << endl << "The file " << filePath_CE_E_arch << " exists." << endl;
            
            // Number of input and output lines is fetched
            inputOutputE = ts->getParametersFromVhFile(fileName_CE_E_arch);
            
            // Input to output mapping is fetched
            matVariable_CE_E = ts->vhArchInputToArray_CE_E( fileName_CE_E_arch, inputOutputE.second, inputOutputE.first );
            
       
            // Begin test
            cout << "Number of inputs: "  << inputOutputE.first  << endl;
            cout << "Number of outputs: " << inputOutputE.second << endl;
            cout << endl;
//            Utilities::printArchMatrix_CE_E( matVariable_CE_E );
            cout << endl;
            // End test
         }
         else // Fill with zeros???
         {
            inputOutputE.first = 0;
            inputOutputE.second = 0;
            vector<vector<int>> matVariable_CE_E(inputOutputE.second, vector<int>(inputOutputE.first, 0));

            cout << "Sector " + to_string(iSector) + "Board " + to_string(iBoard) + " CE_E architecture not detected! To be done --> Generating an empty array!" << endl;
         }

  

// ******************************************
// Read CE_H VH files and architecture matrix
// ******************************************
   
         filesystem::path filePath_CE_H_arch(fileName_CE_H_arch);
         if (filesystem::exists(filePath_CE_H_arch) && filesystem::is_regular_file(filePath_CE_H_arch))
         {
            cout << "The file " << filePath_CE_H_arch << " exists." << endl;
            
            // Number of input and output lines is fetched
            inputOutputH = ts->getParametersFromVhFile(fileName_CE_H_arch);
            
            // Input to output mapping is fetched
            matVariable_CE_H = ts->vhArchInputToArray_CE_H( fileName_CE_H_arch, inputOutputH.second, inputOutputH.first, STC_architecture);
            
       
            // Begin test                    
            cout << "Number of inputs: "  << inputOutputH.first  << endl;
            cout << "Number of outputs: " << inputOutputH.second << endl;
            cout << endl;
//            Utilities::printArchMatrix_CE_H( matVariable_CE_H );
            cout << endl;
            // End test
         }
         else // Fill with zeros???
         {
            inputOutputH.first = 0;
            inputOutputH.second = 0;
            vector<vector<vector<int>>> output(inputOutputH.second, vector<vector<int>>(inputOutputH.second, vector<int>(0, 0)));
            
            cout << "Sector " + to_string(iSector) + "Board " + to_string(iBoard) + " CE_H architecture not detected! Generating an empty array!" << endl;
         }

   
   
// ****************************************************************
// Read CE_E input energies - assumption: inputs are in 5E3M format
// Integer unpacking
// Performing summation
// Overflow checking
// Trimming
// Packing to 4e4M format
// ****************************************************************
   
         filesystem::path filePath_E_inputs(fileName_CE_E_energies);
         if (filesystem::exists(fileName_CE_E_energies) && filesystem::is_regular_file(fileName_CE_E_energies))
         {
            cout << endl << "The file " << fileName_CE_E_energies << " exists." << endl;
            
            
            // Generate inputs in decimal format (up to a value of 255).
            // Assumption: Inputs are in 5E3M format.
            auto inputArray_CE_E = ts->readInputEnergies_CE_E(fileName_CE_E_energies);
            
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
            
            
            // Write energies in hex format to file
            ts->writeToFile(outputValues_CE_E, iSector, iBoard, "CE_E");
            
         }
         else
         {
            vector<int> outputValues_CE_E(inputOutputE.second, 0);
            cout << "Sector " + to_string(iSector) + "Board " + to_string(iBoard) + " E inputs not detected! Filling output with zeros!" << endl;  
         }





// ****************************************************************
// Read CE_H input energies - assumption: inputs are in 5E3M format
// ****************************************************************
   
         filesystem::path filePath_H_inputs(fileName_CE_H_energies);
         if (filesystem::exists(fileName_CE_H_energies) && filesystem::is_regular_file(fileName_CE_H_energies))
         {
            cout << "The file " << fileName_CE_H_energies << " exists." << endl;
            
            
            
            // Read input energies
            auto inputArray_H = ts->readInputEnergies_CE_H(fileName_CE_H_energies, inputOutputH.first);
      
            // Test: print energies
            cout << endl << "inputArray_H" << endl;
            for (const auto& row : inputArray_H)
            {
               for (const auto& energy : row)
               {
                  cout << energy << ", ";
               }
               cout << "Test" << endl;
            }
         
         
         
            // Integer unpacking
            auto decodedInputArrayH = ts->unpackInteger4m(inputArray_H);
      
            // Test: print decoded energies
            cout << "decodedInputArrayH" << endl;
            for (const auto& row : decodedInputArrayH)
            {
                  for (const auto& energy : row)
                  {
                     cout << energy << ", ";
                  }
                  cout << "Test" << endl;
            }
            cout << endl;


   
            // Summation
            vector<uint64_t> summedValues_H;
            int x_dim = matVariable_CE_H.size();
            int y_dim = matVariable_CE_H[0].size();
            
            for (int k = 0; k < 6; ++k)
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

            
            // Write energies in hex format to file
            ts->writeToFile(outputValues_H, iSector, iBoard, "CE_H");
            
         }
         else
         {
            vector<int> outputValues_H(inputOutputH.second, 0);
            cout << "Sector " + to_string(iSector) + "Board " + to_string(iBoard) + " H inputs not detected! Filling output with zeros!" << endl;  
         }         
         

         
      } //End iSector
   } //End iBoard
      
   
   return 0;
}
