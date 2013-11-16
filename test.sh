#!/usr/bin/env bash

function test() {
#	make clean
	rm runtime.c runtime.bc
	ln -s "$1" runtime.c
	make > /dev/null
	./cellatom -j -i 100 -x 1000 -t < connway.ca > /dev/null
}

test runtime-old.c
test runtime-new.c
