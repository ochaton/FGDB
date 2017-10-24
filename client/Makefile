
.PHONY: all

all: build test

build:
	mkdir -p libs/

test: libs
	perl -Ilibs/lib/perl5/ -Ilibs/lib/perl5/x86_64-linux-thread-multi/auto/ test.pl

libs: cpanm
	./cpanm --local-lib=./libs Test::More FindBin Data::MessagePack

cpanm:
	curl -L https://cpanmin.us/ -o cpanm
	chmod a+x cpanm
