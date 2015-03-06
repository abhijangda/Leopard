#include <fstream>
#include <vector>
#include <stdio.h>
#include <sstream>
#include <stdexcept>

#include "helper_functions.h"
#include "vm.h"

using namespace std;
extern VirtualMachine* ptrVM;
extern JIT* currentJIT;
extern unsigned long arrayAddress;

void VirtualMachine::doGarbageCollection ()
{
    gc.collectGarbage ();
}

void VirtualMachine::markAllObjectsUnreachable ()
{
    map<unsigned long, AllocatedObject*>::iterator iter;
    
    for (iter = mapAllocatedObject.begin (); 
         iter != mapAllocatedObject.end (); ++iter)
    {
        iter->second->unsetReachable ();
        iter->second->clearAllAddress ();
    }
}

void VirtualMachine::getReachableForAddress (unsigned long address, 
                                             vector<AllocatedObject*>& vec)
{
    /*AllocatedObject *obj;
    ClassInfo *classInfo;
    ClassInfo *_classInfo;
    list<ClassInfo*> listClassInfos;
    list<ClassInfo*>::iterator listIter;
    unsigned long pos = 0;

    obj = mapAllocatedObject.at (address);
    vec.push_back (obj);
    classInfo = obj->getClassInfo ();
    
    while (_classInfo != LULL)
    {
        listClassInfos.push_back (_classInfo);
    }

    listIter = listClassInfos.begin ();
    
    while (listIter != listClassInfos.end ())
    {
        for (int i = 0; i < (*listIter)->totalMembers (); i++)
        {
            
        }

        listIter++;
    }*/
}
        
void VirtualMachine::callMethod (vector<VariableDescriptor*>* vectorArgs, 
                                 vector<VariableDescriptor*>* stackVarDesc,
                                 ClassInfo *classInfo, MethodInfo *methodInfo)
{
    unsigned long* prev = ptrVM->getCurrentJIT ()->getStackPointerMem ();
    unsigned long* objectAddress = ptrVM->getcalledObjectAddressMem ();
    
    /* Push the caller's stack */
    ptrVM->pushCallerStack (stackVarDesc);
    AllocatedObject *allocObj;
    ClassInfo *allocObjClassInfo;
    JIT* jit;

    if (!methodInfo->getIsStatic ())
    {
        /* We need to find two things here:
           1. Find the virtual method
           2. Find the parent class method
    
        Check if the allocObj's Class is same as the supplied class
        If yes then this class is the required class and the method
            is the required method
        If no then if allocObjClassInfo is the subclass of the classInfo then
            There is either a virtual method call or a inherited method call.
            But inherited method call has been resolved earlier.
            Else Error
        */
        allocObj = ptrVM->mapAllocatedObject [*objectAddress];
        allocObjClassInfo = allocObj->getClassInfo ();
        
        if (classInfo != allocObjClassInfo)
        {
            ClassInfo* _classInfo;
            
            _classInfo = allocObjClassInfo->getParentClassInfo ();
    
            while (_classInfo != LULL && (_classInfo != classInfo))
            {
                _classInfo = _classInfo->getParentClassInfo ();
            }
        
            if (_classInfo == LULL)
            {
                /* ERROR: Invalid Class Name supplied */
            }
            else
            {
                /* As classInfo is parent of allocObjClassInfo, so the call is virtual */
                /* Go up the parents of the allocObjClassInfo and search for the same 
                 * method name 
                 */
                
                _classInfo = allocObjClassInfo;
    
                while (_classInfo != LULL)
                {
                    bool toBreak = false;
    
                    for (int i = 0; i < _classInfo->totalMethods (); i++)
                    {
                        if (_classInfo->getMethodInfo (i)->getName () ==
                            methodInfo->getName ())
                        {
                            methodInfo = _classInfo->getMethodInfo (i);
                            toBreak = true;
                            break;
                        }
                    }
    
                    if (toBreak)
                    {
                        break;
                    }
    
                    _classInfo = _classInfo->getParentClassInfo ();
                }
            }
        }
    }

    try
    {
        /* Search if the code has been already compiled by a JIT 
         * If yes then use it */
        jit = ptrVM->getJITForMethodInfo (methodInfo);
        ptrVM->pushJIT (jit);
    }
    catch (const std::out_of_range& _a)
    {
        /* If no then create a new JIT */
        jit = ptrVM->pushJIT ();
        ptrVM->addJITForMethodInfo (methodInfo, jit);
    }

    
    jit->runMethodCode (vectorArgs, methodInfo->getCode (), prev);
    ptrVM->popJIT ();

    for (int i = 0; i < vectorArgs->size (); i++)
    {
        delete vectorArgs [0][i];
    }

    delete vectorArgs;
    /* Pop the caller's stack */
    ptrVM->popCallerStack ();
}

