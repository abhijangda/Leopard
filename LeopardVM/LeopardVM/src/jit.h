extern "C"
{
    #include <lightning.h>
}

#include <stack>
#include <map>

#include "classinfo.h"
#include "basic_block.h"

#ifndef __JIT_H__
#define __JIT_H__

#define null 0
#define LULL 0

typedef void (*compiledFunctionSig)();

enum LocationType
{
    RegisterLocation,
    MemoryLocation
};

class VariableDescriptor;
class TempDescriptor;
class LocalDescriptor;

class JITStack
{
    private:
        stack<VariableDescriptor> varStack;
        int stackPointer;
        int stackBase;
        int stackSize;
        int stackTempPointer;

    public:
        JITStack () : varStack ()
        {
            stackPointer = 0;
            stackBase = 0;
            stackSize = 0;
        }
    
        void stackPush (jit_state* _jit, int reg, OperatorType type);
        void stackPop (jit_state* _jit, int reg, OperatorType type);
        int allocateStack (jit_state* _jit, int size);
        void copyMemToReg (jit_state* _jit, int loc, int reg, OperatorType type);
        void copyMemiToReg (jit_state* _jit, unsigned long loc, int reg, OperatorType type);
        void copyMemrToReg (jit_state* _jit, int loc, int reg, OperatorType type);
        void copyMemxrToReg (jit_state* _jit, int loc, int loc2, int reg, OperatorType type);
        void copyMemxiToReg (jit_state* _jit, int locreg, int loc, int reg, OperatorType type);
        void allocateTemporary (jit_state *_jit, TempDescriptor* tempDesc);
        void copyRegToMem (jit_state* _jit, int loc, int reg, OperatorType type);
        void copyRegToMemxr (jit_state* _jit, int loc, int reg, OperatorType type);
        void copyRegToMemr (jit_state* _jit, int loc, int reg, OperatorType type);
        void copyRegToMemi (jit_state* _jit, unsigned long loc, int reg, OperatorType type);

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
    
        void spill (jit_state *_jit, JITStack *jitStack);

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
        unsigned long memLocation;
        VarType type;
        CurrentLocation currLoc;
    
    protected:
        VariableDescriptor (int size, VarType type, CurrentLocation location, long memLoc = -1) : 
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
        
        CurrentLocation getCurrLocation ()
        {
            return currLoc;
        }

        void setCurrLocation (LocationType type, int value)
        {
            currLoc.set (type, value);
        }
    
        void setMemLocation (unsigned long memLoc)
        {
            memLocation = memLoc;
        }
    
        long getMemLoc ()
        {
            return memLocation;
        }
    
        VarType getType ()
        {
            return type;
        }
            
        static VariableDescriptor* copyVarDesc (VariableDescriptor* varDesc);

        virtual ~VariableDescriptor (){}
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
        
        Local* getLocalVariable ()
        {
            return localVar;
        }
    
        virtual ~LocalDescriptor (){}
};

class TempDescriptor : public VariableDescriptor
{
    public:
        TempDescriptor (int size, VarType type, CurrentLocation currLoc, int memLoc = -1) : 
            VariableDescriptor (size, type, currLoc, memLoc)
        {
        }
        
        virtual ~TempDescriptor (){}
};

class JITLabel
{
    private:
        jit_node_t* node;
        string labelStr;
    
    public:
        JITLabel (jit_node_t* node, string labelStr)
        {
            this->node = node;
            this->labelStr = labelStr;
        }
        
        jit_node_t *getJITNode ()
        {
            return node;
        }
        
};

class JIT
{
    private:
        MethodCode* currentCode;
        JITStack* jitStack;
        vector<VariableDescriptor*>* varStack;
        vector<VariableDescriptor*>* stackBeforeAlloc;
        vector<VariableDescriptor*>* rootVarStack;
        stack<vector<VariableDescriptor*>* > varStackStack;
        vector <RegisterDescriptor*> vectorIntRegisters;
        vector <RegisterDescriptor*> vectorFloatRegisters;
        vector <LocalDescriptor*> vectorLocalDescriptors;
        vector <TempDescriptor*> vectorTempDescriptors;
        vector<VariableDescriptor*>* vectorArgs;
        vector<BasicBlock*> vectorBasicBlocks;
        long* argsMemLocation;
        map <string, JITLabel*> mapLabels;
        unsigned long *stackPointerMem;
        unsigned long *prevStackPointerMem;

        void allocateMemory (VariableDescriptor *varDesc);
        bool allocateRegister (VariableDescriptor *varDesc);
        void _allocateRegister (VariableDescriptor *varDesc, RegisterDescriptor* regDesc);
        void copyToMemory (VariableDescriptor *varDesc);
        TempDescriptor* createTempDescriptor (int size, OperatorType type, string value, string classType);
        void processPushInstr (int size, OperatorType type, string op);
        void processArithInstr (jit_code_t code_i, jit_code_t code_f, jit_code_t code_d);
        void processBranchInstr (string label, jit_code_t code_i, jit_code_t code_f, jit_code_t code_d);
        void copyVariables (VariableDescriptor *src, VariableDescriptor *dest);
        void storeVariablesToMemory ();
        void restoreVariablesFromMemory ();
        vector<VariableDescriptor*>* getVarStackBeforeAllocation ()
        {
            return stackBeforeAlloc;
        }

        compiledFunctionSig compiledFunction;
        unsigned long memForBasicBlocks;
        void endBasicBlock (int& startBlockInstr, int& endBlockInstr, Instruction* instr);

    public:
        JIT ();
        int runMethodCode (vector<VariableDescriptor*>* vectorArgs, MethodCode* code, 
                           unsigned long *stackPointerMem);
        void convertCode (vector<VariableDescriptor*>* vectorArgs, MethodCode* code);
        jit_state* _jit;
        //static VariableDescriptor* copyVarDesc (VariableDescriptor* varDesc);
        vector<LocalDescriptor*>* getLocalDescriptors ()
        {
            return &vectorLocalDescriptors;
        }
        
        vector<VariableDescriptor*>* getArgumentDescriptors ()
        {
            return vectorArgs;
        }

        unsigned long *getStackPointerMem ()
        {
            return stackPointerMem;
        }
        
        ~JIT ()
        {
            jit_clear_state();
            jit_destroy_state();
            finish_jit();
            delete argsMemLocation;
            delete stackPointerMem;
        }
    
        void callCompiledFunction ()
        {
            compiledFunction ();
        }
};

#endif