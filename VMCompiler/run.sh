#!/usr/bin/sh

f=$(readlink -f $1)
echo $f
cd Compiler/bin/Release
exec mono ./Compiler.exe $f
