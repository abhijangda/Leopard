#include "jit.h"
#include <stdio.h>
#include <string>
#include <cstdlib>

#define INT_DEFAULT_VALUE 0
#define FLOAT_DEFAULT_VALUE 0.0
#define FLOAT_DEBUG

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

static string getStringFromOperatorType (OperatorType type)
{
    if (type == SignedChar)
    {
        return "char";
    }
    else if (type == UnsignedChar)
    {
        return "unsigned char";
    }
    else if (type == Short)
    {
        return "short";
    }
    else if (type == UnsignedShort)
    {
        return "unsigned short";
    }
    else if (type == Integer)
    {
        return "int";
    }
    else if (type == UnsignedInteger)
    {
        return "unsigned int";
    }
    else if (type == Long)
    {
        return "long";
    }
    else if (type == Float)
    {
        return "float";
    }
    else if (type == Double)
    {
        return "double";
    }
    else
    {
        return "reference";
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

void RegisterDescriptor::spill (JITStack *jitStack)
{
    if (getVarDescriptor () == null)
    {
        return;
    }

    jitStack->copyRegToMem (_jit, assignedVar->getMemLoc (), reg, 
                            getOperatorTypeFromString (assignedVar->getType ()));
    assignedVar->setCurrLocation (MemoryLocation, assignedVar->getMemLoc ());
}

JIT::JIT ()
{
    vectorIntRegisters.insert (vectorIntRegisters.end (), new RegisterDescriptor (JIT_R0));
    vectorIntRegisters.insert (vectorIntRegisters.end (), new RegisterDescriptor (JIT_R1));
    vectorIntRegisters.insert (vectorIntRegisters.end (), new RegisterDescriptor (JIT_R2));
    vectorIntRegisters.insert (vectorIntRegisters.end (), new RegisterDescriptor (JIT_V0));
    vectorIntRegisters.insert (vectorIntRegisters.end (), new RegisterDescriptor (JIT_V1));
    vectorIntRegisters.insert (vectorIntRegisters.end (), new RegisterDescriptor (JIT_V2));

    vectorFloatRegisters.insert (vectorFloatRegisters.end (), new RegisterDescriptor (JIT_F0));
    vectorFloatRegisters.insert (vectorFloatRegisters.end (), new RegisterDescriptor (JIT_F1));
    vectorFloatRegisters.insert (vectorFloatRegisters.end (), new RegisterDescriptor (JIT_F2));
    vectorFloatRegisters.insert (vectorFloatRegisters.end (), new RegisterDescriptor (JIT_F3));
    vectorFloatRegisters.insert (vectorFloatRegisters.end (), new RegisterDescriptor (JIT_F4));
    vectorFloatRegisters.insert (vectorFloatRegisters.end (), new RegisterDescriptor (JIT_F5));
}

/* This function allocates memory on stack to variable 
 * It doesn't copy register value to memory 
 */
void JIT::allocateMemory (VariableDescriptor *varDesc)
{
    LocalDescriptor* localDesc = dynamic_cast <LocalDescriptor*> (varDesc);
    
    if (localDesc != null)
    {
        /* Locals are assigned memory at the start of the procedure only 
           So, we can spill as many registers as we want :D :D :D */
        OperatorType type;
        
        type = getOperatorTypeFromString (localDesc->getLocalVariable ()->getType ());
        
        localDesc->setCurrLocation (MemoryLocation, jitStack->getPointer ());
        localDesc->setMemLocation (jitStack->getPointer ());

        if (isIntegerType (type))
        {
            jit_movi (JIT_R0, INT_DEFAULT_VALUE);
            jitStack->stackPush (_jit, JIT_R0, type);
#ifdef FLOAT_DEBUG            
            //jit_movi (JIT_R0, 11);
#endif
        }
        else
        {
            jit_movi_d (JIT_F0, FLOAT_DEFAULT_VALUE);
            jitStack->stackPush (_jit, JIT_F0, type);
        }
    }
    else
    {
        TempDescriptor* tempDesc = dynamic_cast <TempDescriptor*> (varDesc);
        
        if (tempDesc->getMemLoc () ==-1)
        {
            jitStack->allocateTemporary (_jit, tempDesc);
        }
    }
}

void JIT::_allocateRegister (VariableDescriptor *varDesc, RegisterDescriptor* regDesc)
{
    /* Found an empty register */
    /* Load Value in the register */
    regDesc->assignVariable (varDesc);
    varDesc->setCurrLocation (RegisterLocation, 
                              regDesc->getRegister ());
    /* For Local Variable */
    OperatorType type;
    
    type = getOperatorTypeFromString (varDesc->getType ());
    
    /* if getMemLoc () is -1 then, 
     * varDesc is TempDescriptor which hasn't been allocated on memory*/
    if (varDesc->getMemLoc () != -1)
    {
        jitStack->copyMemToReg (_jit, varDesc->getMemLoc (), 
                                regDesc->getRegister (), type);
    }
}

/* This allocates the register and spills the register if required 
 * returns false if variable is already in the register
 * otherwise returns true
 */
bool JIT::allocateRegister (VariableDescriptor *varDesc)
{
    static int intLastRegUsed = 0;
    static int floatLastRegUsed = 0;

    if (varDesc->getCurrLocation ().getLocationType () == RegisterLocation)
    {
        return false;
    }
    
    /* First search through available registers and see if any is available */
    if (isIntegerType (getOperatorTypeFromString (varDesc->getType ())))
    {
        for (int i = 0; i < vectorIntRegisters.size (); i++)
        {
            if (vectorIntRegisters [i]->getVarDescriptor () == null)
            {//printf ("ASSIGNED REG %d\n", i);
                _allocateRegister (varDesc, vectorIntRegisters [i]);

                return true;
            }
        }
    
        /* Every register is filled up, cannot find any register
         * Let us use register next to the last use register */
        /* Spill the register */
        //printf ("ASSIGNED LASTREGUSED %d\n", lastRegUsed);
        vectorIntRegisters [intLastRegUsed%vectorIntRegisters.size ()]->spill (jitStack);
        _allocateRegister (varDesc, vectorIntRegisters [intLastRegUsed%vectorIntRegisters.size ()]);
        intLastRegUsed++;
    }
    else
    {
        /* For Float */
        /* First search through available registers and see if any is available */
        for (int i = 0; i < vectorFloatRegisters.size (); i++)
        {
            if (vectorFloatRegisters [i]->getVarDescriptor () == null)
            {//printf ("ASSIGNED REG %d\n", i);
                _allocateRegister (varDesc, vectorFloatRegisters [i]);

                return true;
            }
        }
    
        /* Every register is filled up, cannot find any register
         * Let us use register next to the last use register */
        /* Spill the register */
        //printf ("ASSIGNED LASTREGUSED %d\n", lastRegUsed);
        vectorFloatRegisters [floatLastRegUsed%vectorFloatRegisters.size ()]->spill (jitStack);
        _allocateRegister (varDesc, vectorFloatRegisters [floatLastRegUsed%vectorFloatRegisters.size ()]);
        floatLastRegUsed++;
    }

    return true;
}

TempDescriptor *JIT::createTempDescriptor (int size, OperatorType type, string value)
{
    TempDescriptor *tempDesc;
    
    /* First time allocate Register to Temporary, 
     * as temporaries are generated just before their use 
     */
    tempDesc = new TempDescriptor (size, getStringFromOperatorType (type), 
                                   CurrentLocation (MemoryLocation, -1), -1);
    /* allocateRegister will not copy value to register 
     * from memory or the supplied "value" (string) 
     * Value has to be copied manually.
     */
    allocateRegister (tempDesc);
    vectorTempDescriptors.insert (vectorTempDescriptors.end (), tempDesc);

    return tempDesc;
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
        
        localDesc = new LocalDescriptor (CurrentLocation (MemoryLocation, -1),
                                         code->getLocal (i));
        vectorLocalDescriptors.insert (vectorLocalDescriptors.end (), localDesc);
        allocateMemory (localDesc);
    }

    TempDescriptor *tempDesc;

    for (int i = 0; i < code->getTotalInstructions (); i++)
    {
        if (code->getInstruction (i)->getByteCode () == 3)
        {
            /* push.b */
            tempDesc = createTempDescriptor (sizeof (char), UnsignedChar, 
                                             code->getInstruction (i)->getOp ());
            jit_movi (tempDesc->getCurrLocation ().getValue (), 
                      atoi (code->getInstruction (i)->getOp ().c_str ()));
            varStack.push (tempDesc);
        }
        else if (code->getInstruction (i)->getByteCode () == 4)
        {
            /* push.s */
            tempDesc = createTempDescriptor (sizeof (short), Short, 
                                             code->getInstruction (i)->getOp ());
            jit_movi (tempDesc->getCurrLocation ().getValue (), 
                      atoi (code->getInstruction (i)->getOp ().c_str ()));
            varStack.push (tempDesc);
        }
        else if (code->getInstruction (i)->getByteCode () == 5)
        {
            /* push.i */
            tempDesc = createTempDescriptor (sizeof (int), Integer, 
                                             code->getInstruction (i)->getOp ());
            jit_movi (tempDesc->getCurrLocation ().getValue (), 
                      atoi (code->getInstruction (i)->getOp ().c_str ()));
            varStack.push (tempDesc);
            //printf ("TO REG %d value %s\n",tempDesc->getCurrLocation ().getValue (), code->getInstruction (i)->getOp ().c_str ());
        }
        else if (code->getInstruction (i)->getByteCode () == 6)
        {
            /* push.l */
            tempDesc = createTempDescriptor (sizeof (long), Long, 
                                             code->getInstruction (i)->getOp ());
            jit_movi (tempDesc->getCurrLocation ().getValue (), 
                      atoi (code->getInstruction (i)->getOp ().c_str ()));
            varStack.push (tempDesc);
        }
        else if (code->getInstruction (i)->getByteCode () == 7)
        {
            /* push.f */
            tempDesc = createTempDescriptor (sizeof (float), Float, 
                                             code->getInstruction (i)->getOp ());
            jit_movi (tempDesc->getCurrLocation ().getValue (), 
                      atoi (code->getInstruction (i)->getOp ().c_str ()));
            varStack.push (tempDesc);
        }
        else if (code->getInstruction (i)->getByteCode () == 8)
        {
            /* push.d */
            tempDesc = createTempDescriptor (sizeof (double), Double, 
                                             code->getInstruction (i)->getOp ());
            jit_movi_d (tempDesc->getCurrLocation ().getValue (), 
                        strtod (code->getInstruction (i)->getOp ().c_str (), NULL));
            varStack.push (tempDesc);
        }
        else if (code->getInstruction (i)->getByteCode () == 9)
        {
            /* push.c */
            tempDesc = createTempDescriptor (sizeof (char), SignedChar, 
                                             code->getInstruction (i)->getOp ());
            jit_movi (tempDesc->getCurrLocation ().getValue (), 
                      atoi (code->getInstruction (i)->getOp ().c_str ()));
            varStack.push (tempDesc);
        }
        else if (code->getInstruction (i)->getByteCode () == 10)
        {
            /* dup */
            VariableDescriptor* tempDesc1 = varStack.top ();
            varStack.pop ();
            tempDesc = createTempDescriptor (tempDesc1->getSize (), 
                                             getOperatorTypeFromString (tempDesc1->getType ()), 
                                             "");

            /* Temporary on the top of the stack 
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a checking 
             * for Temporary whether value is copied or not */
            if (isIntegerType (getOperatorTypeFromString (tempDesc1->getType ())))
            {
                jit_movr (tempDesc->getCurrLocation ().getValue (), 
                          tempDesc1->getCurrLocation ().getValue ());
            }
            else
            {
                if (getOperatorTypeFromString (tempDesc1->getType ()) == Float)
                {
                    jit_movr_f (tempDesc->getCurrLocation ().getValue (), 
                                tempDesc1->getCurrLocation ().getValue ());
                }
                else if (getOperatorTypeFromString (tempDesc1->getType ()) == Double)
                {
                    jit_movr_d (tempDesc->getCurrLocation ().getValue (), 
                                tempDesc1->getCurrLocation ().getValue ());
                }
            }

            varStack.push (tempDesc);
        }
        /* Next are convert instructions, 
         * convert instructions create a new temporary variable */
        else if (code->getInstruction (i)->getByteCode () == 11)
        {
            /* conv.b */
            VariableDescriptor* tempDesc1 = varStack.top ();

            /* Temporary on the top of the stack 
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack.pop ();
            tempDesc = createTempDescriptor (sizeof (unsigned char), UnsignedChar, 
                                             "");
            if (isIntegerType (getOperatorTypeFromString (tempDesc1->getType ())))
            {
                jit_movr (tempDesc->getCurrLocation ().getValue (),
                          tempDesc1->getCurrLocation ().getValue ());
            }
            else if (getOperatorTypeFromString (tempDesc1->getType ()) == Float)
            {
                jit_truncr_f_i (tempDesc->getCurrLocation ().getValue (),
                               tempDesc1->getCurrLocation ().getValue ());
            }    
            else if (getOperatorTypeFromString (tempDesc1->getType ()) == Double)
            {
                jit_truncr_d_i (tempDesc->getCurrLocation ().getValue (),
                               tempDesc1->getCurrLocation ().getValue ());                
            }

            jit_andi (tempDesc->getCurrLocation ().getValue (), 
                      tempDesc->getCurrLocation ().getValue (), 0xFF);
            varStack.push (tempDesc);
        }
        else if (code->getInstruction (i)->getByteCode () == 12)
        {
            /* conv.s */
            VariableDescriptor* tempDesc1 = varStack.top ();

            /* Temporary on the top of the stack 
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack.pop ();
            tempDesc = createTempDescriptor (sizeof (short), Short, 
                                             "");
            if (isIntegerType (getOperatorTypeFromString (tempDesc1->getType ())))
            {
                jit_movr (tempDesc->getCurrLocation ().getValue (),
                          tempDesc1->getCurrLocation ().getValue ());
            }
            else if (getOperatorTypeFromString (tempDesc1->getType ()) == Float)
            {
                jit_truncr_f_i (tempDesc->getCurrLocation ().getValue (),
                               tempDesc1->getCurrLocation ().getValue ());
            }    
            else if (getOperatorTypeFromString (tempDesc1->getType ()) == Double)
            {
                jit_truncr_d_i (tempDesc->getCurrLocation ().getValue (),
                               tempDesc1->getCurrLocation ().getValue ());                
            }

            jit_andi (tempDesc->getCurrLocation ().getValue (), 
                      tempDesc->getCurrLocation ().getValue (), 0xFF);
            varStack.push (tempDesc);
        }
        else if (code->getInstruction (i)->getByteCode () == 13)
        {
            /* conv.i */
            VariableDescriptor* tempDesc1 = varStack.top ();

            /* Temporary on the top of the stack 
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack.pop ();
            tempDesc = createTempDescriptor (sizeof (int), Integer, 
                                             "");
            if (isIntegerType (getOperatorTypeFromString (tempDesc1->getType ())))
            {
                jit_movr (tempDesc->getCurrLocation ().getValue (),
                          tempDesc1->getCurrLocation ().getValue ());

                if (getOperatorTypeFromString (tempDesc1->getType ()) == Long)
                {
                    jit_andi (tempDesc->getCurrLocation ().getValue (),
                              tempDesc->getCurrLocation ().getValue (), 0xFFFFFFFF);
                }
            }
            else if (getOperatorTypeFromString (tempDesc1->getType ()) == Float)
            {
                jit_truncr_f_i (tempDesc->getCurrLocation ().getValue (),
                               tempDesc1->getCurrLocation ().getValue ());
            }    
            else if (getOperatorTypeFromString (tempDesc1->getType ()) == Double)
            {
                jit_truncr_d_i (tempDesc->getCurrLocation ().getValue (),
                               tempDesc1->getCurrLocation ().getValue ());                
            }

            varStack.push (tempDesc);
        }
        else if (code->getInstruction (i)->getByteCode () == 14)
        {
            /* conv.l */
            VariableDescriptor* tempDesc1 = varStack.top ();

            /* Temporary on the top of the stack 
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack.pop ();
            tempDesc = createTempDescriptor (sizeof (long), Long, 
                                             "");
            if (isIntegerType (getOperatorTypeFromString (tempDesc1->getType ())))
            {
                jit_movr (tempDesc->getCurrLocation ().getValue (),
                          tempDesc1->getCurrLocation ().getValue ());
            }
            else if (getOperatorTypeFromString (tempDesc1->getType ()) == Float)
            {
                jit_truncr_f_l (tempDesc->getCurrLocation ().getValue (),
                               tempDesc1->getCurrLocation ().getValue ());
            }    
            else if (getOperatorTypeFromString (tempDesc1->getType ()) == Double)
            {
                jit_truncr_d_l (tempDesc->getCurrLocation ().getValue (),
                               tempDesc1->getCurrLocation ().getValue ());                
            }

            varStack.push (tempDesc);
        }
        else if (code->getInstruction (i)->getByteCode () == 15)
        {
            /* conv.f */
            VariableDescriptor* tempDesc1 = varStack.top ();

            /* Temporary on the top of the stack 
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack.pop ();
            tempDesc = createTempDescriptor (sizeof (float), Float, 
                                             "");
            if (isIntegerType (getOperatorTypeFromString (tempDesc1->getType ())))
            {
                jit_extr_f (tempDesc->getCurrLocation ().getValue (),
                            tempDesc1->getCurrLocation ().getValue ());
            }
            else if (getOperatorTypeFromString (tempDesc1->getType ()) == Double)
            {
                jit_extr_f_d (tempDesc->getCurrLocation ().getValue (),
                              tempDesc1->getCurrLocation ().getValue ());
            }
            
            varStack.push (tempDesc);
        }
        else if (code->getInstruction (i)->getByteCode () == 16)
        {
            /* conv.d */
            VariableDescriptor* tempDesc1 = varStack.top ();

            /* Temporary on the top of the stack 
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack.pop ();
            tempDesc = createTempDescriptor (sizeof (float), Float, 
                                             "");
            if (isIntegerType (getOperatorTypeFromString (tempDesc1->getType ())))
            {
                jit_extr_f (tempDesc->getCurrLocation ().getValue (),
                            tempDesc1->getCurrLocation ().getValue ());
            }
            else if (getOperatorTypeFromString (tempDesc1->getType ()) == Double)
            {
                jit_extr_d_f (tempDesc->getCurrLocation ().getValue (),
                              tempDesc1->getCurrLocation ().getValue ());
            }
            
            varStack.push (tempDesc);
        }
        else if (code->getInstruction (i)->getByteCode () == 17)
        {
            /* conv.c */
            VariableDescriptor* tempDesc1 = varStack.top ();

            /* Temporary on the top of the stack 
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack.pop ();
            tempDesc = createTempDescriptor (sizeof (char), SignedChar, 
                                             "");
            if (isIntegerType (getOperatorTypeFromString (tempDesc1->getType ())))
            {
                jit_movr (tempDesc->getCurrLocation ().getValue (),
                          tempDesc1->getCurrLocation ().getValue ());
            }
            else if (getOperatorTypeFromString (tempDesc1->getType ()) == Float)
            {
                jit_truncr_f_i (tempDesc->getCurrLocation ().getValue (),
                               tempDesc1->getCurrLocation ().getValue ());
            }    
            else if (getOperatorTypeFromString (tempDesc1->getType ()) == Double)
            {
                jit_truncr_d_i (tempDesc->getCurrLocation ().getValue (),
                               tempDesc1->getCurrLocation ().getValue ());                
            }

            jit_andi (tempDesc->getCurrLocation ().getValue (), 
                      tempDesc->getCurrLocation ().getValue (), 0xFF);
            varStack.push (tempDesc);
        }
        /* Branch Instructions */
        /* Arithmetic Instructions */
        else if (code->getInstruction (i)->getByteCode () == 27)
        {
            /* add */
            VariableDescriptor* tempDesc1 = varStack.top ();
            varStack.pop ();
            VariableDescriptor* tempDesc2 = varStack.top ();
            varStack.pop ();
            
            if (allocateRegister (tempDesc1) &&
                dynamic_cast <VariableDescriptor*> (tempDesc1) != null)
            {
                jitStack->copyMemToReg (_jit, tempDesc1->getMemLoc (), 
                                        tempDesc1->getCurrLocation ().getValue (),
                                        getOperatorTypeFromString (tempDesc1->getType ()));
            }

            if (allocateRegister (tempDesc2) &&
                dynamic_cast <VariableDescriptor*> (tempDesc2) != null)
            {
                jitStack->copyMemToReg (_jit, tempDesc2->getMemLoc (), 
                                        tempDesc2->getCurrLocation ().getValue (),
                                        getOperatorTypeFromString (tempDesc2->getType ()));
            }

            TempDescriptor *tempDesc3;

            tempDesc3 = createTempDescriptor (tempDesc1->getSize (),
                                              getOperatorTypeFromString (tempDesc1->getType ()), "");
            
            // Register is always allocated to a newly created temporary
            if (isIntegerType (getOperatorTypeFromString (tempDesc1->getType ())))
            {
                jit_addr (tempDesc3->getCurrLocation ().getValue (), 
                          tempDesc1->getCurrLocation ().getValue (), 
                          tempDesc2->getCurrLocation ().getValue ());
            }
            else
            {
                if (getOperatorTypeFromString (tempDesc1->getType ()) == Float)
                {
                    jit_addr_f (tempDesc3->getCurrLocation ().getValue (), 
                                tempDesc1->getCurrLocation ().getValue (), 
                                tempDesc2->getCurrLocation ().getValue ());
                }
                else if (getOperatorTypeFromString (tempDesc1->getType ()) == Double)
                {
                    jit_addr_d (tempDesc3->getCurrLocation ().getValue (), 
                                tempDesc1->getCurrLocation ().getValue (), 
                                tempDesc2->getCurrLocation ().getValue ());
                }
            }

            varStack.push (tempDesc3);
        }
        else if (code->getInstruction (i)->getByteCode () == 28)
        {
            // sub
            VariableDescriptor* tempDesc1 = varStack.top ();
            varStack.pop ();
            VariableDescriptor* tempDesc2 = varStack.top ();
            varStack.pop ();
            
            if (allocateRegister (tempDesc1) &&
                dynamic_cast <VariableDescriptor*> (tempDesc1) != null)
            {
                jitStack->copyMemToReg (_jit, tempDesc1->getMemLoc (), 
                                        tempDesc1->getCurrLocation ().getValue (),
                                        getOperatorTypeFromString (tempDesc1->getType ()));
            }

            if (allocateRegister (tempDesc2) &&
                dynamic_cast <VariableDescriptor*> (tempDesc2) != null)
            {
                jitStack->copyMemToReg (_jit, tempDesc2->getMemLoc (), 
                                        tempDesc2->getCurrLocation ().getValue (),
                                        getOperatorTypeFromString (tempDesc2->getType ()));
            }

            TempDescriptor *tempDesc3;

            tempDesc3 = createTempDescriptor (tempDesc1->getSize (),
                                              getOperatorTypeFromString (tempDesc1->getType ()), "");
            
            // Register is always allocated to a newly created temporary
            if (isIntegerType (getOperatorTypeFromString (tempDesc1->getType ())))
            {
                jit_subr (tempDesc3->getCurrLocation ().getValue (), 
                          tempDesc2->getCurrLocation ().getValue (), 
                          tempDesc1->getCurrLocation ().getValue ());
            }
            else
            {
                if (getOperatorTypeFromString (tempDesc1->getType ()) == Float)
                {
                    jit_subr_f (tempDesc3->getCurrLocation ().getValue (), 
                                tempDesc2->getCurrLocation ().getValue (), 
                                tempDesc1->getCurrLocation ().getValue ());
                }
                else if (getOperatorTypeFromString (tempDesc1->getType ()) == Double)
                {
                    jit_subr_d (tempDesc3->getCurrLocation ().getValue (), 
                                tempDesc2->getCurrLocation ().getValue (), 
                                tempDesc1->getCurrLocation ().getValue ());
                }
            }

            varStack.push (tempDesc3);
        }
        else if (code->getInstruction (i)->getByteCode () == 29)
        {
            // mul
            VariableDescriptor* tempDesc1 = varStack.top ();
            varStack.pop ();
            VariableDescriptor* tempDesc2 = varStack.top ();
            varStack.pop ();
            
            if (allocateRegister (tempDesc1) &&
                dynamic_cast <VariableDescriptor*> (tempDesc1) != null)
            {
                jitStack->copyMemToReg (_jit, tempDesc1->getMemLoc (),
                                        tempDesc1->getCurrLocation ().getValue (),
                                        getOperatorTypeFromString (tempDesc1->getType ()));
            }

            if (allocateRegister (tempDesc2) &&
                dynamic_cast <VariableDescriptor*> (tempDesc2) != null)
            {
                jitStack->copyMemToReg (_jit, tempDesc2->getMemLoc (),
                                        tempDesc2->getCurrLocation ().getValue (),
                                        getOperatorTypeFromString (tempDesc2->getType ()));
            }

            TempDescriptor *tempDesc3;

            tempDesc3 = createTempDescriptor (tempDesc1->getSize (),
                                              getOperatorTypeFromString (tempDesc1->getType ()), "");
            
            // Register is always allocated to a newly created temporary
            if (isIntegerType (getOperatorTypeFromString (tempDesc1->getType ())))
            {
                jit_mulr (tempDesc3->getCurrLocation ().getValue (), 
                          tempDesc1->getCurrLocation ().getValue (), 
                          tempDesc2->getCurrLocation ().getValue ());
            }
            else
            {
                if (getOperatorTypeFromString (tempDesc1->getType ()) == Float)
                {
                    jit_mulr_f (tempDesc3->getCurrLocation ().getValue (), 
                                tempDesc1->getCurrLocation ().getValue (), 
                                tempDesc2->getCurrLocation ().getValue ());
                }
                else if (getOperatorTypeFromString (tempDesc1->getType ()) == Double)
                {
                    jit_mulr_d (tempDesc3->getCurrLocation ().getValue (), 
                                tempDesc1->getCurrLocation ().getValue (), 
                                tempDesc2->getCurrLocation ().getValue ());
                }
            }
            varStack.push (tempDesc3);
        }
        else if (code->getInstruction (i)->getByteCode () == 30)
        {
            //div
            VariableDescriptor* tempDesc1 = varStack.top ();
            varStack.pop ();
            VariableDescriptor* tempDesc2 = varStack.top ();
            varStack.pop ();

            if (allocateRegister (tempDesc1) &&
                dynamic_cast <VariableDescriptor*> (tempDesc1) != null)
            {
                jitStack->copyMemToReg (_jit, tempDesc1->getMemLoc (), 
                                        tempDesc1->getCurrLocation ().getValue (),
                                        getOperatorTypeFromString (tempDesc1->getType ()));
            }

            if (allocateRegister (tempDesc2) &&
                dynamic_cast <VariableDescriptor*> (tempDesc1) != null)
            {
                jitStack->copyMemToReg (_jit, tempDesc2->getMemLoc (), 
                                        tempDesc2->getCurrLocation ().getValue (),
                                        getOperatorTypeFromString (tempDesc2->getType ()));
            }

            TempDescriptor *tempDesc3;

            tempDesc3 = createTempDescriptor (tempDesc1->getSize (),
                                              getOperatorTypeFromString (tempDesc1->getType ()), "");
            //Register is always allocated to a newly created temporary
            if (isIntegerType (getOperatorTypeFromString (tempDesc1->getType ())))
            {
                jit_divr (tempDesc3->getCurrLocation ().getValue (), 
                          tempDesc1->getCurrLocation ().getValue (), 
                          tempDesc2->getCurrLocation ().getValue ());
            }
            else
            {
                if (getOperatorTypeFromString (tempDesc1->getType ()) == Float)
                {
                    jit_divr_f (tempDesc3->getCurrLocation ().getValue (), 
                                tempDesc1->getCurrLocation ().getValue (), 
                                tempDesc2->getCurrLocation ().getValue ());
                }
                else if (getOperatorTypeFromString (tempDesc1->getType ()) == Double)
                {
                    jit_divr_d (tempDesc3->getCurrLocation ().getValue (), 
                                tempDesc1->getCurrLocation ().getValue (), 
                                tempDesc2->getCurrLocation ().getValue ());
                }
            }

            varStack.push (tempDesc);
        }
        else if (code->getInstruction (i)->getByteCode () == 31)
        {
        }
        else if (code->getInstruction (i)->getByteCode () == 100)
        {
            //Create a new Label
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
    jit_pushargi((jit_word_t)" %f ll\n");
    jit_ellipsis();
    //printf ("TOP %d \n", vectorTempDescriptors [vectorTempDescriptors.size () - 1]->getCurrLocation ().getValue ());
    //jit_pushargr_d(vectorTempDescriptors [vectorTempDescriptors.size () - 1]->getCurrLocation ().getValue ());
    jit_pushargr_d(varStack.top ()->getCurrLocation ().getValue ());
    //jit_movi_d (JIT_F1, 9.60);
    //jit_pushargr_d (JIT_F0);
    jit_finishi((jit_pointer_t)printf);
    jit_ret();
    jit_epilog();
      
    myFunction = (pvfi)jit_emit();

    myFunction();
    jit_clear_state();
    //jit_disassemble();
    jit_destroy_state();
    finish_jit();
    return 0;
}