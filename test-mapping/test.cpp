#include <iostream>
#include "HgcConfigReader2.hpp"
#include "GUID.hpp"

int main() {

  // Opening the decoders xml file essentially populates C++ mapping objects with all info we currently need
  OpenDecoders( "./mapping/scenarios/SeparateTD.120.MixedTypes.PatchPanel_v3/S1.Decoders.xml"  );
  // However can open subset of xml files if you want e.g. :
  // OpenRegions("./mapping/scenarios/SeparateTD.120.MixedTypes.PatchPanel_v3/S1.regions.xml");
  // OpenPatch( "./mapping/patch/Patch.Rotated.xml"  );

  // Prints to screen objects that have been read in, and how many of them there are
  for ( auto& ptr : printers ) (*ptr)();

  // ==========================================================
  // Example of extracting modules connected to a S1 FPGA
  // ==========================================================
  // Get all S1 boards
  const auto& S1s = S1::instances;
  std::cout << "Total number of S1s : " << S1s.size() << std::endl;
  // Get the "first" S1 board
  const auto testS1 = S1s.begin()->second;
  // Get all modules by going via regions->motherboards->modules
  std::set<const Module*> modules;
  for (const auto& region : testS1->regions) {
      for (const auto& motherboard : region->motherboards) {
          for (const auto& module : motherboard->modules) {
              modules.insert(module);
          }
      }
  }
  // Print to screen IDs of modules (just first ten)
  unsigned iModule = 0;
  std::cout << "Module (ID, u, v, layer) : " << std ::endl;
  for ( const auto& module : modules ) {
    const auto& moduleUV = get_module_uv( module->ID );
    const auto modulePlane = get_plane( module->ID );
    std::cout << module->ID << " " << moduleUV.first << " " << moduleUV.second << " " << modulePlane << std::endl;

    ++iModule;
    if ( iModule > 10 ) {
      std::cout << "... and many more ..." << std::endl;
      break;
    }
  }

  // ==========================================================
  // Example of checking consistency of mapping
  // ==========================================================
  bool allOK = true;
  // Loop all S1 FPGAS
  for (const auto& [id, s1] : S1s) {
      // For each S1 FPGA, can get connected modules via regions->motherboards->modules
      // and decoders->motherboards->modules
      // Collect both and compare

      // Collect via Regions
      std::set<const Module*> modulesViaRegions;
      for (const auto& region : s1->regions) {
          for (const auto& motherboard : region->motherboards) {
              for (const auto& module : motherboard->modules) {
                  modulesViaRegions.insert(module);
              }
          }
      }

      // Collect modules via Decoders
      std::set<const Module*> modulesViaDecoders;
      for (const auto& decoder : s1->decoders) {
          for (const auto& motherboard : decoder->motherboards) {
              for (const auto& module : motherboard->modules) {
                  modulesViaDecoders.insert(module);
              }
          }
      }
      // Compare the sets
      if (modulesViaRegions != modulesViaDecoders) {
          std::cout << "  The modules are different between the two paths." << std::endl;
          allOK = false;
          break;
      }
  }
  if ( allOK ) {
      std::cout << "All comparisons were fine" << std::endl;
  }

  // ==========================================================
  // Example exploring output from a S1 FPGA
  // ==========================================================
  // Using same "first" S1 board as before as a test
  // Get all output channels from this FPGA
  std::set<S1Channel*> channels;
  std::cout << "Number of output channels : " << testS1->channels.size() << std::endl;
  unsigned iChannel = 0;
  for ( const auto& channel : testS1->channels ) {
    std::cout << "Channel : " << iChannel << " :  " << channel->ID << std::endl;
    channels.insert(channel);
    ++iChannel;
  }

  // Now look at first channel of this FPGA
  const auto testChannel = *channels.begin();
  std::cout << "Number of frames on this channel : " << testChannel->frames.size() << std::endl;
  unsigned iFrame = 0;
  for ( const auto& frame : testChannel->frames ) {
    const auto& frameData = frame->data;
    // frame->ID : clock within TMUX period
    // frameData.first : index of the TC within the phi-sorted Motherboard/Module 
    // frameData.second : module ID
    std::cout << "Frame info : " << frame->ID << " " << frameData.first << " " << frameData.second << std::endl;

    ++iFrame;
    if ( iFrame > 10 ) {
      std::cout << "... and many more ..." << std::endl;
      break;
    }
  }
}
