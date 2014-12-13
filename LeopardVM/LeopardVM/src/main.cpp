#include <iostream>

#include "vm.h"

using namespace std;

int main (int argc, char *argv[])
{
    if (argc == 1)
    {
        cout <<"Please supply the Leopard Executable file"<<endl;
        return 0;
    }

    char *filename = argv[1];
    VirtualMachine vm;
    string s (filename);
    init_jit(argv[0]);
    vm.start (s);
    
    return 0;
}