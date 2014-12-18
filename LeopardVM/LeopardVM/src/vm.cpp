#include <fstream>
#include <vector>

#include "helper_functions.h"
#include "vm.h"

using namespace std;

int VirtualMachine::start (string filename)
{
    /* Process file */
    if (read (filename) != 0)
        return -1;
    
    /* Start the main function */
    MethodInfo* mainMethodInfo;

    for (int i = 0; i < vectorClassInfo.size (); i++)
    {
        if (vectorClassInfo [i]->getName () == mainClass)
        {
            for (int j = 0; j < vectorClassInfo [i]->totalMethods (); i++)
            {
                if (vectorClassInfo [i]->getMethodInfo (j)->getName () == mainFunction)
                {
                    mainMethodInfo = vectorClassInfo [i]->getMethodInfo (j);
                    break;
                }
            }
        
            break;
        }
    }

    jit->runMethodCode (mainMethodInfo->getCode ());
}

int VirtualMachine::read (const string filename)
{
    byte *memblock;
    long memblockSize;
    long memblockIter;
    int classNameLength, functionNameLength;
    int totalClasses;
    int i;
    ifstream fileStream (filename.c_str (), ios::in | ios::binary | ios::ate);
    
    if (!fileStream.is_open ())
        return -1;
    
    memblockIter = -1;
    memblockSize = fileStream.tellg ();
    memblock = new byte [memblockSize];
    fileStream.seekg (0, ios::beg);
    fileStream.read ((char *)memblock, memblockSize);
    
    /* Extract data */
    /* is the system little endian */
    isLittleEndian = memblock [++memblockIter];
    
    /* Get the name of main class */
    classNameLength = memblock [++memblockIter];
    
    i = 0;
    while (i < classNameLength)
    {
        mainClass += char_to_string ((char)memblock [++memblockIter]);
        i++;
    }
    
    /* Get the name of main function */
    functionNameLength = memblock [++memblockIter];
    
    i = 0;
    while (i < functionNameLength)
    {
        mainFunction += char_to_string ((char)memblock [++memblockIter]);
        i++;
    }

    /* Get each classinfo */
    totalClasses = memblock [++memblockIter];
    i = 0;
    while (i < totalClasses)
    {
        int j = 0;
        int len = memblock[++memblockIter];
        int classSize = 0;
        string className = "";
        string parentName = "";
        int n_members, n_methods;
        vector<MemberInfo*> vectorMemberInfo;
        vector<MethodInfo*> vectorMethodInfo;
        
        while (j < len)
        {
            className += char_to_string ((char)memblock [++memblockIter]);
            j++;
        }
        
        classSize = convert_byte_array_to_integer (memblock + memblockIter + 1, 
                                                   isLittleEndian);
        memblockIter += 4;
        len = memblock [++memblockIter];
        parentName = read_string_from_byte_array (memblock + memblockIter, len);
        memblockIter += len;
        j = 0;
        n_members = memblock [++memblockIter];

        /* Read all members of this class */
        while (j < n_members)
        {
            int firstinfo = memblock [++memblockIter];
            int typelength = memblock [++memblockIter];
            string type_name = read_string_from_byte_array (memblock + memblockIter, typelength);
            memblockIter += typelength;
            int membernamelen = memblock [++memblockIter];
            string membername = read_string_from_byte_array (memblock + memblockIter, membernamelen);
            memblockIter += membernamelen;
            vectorMemberInfo.insert (vectorMemberInfo.end (), (new MemberInfo (membername, type_name, 
                                                                (bool)(firstinfo >> 2), 
                                                                (AccessSpecifier)(firstinfo - ((firstinfo >> 2) << 2)))));
            j++;
        }
        
        /* Read all methods of this class */
        j = 0;
        n_methods = memblock [++memblockIter];
        while (j < n_methods)
        {
            vector <Local*> vectorLocals;
            vector <Instruction*> vectorInstructions;
            vector <string> vectorParamTypes;
            int firstinfo = memblock [++memblockIter];
            int typelength = memblock [++memblockIter];
            string type_name = read_string_from_byte_array (memblock + memblockIter, typelength);
            memblockIter += typelength;
            int membernamelen = memblock [++memblockIter];
            string membername = read_string_from_byte_array (memblock + memblockIter, membernamelen);
            memblockIter += membernamelen;
            
            /* Read parameters */
            int n_params = memblock [++memblockIter];
            int k = 0;
            while (k < n_params)
            {
                int paramTypeLength = memblock [++memblockIter];
                string paramType = read_string_from_byte_array (memblock + paramTypeLength, paramTypeLength);
                paramTypeLength += paramTypeLength;
                vectorParamTypes.insert (vectorParamTypes.end (), paramType);
                k++;
            }

            int n_locals = convert_byte_array_to_integer (memblock + memblockIter + 1, 
                                                          isLittleEndian);
            memblockIter += 4;
            k = 0;
            /* Reading locals */
            while (k < n_locals)
            {
                int local_number = convert_byte_array_to_integer (memblock + memblockIter + 1, 
                                                                  isLittleEndian);
                memblockIter += 4;
                typelength = memblock [++memblockIter];
                type_name = read_string_from_byte_array (memblock + memblockIter, typelength);
                memblockIter += typelength;
                int local_size = convert_byte_array_to_integer (memblock + memblockIter + 1,
                                                                isLittleEndian);
                vectorLocals.insert (vectorLocals.end (), 
                                     (new Local (local_number, local_size, type_name)));
                memblockIter += 4;
                k++;
            }
            
            /* Reading instructions */
            long n_instructions = convert_byte_array_to_long (memblock + memblockIter + 1, 
                                                              isLittleEndian);
            memblockIter += 8;
            k = 0;
            while (k < n_instructions)
            {
                byte byteCode = memblock [++memblockIter];
                byte op_size = memblock [++memblockIter];
                string op = read_string_from_byte_array (memblock + memblockIter, op_size);
                memblockIter += op_size;
                
                //printf ("K %d %d %d %s\n", k, byteCode, op_size, op.c_str ());
                vectorInstructions.insert (vectorInstructions.end (),
                                           (new Instruction (byteCode, op_size, op)));
                k++;
            }

            MethodCode *methodCode = new MethodCode (vectorLocals, vectorInstructions);
            vectorMethodInfo.insert (vectorMethodInfo.end (), 
                                     (new MethodInfo (membername, type_name, (bool)(firstinfo >> 2),
                                      (AccessSpecifier)(firstinfo - ((firstinfo >> 2) << 2)),
                                       vectorParamTypes, methodCode)));
            j++;
        }
        
        vectorClassInfo.insert (vectorClassInfo.end (), new ClassInfo (className, classSize, parentName, 
                                                                       vectorMemberInfo, vectorMethodInfo));
        i++;
    }
    
    fileStream.close ();
    
    delete [] memblock;
    
    return 0;
}