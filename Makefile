CC=c99
#LLVM_LIBS=analysis archive bitreader bitwriter codegen core engine executionengine instrumentation interpreter ipa ipo jit linker native nativecodegen scalaropts selectiondag support target transformutils
LLVM_LIBS=all

all: cellatom

cellatom: interpreter.o main.o grammar.o compiler.o runtime.bc
	c++ compiler.o interpreter.o grammar.o main.o `llvm-config --ldflags --libs ${LLVM_LIBS}` -o cellatom -ldl

interpreter.o: interpreter.c AST.h
main.o: main.c AST.h grammar.h

runtime.bc: runtime.c
	clang -c -emit-llvm runtime.c -o runtime.bc -lpthread -O0

compiler.o: compiler.cc AST.h
	c++ -std=c++0x `llvm-config --cxxflags` -c compiler.cc -O3 #-g -O0 -fno-inline
grammar.h: grammar.c

grammar.c: grammar.y AST.h lemon
	./lemon grammar.y


lemon: lemon.c
	cc lemon.c -o lemon

clean:
	rm -f interpreter.o main.o grammar.o compiler.o runtime.bc grammar.h grammar.out cellatom lemon
