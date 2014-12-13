#include "jit.h"

void JITStack::allocateStack (jit_state* _jit, int size)
{
    if (stackPointer == stackBase && stackBase == 0)
    {
        stackPointer = stackBase = jit_allocai (size);
        stackSize = size;
    }
    else
    {
        stackSize += size;
        jit_allocai (size);
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
            jit_ldxi_c (reg, JIT_FP, stackPointer);
            stackPointer -=  sizeof (char);
            break;
                                
        case UnsignedChar:
            jit_ldxi_uc (reg, JIT_FP, stackPointer);
            stackPointer -=  sizeof (unsigned char);
            break;
        
        case Short:
            jit_ldxi_s (reg, JIT_FP, stackPointer);
            stackPointer -=  sizeof (short);
            break;
        
        case UnsignedShort:
            jit_ldxi_us (reg, JIT_FP, stackPointer);
            stackPointer -= sizeof (unsigned short);
            break;
        
        case Integer:
            stackPointer -= sizeof (int);
            jit_ldxi_i (reg, JIT_FP, stackPointer);
            break;
        
        case UnsignedInteger:
            jit_ldxi_ui (reg, JIT_FP, stackPointer);
            stackPointer -= sizeof (unsigned int);
            break;
        
        case Long:
        case Reference:
            jit_ldxi_l (reg, JIT_FP, stackPointer);
            stackPointer -= sizeof (long);
            break;
        
        case Float:
            jit_ldxi_f (reg, JIT_FP, stackPointer);
            stackPointer -= sizeof (float);
            break;
        
        case Double:
            jit_ldxi_d (reg, JIT_FP, stackPointer);
            stackPointer -= sizeof (double);
            break;
        
        default:
            break;
    }
}