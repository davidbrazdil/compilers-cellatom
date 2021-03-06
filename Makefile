CC=clang
#LLVM_LIBS=analysis dispatch archive bitreader bitwriter codegen core engine executionengine instrumentation interpreter ipa ipo jit linker native nativecodegen scalaropts selectiondag support target transformutils
LLVM_LIBS=all
EXTRA_LIBS=-ldispatch

all: cellatom

cellatom: interpreter.o main.o grammar.o compiler.o runtime.bc
	c++ compiler.o interpreter.o grammar.o main.o `llvm-config --ldflags --libs ${LLVM_LIBS}` ${EXTRA_LIBS} -o cellatom # -ldl

interpreter.o: interpreter.c AST.h
main.o: main.c AST.h grammar.h

runtime.bc: runtime.c
	clang -fblocks -c -emit-llvm runtime.c -I/usr/local/include/ -o runtime.bc -O0

compiler.o: compiler.cc AST.h
	clang `llvm-config --cxxflags` -c compiler.cc -O3 #-g -O0 -fno-inline -std=c++0x
grammar.h: grammar.c

grammar.c: grammar.y AST.h lemon
	./lemon grammar.y


lemon: lemon.c
	clang lemon.c -o lemon

clean:
	rm -f interpreter.o main.o grammar.o compiler.o runtime.bc grammar.h grammar.out cellatom lemon
