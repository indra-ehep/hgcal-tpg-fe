all: emul_test-beam_Sep23 findEMax read_econt_Jul24 GenerateEmpRxFile

emul_test-beam_Sep23:  emul_test-beam_Sep23.cpp
	g++ -I common/inc emul_test-beam_Sep23.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o emul_test-beam_Sep23.exe

findEMax: findEMax.cpp
	g++ -I common/inc findEMax.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o findEMax.exe

read_econt_Jul24: read_econt_Jul24.cpp
	g++ -I common/inc read_econt_Jul24.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o read_econt_Jul24.exe

GenerateEmpRxFile: GenerateEmpRxFile.cpp TPGStage1Emulation/Stage1IOFwCfg.hh TPGStage1Emulation/Stage1IO.hh TPGStage1Emulation/Stage1PassthroughFwCfg.hh TPGStage1Emulation/Stage1Passthrough.hh
	g++ -I common/inc -I TPGStage1Emulation -I . GenerateEmpRxFile.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o GenerateEmpRxFile.exe

clean:
	rm *.exe
