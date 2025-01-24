#include <iostream>
#include "L1Trigger/DemonstratorTools/interface/utilities.h"


int main(int argc, char** argv)
{
    // ===============================================
    // Example for reading in data from a pattern file
    // ===============================================
    std::string inputFileName = "EMPTools/testInput.txt";
    l1t::demo::BoardData inputs = l1t::demo::read( inputFileName, l1t::demo::FileFormat::EMPv2 );
    auto nChannels = inputs.size();
    std::cout << "Board data name : " << inputs.name() << std::endl;
    for ( const auto& channel : inputs ) {
        std::cout << "Data on channel : " << channel.first << std::endl;
        unsigned int iFrame = 0;
        for ( const auto& frame : channel.second ) {
	  std::cout << frame.startOfOrbit << frame.startOfPacket << frame.endOfPacket << frame.valid << " " << std::hex << frame.data << std::dec << std::endl;
            ++iFrame;
            if (iFrame > 5 ) break;
        }
    }

    // ==============================================
    // Example for writing out data to a pattern file
    // ==============================================
    l1t::demo::BoardData boardData("ExampleBoard");

    // Number of frames
    const size_t numFrames = 10;

    // Channels to define
    std::vector<size_t> channels = {14, 15};

    for (size_t channel : channels) {
        // Create a Channel object (vector of Frames)
        l1t::demo::BoardData::Channel channelData;

        for (size_t i = 0; i < numFrames; ++i) {
            // Create a Frame object and populate it
            l1t::demo::Frame frame;
            frame.data = i;
            frame.valid = true;
            frame.startOfOrbit = (i == 0);
            frame.endOfPacket = (i == numFrames - 1);

            // Add the frame to the channel data
            channelData.push_back(frame);
        }

        // Add the channel to the BoardData object
        boardData.add(channel, channelData);
    }

    // Write the BoardData to file in EMP format
    write(boardData, "EMPTools/testOutput.txt.gz", l1t::demo::FileFormat::EMPv2);

}