MethodInfo *VirtualMachine::getMethodInfoOfClass (string cname, string mname)
{
    ClassInfo *classInfo = getClassInfoForName (cname);
    
    while (classInfo)
    {
        for (int i = 0; i < classInfo->totalMethods (); i++)
        {
            if (classInfo->getMethodInfo (i)->getName () == mname)
            {
                return classInfo->getMethodInfo (i);
            }
        }
        
        classInfo = classInfo->getParentClassInfo ();
    }
    
    return LULL;
}

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

unsigned int VirtualMachine::getPosForField (string cname, string fname, MemberInfo** type)
{
    ClassInfo *classInfo;
    int size;
    unsigned long pos = 0;
    ClassInfo* _classInfo;

    classInfo = getClassInfoForName (cname);
    
    /* Field could be in a parent class. Find the class containing field */
    while (classInfo != LULL)
    {
        bool toBreak = false;

        for (int i = 0; i < classInfo->totalMembers (); i++)
        {
            if (classInfo->getMemberInfo (i)->getName () == fname)
            {
                if (type)
                {
                    *type = classInfo->getMemberInfo (i);
                }

                toBreak = true;
                break;
            }
        }

        if (toBreak)
        {
            break;
        }

        classInfo = classInfo->getParentClassInfo ();
    }

    if (classInfo->getParentClassInfo ())
    {
        pos = classInfo->getSize ();
    }

    if ((*type)->getIsStatic () == true)
    {
        return ptrVM->getAllocObjForStaticMember (*type)->getMemBlock ()->getStartPos ();
    }

    for (int i = 0; i < classInfo->totalMembers (); i++)
    {
        if (classInfo->getMemberInfo (i)->getName () == fname)
        {
            break;
        }
        
        OperatorType optype;

        optype = getOperatorTypeFromString (classInfo->getMemberInfo (i)->getType ());
        pos += getSizeFromOperatorType (optype);
    }
    
    return pos;
}

AllocatedObject* VirtualMachine::_allocateObject (vector<VariableDescriptor*>* stackVarDesc,
                                                  string type)
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

        if (opTypeChild == Reference && !classInfo->getMemberInfo (i)->getIsStatic ())
        {
            AllocatedObject *obj;
            byte *mem;

            obj = _allocateObject (stackVarDesc, classInfo->getMemberInfo (i)->getType ());
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

    ptrVM->getGarbageCollector ()->pushObject (allocatedObject);

    return allocatedObject;
}

