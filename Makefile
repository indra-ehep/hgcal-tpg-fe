#all: emul_test-beam_Sep23 findEMax read_econt_Jul24 GenerateEmpRxFile

CPPFLAGS=-I$(HOME)/Software/yaml-cpp/include -I common/inc -I inc -I TPGStage1Emulation/ -I TPGFEEmulation/ -I`root-config --incdir` 
BOOST=/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/boost/1.80.0-87b5de10acd2f2c8a325345ad058b814
LDFLAGS=-L$(HOME)/Software/yaml-cpp/lib64 -L$(BOOST)/lib
CPPFLAGSSTAGE2=-I inc -I TPGStage2Emulation/ -I EMPTools/ -I EMPTools/CMSSWCode -I EMPTools/HLS_arbitrary_Precision_Types/include/ -I`root-config --incdir` 

all:  writeStage2EMPRx.exe readNTuple.exe  fillInputData.exe testStage2SemiClustering.exe stage2HtoTauTauEnergyCorrelation.exe ntupleMCInfo.exe vbfjet.exe EmulTowerPRRTest.exe TowerPreEmulTest.exe  TestUnpackerTCProcInterface.exe #findFixedpattern.exe findEMax.exe tpgdata_3T_tcproc.exe  tpgdata_3T_fe.exe  scanadc_Sep24.exe dump_event.exe tpgdata_2T_fe.exe emul_Sep24.exe emul_3T_Sep24.exe  #loop_emul_Sep24.exe scanconfigval_Sep24.exe emul_Sep24.exe validateFixedADC.exe # findEMax.exe GenerateEmpRxFile.exe dump_event.exe emul_test-beam_Sep23.exe 


emul_test-beam_Sep23.exe:  test-beam_Sep23_macros/emul_test-beam_Sep23.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ $(LDFLAGS) $(CPPFLAGS) test-beam_Sep23_macros/emul_test-beam_Sep23.cpp  -l yaml-cpp `root-config --libs --cflags` -o emul_test-beam_Sep23.exe

findEMax.exe: test-beam_Sep23_macros/findEMax.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++  $(LDFLAGS) $(CPPFLAGS)  test-beam_Sep23_macros/findEMax.cpp  -l yaml-cpp `root-config --libs --cflags` -o findEMax.exe

read_econt_Jul24.exe: test-beam_Aug24_macros/read_econt_Jul24.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ -I common/inc -I inc -I . test-beam_Aug24_macros/read_econt_Jul24.cpp  -l yaml-cpp `root-config --libs --cflags` -o read_econt_Jul24.exe

validation_lpGBTs.exe: test-beam_Aug24_macros/validation_lpGBTs.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ -I common/inc -I inc test-beam_Aug24_macros/validation_lpGBTs.cpp  -l yaml-cpp `root-config --libs --cflags` -o validation_lpGBTs.exe

emul_Jul24.exe: test-beam_Aug24_macros/emul_Jul24.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ $(LDFLAGS) -I. TPGStage1Emulation/HGCalLayer1PhiOrderFwImpl.cc $(CPPFLAGS) test-beam_Aug24_macros/emul_Jul24.cpp  -l yaml-cpp `root-config --libs --cflags` -o emul_Jul24.exe -lm

tpgdata_3T_tcproc.exe: test-beam_Sep24_macros/tpgdata_3T_tcproc.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ $(LDFLAGS) -I. TPGStage1Emulation/HGCalLayer1PhiOrderFwImpl.cc $(CPPFLAGS) test-beam_Sep24_macros/tpgdata_3T_tcproc.cpp  -l yaml-cpp `root-config --libs --cflags` -o tpgdata_3T_tcproc.exe -lm

tpgdata_3T_fe.exe: test-beam_Sep24_macros/tpgdata_3T_fe.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ $(LDFLAGS) $(CPPFLAGS) test-beam_Sep24_macros/tpgdata_3T_fe.cpp  -l yaml-cpp `root-config --libs --cflags` -o tpgdata_3T_fe.exe -lm

tpgdata_2T_fe.exe: test-beam_Sep24_macros/tpgdata_2T_fe.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ $(LDFLAGS) $(CPPFLAGS) test-beam_Sep24_macros/tpgdata_2T_fe.cpp  -l yaml-cpp `root-config --libs --cflags` -o tpgdata_2T_fe.exe -lm

