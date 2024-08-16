#all: emul_test-beam_Sep23 findEMax read_econt_Jul24 GenerateEmpRxFile
LDFLAGS=-L$(HOME)/Software/yaml-cpp/lib64
CPPFLAGS=-I$(HOME)/Software/yaml-cpp/include

all: emul_Jul24

emul_test-beam_Sep23:  test-beam_Sep23_macros/emul_test-beam_Sep23.cpp
	g++ -I common/inc -I . test-beam_Sep23_macros/emul_test-beam_Sep23.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o emul_test-beam_Sep23.exe

findEMax: test-beam_Sep23_macros/findEMax.cpp
	g++ -I common/inc -I . test-beam_Sep23_macros/findEMax.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o findEMax.exe

read_econt_Jul24: test-beam_Aug24_macros/read_econt_Jul24.cpp
	g++ -I common/inc -I . test-beam_Aug24_macros/read_econt_Jul24.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o read_econt_Jul24.exe

validation_lpGBTs: test-beam_Aug24_macros/validation_lpGBTs.cpp
	g++ -I common/inc -I TPGStage1Emulation/ -I . test-beam_Aug24_macros/validation_lpGBTs.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o validation_lpGBTs.exe

emul_Jul24: test-beam_Aug24_macros/emul_Jul24.cpp
	g++ -I common/inc $(LDFLAGS) $(CPPFLAGS) -I TPGStage1Emulation/ -I . test-beam_Aug24_macros/emul_Jul24.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o emul_Jul24.exe

GenerateEmpRxFile: GenerateEmpRxFile.cpp TPGStage1Emulation/Stage1IOFwCfg.hh TPGStage1Emulation/Stage1IO.hh TPGStage1Emulation/Stage1PassthroughFwCfg.hh TPGStage1Emulation/Stage1Passthrough.hh
	g++ -I common/inc -I TPGStage1Emulation -I . GenerateEmpRxFile.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o GenerateEmpRxFile.exe

clean:
	rm *.exe
