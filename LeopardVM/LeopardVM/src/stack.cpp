#include "jit.h"
#include <stdio.h>

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

void JITStack::copyMemrToReg (jit_state* _jit, int loc, int reg, OperatorType type)
{
    switch (type)
    {
        case SignedChar:
            jit_ldr_c (reg, loc);
            break;
                                
        case UnsignedChar:
            jit_ldr_uc (reg, loc);
            break;
        
        case Short:
            jit_ldr_s (reg, loc);
            break;
        
        case UnsignedShort:
            jit_ldr_us (reg, loc);
            break;
        
        case Integer:
            jit_ldr_i (reg, loc);
            break;
        
        case UnsignedInteger:
            jit_ldr_ui (reg, loc);
            break;
        
        case Long:
        case Reference:
            jit_ldr_l (reg, loc);
            break;
        
        case Float:
            jit_ldr_f (reg, loc);
            break;
        
        case Double:
            jit_ldr_d (reg, loc);
            break;
        
        default:
            break;
    }
}

void JITStack::copyMemxrToReg (jit_state* _jit, int loc, int loc2, int reg, OperatorType type)
{
    switch (type)
    {
        case SignedChar:
            jit_ldxr_c (reg, loc, loc2);
            break;
                                
        case UnsignedChar:
            jit_ldxr_uc (reg, loc, loc2);
            break;
        
        case Short:
            jit_ldxr_s (reg, loc, loc2);
            break;
        
        case UnsignedShort:
            jit_ldxr_us (reg, loc, loc2);
            break;
        
        case Integer:
            jit_ldxr_i (reg, loc, loc2);
            break;
        
        case UnsignedInteger:
            jit_ldxr_ui (reg, loc, loc2);
            break;
        
        case Long:
        case Reference:
            jit_ldxr_l (reg, loc, loc2);
            break;
        
        case Float:
            jit_ldxr_f (reg, loc, loc2);
            break;
        
        case Double:
            jit_ldxr_d (reg, loc, loc2);
            break;
        
        default:
            break;
    }
}

void JITStack::copyMemxiToReg (jit_state* _jit, int locreg, int loc, int reg, OperatorType type)
{
    switch (type)
    {
        case SignedChar:
            jit_ldxi_c (reg, locreg, loc);
            break;
                                
        case UnsignedChar:
            jit_ldxi_uc (reg, locreg, loc);
            break;
        
        case Short:
            jit_ldxi_s (reg, locreg, loc);
            break;
        
        case UnsignedShort:
            jit_ldxi_us (reg, locreg, loc);
            break;
        
        case Integer:
            jit_ldxi_i (reg, locreg, loc);
            break;
        
        case UnsignedInteger:
            jit_ldxi_ui (reg, locreg, loc);
            break;
        
        case Long:
        case Reference:
            jit_ldxi_l (reg, locreg, loc);
            break;
        
        case Float:
            jit_ldxi_f (reg, locreg, loc);
            break;
        
        case Double:
            jit_ldxi_d (reg, locreg, loc);
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

void JITStack::copyRegToMemr (jit_state* _jit, int loc, int reg, OperatorType type)
{
    switch (type)
    {
        case SignedChar:
            jit_str_c (loc, reg);
            break;
                                
        /*case UnsignedChar:
            jit_stxr_uc (loc, JIT_FP, reg);
            stackPointer += sizeof (unsigned char);
            break;
        */
        case Short:
            jit_str_s (loc, reg);
            break;
        
        /*case UnsignedShort:
            jit_stxirus (loc, JIT_FP, reg);
            break;
        */
        case Integer:
            jit_str_i (loc, reg);
            return;
        
        /*case UnsignedInteger:
            jit_stxr_ui (loc, JIT_FP, reg);
            break;
        */
        case Long:
        case Reference:
            jit_str_l (loc, reg);
            break;
        
        case Float:
            jit_str_f (loc, reg);
            break;
        
        case Double:
            jit_str_d (loc, reg);
            break;
        
        default:
            break;
    }
}

void JITStack::copyRegToMemxr (jit_state* _jit, int loc, int reg, OperatorType type)
{
    switch (type)
    {
        case SignedChar:
            jit_stxr_c (loc, JIT_FP, reg);
            break;
                                
        /*case UnsignedChar:
            jit_stxr_uc (loc, JIT_FP, reg);
            stackPointer += sizeof (unsigned char);
            break;
        */
        case Short:
            jit_stxr_s (loc, JIT_FP, reg);
            break;
        
        /*case UnsignedShort:
            jit_stxirus (loc, JIT_FP, reg);
            break;
        */
        case Integer:
            jit_stxr_i (loc, JIT_FP, reg);
            return;
        
        /*case UnsignedInteger:
            jit_stxr_ui (loc, JIT_FP, reg);
            break;
        */
        case Long:
        case Reference:
            jit_stxr_l (loc, JIT_FP, reg);
            break;
        
        case Float:
            jit_stxr_f (loc, JIT_FP, reg);
            break;
        
        case Double:
            jit_stxr_d (loc, JIT_FP, reg);
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