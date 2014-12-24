#include "jit.h"
#include <stdio.h>
#include <string>
#include <cstdlib>
#include <stdexcept>

#define INT_DEFAULT_VALUE 0
#define FLOAT_DEFAULT_VALUE 0.0
#undef FLOAT_DEBUG
#define ARITH_START_CODE 27
#define ARITH_END_CODE 47
/* Also add exceptions like Stack Underflow in stack::pop () and stack::top () */
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
 
    varStackStack.push (new stack<VariableDescriptor*> ());
    varStack = varStackStack.top ();
    rootVarStack = varStack;
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

inline void JIT::processPushInstr (int size, OperatorType type, Instruction* instr)
{
    TempDescriptor* tempDesc;
    tempDesc = createTempDescriptor (size, type, instr->getOp ());

    if (isIntegerType (type))
    {
        jit_movi (tempDesc->getCurrLocation ().getValue (),
                  atoi (instr->getOp ().c_str ()));
    }
    else if (type == Float)
    {
        jit_movi_f (tempDesc->getCurrLocation ().getValue (),
                    atof (instr->getOp ().c_str ()));
    }
    else
    {
        jit_movi_d (tempDesc->getCurrLocation ().getValue (),
                    strtod (instr->getOp ().c_str (), NULL));
    }

    varStack->push (tempDesc);
}

void JIT::processArithInstr (jit_code_t code_i, jit_code_t code_f, jit_code_t code_d)
{
    /* Initialization code for Arithmetic Insructions */
    VariableDescriptor* tempDesc1 = varStack->top ();
    varStack->pop ();
    VariableDescriptor* tempDesc2 = varStack->top ();
    varStack->pop ();
 
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

    if (isIntegerType (getOperatorTypeFromString (tempDesc1->getType ())))
    {
        jit_new_node_www (code_i, tempDesc3->getCurrLocation ().getValue (),
                  tempDesc1->getCurrLocation ().getValue (),
                  tempDesc2->getCurrLocation ().getValue ());
    }
    else
    {
        if (getOperatorTypeFromString (tempDesc1->getType ()) == Float)
        {
            jit_new_node_www (code_f, tempDesc3->getCurrLocation ().getValue (),
                        tempDesc1->getCurrLocation ().getValue (),
                        tempDesc2->getCurrLocation ().getValue ());
        }
        else if (getOperatorTypeFromString (tempDesc1->getType ()) == Double)
        {
            jit_new_node_www (code_d, tempDesc3->getCurrLocation ().getValue (),
                              tempDesc1->getCurrLocation ().getValue (),
                              tempDesc2->getCurrLocation ().getValue ());
        }
    }
 
    varStack->push (tempDesc3);
}

