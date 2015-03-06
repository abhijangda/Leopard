#include "jit.h"
#include "vm.h"
#include "main.h"

#include <stdio.h>
#include <string>
#include <cstdlib>
#include <stdexcept>

#define INT_DEFAULT_VALUE 0
#define FLOAT_DEFAULT_VALUE 0.0
#undef FLOAT_DEBUG
#define ARITH_START_CODE 27
#define ARITH_END_CODE 47
/* TODO: Also add exceptions like Stack Underflow in stack::pop_back () and stack::top () */
/* TODO: There are different ways functions like "allocateArray" returns value for 
 * different architectures, Create cases to store the register value where 
 * functions value is returned. For different processors type 
 * (x86, SPARC, PORWERPC) it will be different.
 */
 
JIT* currentJIT;
unsigned long arrayAddress;
static int func ();
extern VirtualMachine* ptrVM;

VariableDescriptor* VariableDescriptor::copyVarDesc (VariableDescriptor* varDesc)
{
    LocalDescriptor* localDesc;
    TempDescriptor* tempDesc;

    localDesc = dynamic_cast <LocalDescriptor*> (varDesc);

    if (localDesc != LULL)
    {
        return new LocalDescriptor (*localDesc);
    }
            
    tempDesc = dynamic_cast <TempDescriptor*> (varDesc);

    if (tempDesc != LULL)
    {
        return new TempDescriptor (*tempDesc);
    }
        
    return new VariableDescriptor (*varDesc);
}
    
int getSizeFromOperatorType (OperatorType type)
{
    switch (type)
    {
        case SignedChar:
        case UnsignedChar:
            return 1;
        case Short:
        case UnsignedShort:
            return 2;
        case Integer:
        case UnsignedInteger:
        case Float:
            return 4;
        case Long:
        case Double:
        case Reference:
            return 8;      
    }   

    return 0;
}

OperatorType getOperatorTypeFromString (string type)
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

string getStringFromOperatorType (OperatorType type)
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

void RegisterDescriptor::spill (jit_state *_jit, JITStack *jitStack)
{
    if (getVarDescriptor () == null)
    {
        return;
    }

    if (assignedVar->getMemLoc () == -1)
    {
        /* Allocate Memory if not allocated, Local Variable is 
         * allocated memory always only a temporary variable is 
         * required to allocate memory 
         */
        jitStack->allocateTemporary (_jit, dynamic_cast <TempDescriptor*> (assignedVar));
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
 
    varStackStack.push (new vector<VariableDescriptor*> ());
    varStack = varStackStack.top ();
    rootVarStack = varStack;
    
    compiledFunction = LULL;
    argsMemLocation = LULL;
    stackPointerMem = new unsigned long;
}

/* This function allocates memory on stack to variable
 * It doesn't copy register value to memory
 */
void JIT::allocateMemory (VariableDescriptor *varDesc)
{
    if (varDesc->getMemLoc () != -1)
    {
        /* Memory already allocated */
        return;
    }
    
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
            jitStack->stackPush(_jit, JIT_F0, type);
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
        //printf ("ASSIGNED LASTREGUSED %d\n", intLastRegUsed);
        vectorIntRegisters [intLastRegUsed%vectorIntRegisters.size ()]->spill (_jit, jitStack);
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
        vectorFloatRegisters [floatLastRegUsed%vectorFloatRegisters.size ()]->spill (_jit, jitStack);
        _allocateRegister (varDesc, vectorFloatRegisters [floatLastRegUsed%vectorFloatRegisters.size ()]);
        floatLastRegUsed++;
    }

    return true;
}

TempDescriptor *JIT::createTempDescriptor (int size, OperatorType type, 
                                           string value = "", string classType = "")
{
    TempDescriptor *tempDesc;
 
    /* First time allocate Register to Temporary,
     * as temporaries are generated just before their use
     */
    if (type != Reference)
    {
        tempDesc = new TempDescriptor (size, getStringFromOperatorType (type),
                                       CurrentLocation (MemoryLocation, -1), -1);
    }
    else
    {
        tempDesc = new TempDescriptor (size, classType,
                                       CurrentLocation (MemoryLocation, -1), -1);
    }
    
    /* allocateRegister will not copy value to register
     * from memory or the supplied "value" (string)
     * Value has to be copied manually.
     */
    allocateRegister (tempDesc);
    vectorTempDescriptors.insert (vectorTempDescriptors.end (), tempDesc);

    return tempDesc;
}

