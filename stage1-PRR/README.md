# Information about using the code in this folder

This folder contains code that was written at the time of the stage 1 PRR but also some things written afterwards and that are still in use (to keep everything together)

Several pieces of code are used for creating EMP files with e-link data, running the emulator (TC processor) and comparing with the FW output.

**Note that this code relies on the presence of Boost, libraries, the easiest is to check out `CMSSW_14_0_0_pre1`, and to run `cmsenv` from `CMSSW_14_0_0_pre1/src` before trying to compile the code.**

`UnpackerTCProcInterfaceEMPAndDirect.cpp` creates an EMP file with randomly generated TC data, and writes it out as `stage1-PRR/RandomlyGenerated.txt`. It has one command line argument, running `./UnpackerTCProcInterfaceEMPAndDirect.exe 1` will generate the EMP file and run the TC processor directly on the randomly generated data (without parsing the EMP file), writing out the output to a root file `TCProcessor_EmulationResults_Direct.root`. It also prints the output to screen.

`TestUnpackerTCProcInterface.cpp` takes as input an EMP file (`stage1-PRR/RandomGenWithBXBits_Corrected.txt`, hardcoded) and produces a root file with the output of the TC processor emulator (`TCProcessor_EmulationResults.root`, also hardcoded). If the code is run as `./TestUnpackerTCProcInterface.exe 1` the output is also printed to screen. It is the output of this code that is compared with the FW data. 

`EmulEMPCompTC.cpp` takes an EMP output file (e.g. `tx_summary_may12_nodelay_noplayloop.txt`), reads it and stores the data in a root tree. This code has several command line arguments

```
     ("ignore-first-frames",po::value<int>(&ignoreFirstNFrame_)->default_value(2),"Numer of frames at the start of the tx file to be ignored")
    ("ignore-first-channels",po::value<int>(&ignoreFirstNChn_)->default_value(4),"Numer of channels at the start of the tx file to be ignored")
     ("offset-bch",po::value<int>(&offsetBCHi_)->default_value(10),"BC high occupancy latency")
     ("offset-bcl",po::value<int>(&offsetBCLo_)->default_value(4),"BC low occupancy latency")
     ("offset-stc",po::value<int>(&offsetSTC_)->default_value(12),"STC latency")
     ("input-file",po::value<std::string>(&inputFWFileName)->default_value(""),"Input filename")
     ("output-file",po::value<std::string>(&outputFileName)->default_value("FirmwareResults.root"),"Output filename")
     ("print",po::value<bool>(&doPrint_)->default_value(false),"Print TCs to screen");
```
The first 4 channels are not instrumented in the test setup, so we do not need to store the data there, and it was found (with the version of the firmware with which tests were run), that the first 2 frames of each file do not contain output data, hence the first two arguments.

The latencies for the high and low occupancy BC and the STC unpackers being different, there are options to set the offset, ie which output bunch crossing matches the data from which input bunch crossing. If the offset changes, it needs to be re-measured, the way to do this is to run this script with `--print` set to true and to compare the TC addresses and energies for the different modules at different 'output events' (output bunch crossings or frames - relationship between frame and  bunch crossing: `(frame-2)/8`, assuming the first two frames continue not to contain useful data) with the emulator output (easiest using the root tree and searching, for the specific module under study, for the TCs with address/energy that match those observed in the printed output from parsing the firmware tx file).

The tx file (input) must be specified on the command line (the available example one can be used). The output file name can also be modified from the command line. 

Afterwards, the script `compareFWAndEmTrees.py` can be used to compare the two root files (one from the emulator and one from the FW, examples have also been committed in this folder). This loads the data into a dict (format: [event][module][column]=[(tc energy 0,tc address 0),(tc energy 1, tc address 1)] , or only a single (energy,address) combination if the column/bin can only hold a single TC). This compares the two dicts and prints any differences. Some are expected (e.g. missing first event in the tx file for some of the module types, missing some events at the end due to the latency)

## Format of the root trees that are prodcued
The emulator root trees contain one entry per trigger cell, storing event (bunch crossing), module number (0-300), column number, address, energy. Additionally, the first output link is stored (this is related to the module number).

In the FW root trees, we store the 'output event' = the bunch crossing found in the tx file, the equivalent input event (bunch crossing in the tx file adjusted for the unpacker latency) - so that we can compare with the emulator-, column number, address and energy. 

## Other notes
To run an EMP capture using the EMP input file (for producing FW output tx file), see the instructions provided during the HGCAL BE meetings.

Tests so far have been performed with this version of the firmware: `stage1_tc_chain_18mar_llrgrhgtrig_in2p3_fr_250325_1022`
