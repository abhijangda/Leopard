Leopard
=======

An Imperative, Object Oriented, Strong, Type Safe, Reflective, Garbage Collection and JITed Language.
Compiler and Assembler has been written in C#.
Virtual Machine will be written in C++.

To run Compiler, please ensure you have installed following:
* Mono Runtime

To compile Compiler run following command:

    cd VMCompiler

    ./autogen.sh

    make

    ./run.sh <file to compile>


Current Status:
* Compiler is completed. It gives the VM's bytecode instructions for a given output. Semantic Analysis and Error Reporting needs to be improved.
* Assembler is pending
* Virtual Machine is pending