void JIT::copyVariables (VariableDescriptor *src, VariableDescriptor *dest)
{
    if (src->getType () != dest->getType ())
    {
        /* ERROR: Both VarDescs must be of the same type */
        return;
    }
    
    /* If both are in the memory allocate register to source */
    if (src->getCurrLocation ().getLocationType () == MemoryLocation &&
        dest->getCurrLocation ().getLocationType () == MemoryLocation)
    {
        if (allocateRegister (src) &&
            dynamic_cast <VariableDescriptor*> (src) != null)
        {
            jitStack->copyMemToReg (_jit, src->getMemLoc (),
                                    src->getCurrLocation ().getValue (),
                                    getOperatorTypeFromString (src->getType ()));
        }
    }

    if (dest->getCurrLocation ().getLocationType () == RegisterLocation)
    {
        if (src->getType () == "float")
        {
            jit_movr_f (dest->getCurrLocation ().getValue (), 
                        src->getCurrLocation ().getValue ());
        }
        else if (src->getType () == "double")
        {
            jit_movr_d (dest->getCurrLocation ().getValue (), 
                        src->getCurrLocation ().getValue ());
        }
        else
        {
            jit_movr (dest->getCurrLocation ().getValue (), 
                      src->getCurrLocation ().getValue ());
        }
    }
    else
    {
        jitStack->copyRegToMem (_jit, dest->getMemLoc (), 
                                src->getCurrLocation().getValue (),
                                getOperatorTypeFromString (src->getType ()));
    }
}

inline void JIT::processPushInstr (int size, OperatorType type, string op)
{
    TempDescriptor* tempDesc;
    tempDesc = createTempDescriptor (size, type, op);

    if (isIntegerType (type))
    {
        jit_movi (tempDesc->getCurrLocation ().getValue (),
                  atoi (op.c_str ()));
    }
    else if (type == Float)
    {
        jit_movi_f (tempDesc->getCurrLocation ().getValue (),
                    atof (op.c_str ()));
    }
    else
    {
        jit_movi_d (tempDesc->getCurrLocation ().getValue (),
                    strtod (op.c_str (), NULL));
    }

    varStack->push_back (tempDesc);
}

void JIT::processArithInstr (jit_code_t code_i, jit_code_t code_f, jit_code_t code_d)
{
    /* Initialization code for Arithmetic Insructions */
    VariableDescriptor* tempDesc1 = varStack->back ();
    varStack->pop_back ();
    VariableDescriptor* tempDesc2 = varStack->back ();
    varStack->pop_back ();
 
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
 
    varStack->push_back (tempDesc3);
}

void JIT::storeVariablesToMemory ()
{
    int i = 0;    

    while (i < varStack->size ())
    {
        VariableDescriptor *varDesc;
        
        varDesc = varStack->at (i);
        
        if (varDesc->getMemLoc () == -1)
        {
            jitStack->allocateTemporary (_jit, dynamic_cast<TempDescriptor*> (varDesc));
        }
    
        if (varDesc->getCurrLocation ().getLocationType () == RegisterLocation)
        {
            jitStack->copyRegToMem (_jit, varDesc->getMemLoc (), 
                                    varDesc->getCurrLocation ().getValue (), 
                                    getOperatorTypeFromString (varDesc->getType ()));
            varDesc->setCurrLocation (MemoryLocation, varDesc->getMemLoc ());
        }
        
        i++;
    }

    i = 0;
    while (i < vectorLocalDescriptors.size ())
    {
        VariableDescriptor *varDesc;
        
        varDesc = vectorLocalDescriptors.at (i);

        if (varDesc->getCurrLocation ().getLocationType () == RegisterLocation)
        {
            jitStack->copyRegToMem (_jit, varDesc->getMemLoc (), 
                                    varDesc->getCurrLocation ().getValue (), 
                                    getOperatorTypeFromString (varDesc->getType ()));
            varDesc->setCurrLocation (MemoryLocation, varDesc->getMemLoc ());
        }
        
        i++;
    }

    for (i = 0; i < vectorIntRegisters.size (); i++)
    {
        vectorIntRegisters [i]->assignVariable (LULL);
    }
}

void JIT::restoreVariablesFromMemory ()
{    
    int i = 0;
    
    while (i < varStack->size ())
    {
        VariableDescriptor *varDesc;
        
        varDesc = varStack->at (i);
        allocateRegister (varDesc);
    
        if (varDesc->getCurrLocation ().getLocationType () == RegisterLocation)
        {
            jitStack->copyMemToReg (_jit, varDesc->getMemLoc (), 
                                    varDesc->getCurrLocation ().getValue (), 
                                    getOperatorTypeFromString (varDesc->getType ()));
        }
        
        i++;
    }
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
            VariableDescriptor* tempDesc1 = varStack->back ();
            varStack->pop_back ();
            VariableDescriptor* tempDesc2 = varStack->back ();
            //varStack->push_back (tempDesc1);
            varStack->pop_back ();
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
        //printf ("FOUND %s\n", _label.c_str ());
    }
    catch (const std::out_of_range& a)
    {
        /* Label not found hence the label is not defined
         * so it is used for a forward jump */
        if (code_i == -1 && code_f == -1 && code_d == -1)
        {
            /* br <label> */
            label = new JITLabel (jit_jmpi (), _label);
        }
        else
        {
            /* Other branch instructions */
            jit_node_t* label_node;
            VariableDescriptor* tempDesc1 = varStack->back ();
            varStack->pop_back ();
            VariableDescriptor* tempDesc2 = varStack->back ();
            //varStack->push_back (tempDesc1);
            varStack->pop_back ();
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
        varStack = new vector<VariableDescriptor*> (*varStackStack.top ());
        varStackStack.push (varStack);
        //printf ("NOT FOUND %s\n", _label.c_str ());
    }
}

