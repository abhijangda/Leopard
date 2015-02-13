#include "classinfo.h"

MemberInfo::MemberInfo (string _name, string _type, bool _isStatic, AccessSpecifier _accessSpec)
{
    name = _name;
    type = _type;
    isStatic = _isStatic;
    accessSpec = _accessSpec;
}

Local::Local (int _n, int _size, string _type)
{
    number = _n;
    size = _size;
    type = _type;
}

MethodCode::MethodCode (vector<Local*> vLocal, vector<Instruction*> vInstr) : vectorLocals (), vectorInstructions ()
//MethodCode::MethodCode  ()
{
    for (int i = 0; i < vLocal.size (); i++)
    {
        vectorLocals.insert (vectorLocals.end (), vLocal [i]);
    }
    
    for (int i = 0; i < vInstr.size (); i++)
    {
        vectorInstructions.insert (vectorInstructions.end (), vInstr [i]);
    }
}

void MethodCode::addLocal (Local& local)
{
    //vectorLocals.insert (vectorLocals.end (), local);
}

void MethodCode::addInstruction (Instruction& instruction)
{
    //vectorInstructions.insert (instruction);
}

MethodInfo::MethodInfo (string _name, string _type, bool _isStatic, AccessSpecifier _accessSpec, 
                        vector<string> paramTypes, MethodCode* _code) :
 MemberInfo (_name, _type, _isStatic, _accessSpec), vectorParamTypes ()
{
    for (int i = 0; i < paramTypes.size (); i++)
    {
        vectorParamTypes.insert (vectorParamTypes.end (), paramTypes[i]);
    }

    code = _code;
}

void MethodInfo::addParamType (string type)
{
    vectorParamTypes.insert (vectorParamTypes.end (), type);
}
            
ClassInfo::ClassInfo (string _name, int _size, string _parent, vector <MemberInfo*> vMemInfo, 
                      vector <MethodInfo*> vMethInfo) :
    vectorMethods (), vectorMembers ()
{
    name = _name;
    size = _size;
    parent = _parent;

    for (int i = 0; i < vMemInfo.size (); i++)
    {
        vectorMembers.insert (vectorMembers.end (), vMemInfo [i]);
    }
    
    for (int i = 0; i < vMethInfo.size (); i++)
    {
        vectorMethods.insert (vectorMethods.end (), vMethInfo [i]);
    }
}

int getPosForField (string name)
{
}

int ClassInfo::addMethodInfo (MethodInfo& methodInfo)
{
    //vectorMethods.insert (vectorMethods.end (), methodInfo);
}

int ClassInfo::addMemberInfo (MemberInfo& memberInfo)
{
    //vectorMembers.insert (vectorMembers.end (), memberInfo);
}