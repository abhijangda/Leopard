extern "C"
{
    #include <lightning.h>
}

#include <stack>

#include "classinfo.h"

#ifndef __JIT_H__
#define __JIT_H__

typedef void (*pvfi)();

enum OperatorType
{
    SignedChar,
    UnsignedChar,
    Short,
    UnsignedShort,
    Integer,
    UnsignedInteger,
    Long,
    Float,
    Double,
    Reference,
};

typedef enum OperatorType OperatorType;

enum LocationType
{
    RegisterLocation,
    MemoryLocation
};

class VariableDescriptor;

class JITStack
{
    private:
        stack<VariableDescriptor> varStack;
        int stackPointer;
        int stackBase;
        int stackSize;

    public:
        JITStack () : varStack ()
        {
            stackPointer = 0;
            stackBase = 0;
            stackSize;
        }
    
        void stackPush (jit_state* _jit, int reg, OperatorType type);
        void stackPop (jit_state* _jit, int reg, OperatorType type);
        void allocateStack (jit_state* _jit, int size);

        int getPointer () 
        {
            return stackPointer;
        }
        
        int getBase ()
        {
            return stackBase;
        }
};

class CurrentLocation
{
    private:
        LocationType type;
        int value;

    public:
        CurrentLocation (LocationType type, int value)
        {
            this->type = type;
            this->value = value;
        }
    
        int getValue ()
        {
            return value;
        }
        
        LocationType getLocationType ()
        {
            return type;
        }
    
        void set (LocationType type, int value)
        {
            this->type = type;
            this->value = value;
        }
};

class RegisterDescriptor
{
    private:
        VariableDescriptor* assignedVar;
        int reg;
        
    public:
        RegisterDescriptor (int reg)
        {
            this->reg = reg;
            assignedVar = null;
        }
    
        int getRegister ()
        {
            return reg;
        }
        
        VariableDescriptor *getVarDescriptor ()
        {
            return assignedVar;
        }
    
        void assignVariable (VariableDescriptor *var)
        {
            assignedVar = var;
        }
};

typedef string VarType;

class VariableDescriptor
{
    private:
        int size;
        int memLocation;
        VarType type;
        CurrentLocation currLoc;

    protected:
        VariableDescriptor (int size, VarType type, CurrentLocation location, int memLoc = -1) : 
            currLoc (location.getLocationType (), location.getValue ())
        {
            this->size = size;
            this->memLocation = memLoc;
            this->type = type;
        }
    
    public:
        int getSize () 
        {
            return size;
        }
    
        void setCurrLocation (LocationType type, int value)
        {
            currLoc.set (type, value);
        }
    
        void setMemLocation (int memLoc)
        {
            memLocation = memLoc;
        }
};

class LocalDescriptor : public VariableDescriptor
{
    private:
        Local* localVar;

    public:
        LocalDescriptor (CurrentLocation currLoc, Local* local, int memLoc = -1) : 
            VariableDescriptor (local->getSize (), local->getType (), currLoc, memLoc)
        {
            localVar = local;
        }
};

class TempDescriptor : public VariableDescriptor
{
    public:
        TempDescriptor (int size, VarType type, CurrentLocation currLoc, int memLoc = -1) : 
            VariableDescriptor (size, type, currLoc, memLoc)
        {
        }
};

class JIT
{
    private:
        MethodCode* currentCode;
        JITStack* jitStack;
        vector <RegisterDescriptor*> vectorIntRegisters;
        vector <RegisterDescriptor*> vectorFloatRegisters;
        vector <LocalDescriptor*> vectorLocalDescriptors;

        void allocateMemory (VariableDescriptor *varDesc);
        void allocateRegister (VariableDescriptor *varDesc);
        void copyToMemory (VariableDescriptor *varDesc);

    public:
        JIT ();
        int runMethodCode (MethodCode* code);
        void convertCode (MethodCode* code);
};

#endif