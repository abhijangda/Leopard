#include <iostream>

#include "vm.h"

using namespace std;

VirtualMachine* ptrVM;

int main (int argc, char *argv[])
{
    if (argc == 1)
    {
        cout <<"Please supply the Leopard Executable file"<<endl;
        return 0;
    }

    char *filename = argv[1];
    //VirtualMachine vm;
    ptrVM = new VirtualMachine ();
    string s (filename);
    init_jit(argv[0]);
    ptrVM->start (s);
    delete ptrVM;

    return 0;
}