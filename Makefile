#all: emul_test-beam_Sep23 findEMax read_econt_Jul24 GenerateEmpRxFile
LDFLAGS=-L$(HOME)/Software/yaml-cpp/lib64
CPPFLAGS=-I$(HOME)/Software/yaml-cpp/include

all: dump_event.exe emul_Jul24_1.exe 

emul_test-beam_Sep23.exe:  test-beam_Sep23_macros/emul_test-beam_Sep23.cpp *.hh *.h TPGStage1Emulation/*.hh common/inc/*.h
	g++ -I common/inc $(LDFLAGS) $(CPPFLAGS) -I TPGStage1Emulation/ -I . test-beam_Sep23_macros/emul_test-beam_Sep23.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o emul_test-beam_Sep23.exe

findEMax.exe: test-beam_Sep23_macros/findEMax.cpp *.hh *.h TPGStage1Emulation/*.hh common/inc/*.h
	g++ -I common/inc $(LDFLAGS) $(CPPFLAGS) -I TPGStage1Emulation/ -I . test-beam_Sep23_macros/findEMax.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o findEMax.exe

read_econt_Jul24.exe: test-beam_Aug24_macros/read_econt_Jul24.cpp *.hh *.h TPGStage1Emulation/*.hh common/inc/*.h
	g++ -I common/inc -I . test-beam_Aug24_macros/read_econt_Jul24.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o read_econt_Jul24.exe

validation_lpGBTs.exe: test-beam_Aug24_macros/validation_lpGBTs.cpp *.hh *.h TPGStage1Emulation/*.hh common/inc/*.h
	g++ -I common/inc -I TPGStage1Emulation/ -I . test-beam_Aug24_macros/validation_lpGBTs.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o validation_lpGBTs.exe

emul_Jul24.exe: test-beam_Aug24_macros/emul_Jul24.cpp *.hh *.h TPGStage1Emulation/*.hh common/inc/*.h
	g++ -I common/inc $(LDFLAGS) $(CPPFLAGS) -I TPGStage1Emulation/ -I . test-beam_Aug24_macros/emul_Jul24.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o emul_Jul24.exe -lm

emul_Jul24_1.exe: test-beam_Aug24_macros/emul_Jul24.cpp *.hh *.h TPGStage1Emulation/*.hh common/inc/*.h
	g++ -I common/inc $(LDFLAGS) $(CPPFLAGS) -I TPGStage1Emulation/ -I . test-beam_Aug24_macros/emul_Jul24.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o emul_Jul24_1.exe -lm

emul_Jul24_2.exe: test-beam_Aug24_macros/emul_Jul24.cpp *.hh *.h TPGStage1Emulation/*.hh common/inc/*.h
	g++ -I common/inc $(LDFLAGS) $(CPPFLAGS) -I TPGStage1Emulation/ -I . test-beam_Aug24_macros/emul_Jul24.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o emul_Jul24_2.exe -lm

dump_event.exe: test-beam_Aug24_macros/dump_event.cpp *.hh *.h TPGStage1Emulation/*.hh common/inc/*.h
	g++ -I common/inc $(LDFLAGS) $(CPPFLAGS) -I TPGStage1Emulation/ -I . test-beam_Aug24_macros/dump_event.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o dump_event.exe

GenerateEmpRxFile.exe: GenerateEmpRxFile.cpp *.hh *.h TPGStage1Emulation/*.hh common/inc/*.h
	g++ -I common/inc $(LDFLAGS) $(CPPFLAGS) -I TPGStage1Emulation -I . GenerateEmpRxFile.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o GenerateEmpRxFile.exe

clean:
	rm *.exe