void JIT::convertCode (vector<VariableDescriptor*>* vectorArgs, MethodCode *code)
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

    if (vectorArgs->size () > 0)
    {
        /* Copy previous stack pointer to JIT_R2 */
        jit_ldi_l (JIT_R2, (jit_pointer_t)prevStackPointerMem);
    }

    /* Copy Arguments from previous stack to this stack */
    for (int i = 0; i < vectorArgs->size (); i++)
    {
        if (dynamic_cast <LocalDescriptor*> (vectorArgs[0][i]) == NULL &&
            dynamic_cast <TempDescriptor*> (vectorArgs[0][i]) == NULL)
        {
            continue;
        }
        long * loc = argsMemLocation + i;

        /* Copy previous value to JIT_R1 */
        jit_ldi_l (JIT_V0, (jit_pointer_t)loc);
        jitStack->copyMemxrToReg (_jit, JIT_R2, JIT_V0, JIT_R1,
                                  getOperatorTypeFromString (vectorArgs[0][i]->getType ()));
        allocateMemory (vectorArgs[0][i]);
        /* Copy value to newly allocated memory */
        jitStack->copyRegToMem (_jit, vectorArgs[0][i]->getMemLoc (), JIT_R1, 
                                getOperatorTypeFromString (vectorArgs[0][i]->getType ()));
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
                                  &&L65, &&L66, &&L67
                                };

    for (int i = 0; i < code->getTotalInstructions (); i++)
    {
        int instrByteCode = code->getInstruction (i)->getByteCode ();
        //printf ("BB%d\n", instrByteCode);
        if (instrByteCode == 100)
        goto L100;

        goto *labelArray [instrByteCode - 1];

        L3:
        {
            /* push.b */
            processPushInstr (sizeof (char), UnsignedChar,
                              code->getInstruction (i)->getOp ());
            continue;
        }

        L4:
        {
            /* push.s */
            processPushInstr (sizeof (short), Short,
                              code->getInstruction (i)->getOp ());
            continue;
        }
        L5:
        {
            /* push.i */
            processPushInstr (sizeof (int), Integer,
                              code->getInstruction (i)->getOp ());
            continue;
        }
        L6:
        {
            /* push.l */
            processPushInstr (sizeof (long), Long,
                              code->getInstruction (i)->getOp ());
            continue;
        }
        L7:
        {
            /* push.f */
            processPushInstr (sizeof (float), Float,
                              code->getInstruction (i)->getOp ());
            continue;
        }
        L8:
        {
            /* push.d */
            processPushInstr (sizeof (double), Double,
                              code->getInstruction (i)->getOp ());
 
            continue;
        }
        L9:
        {
            /* push.c */
            processPushInstr (sizeof (char), SignedChar,
                              code->getInstruction (i)->getOp ());
            continue;
        }
        L10:
        {
            /* dup */
            VariableDescriptor* tempDesc1 = varStack->back ();
            varStack->pop_back ();
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

            varStack->push_back (tempDesc);
            continue;
        }
        /* Next are convert instructions,
         * convert instructions create a new temporary variable */
        L11:
        {
            /* conv.b */
            VariableDescriptor* tempDesc1 = varStack->back ();

            /* Temporary on the top of the stack
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack->pop_back ();
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
            varStack->push_back (tempDesc);
            continue;
        }
        L12:
        {
            /* conv.s */
            VariableDescriptor* tempDesc1 = varStack->back ();

            /* Temporary on the top of the stack
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack->pop_back ();
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
            varStack->push_back (tempDesc);
            continue;
        }
        L13:
        {
            /* conv.i */
            VariableDescriptor* tempDesc1 = varStack->back ();

            /* Temporary on the top of the stack
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack->pop_back ();
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

            varStack->push_back (tempDesc);
            continue;
        }
        L14:
        {
            /* conv.l */
            VariableDescriptor* tempDesc1 = varStack->back ();

            /* Temporary on the top of the stack
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack->pop_back ();
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

            varStack->push_back (tempDesc);
            continue;
        }
        L15:
        {
            /* conv.f */
            VariableDescriptor* tempDesc1 = varStack->back ();

            /* Temporary on the top of the stack
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack->pop_back ();
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
 
            varStack->push_back (tempDesc);
            continue;
        }
        L16:
        {
            /* conv.d */
            VariableDescriptor* tempDesc1 = varStack->back ();

            /* Temporary on the top of the stack
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack->pop_back ();
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
 
            varStack->push_back (tempDesc);
            continue;
        }
        L17:
        {
            /* conv.c */
            VariableDescriptor* tempDesc1 = varStack->back ();

            /* Temporary on the top of the stack
             * would be in a register for sure */
            allocateRegister (tempDesc1);
            /* So above call doesn't require a check for Temporary */
            varStack->pop_back ();
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
            varStack->push_back (tempDesc);
            continue;
        }
 
        L18:
        {
            continue;
        }
 
        L19:
        {
            //call
            storeVariablesToMemory ();
            
            VariableDescriptor* varDesc;
            string instr;
            string className;
            string methodName;
            size_t pos;
            MethodInfo *methodInfo;
            ClassInfo *classInfo;
            vector <VariableDescriptor*>* vectorArgs;
            vector<VariableDescriptor*>* stackVar;
            
            stackVar = new vector<VariableDescriptor*> (*varStack);
            instr = code->getInstruction (i)->getOp ();
            pos = instr.find (".");
            className = instr.substr (0, pos);
            methodName = instr.substr (pos + 1, instr.length () - pos - 3);      
            classInfo = ptrVM->getClassInfoForName (className);
            methodInfo = ptrVM->getMethodInfoOfClass (className, methodName);
            vectorArgs = new vector <VariableDescriptor*> (methodInfo->getParamCount ());

            for (int i = 0; i < methodInfo->getParamCount (); i++)
            {
                VariableDescriptor *desc = VariableDescriptor::copyVarDesc (varStack->back ());
                vectorArgs[0][methodInfo->getParamCount () - i - 1] = desc;
                varStack->pop_back ();
            }

            if (!methodInfo->getIsStatic ())
            {
                vectorArgs[0].insert (vectorArgs->begin (), 
                                      VariableDescriptor::copyVarDesc (varStack->back ()));
                varStack->pop_back ();
                jit_ldxi (JIT_R0, JIT_FP, vectorArgs[0][0]->getMemLoc ());
                jit_sti ((jit_pointer_t)ptrVM->getcalledObjectAddressMem (), JIT_R0);
            }
            else
            {
                jit_movi (JIT_R0, 0);
                jit_sti ((jit_pointer_t)ptrVM->getcalledObjectAddressMem (), JIT_R0);
            }
    
            jit_pushargi ((jit_word_t)vectorArgs);
            jit_pushargi ((jit_word_t)stackVar);
            jit_pushargi ((jit_word_t)classInfo);
            jit_pushargi ((jit_word_t)methodInfo);
            jit_finishi ((jit_pointer_t)VirtualMachine::callMethod);

            if (methodInfo->getType () != "void")
            {
                /* If the type of return value is not void then 
                 * that value is stored in the JIT_R0 register. 
                 */
                OperatorType optype = getOperatorTypeFromString (methodInfo->getType ());
                TempDescriptor *tempDesc;

                tempDesc = createTempDescriptor (getSizeFromOperatorType (optype), optype, "", 
                                                 methodInfo->getType ());
                tempDesc->setMemLocation ((unsigned long)ptrVM->getReturnValueMem ());
                restoreVariablesFromMemory ();
                allocateRegister (tempDesc);
                /* Copy the return value from return val memory to the register. 
                 * We would have to copy the value as long,
                 * as the memory is of long type */
                jit_ldi_l (tempDesc->getCurrLocation ().getValue (), 
                         (jit_pointer_t)tempDesc->getMemLoc ());
                varStack->push_back (tempDesc);
            }
            else
            {
                restoreVariablesFromMemory ();
            }            

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
            VariableDescriptor* tempDesc = varStack->back ();
    
            processPushInstr (tempDesc->getSize (), getOperatorTypeFromString (tempDesc->getType ()),
                              "0");
            goto L22;
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
            //newarr           
            /* On x86-64 return values are stored in RAX register. Save it first */
            VariableDescriptor *assignedVar;
            VariableDescriptor* tempDesc1;

            assignedVar = vectorIntRegisters [0]->getVarDescriptor ();
            
            if (assignedVar != LULL)
            {
                allocateMemory (assignedVar);
                jitStack->copyRegToMem (_jit, assignedVar->getMemLoc (),
                                        assignedVar->getCurrLocation ().getValue (),
                                        getOperatorTypeFromString (assignedVar->getType ()));
            }

            vectorIntRegisters [0]->assignVariable (LULL);
            
            /* Get the requested size */
            tempDesc1 = varStack->back ();
            varStack->pop_back ();

            if (allocateRegister (tempDesc1) &&
                dynamic_cast <TempDescriptor*> (tempDesc1) != LULL)
            {
                jitStack->copyMemToReg (_jit, tempDesc1->getMemLoc (),
                                        tempDesc1->getCurrLocation ().getValue (),
                                        getOperatorTypeFromString (tempDesc1->getType ()));
            }
            char *type = new char[10];
            strcpy (type, code->getInstruction(i)->getOp ().c_str ());
            //printf ("%s\n", type);
            jit_pushargi ((jit_word_t)type);
            jit_pushargr (tempDesc1->getCurrLocation ().getValue ());
            jit_finishi ((jit_pointer_t)VirtualMachine::allocateArray);
            
            TempDescriptor *tempDesc = createTempDescriptor (8, Long, "");
            vectorIntRegisters [0]->assignVariable (tempDesc);
            
            varStack->push_back (tempDesc);
            continue;
        }
 
        L54:
        {
            // push.str
             continue;
        }
 
        L55:
        {
            // push.len
            VariableDescriptor *arrayDesc;
            LocalDescriptor *arrayLocalDesc;
            LocalArray *localArray;

            arrayDesc = varStack->back ();
            varStack->pop_back ();
            arrayLocalDesc = dynamic_cast <LocalDescriptor*> (arrayDesc);
            localArray = dynamic_cast <LocalArray*> (arrayLocalDesc->getLocalVariable ());
            
            TempDescriptor *tempDesc;
            tempDesc = createTempDescriptor (4, Integer, "");
            jit_movi (tempDesc->getCurrLocation ().getValue (), localArray->getElements ());
            
            varStack->push_back (tempDesc);            
            continue;
        }
 
        L56:
        {
            // push.elema
            VariableDescriptor *arrayDesc;
            VariableDescriptor *indexDesc;
            LocalDescriptor *arrayLocalDesc;
           
            indexDesc = varStack->back ();
            varStack->pop_back ();
            
            arrayDesc = varStack->back ();
            varStack->pop_back ();

            arrayLocalDesc = dynamic_cast <LocalDescriptor*> (arrayDesc);
            TempDescriptor *tempDesc = createTempDescriptor (8, Long, "");
            OperatorType optype = getOperatorTypeFromString (arrayDesc->getType ());
            int opsize = getSizeFromOperatorType (optype);
        
            if (allocateRegister (indexDesc) &&
                dynamic_cast <TempDescriptor*> (indexDesc) != LULL)
            {
                jitStack->copyMemToReg (_jit, indexDesc->getMemLoc (),
                                        indexDesc->getCurrLocation ().getValue (),
                                        getOperatorTypeFromString (indexDesc->getType ()));
            }
        
            if (allocateRegister (arrayDesc) &&
                dynamic_cast <TempDescriptor*> (arrayDesc) != LULL)
            {
                jitStack->copyMemToReg (_jit, arrayDesc->getMemLoc (),
                                        arrayDesc->getCurrLocation ().getValue (),
                                        getOperatorTypeFromString (arrayDesc->getType ()));
            }
            
            /* Set memory of temporary descriptor 
               to that of the element of array */
            /* Calculate the effective address and load */
            jit_muli (tempDesc->getCurrLocation().getValue (), 
                      indexDesc->getCurrLocation().getValue (), opsize);
            jit_addr (tempDesc->getCurrLocation().getValue (),
                      arrayDesc->getCurrLocation().getValue (),
                      tempDesc->getCurrLocation().getValue ());
            
            varStack->push_back (tempDesc);
            continue;
        }
 
        L57:
        {
            // stelem
            VariableDescriptor *arrayDesc;
            VariableDescriptor *indexDesc;
            LocalDescriptor *arrayLocalDesc;
            VariableDescriptor *valueDesc;
            
            indexDesc = varStack->back ();
            varStack->pop_back ();

            arrayDesc = varStack->back ();
            varStack->pop_back ();
            
            valueDesc = varStack->back ();
            varStack->pop_back ();
            //jit_finishi ((jit_pointer_t)func);
            arrayLocalDesc = dynamic_cast <LocalDescriptor*> (arrayDesc);
            TempDescriptor *tempDesc = createTempDescriptor (8, Long, "");
            OperatorType optype = getOperatorTypeFromString (arrayDesc->getType ());
            int opsize = getSizeFromOperatorType (optype);

            if (allocateRegister (valueDesc) &&
                dynamic_cast <TempDescriptor*> (valueDesc) != LULL)
            {
                jitStack->copyMemToReg (_jit, valueDesc->getMemLoc (),
                                        valueDesc->getCurrLocation ().getValue (),
                                        getOperatorTypeFromString (valueDesc->getType ()));
            }
        
            if (allocateRegister (indexDesc) &&
                dynamic_cast <TempDescriptor*> (indexDesc) != LULL)
            {
                jitStack->copyMemToReg (_jit, indexDesc->getMemLoc (),
                                        indexDesc->getCurrLocation ().getValue (),
                                        getOperatorTypeFromString (indexDesc->getType ()));
            }
        
            if (allocateRegister (arrayDesc) &&
                dynamic_cast <TempDescriptor*> (arrayDesc) != LULL)
            {
                jitStack->copyMemToReg (_jit, arrayDesc->getMemLoc (),
                                        arrayDesc->getCurrLocation ().getValue (),
                                        getOperatorTypeFromString (arrayDesc->getType ()));
            }
            
            /* Set memory of temporary descriptor
               to that of the element of array */
            /* Calculate the effective address and load */
            jit_muli (tempDesc->getCurrLocation().getValue (), 
                      indexDesc->getCurrLocation().getValue (), opsize);
            jit_addr (tempDesc->getCurrLocation().getValue (),
                      arrayDesc->getCurrLocation().getValue (),
                      tempDesc->getCurrLocation().getValue ());
            jitStack->copyRegToMemr (_jit, tempDesc->getCurrLocation ().getValue (),
                                    valueDesc->getCurrLocation ().getValue (),
                                    getOperatorTypeFromString (arrayLocalDesc->getType ()));     
            continue;
        }
 
        L58:
        {
            // ldelem
            VariableDescriptor *arrayDesc;
            VariableDescriptor *indexDesc;
            LocalDescriptor *arrayLocalDesc;

            indexDesc = varStack->back ();
            varStack->pop_back ();
            
            arrayDesc = varStack->back ();
            varStack->pop_back ();
            OperatorType optype = getOperatorTypeFromString (arrayDesc->getType ());
            int opsize = getSizeFromOperatorType (optype);

            arrayLocalDesc = dynamic_cast <LocalDescriptor*> (arrayDesc);
            TempDescriptor *tempDesc = createTempDescriptor (arrayLocalDesc->getSize (), 
                                                             getOperatorTypeFromString (arrayDesc->getType ()),
                                                             "");
            TempDescriptor *tempDesc1 = createTempDescriptor (8, Long, "");
            
            if (allocateRegister (indexDesc) &&
                dynamic_cast <TempDescriptor*> (indexDesc) != LULL)
            {
                jitStack->copyMemToReg (_jit, indexDesc->getMemLoc (),
                                        indexDesc->getCurrLocation ().getValue (),
                                        getOperatorTypeFromString (indexDesc->getType ()));
            }
        
            if (allocateRegister (arrayDesc) &&
                dynamic_cast <TempDescriptor*> (arrayDesc) != LULL)
            {
                jitStack->copyMemToReg (_jit, arrayDesc->getMemLoc (),
                                        arrayDesc->getCurrLocation ().getValue (),
                                        getOperatorTypeFromString (arrayDesc->getType ()));
            }
            
            /* Calculate the effective address and load */
            jit_muli (tempDesc1->getCurrLocation().getValue (), 
                      indexDesc->getCurrLocation().getValue (), opsize);
            
            jit_addr (tempDesc1->getCurrLocation().getValue (),
                      arrayDesc->getCurrLocation().getValue (),
                      tempDesc1->getCurrLocation().getValue ());
            jitStack->copyMemrToReg (_jit, tempDesc1->getCurrLocation ().getValue (),
                                     tempDesc->getCurrLocation ().getValue (),
                                     getOperatorTypeFromString (arrayLocalDesc->getType ()));
            varStack->push_back (tempDesc);
            continue;
        }
 
        L59:
        {
            // new <type>
            /* TODO: Call Constructor Also */
            /* TODO: Keep track of all new stackVar created to free all of them */
            VariableDescriptor *assignedVar;
            TempDescriptor *tempDesc;
            char *type = new char[10];
            vector<VariableDescriptor*>* stackVar;
            
            storeVariablesToMemory ();
            stackVar = new vector<VariableDescriptor*> ();
            /* Have to create a copy of varStack. As we dont want 
             * stackVar to be changed */
            for (int j = 0; j < varStack->size (); j++)
            {
                /* Allocate memory or update memory of all 
                 * temporaries if they are reference */
                /* Only push temporary descriptors to the stackVar
                 * No need of pushing LocalDescriptor as we have
                 * already passed them */
                TempDescriptor* tempDesc;
                
                tempDesc = dynamic_cast <TempDescriptor*> (varStack->at(j));

                if (getOperatorTypeFromString (varStack->at(j)->getType ()) == Reference &&
                    tempDesc != null)
                {
                    stackVar->push_back (tempDesc);
                }
            }
        
            strcpy (type, code->getInstruction(i)->getOp ().c_str ());            

            jit_pushargi ((jit_word_t)stackVar);
            jit_pushargi ((jit_word_t)type);
            jit_finishi ((jit_pointer_t)VirtualMachine::allocateObject);
            tempDesc = createTempDescriptor (8, Reference, 
                                             "", code->getInstruction(i)->getOp ());
            allocateMemory (tempDesc);
            jit_retval_l (JIT_R0);
            jitStack->copyRegToMem (_jit, tempDesc->getMemLoc (), JIT_R0, Reference);
            restoreVariablesFromMemory ();

            if (allocateRegister (tempDesc))
            {
                jitStack->copyMemToReg (_jit, tempDesc->getMemLoc (),
                                        tempDesc->getCurrLocation ().getValue (),
                                        getOperatorTypeFromString (tempDesc->getType ()));
            }
            
            varStack->push_back (tempDesc);
            continue;
        }
 
        L60:
        {
            //returnval
            
            VariableDescriptor *returnDesc = varStack->back ();
            varStack->pop_back ();
            
            if (allocateRegister (returnDesc) &&
                dynamic_cast <TempDescriptor*> (returnDesc) != LULL)
            {
                jitStack->copyMemToReg (_jit, returnDesc->getMemLoc (),
                                        returnDesc->getCurrLocation ().getValue (),
                                        getOperatorTypeFromString (returnDesc->getType ()));
            }
        
            /* Copy the return value to VM's returnValMem */
            jit_sti_l (ptrVM->getReturnValueMem (), 
                       returnDesc->getCurrLocation ().getValue ());
            continue;
        }
 
        L61:
        {
            // return
            jit_ret();
            continue;
        }
 
        L62:
        {
            // ldloc
            int index = atoi (code->getInstruction (i)->getOp ().c_str ());
            LocalDescriptor* localDesc = vectorLocalDescriptors [index];

            allocateRegister (localDesc);
            varStack->push_back (localDesc);
            continue;
        }
 
        L63:
        {
            // stloc
            int index = atoi (code->getInstruction (i)->getOp ().c_str ());
            LocalDescriptor* localDesc = vectorLocalDescriptors [index];
            VariableDescriptor* varDesc = varStack->back ();
            varStack->pop_back ();

            //Topmost value will always be in the register
            if (localDesc->getCurrLocation().getLocationType () == RegisterLocation)
            {
                jit_movr (localDesc->getCurrLocation().getValue (),
                          varDesc->getCurrLocation().getValue ());
                // FIXME: Update value in memory
                jitStack->copyRegToMem (_jit, localDesc->getMemLoc (), 
                                        varDesc->getCurrLocation().getValue (),
                                        getOperatorTypeFromString (localDesc->getType ()));
            }
            else
            {
                jitStack->copyRegToMem (_jit, localDesc->getMemLoc (), 
                                        varDesc->getCurrLocation().getValue (),
                                        getOperatorTypeFromString (localDesc->getType ()));
            }

            continue;
        }
 
        L64:
        {
            //ldfield
            VariableDescriptor* varDesc;
            TempDescriptor *tempDesc;
            TempDescriptor *tempDesc1;
            string instr;
            string className;
            string fieldName;
            MemberInfo *info;

            size_t pos;
            OperatorType optype;
            
            instr = code->getInstruction (i)->getOp ();
            pos = instr.find (".");
            className = instr.substr (0, pos);
            fieldName = instr.substr (pos + 1, instr.length () - pos - 1);            
            
            pos = ptrVM->getPosForField (className, fieldName, &info);
            optype = getOperatorTypeFromString (info->getType());
            tempDesc = createTempDescriptor (getSizeFromOperatorType (optype),
                                             optype, "", info->getType());
            allocateRegister (tempDesc);

            if (!info->getIsStatic ())
            {
                /* If the member is not static only then pop the stack to get 
                 * the address of object */
                varDesc = varStack->back ();
                varStack->pop_back ();
            
                tempDesc1 = createTempDescriptor (8, Long);
                allocateRegister (tempDesc1);
    
                if (allocateRegister (varDesc) &&
                    dynamic_cast <TempDescriptor*> (varDesc) != LULL)
                {
                    jitStack->copyMemToReg (_jit, varDesc->getMemLoc (),
                                            varDesc->getCurrLocation ().getValue (),
                                            getOperatorTypeFromString (varDesc->getType ()));
                }
    
                jit_addi (tempDesc1->getCurrLocation().getValue (),
                          varDesc->getCurrLocation ().getValue(),
                          pos);
                jitStack->copyMemrToReg (_jit, tempDesc1->getCurrLocation ().getValue (),
                                         tempDesc->getCurrLocation ().getValue (),
                                         optype);
                varStack->push_back (tempDesc);
                delete tempDesc1;
            }
            else
            {
                /* But if it is static then do not pop the object */
                if (optype == Reference)
                {
                    /* If a reference then copy the value of reference to register*/
                    jit_movi (tempDesc->getCurrLocation ().getValue (), pos);
                }
                else
                {
                    /* Otherwise copy the value of the memory address to the register*/
                    jitStack->copyMemiToReg (_jit, pos,
                                             tempDesc->getCurrLocation ().getValue (),
                                             optype);
                }
                varStack->push_back (tempDesc);
            }

            continue;
        }
 
        L65:
        {
            // stfield
            VariableDescriptor* varDesc;
            VariableDescriptor* valueDesc;
            TempDescriptor *tempDesc1;
            string instr;
            string className;
            string fieldName;
            MemberInfo* info;
            size_t pos;

            OperatorType optype;
            
            instr = code->getInstruction (i)->getOp ();
            pos = instr.find (".");
            className = instr.substr (0, pos);
            fieldName = instr.substr (pos + 1, instr.length () - pos - 1);           
            
            valueDesc = varStack->back ();
            varStack->pop_back ();
            
            pos = ptrVM->getPosForField (className, fieldName, &info);
            optype = getOperatorTypeFromString (info->getType ());

            if (info->getIsStatic ())
            {
                if (allocateRegister (valueDesc) &&
                    dynamic_cast <TempDescriptor*> (valueDesc) != LULL)
                {
                    jitStack->copyMemToReg (_jit, valueDesc->getMemLoc (),
                                            valueDesc->getCurrLocation ().getValue (),
                                            getOperatorTypeFromString (valueDesc->getType ()));
                }
                /* But if it is static then do not pop the object */
                jitStack->copyRegToMemi (_jit, pos,
                                         valueDesc->getCurrLocation ().getValue (),
                                         optype);
            }
            else
            {
                varDesc = varStack->back ();
                varStack->pop_back ();            

                tempDesc1 = createTempDescriptor (8, Long);
                allocateRegister (tempDesc1);
            
                if (allocateRegister (valueDesc) &&
                    dynamic_cast <TempDescriptor*> (valueDesc) != LULL)
                {
                    jitStack->copyMemToReg (_jit, valueDesc->getMemLoc (),
                                            valueDesc->getCurrLocation ().getValue (),
                                            getOperatorTypeFromString (valueDesc->getType ()));
                }

                if (allocateRegister (varDesc) &&
                    dynamic_cast <TempDescriptor*> (varDesc) != LULL)
                {
                    jitStack->copyMemToReg (_jit, varDesc->getMemLoc (),
                                            varDesc->getCurrLocation ().getValue (),
                                            getOperatorTypeFromString (varDesc->getType ()));
                }
    
                jit_addi (tempDesc1->getCurrLocation().getValue (),
                          varDesc->getCurrLocation ().getValue(),
                          pos);
                jitStack->copyRegToMemr (_jit, tempDesc1->getCurrLocation ().getValue (),
                                         valueDesc->getCurrLocation ().getValue (),
                                         optype);
                                        
                vectorIntRegisters[tempDesc1->getCurrLocation ().getValue ()]->assignVariable (LULL);
                delete tempDesc1;
            }

            continue;
        }
 
        L66:
        {
            //ldarg
            string arg;
            int narg;
            VariableDescriptor *varDesc;

            arg = code->getInstruction (i)->getOp ();
            narg = atoi (arg.c_str ());
            varDesc = vectorArgs [0][narg];

            if (allocateRegister (varDesc) &&
                dynamic_cast <TempDescriptor*> (varDesc) != LULL)
            {
                jitStack->copyMemToReg (_jit, varDesc->getMemLoc (),
                                        varDesc->getCurrLocation ().getValue (),
                                        getOperatorTypeFromString (varDesc->getType ()));
            }

            varStack->push_back (varDesc);
            continue;
        }
        
        L67:
        {
            //starg
            int index = atoi (code->getInstruction (i)->getOp ().c_str ());
            VariableDescriptor* argDesc = vectorArgs[0][index];
            VariableDescriptor* varDesc = varStack->back ();
            varStack->pop_back ();

            //Topmost value will always be in the register
            if (argDesc->getCurrLocation().getLocationType () == RegisterLocation)
            {
                jit_movr (argDesc->getCurrLocation().getValue (),
                          varDesc->getCurrLocation().getValue ());
                // FIXME: Update value in memory
                jitStack->copyRegToMem (_jit, argDesc->getMemLoc (), 
                                        varDesc->getCurrLocation().getValue (),
                                        getOperatorTypeFromString (argDesc->getType ()));
            }
            else
            {
                jitStack->copyRegToMem (_jit, argDesc->getCurrLocation().getValue (), 
                                        varDesc->getCurrLocation().getValue (),
                                        getOperatorTypeFromString (argDesc->getType ()));
            }

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
                varStack = new vector<VariableDescriptor*> (*varStackStack.top ());
                varStackStack.push (varStack);
            }
 
            continue;
        }
    }
}

