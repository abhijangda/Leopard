#include <fstream>
#include <vector>
#include <stdio.h>
#include <sstream>

#include "helper_functions.h"
#include "vm.h"

using namespace std;
extern VirtualMachine* ptrVM;
extern JIT* currentJIT;
extern unsigned long arrayAddress;

/* Returns the address of the allocated object */
unsigned long VirtualMachine::allocateArray (char* type, int size)
{
    string stype (type);
    OperatorType optype = getOperatorTypeFromString (stype);
    int opsize = getSizeFromOperatorType (optype);

    if (optype == Reference)
    {
        /* A Class has to be allocated */
        return 0;
    }
    
    MemoryBlock *memBlock = ptrVM->getHeapAllocator ()->allocate (opsize*size);
    arrayAddress = memBlock->getStartPos();

    return memBlock->getStartPos ();
}

AllocatedObject *VirtualMachine::getAllocatedObjectForStartPos (unsigned long startPos)
{
    return mapAllocatedObject [startPos];
}

unsigned int VirtualMachine::getPosForField (string cname, string fname, string* type)
{
    ClassInfo *classInfo;
    int size;
    unsigned pos = 0;

    classInfo = getClassInfoForName (cname);

    for (int i = 0; i < classInfo->totalMembers (); i++)
    {
        if (classInfo->getMemberInfo (i)->getName () == fname)
        {
            if (type)
            {
                *type = classInfo->getMemberInfo (i)->getType ();
            }

            break;
        }
        
        OperatorType optype;

        optype = getOperatorTypeFromString (classInfo->getMemberInfo (i)->getType ());
        pos += getSizeFromOperatorType (optype);
    }
    
    return pos;
}

AllocatedObject* VirtualMachine::_allocateObject (string type)
{
    ClassInfo *classInfo;
    int size;
    AllocatedObject *allocatedObject;
    MemoryBlock *memBlock;
    int pos = 0;

    classInfo = getClassInfoForName (type);
    size = classInfo->getSize ();
    memBlock = getHeapAllocator ()->allocate (size);
    allocatedObject = new AllocatedObject (classInfo, memBlock);
    /* Store the allocated objects according to their start positions, keys */
    mapAllocatedObject [memBlock->getStartPos ()] = allocatedObject;

    for (int i = 0; i < classInfo->totalMembers (); i++)
    {
        OperatorType opTypeChild;
        
        opTypeChild = getOperatorTypeFromString (classInfo->getMemberInfo (i)->getType ());

        if (opTypeChild == Reference)
        {
            AllocatedObject *obj;
            byte *mem;

            obj = _allocateObject (classInfo->getMemberInfo (i)->getType ());
            mem = memBlock->getMemory () + pos;
            unsigned long memstart = obj->getMemBlock ()->getStartPos ();
            unsigned long *lmem = (unsigned long *)mem;
            /* Set the obtained pointer of child object to its 
             * corresponding field in parent
             */
            lmem [0] = memstart;
            allocatedObject->addChild (obj);
        }
    
        pos += getSizeFromOperatorType (opTypeChild);
    }

    return allocatedObject;
}

/* Create an object of class and returns the address of the object */
unsigned long VirtualMachine::allocateObject (char* type)
{
    string stype (type);
    AllocatedObject *obj = ptrVM->_allocateObject (stype);
    
    return obj->getMemBlock()->getStartPos ();
}

ClassInfo *VirtualMachine::getClassInfoForName (string name)
{    
    for (int i = 0; i < vectorClassInfo.size (); i++)
    {
        if (vectorClassInfo [i]->getName () == name)
        {
            return vectorClassInfo[i];
        }
    }

    return LULL;
}

int VirtualMachine::start (string filename)
{
    /* Process file */
    if (read (filename) != 0)
        return -1;
    
    /* Start the main function */
    MethodInfo* mainMethodInfo;

    for (int i = 0; i < vectorClassInfo.size (); i++)
    {
        //printf ("Names %s\n", vectorClassInfo[i]->getName().c_str());
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
    isLittleEndian = 0;
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
        //printf ("N %s S %d\n", className.c_str (), classSize);
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
            //printf ("%s type name\n", type_name.c_str ());
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
                int bracket_pos = 0;

                if ((bracket_pos = type_name.find ('[')) != string::npos)
                {
                    string _type_name = type_name;
                    int n_elements = 0;

                    type_name = _type_name.substr (0, bracket_pos);
                    n_elements = convert_string_to_int (_type_name.substr (bracket_pos + 1, 
                                                                           _type_name.find (']') - bracket_pos - 1));
                    vectorLocals.insert (vectorLocals.end (),
                                         (new LocalArray (local_number, local_size, type_name, n_elements)));
                }
                else
                {
                    vectorLocals.insert (vectorLocals.end (), 
                                         (new Local (local_number, local_size, type_name)));
                }

                memblockIter += 4;
                k++;
            }
        
            //printf ("LOCALS %d\n", vectorLocals.size ());
            
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