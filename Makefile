CXX = clang++ -fomit-frame-pointer -I./minisat
CXXFLAGS = -std=c++17 -g $(OPT) 
LIBS = minisat/simp/lib.a
OPT = -g -O3
EXE = circuit
OBJ = circuit.o minisat.o
DEP = $(OBJ:.o=.d)

.PHONY: all clean

all: $(EXE)

$(EXE) : $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) $(LIBS) -o $(EXE)

%.o: %.cc
	$(CXX) -MMD $(CXXFLAGS) -c $<

-include $(DEP)

clean:
	rm -rf $(EXE) $(OBJ) $(DEP) cfg_*
