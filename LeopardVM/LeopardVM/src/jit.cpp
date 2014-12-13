#include "jit.h"
#include <stdio.h>
#include <string>
#include <cstdlib>

#define INT_DEFAULT_VALUE 0
#define FLOAT_DEFAULT_VALUE 0.0

static jit_state* _jit;

static OperatorType getOperatorTypeFromString (string type)
{
    if (type == "char")
    {
        return SignedChar;
    }
    else if (type == "unsigned char")
    {
        return UnsignedChar;
    }
    else if (type == "short")
    {
        return Short;
    }
    else if (type == "unsigned short")
    {
        return UnsignedShort;
    }
    else if (type == "int")
    {
        return Integer;
    }
    else if (type == "unsigned int")
    {
        return UnsignedInteger;
    }
    else if (type == "long")
    {
        return Long;
    }
    else if (type == "float")
    {
        return Float;
    }
    else if (type == "double")
    {
        return Double;
    }
    else
    {
        return Reference;
    }
}

static bool isIntegerType (OperatorType type)
{
    if (type == Float || type == Double)
    {
        return false;
    }
    
    return true;
}

JIT ()
{
    vectorIntRegisters.add (new RegisterDescriptor (JIT_R0));
    vectorIntRegisters.add (new RegisterDescriptor (JIT_R1));
    vectorIntRegisters.add (new RegisterDescriptor (JIT_R2));
    vectorIntRegisters.add (new RegisterDescriptor (JIT_V0));
    vectorIntRegisters.add (new RegisterDescriptor (JIT_V1));
    vectorIntRegisters.add (new RegisterDescriptor (JIT_V2));

    vectorFloatRegisters.add (new RegisterDescriptor (JIT_F0));
    vectorFloatRegisters.add (new RegisterDescriptor (JIT_F1));
    vectorFloatRegisters.add (new RegisterDescriptor (JIT_F2));
    vectorFloatRegisters.add (new RegisterDescriptor (JIT_F3));
    vectorFloatRegisters.add (new RegisterDescriptor (JIT_F4));
    vectorFloatRegisters.add (new RegisterDescriptor (JIT_F5));
}

void JIT::allocateMemory (VariableDescriptor *varDesc)
{
    LocalDescriptor* localDesc = dynamic_cast <LocalDescriptor> (varDesc);
    
    if (localDesc != null)
    {
        /* Locals are assigned memory at the start of the procedure only 
           So, we can spill as many registers as we want :D :D :D */
        OperatorType type;
        
        type = getOperatorTypeFromString (localDesc->localVar->getType ());
        jit_movi (JIT_R0, INT_DEFAULT_VALUE);
        jit_movi (JIT_F0, FLOAT_DEFAULT_VALUE);
        
        localDesc->setCurrentLocation (MemoryLocation, jitStack->getPointer ());
        localDesc->setMemLocation (jitStack->getPointer ());

        if (isIntegerType (type))
        {
            jitStack->stackPush (_jit, JIT_R0, type);
        }
        else
        {
            jitStack->stackPush (_jit, JIT_F0, type);
        }
    }
    else
    {
        TempDescriptor* tempDesc = dynamic_cast <TempDescriptor> (varDesc);
        
        
    }
}

void JIT::allocateRegister (VariableDescriptor *varDesc)
{
    /* First search through available registers and see if any is available */
    if (isIntegerType (getOperatorTypeFromString (varDesc->type)))
    {
        for (int i = 0; i < vectorIntRegisters.size (); i++)
        {
            if (vectorIntRegisters [i]->getVarDescriptor () == null)
            {
                /* Load Value in the register */
                vectorIntRegisters [i]->assignVar (varDesc);
                
                return;
            }
        }
    }
}

void JIT::convertCode (MethodCode *code)
{
    int size = 0;

    /* Get total size of locals */
    for (int i = 0; i < code->getTotalLocals (); i++)
    {
        size += code->getLocal (i)->getSize ();
    }

    /* Allocate total local size */
    jitStack-> allocateStack (_jit, size);
    
    /* Assign memory to locals */
    for (int i = 0; i < code->getTotalLocals (); i++)
    {
        LocalDescriptor *localDesc;
        
        localDesc = new LocalDescriptor (currLoc (MemoryLocation, -1),
                                         code->getLocal (i));
        vectorLocalDescriptors.insert (vectorLocalDescriptors.end (), localDesc);
        allocateMemory (localDesc);
    }

    for (int i = 0; i < code->getTotalInstructions (); i++)
    {
        if (code->getInstruction (i)->getByteCode () == 3)
        {
            /* push.b */
            jit_movi (JIT_R0, 
                      atoi (code->getInstruction (i)->getOp ().c_str ()));
            jitStack->stackPush (_jit, JIT_R0, SignedChar);
        }
        else if (code->getInstruction (i)->getByteCode () == 4)
        {
            /* push.s */
            jit_movi (JIT_R0, 
                      atoi (code->getInstruction (i)->getOp ().c_str ()));
            jitStack->stackPush (_jit, JIT_R0, Short);
        }
        else if (code->getInstruction (i)->getByteCode () == 5)
        {
            /* push.i */
            jit_movi (JIT_R0, 
                      atoi (code->getInstruction (i)->getOp ().c_str ()));
            jitStack->stackPush (_jit, JIT_R0, Integer);
        }
        else if (code->getInstruction (i)->getByteCode () == 6)
        {
            /* push.l */
            jit_movi (JIT_R0, 
                      atoi (code->getInstruction (i)->getOp ().c_str ()));
            jitStack->stackPush (_jit, JIT_R0, Long);
        }
    }
}

int JIT::runMethodCode (MethodCode *code)
{
    /* Before a function call Push all Registers on stack */
    /* Before returning pop all registers from stack */
    pvfi myFunction;       
    jit_node_t *in;                 
    
    jitStack = new JITStack ();
    
    _jit = jit_new_state();

    jit_prolog();
    convertCode (code);
    jit_pushargi((jit_word_t)" %d ll\n");
    jit_ellipsis();
    jit_pushargr(JIT_R0);
    jit_finishi((jit_pointer_t)printf);
    jit_ret();
    jit_epilog();
      
    myFunction = (pvfi)jit_emit();

    myFunction();
    jit_clear_state();
    
    jit_destroy_state();
    finish_jit();
    
    delete jitStack;
    return 0;
}