static int func ()
{
    return 2;
}

int JIT::runMethodCode (vector<VariableDescriptor*>* vectorArgs, MethodCode *code, 
                        unsigned long *prevSPMem)
{
    /* Before a function call Push all Registers on stack */
    /* Before returning pop all registers from stack */
    this->vectorArgs = vectorArgs;
    jit_node_t *in;
 
    prevStackPointerMem = prevSPMem;
    //currentJIT = this;
    if (argsMemLocation == LULL)
        argsMemLocation = new long[vectorArgs->size ()];

    for (int i = 0; i < vectorArgs->size (); i++)
    {
        argsMemLocation [i] = vectorArgs->at (i)->getMemLoc ();
    }

    if (compiledFunction == LULL)
    {
        jitStack = new JITStack ();
 
        _jit = jit_new_state();
        jit_prolog();
        jit_sti ((jit_pointer_t)stackPointerMem, JIT_FP);
        convertCode (vectorArgs, code);
        //printf ("RUNNING CODE\n");
        //jit_node_t *jump;
        //jit_movi (JIT_R0, 10);
        //jump = jit_beqi (JIT_R0, 10);
        //jit_movi (JIT_R0, 3);
        //jit_patch (jump);
        //jit_finishi((jit_pointer_t)func);
        //jit_retval_i(JIT_R1);
        //jit_finishi ((jit_pointer_t)func);
        //jit_ldi_l (JIT_R1, (jit_pointer_t)vectorTempDescriptors [vectorTempDescriptors.size () - 1]->getCurrLocation ().getValue ());
    
        jit_pushargi((jit_word_t)" %ld ll\n");
        jit_ellipsis();
        //printf ("TOP %d \n", vectorTempDescriptors [vectorTempDescriptors.size () - 1]->getCurrLocation ().getValue ());
        //jit_pushargr(vectorTempDescriptors [vectorTempDescriptors.size () - 1]->getCurrLocation ().getValue ());
        //jit_pushargr );
        jit_pushargr(varStack->back ()->getCurrLocation ().getValue ());
        //jit_pushargr (JIT_FP);
        //jit_pushargr(vectorLocalDescriptors [0]->getCurrLocation ().getValue ());
        //jit_movi_d (JIT_F1, 9.60);
        //jit_pushargr_d (JIT_F0);
        //jit_pushargr (JIT_V2);
        jit_finishi((jit_pointer_t)printf);
        jit_ret();
        jit_epilog();
    
        compiledFunction = (compiledFunctionSig)jit_emit();
    }

    compiledFunction();

    return 0;
}