findFixedpattern.exe: test-beam_Sep24_macros/findFixedpattern.C inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ $(LDFLAGS) $(CPPFLAGS) test-beam_Sep24_macros/findFixedpattern.C  -l yaml-cpp `root-config --libs --cflags` -o findFixedpattern.exe -lm

scanconfigval_Sep24.exe: test-beam_Sep24_macros/scanconfigval_Sep24.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ $(LDFLAGS) $(CPPFLAGS) test-beam_Sep24_macros/scanconfigval_Sep24.cpp  -l yaml-cpp `root-config --libs --cflags` -o scanconfigval_Sep24.exe -lm

scanadc_Sep24.exe: test-beam_Sep24_macros/scanadc_Sep24.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ $(LDFLAGS) $(CPPFLAGS) test-beam_Sep24_macros/scanadc_Sep24.cpp  -l yaml-cpp `root-config --libs --cflags` -o scanadc_Sep24.exe -lm

emul_Sep24.exe: test-beam_Sep24_macros/emul_Sep24.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ $(LDFLAGS) $(CPPFLAGS) test-beam_Sep24_macros/emul_Sep24.cpp  -l yaml-cpp `root-config --libs --cflags` -o emul_Sep24.exe -lm

emul_3T_Sep24.exe: test-beam_Sep24_macros/emul_3T_Sep24.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ $(LDFLAGS) $(CPPFLAGS) test-beam_Sep24_macros/emul_3T_Sep24.cpp  -l yaml-cpp `root-config --libs --cflags` -o emul_3T_Sep24.exe -lm

loop_emul_Sep24.exe: test-beam_Sep24_macros/loop_emul_Sep24.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ $(LDFLAGS) $(CPPFLAGS) test-beam_Sep24_macros/loop_emul_Sep24.cpp  -l yaml-cpp `root-config --libs --cflags` -o loop_emul_Sep24.exe -lm

validateFixedADC.exe: test-beam_Sep24_macros/validateFixedADC.C inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ $(LDFLAGS) $(CPPFLAGS) test-beam_Sep24_macros/validateFixedADC.C  -l yaml-cpp `root-config --libs --cflags` -o validateFixedADC.exe -lm

dump_event.exe: test-beam_Sep24_macros/dump_event.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ $(LDFLAGS) $(CPPFLAGS) test-beam_Sep24_macros/dump_event.cpp  -l yaml-cpp `root-config --libs --cflags` -o dump_event.exe

GenerateEmpRxFile.exe: TPGStage1Emulation/GenerateEmpRxFile.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ $(LDFLAGS) $(CPPFLAGS)  TPGStage1Emulation/GenerateEmpRxFile.cpp  -l yaml-cpp `root-config --libs --cflags` -o GenerateEmpRxFile.exe

TowerPreEmulTest.exe: TPGStage1Emulation/TowerSums.h TPGStage1Emulation/Utilities.h
	g++ $(CPPFLAGS) -c TPGStage1Emulation/Utilities.cpp -o Utilities.o
	g++ $(CPPFLAGS) -c TPGStage1Emulation/TowerSums.cpp -o TowerSums.o	
	g++ $(CPPFLAGS) stage1-PRR/TowerPreEmulTest.cpp  Utilities.o TowerSums.o -o TowerPreEmulTest.exe
	rm *.o

