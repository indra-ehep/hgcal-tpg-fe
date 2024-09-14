#all: emul_test-beam_Sep23 findEMax read_econt_Jul24 GenerateEmpRxFile
LDFLAGS=-L$(HOME)/Software/yaml-cpp/lib64
CPPFLAGS=-I$(HOME)/Software/yaml-cpp/include -I common/inc -I inc -I TPGStage1Emulation/ -I TPGFEEmulation/ -I`root-config --incdir` 

all: emul_Jul24.exe #findEMax.exe GenerateEmpRxFile.exe dump_event.exe emul_test-beam_Sep23.exe 

emul_test-beam_Sep23.exe:  test-beam_Sep23_macros/emul_test-beam_Sep23.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ $(LDFLAGS) $(CPPFLAGS) test-beam_Sep23_macros/emul_test-beam_Sep23.cpp  -l yaml-cpp `root-config --libs --cflags` -o emul_test-beam_Sep23.exe

findEMax.exe: test-beam_Sep23_macros/findEMax.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++  $(LDFLAGS) $(CPPFLAGS)  test-beam_Sep23_macros/findEMax.cpp  -l yaml-cpp `root-config --libs --cflags` -o findEMax.exe

read_econt_Jul24.exe: test-beam_Aug24_macros/read_econt_Jul24.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ -I common/inc -I inc -I . test-beam_Aug24_macros/read_econt_Jul24.cpp  -l yaml-cpp `root-config --libs --cflags` -o read_econt_Jul24.exe

validation_lpGBTs.exe: test-beam_Aug24_macros/validation_lpGBTs.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ -I common/inc -I inc test-beam_Aug24_macros/validation_lpGBTs.cpp  -l yaml-cpp `root-config --libs --cflags` -o validation_lpGBTs.exe

emul_Jul24.exe: test-beam_Aug24_macros/emul_Jul24.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ $(LDFLAGS) $(CPPFLAGS) test-beam_Aug24_macros/emul_Jul24.cpp  -l yaml-cpp `root-config --libs --cflags` -o emul_Jul24.exe -lm

dump_event.exe: test-beam_Aug24_macros/dump_event.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ $(LDFLAGS) $(CPPFLAGS) test-beam_Aug24_macros/dump_event.cpp  -l yaml-cpp `root-config --libs --cflags` -o dump_event.exe

GenerateEmpRxFile.exe: TPGStage1Emulation/GenerateEmpRxFile.cpp inc/*.*  TPGFEEmulation/*.hh TPGStage1Emulation/*.hh common/inc/*.h
	g++ $(LDFLAGS) $(CPPFLAGS)  TPGStage1Emulation/GenerateEmpRxFile.cpp  -l yaml-cpp `root-config --libs --cflags` -o GenerateEmpRxFile.exe

clean:
	rm *.exe
