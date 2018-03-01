#!/bin/bash

SRCDIR="../src/"
BINDIR="../bin/"

cd $SRCDIR &&
make -f makefile &&
mv ./hcfilesvr $BINDIR && rm -rf ./*.o