void JIT::processBranchInstr (string _label, jit_code_t code_i, jit_code_t code_f, jit_code_t code_d)
{
    JITLabel *label = null;
 
    try
    {
        label = mapLabels.at (_label);
        /* Label is found hence the label is already defined.
         * so it is used for a backward jump */
        jit_node_t *jump;

        if (code_i == -1 && code_f == -1 && code_d == -1)
        {
            /* br <label> */
            jump = jit_jmpi ();
        }
        else
        {
            /* Other branch instructions */
            VariableDescriptor* tempDesc1 = varStack->top ();
            varStack->pop ();
            VariableDescriptor* tempDesc2 = varStack->top ();
            varStack->push (tempDesc1);

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
                                        getOperatorTypeFromString (tempDesc1->getType ()));
            }
        
            if (isIntegerType (getOperatorTypeFromString (tempDesc1->getType ())))
            {
                jump = jit_new_node_pww (code_i, NULL,
                                               tempDesc1->getCurrLocation ().getValue (),
                                               tempDesc2->getCurrLocation ().getValue ());
            }
            else
            {
                if (getOperatorTypeFromString (tempDesc1->getType ()) == Float)
                {
                    jump = jit_new_node_pww (code_f, NULL,
                                                   tempDesc1->getCurrLocation ().getValue (),
                                                   tempDesc2->getCurrLocation ().getValue ());
                }
                else if (getOperatorTypeFromString (tempDesc1->getType ()) == Double)
                {
                    jump = jit_new_node_pww (code_d, NULL,
                                                   tempDesc1->getCurrLocation ().getValue (),
                                                   tempDesc2->getCurrLocation ().getValue ());
                }
            }
        }

        jit_patch_at (jump, label->getJITNode ());
        varStackStack.pop ();
        varStack = varStackStack.top ();
        printf ("FOUND %s\n", _label.c_str ());
    }
    catch (const std::out_of_range& a)
    {
        /* Label not found hence the label is not defined
         * so it is used for a forward jump */
        if (code_i == -1 && code_f == -1 && code_d == -1)
        {
            /* br <label> */
            label = new JITLabel (jit_label (), _label);
        }
        else
        {
            /* Other branch instructions */
            jit_node_t* label_node;
            VariableDescriptor* tempDesc1 = varStack->top ();
            varStack->pop ();
            VariableDescriptor* tempDesc2 = varStack->top ();
            varStack->push (tempDesc1);

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
                                        getOperatorTypeFromString (tempDesc1->getType ()));
            }
        
            if (isIntegerType (getOperatorTypeFromString (tempDesc1->getType ())))
            {
                label_node = jit_new_node_pww (code_i, NULL,
                                               tempDesc1->getCurrLocation ().getValue (),
                                               tempDesc2->getCurrLocation ().getValue ());
            }
            else
            {
                if (getOperatorTypeFromString (tempDesc1->getType ()) == Float)
                {
                    label_node = jit_new_node_pww (code_f, NULL,
                                                   tempDesc1->getCurrLocation ().getValue (),
                                                   tempDesc2->getCurrLocation ().getValue ());
                }
                else if (getOperatorTypeFromString (tempDesc1->getType ()) == Double)
                {
                    label_node = jit_new_node_pww (code_d, NULL,
                                                   tempDesc1->getCurrLocation ().getValue (),
                                                   tempDesc2->getCurrLocation ().getValue ());
                }
            }
            label = new JITLabel (label_node, _label);
        }

        mapLabels [_label] = label;
        varStack = new stack<VariableDescriptor*> (*varStackStack.top ());
        varStackStack.push (varStack);
        printf ("NOT FOUND %s\n", _label.c_str ());
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
 
        localDesc = new LocalDescriptor (CurrentLocation (MemoryLocation, -1),
                                         code->getLocal (i));
        vectorLocalDescriptors.insert (vectorLocalDescriptors.end (), localDesc);
        allocateMemory (localDesc);
    }

    TempDescriptor *tempDesc;
    static void *labelArray[] = { null, null, &&L3, &&L4, &&L5, &&L6, &&L7, &&L8,
                                  &&L9, &&L10, &&L11, &&L12, &&L13, &&L14, &&L15,
                                  &&L16, &&L17, &&L18, &&L19, &&L20, &&L21, &&L22,
                                  &&L23, &&L24, &&L25, &&L26, &&L27, &&L28, &&L29,
                                  &&L30, &&L31, &&L32, &&L33, &&L34, &&L35, &&L36,
                                  &&L37, &&L38, &&L39, &&L40, &&L41, &&L42, &&L43,
                                  &&L44, &&L45, &&L46, &&L47, &&L48, &&L49, &&L50,
                                  &&L51, &&L52, &&L53, &&L54, &&L55, &&L56, &&L57,
                                  &&L58, &&L59, &&L60, &&L61, &&L62, &&L63, &&L64,
                                  &&L65
                                };

    for (int i = 0; i < code->getTotalInstructions (); i++)
    {
        int instrByteCode = code->getInstruction (i)->getByteCode ();
        printf ("BB%d\n", instrByteCode);
        if (instrByteCode == 100)
        goto L100;

        goto *labelArray [instrByteCode - 1];

        L3:
        {
            /* push.b */
            processPushInstr (sizeof (char), UnsignedChar,
                              code->getInstruction (i));
            continue;
        }

        L4:
        {
            /* push.s */
            processPushInstr (sizeof (short), Short,
                              code->getInstruction (i));
            continue;
        }
        L5:
        {
            /* push.i */
            processPushInstr (sizeof (int), Integer,
                              code->getInstruction (i));
            printf ("TO REG %d value %s\n",varStack->top()->getCurrLocation ().getValue (), code->getInstruction (i)->getOp ().c_str ());
            continue;
        }
        L6:
        {
            /* push.l */
            processPushInstr (sizeof (long), Long,
                              code->getInstruction (i));
            continue;
        }
        L7:
        {
            /* push.f */
            processPushInstr (sizeof (float), Float,
                              code->getInstruction (i));
            continue;
        }
        L8:
        {
            /* push.d */
            processPushInstr (sizeof (double), Double,
                              code->getInstruction (i));
 
            continue;
        }
        L9:
        {
            /* push.c */
            processPushInstr (sizeof (char), SignedChar,
                              code->getInstruction (i));
            continue;
        }
        L10:
        {
            /* dup */
            VariableDescriptor* tempDesc1 = varStack->top ();
            varStack->pop ();
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

            varStack->push (tempDesc);
            continue;
        }
        /* Next are convert instructions,
         * convert instructions create a new temporary variable */
        L11:
        {
            /* conv.b */
            VariableDescriptor* tempDesc1 = varStack->top ();

            /* Temporary on the top of the stack
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack->pop ();
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
            varStack->push (tempDesc);
            continue;
        }
        L12:
        {
            /* conv.s */
            VariableDescriptor* tempDesc1 = varStack->top ();

            /* Temporary on the top of the stack
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack->pop ();
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
                      tempDesc->getCurrLocation ().getValue (), 0xFFFF);
            varStack->push (tempDesc);
            continue;
        }
        L13:
        {
            /* conv.i */
            VariableDescriptor* tempDesc1 = varStack->top ();

            /* Temporary on the top of the stack
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack->pop ();
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

            varStack->push (tempDesc);
            continue;
        }
        L14:
        {
            /* conv.l */
            VariableDescriptor* tempDesc1 = varStack->top ();

            /* Temporary on the top of the stack
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack->pop ();
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

            varStack->push (tempDesc);
            continue;
        }
        L15:
        {
            /* conv.f */
            VariableDescriptor* tempDesc1 = varStack->top ();

            /* Temporary on the top of the stack
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack->pop ();
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
 
            varStack->push (tempDesc);
            continue;
        }
        L16:
        {
            /* conv.d */
            VariableDescriptor* tempDesc1 = varStack->top ();

            /* Temporary on the top of the stack
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack->pop ();
            tempDesc = createTempDescriptor (sizeof (float), Float,
                                             "");
            if (isIntegerType (getOperatorTypeFromString (tempDesc1->getType ())))
            {
                jit_extr_d (tempDesc->getCurrLocation ().getValue (),
                            tempDesc1->getCurrLocation ().getValue ());
            }
            else if (getOperatorTypeFromString (tempDesc1->getType ()) == Double)
            {
                jit_extr_d_f (tempDesc->getCurrLocation ().getValue (),
                              tempDesc1->getCurrLocation ().getValue ());
            }
 
            varStack->push (tempDesc);
            continue;
        }
        L17:
        {
            /* conv.c */
            VariableDescriptor* tempDesc1 = varStack->top ();

            /* Temporary on the top of the stack
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack->pop ();
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
            varStack->push (tempDesc);
            continue;
        }
 
        L18:
        {
            continue;
        }
 
        L19:
        {
            continue;
        }
        /* Branch Instructions */
        L20:
        {
            processBranchInstr (code->getInstruction (i)->getOp (), (jit_code_t)-1, (jit_code_t)-1, (jit_code_t)-1);
            continue;
        }
        L21:
        {
            /* brzero <label> */
            continue;
        }
        L22:
        {
            /* beq <label> */
            processBranchInstr (code->getInstruction (i)->getOp (), jit_code_beqr, jit_code_beqr_f, jit_code_beqr_d);
            continue;
        }
        L23:
        {
            /* bge <label> */
            processBranchInstr (code->getInstruction (i)->getOp (), jit_code_bger, jit_code_bger_f, jit_code_bger_d);
            continue;
        }
        L24:
        {
            /* bgt <label> */
            processBranchInstr (code->getInstruction (i)->getOp (), jit_code_bgtr, jit_code_bgtr_f, jit_code_bgtr_d);
            continue;
        }
        L25:
        {
            /* ble <label> */
            processBranchInstr (code->getInstruction (i)->getOp (), jit_code_bler, jit_code_bler_f, jit_code_bler_d);
            continue;
        }
        L26:
        {
            /* blt <label> */
            processBranchInstr (code->getInstruction (i)->getOp (), jit_code_bltr, jit_code_bltr_f, jit_code_bltr_d);
            continue; 
        }
        /* Arithmetic Instructions */
        L27:
        {
            /* add */
            processArithInstr (jit_code_addr, jit_code_addr_f, jit_code_addr_d);
            continue;
        }

        L28:
        {
            // sub
            processArithInstr (jit_code_subr, jit_code_subr_f, jit_code_subr_d);
            continue;
        }
 
        L29:
        {
            // mul
            processArithInstr (jit_code_mulr, jit_code_mulr_f, jit_code_mulr_d);
            continue;
        }
 
        L30:
        {
            //div
            processArithInstr (jit_code_divr, jit_code_divr_f, jit_code_divr_d);
            continue;
        }

        L31:
        {
            continue;
        }
 
        L32:
        {
            processArithInstr (jit_code_remr_u, jit_code_remr_u, jit_code_remr_u);
            continue;
        }
 
        L33:
        {
            continue;
        }
 
        L34:
        {
            // and
            processArithInstr (jit_code_andr, jit_code_andr, jit_code_andr);
            continue;
        }
 
        L35:
        {
            // or
            processArithInstr (jit_code_orr, jit_code_orr, jit_code_orr);
            continue;
        }
 
        L36:
        {
            // xor
            processArithInstr (jit_code_xorr, jit_code_xorr, jit_code_xorr);
            continue;
        }
 
        L37:
        {
            // shl
            processArithInstr (jit_code_lshr, jit_code_lshr, jit_code_lshr);
            continue;
        }
 
        L38:
        {
            // shr
            processArithInstr (jit_code_rshr, jit_code_rshr, jit_code_rshr);
            continue;
        }
 
        L39:
        {
            //processArithInstr (jit_code_xorr, jit_code_xorr, jit_code_xorr);
            continue;
        }
 
        L40:
        {
            // neg
            processArithInstr (jit_code_negr, jit_code_negr, jit_code_negr);
            continue;
        }
 
        L41:
        {
            // not
            processArithInstr (jit_code_comr, jit_code_comr, jit_code_comr);
            continue;
        }

        L42:
        {
            // le
            processArithInstr (jit_code_ler_u, jit_code_ler_f, jit_code_ler_d);
            continue;
        }
 
        L43:
        {
            // ge
            processArithInstr (jit_code_ger_u, jit_code_ger_f, jit_code_ger_d);
            continue;
        }
 
        L44:
        {
            // lt
            processArithInstr (jit_code_ltr, jit_code_ltr_f, jit_code_ltr_d);
            continue;
        }
 
        L45:
        {
            // gt
            processArithInstr (jit_code_gtr, jit_code_gtr_f, jit_code_gtr_d);
            continue;
        }
 
        L46:
        {
            // eq
            processArithInstr (jit_code_eqr, jit_code_eqr_f, jit_code_eqr_d);
            continue;
        }
 
        L47:
        {
            // ne
            processArithInstr (jit_code_ner, jit_code_ner_f, jit_code_ner_d);
            continue;
        }
 
        L48:
        {
            // push.str
             continue;
        }
 
        L49:
        {
            // push.str
             continue;
        }
 
        L50:
        {
            // push.str
             continue;
        }
 
        L51:
        {
            // push.str
             continue;
        }
 
        L52:
        {
            // push.str
             continue;
        }
 
        L53:
        {
            // push.str
             continue;
        }
 
        L54:
        {
            // push.str
             continue;
        }
 
        L55:
        {
            // push.str
             continue;
        }
 
        L56:
        {
            // push.str
             continue;
        }
 
        L57:
        {
            // push.str
             continue;
        }
 
        L58:
        {
            // push.str
             continue;
        }
 
        L59:
        {
            // push.str
             continue;
        }
 
        L60:
        {
            // push.str
             continue;
        }
 
        L61:
        {
            // push.str
             continue;
        }
 
        L62:
        {
            // ldloc
            int index = atoi (code->getInstruction (i)->getOp ().c_str ());
            LocalDescriptor* localDesc = vectorLocalDescriptors [index];
            
            allocateRegister (localDesc);
            varStack->push (localDesc);
            continue;
        }
 
        L63:
        {
            // stloc
            int index = atoi (code->getInstruction (i)->getOp ().c_str ());
            LocalDescriptor* localDesc = vectorLocalDescriptors [index];
            VariableDescriptor* varDesc = varStack->top ();
            varStack->pop ();

            //Topmost value will always be in the register
            RegisterDescriptor* regDesc = vectorIntRegisters [varDesc->getCurrLocation().getValue ()];
            regDesc->assignVariable (localDesc);
            vectorIntRegisters [localDesc->getCurrLocation().getValue ()]->assignVariable (null);
            localDesc->setCurrLocation (RegisterLocation,
                                        regDesc->getRegister ());

            continue;
        }
 
        L64:
        {
            // push.str
             continue;
        }
 
        L65:
        {
            // push.str
             continue;
        }
 
        L100:
        {
            //Create a new Label
            /* Backward jump is a jump when first Label is defined and then
             * jump instruction is used to jump to it
             * In forward, instruction uses that jump before the jump
             * is defined
             * Search if a label is already in the mapLabels
             * This can happen if there has to be forward jump. In this case
             * Label is created by branch instructions */
            JITLabel *label = null;
 
            try
            {
                label = mapLabels.at (code->getInstruction (i)->getOp ());
                /* Label is found hence the label is used for a forward jump */
                jit_patch (label->getJITNode ());
                varStackStack.pop ();
                varStack = varStackStack.top ();
            }
            catch (const std::out_of_range& a)
            {
                /* Label not found hence the label is used for a backward jump */
                label = new JITLabel (jit_label (), code->getInstruction (i)->getOp ());
                mapLabels [code->getInstruction (i)->getOp ()] = label;
                varStack = new stack<VariableDescriptor*> (*varStackStack.top ());
                varStackStack.push (varStack);
            }
 
            continue;
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
    //printf ("RUNNING CODE\n");
    //jit_node_t *jump;
    //jit_movi (JIT_R0, 10);
    //jump = jit_beqi (JIT_R0, 10);
    //jit_movi (JIT_R0, 3);
    //jit_patch (jump);
    jit_pushargi((jit_word_t)" %d ll\n");
    jit_ellipsis();
    //printf ("TOP %d \n", vectorTempDescriptors [vectorTempDescriptors.size () - 1]->getCurrLocation ().getValue ());
    //jit_pushargr_d(vectorTempDescriptors [vectorTempDescriptors.size () - 1]->getCurrLocation ().getValue ());
    jit_pushargr(varStack->top ()->getCurrLocation ().getValue ());
    //jit_movi_d (JIT_F1, 9.60);
    //jit_pushargr_d (JIT_F0);
    //jit_pushargr (JIT_R0);
    jit_finishi((jit_pointer_t)printf);
    jit_ret();
    jit_epilog();
 
    myFunction = (pvfi)jit_emit();

    myFunction();
    jit_clear_state();
    jit_disassemble();
    jit_destroy_state();
    finish_jit();
    return 0;
}