TestUnpackerTCProcInterface.exe: stage1-PRR/TestUnpackerTCProcInterface.cpp TPGFEEmulation/*.hh  TPGStage1Emulation/*.hh common/inc/*.h inc/*.*
	g++ -I TPGStage1Emulation/ -I. TPGStage1Emulation/HGCalLayer1PhiOrderFwImpl.cc -I. -I common/inc -I inc -I TPGFEEmulation/ stage1-PRR/TestUnpackerTCProcInterface.cpp -L /opt/local/lib  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` EMPTools/CMSSWCode/L1Trigger/DemonstratorTools/src/* -IEMPTools/CMSSWCode/ -IEMPTools/HLS_arbitrary_Precision_Types/include/ -I$(BOOST)/include -lboost_iostreams -lz -llzma $(LDFLAGS) $(CPPFLAGS) `root-config --libs --cflags` -o TestUnpackerTCProcInterface.exe

# TestEMP: EMPTools/test.cpp EMPTools/CMSSWCode/L1Trigger/DemonstratorTools/src/* $(BOOST)/include
# 	g++  EMPTools/test.cpp EMPTools/CMSSWCode/L1Trigger/DemonstratorTools/src/* -IEMPTools/CMSSWCode/ -IEMPTools/HLS_arbitrary_Precision_Types/include/ -I$(BOOST)/include -lboost_iostreams -lz -llzma -o TestEMP.exe

TestEMP: EMPTools/test.cpp EMPTools/CMSSWCode/L1Trigger/DemonstratorTools/src/*
	g++  EMPTools/test.cpp EMPTools/CMSSWCode/L1Trigger/DemonstratorTools/src/* -IEMPTools/CMSSWCode/ -IEMPTools/HLS_arbitrary_Precision_Types/include/  -lboost_iostreams -lz -llzma -o TestEMP.exe

EmulTowerPRRTest.exe: stage1-PRR/EmulTowerPRRTest.cpp EMPTools/CMSSWCode/L1Trigger/DemonstratorTools/src/*
	g++ $(CPPFLAGS) -c TPGStage1Emulation/Utilities.cpp -o Utilities.o
	g++ $(CPPFLAGS) -c TPGStage1Emulation/TowerSums.cpp -o TowerSums.o	
	g++ $(LDFLAGS) $(CPPFLAGS) stage1-PRR/EmulTowerPRRTest.cpp EMPTools/CMSSWCode/L1Trigger/DemonstratorTools/src/* -I$(BOOST)/include -IEMPTools/CMSSWCode/ -IEMPTools/HLS_arbitrary_Precision_Types/include/ `root-config --libs --cflags` -lboost_iostreams -lz -llzma -l yaml-cpp -lm Utilities.o TowerSums.o -o EmulTowerPRRTest.exe
	rm *.o	

readNTuple.exe: test-stage2_Nov24/readNTuple.cpp inc/*.*  TPGStage2Emulation/*.hh
	g++ $(CPPFLAGSSTAGE2) test-stage2_Nov24/readNTuple.cpp `root-config --libs --cflags` -o readNTuple.exe

fillInputData.exe: test-stage2_Nov24/fillInputData.cpp inc/*.*  TPGStage2Emulation/*.hh
	g++ $(CPPFLAGSSTAGE2) test-stage2_Nov24/fillInputData.cpp `root-config --libs --cflags` -o fillInputData.exe

testStage2SemiClustering.exe: test-stage2_Nov24/testStage2SemiClustering.cpp inc/*.*  TPGStage2Emulation/*.hh
	g++ $(CPPFLAGSSTAGE2) test-stage2_Nov24/testStage2SemiClustering.cpp `root-config --libs --cflags` -o testStage2SemiClustering.exe

ntupleMCInfo.exe: test-stage2_Nov24/ntupleMCInfo.cpp inc/*.*  TPGStage2Emulation/*.hh
	g++ $(CPPFLAGSSTAGE2) test-stage2_Nov24/ntupleMCInfo.cpp `root-config --libs --cflags` -o ntupleMCInfo.exe -lEG

stage2HtoTauTauEnergyCorrelation.exe: test-stage2_Nov24/stage2HtoTauTauEnergyCorrelation.cpp inc/*.*  TPGStage2Emulation/*.hh
	g++ $(CPPFLAGSSTAGE2) test-stage2_Nov24/stage2HtoTauTauEnergyCorrelation.cpp `root-config --libs --cflags` -o stage2HtoTauTauEnergyCorrelation.exe -lEG

vbfjet.exe: test-stage2_Nov24/vbfjet.cpp inc/*.*  TPGStage2Emulation/*.hh
	g++ $(CPPFLAGSSTAGE2) test-stage2_Nov24/vbfjet.cpp `root-config --libs --cflags` -o vbfjet.exe -lEG

writeStage2EMPRx.exe: test-stage2_Nov24/writeStage2EMPRx.cpp inc/*.*  TPGStage2Emulation/*.hh
	g++ $(CPPFLAGSSTAGE2) EMPTools/CMSSWCode/L1Trigger/DemonstratorTools/src/* test-stage2_Nov24/writeStage2EMPRx.cpp `root-config --libs --cflags` -L$(BOOST)/lib  -lboost_iostreams -lz -llzma -o writeStage2EMPRx.exe


clean:
	rm *.exe