/* Create an object of class and returns the address of the object */
unsigned long VirtualMachine::allocateObject (vector<VariableDescriptor*>* stackVarDesc, char* type)
{
    string stype (type);
    
    /* Push current method's stack */
    ptrVM->pushCallerStack (stackVarDesc);
    AllocatedObject *obj = ptrVM->_allocateObject (stackVarDesc, stype);

    /* Pop the current method's stack */
    ptrVM->popCallerStack ();

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

RootSet* VirtualMachine::getRootSet ()
{
    stack <JIT*>* _stackJIT;

    _stackJIT = new stack<JIT*> (stackJIT);
    
    /*while (_stackJIT.size () != 0)
    {
        vector<LocalDescriptor*> vectorLocalDesc;
        
        vectorLocalDesc = *_stackJIT.top()->getLocalDescriptors ();

        for (int i = 0; i < vectorLocalDesc.size (); i++)
        {
            rootSet.push_back (vectorLocalDesc [i]);
        }

        vector<VariableDescriptor*> vectorVarDesc;
        
        vectorVarDesc = *_stackJIT.top()->getArgumentDescriptors ();

        for (int i = 0; i < vectorVarDesc.size (); i++)
        {
            rootSet.push_back (vectorVarDesc [i]);
        }

        _stackJIT.pop ();
    }*/

    vector<AllocatedObject*>* vecStaticAllocatedObj;
    
    vecStaticAllocatedObj = new vector<AllocatedObject*> ();
    map<MemberInfo*, AllocatedVariable*>::iterator iter;

    for (iter = mapStaticMembersAllocated.begin ();
         iter !=  mapStaticMembersAllocated.end ();
         ++iter)
    {
        if (getOperatorTypeFromString (iter->first->getType ()) == Reference)
        {
            vecStaticAllocatedObj->push_back (dynamic_cast <AllocatedObject*> (iter->second));
        }
    }
    
    return new RootSet (&vectorStackVarDesc, _stackJIT, vecStaticAllocatedObj);
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
            for (int j = 0; j < vectorClassInfo [i]->totalMethods (); j++)
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

    /* Initialize and allocate storage to the static members of all classes */
    for (int i = 0; i < vectorClassInfo.size (); i++)
    {
        for (int j = 0; j < vectorClassInfo[i]->totalMembers (); j++)
        {
            if (vectorClassInfo[i]->getMemberInfo (j)->getIsStatic ())
            {
                vectorStaticMembers.push_back (vectorClassInfo[i]->getMemberInfo (j));
                allocateStaticMember (vectorClassInfo[i], 
                                      vectorClassInfo[i]->getMemberInfo (j));
            }
        }
    }

    stackJIT.push (jit);
    mapJITForMethod [mainMethodInfo] = jit;
    jit->runMethodCode (new vector<VariableDescriptor*> (), 
                        mainMethodInfo->getCode (), NULL);
}

AllocatedVariable* VirtualMachine::allocateStaticMember (ClassInfo *classInfo, 
                                                       MemberInfo *memberInfo)
{
    /* Allocate the static member */

    string type = memberInfo->getType ();
    OperatorType optype = getOperatorTypeFromString (type);
    int size = getSizeFromOperatorType (optype);
    MemoryBlock *memBlock;

    if (optype == Reference)
    {
        AllocatedObject *allocatedObject;

        allocatedObject = _allocateObject (LULL, type);
        /* Store the allocated objects according to their start positions, keys */
        mapStaticMembersAllocated [memberInfo] = allocatedObject;

        return allocatedObject;
    }
    else
    {
        AllocatedPrimitive *allocatedObject;

        memBlock = getHeapAllocator ()->allocate (size);
        allocatedObject = new AllocatedPrimitive (type, memBlock);
        /* Store the allocated objects according to their start positions, keys */
        mapStaticMembersAllocated [memberInfo] = allocatedObject;
        
        return allocatedObject;
    }
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
        ClassInfo* parent = LULL;
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
                                                                (bool)(firstinfo & 0x1), 
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
                string paramType = read_string_from_byte_array (memblock + memblockIter, paramTypeLength);
                memblockIter += paramTypeLength;
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
                string type_name = read_string_from_byte_array (memblock + memblockIter, typelength);
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
                                     (new MethodInfo (membername, type_name, (bool)(firstinfo & 0x1),
                                      (AccessSpecifier)(firstinfo - ((firstinfo >> 2) << 2)),
                                       vectorParamTypes, methodCode)));
            
            j++;
        }

        if (parentName != "None")
        {
            parent = getClassInfoForName (parentName);
        }
        else
        {
            parent = LULL;
        }

        vectorClassInfo.insert (vectorClassInfo.end (), new ClassInfo (className, classSize, parent, 
                                                                       vectorMemberInfo, vectorMethodInfo));
        if (parent != LULL)
        {
            parent->addChildClass (vectorClassInfo [vectorClassInfo.size () - 1]);
        }

        i++;
    }
    
    fileStream.close ();
    
    delete [] memblock;
    
    return 0;
}