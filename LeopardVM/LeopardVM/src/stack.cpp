#include "jit.h"

int JITStack::allocateStack (jit_state* _jit, int size)
{
    if (stackPointer == stackBase && stackBase == 0)
    {
        stackPointer = stackBase = jit_allocai (size);
        stackSize = size;
        return stackPointer;
    }
    else
    {
        stackSize += size;
        return jit_allocai (size);
    }
}

void JITStack::allocateTemporary (jit_state *_jit, TempDescriptor* tempDesc)
{
    stackTempPointer = allocateStack (_jit, tempDesc->getSize ());
    tempDesc->setMemLocation (stackTempPointer);
}

void JITStack::copyMemToReg (jit_state* _jit, int loc, int reg, OperatorType type)
{
    switch (type)
    {
        case SignedChar:
            jit_ldxi_c (reg, JIT_FP, loc);
            break;
                                
        case UnsignedChar:
            jit_ldxi_uc (reg, JIT_FP, loc);
            break;
        
        case Short:
            jit_ldxi_s (reg, JIT_FP, loc);
            break;
        
        case UnsignedShort:
            jit_ldxi_us (reg, JIT_FP, loc);
            break;
        
        case Integer:
            jit_ldxi_i (reg, JIT_FP, loc);
            break;
        
        case UnsignedInteger:
            jit_ldxi_ui (reg, JIT_FP, loc);
            break;
        
        case Long:
        case Reference:
            jit_ldxi_l (reg, JIT_FP, loc);
            break;
        
        case Float:
            jit_ldxi_f (reg, JIT_FP, loc);
            break;
        
        case Double:
            jit_ldxi_d (reg, JIT_FP, loc);
            break;
        
        default:
            break;
    }
}

void JITStack::copyRegToMem (jit_state* _jit, int loc, int reg, OperatorType type)
{
    switch (type)
    {
        case SignedChar:
            jit_stxi_c (loc, JIT_FP, reg);
            break;
                                
        /*case UnsignedChar:
            jit_stxi_uc (loc, JIT_FP, reg);
            stackPointer += sizeof (unsigned char);
            break;
        */
        case Short:
            jit_stxi_s (loc, JIT_FP, reg);
            break;
        
        /*case UnsignedShort:
            jit_stxi_us (loc, JIT_FP, reg);
            break;
        */
        case Integer:
            jit_stxi_i (loc, JIT_FP, reg);
            return;
        
        /*case UnsignedInteger:
            jit_stxi_ui (loc, JIT_FP, reg);
            break;
        */
        case Long:
        case Reference:
            jit_stxi_l (loc, JIT_FP, reg);
            break;
        
        case Float:
            jit_stxi_f (loc, JIT_FP, reg);
            break;
        
        case Double:
            jit_stxi_d (loc, JIT_FP, reg);
            break;
        
        default:
            break;
    }
}

void JITStack::stackPush (jit_state* _jit, int reg, OperatorType type)
{
    switch (type)
    {
        case SignedChar:
            jit_stxi_c (stackPointer, JIT_FP, reg);
            stackPointer += sizeof (char);
            break;
                                
        /*case UnsignedChar:
            jit_stxi_uc (stackPointer, JIT_FP, reg);
            stackPointer += sizeof (unsigned char);
            break;
        */
        case Short:
            jit_stxi_s (stackPointer, JIT_FP, reg);
            stackPointer += sizeof (short);
            break;
        
        /*case UnsignedShort:
            jit_stxi_us (stackPointer, JIT_FP, reg);
            stackPointer += sizeof (unsigned short);
            break;
        */
        case Integer:
            jit_stxi_i (stackPointer, JIT_FP, reg);
            stackPointer += sizeof (int);
            return;
        
        /*case UnsignedInteger:
            jit_stxi_ui (stackPointer, JIT_FP, reg);
            stackPointer += sizeof (unsigned int);
            break;
        */
        case Long:
        case Reference:
            jit_stxi_l (stackPointer, JIT_FP, reg);
            stackPointer += sizeof (long);
            break;
        
        case Float:
            jit_stxi_f (stackPointer, JIT_FP, reg);
            stackPointer += sizeof (float);
            break;
        
        case Double:
            jit_stxi_d (stackPointer, JIT_FP, reg);
            stackPointer += sizeof (double);
            break;
        
        default:
            break;
    }
}

void JITStack::stackPop (jit_state* _jit, int reg, OperatorType type)
{
    switch (type)
    {
        case SignedChar:
            stackPointer -=  sizeof (char);
            jit_ldxi_c (reg, JIT_FP, stackPointer);
            break;
                                
        case UnsignedChar:
            stackPointer -=  sizeof (unsigned char);
            jit_ldxi_uc (reg, JIT_FP, stackPointer);
            break;
        
        case Short:
            stackPointer -=  sizeof (short);
            jit_ldxi_s (reg, JIT_FP, stackPointer);
            break;
        
        case UnsignedShort:
            stackPointer -= sizeof (unsigned short);
            jit_ldxi_us (reg, JIT_FP, stackPointer);
            break;
        
        case Integer:
            stackPointer -= sizeof (int);
            jit_ldxi_i (reg, JIT_FP, stackPointer);
            break;
        
        case UnsignedInteger:
            stackPointer -= sizeof (unsigned int);
            jit_ldxi_ui (reg, JIT_FP, stackPointer);
            break;
        
        case Long:
        case Reference:
            stackPointer -= sizeof (long);
            jit_ldxi_l (reg, JIT_FP, stackPointer);
            break;
        
        case Float:
            stackPointer -= sizeof (float);
            jit_ldxi_f (reg, JIT_FP, stackPointer);
            break;
        
        case Double:
            stackPointer -= sizeof (double);
            jit_ldxi_d (reg, JIT_FP, stackPointer);
            break;
        
        default:
            break;
    }
}