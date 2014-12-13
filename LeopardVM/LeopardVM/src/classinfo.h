#include <string>
#include <vector>
#include <iostream>

#include "main.h"

#ifndef __CLASSINFO_H__
#define __CLASSINFO_H__

using namespace std;

enum AccessSpecifier 
{
    Public = 0,
    Private,
    Protected
};

class MemberInfo
{
    protected:
        AccessSpecifier accessSpec;
        bool isStatic;
        string type;
        string name;
    
    public:
        bool getIsStatic () const
        {
            return isStatic;
        }

        string getType () const
        {
            return type;
        }

        string getName () const
        {
            return name;
        }

        AccessSpecifier getAcessSpecifier () const
        {
            return accessSpec;
        }

        MemberInfo (string _name, string _type, bool _isStatic, AccessSpecifier _accessSpec);
};

class Instruction
{
    private:
        byte byteCode;
        int op_size;
        string op;
    
    public:
        Instruction (byte _byteCode, int _op_size, string _op)
        {
            byteCode = _byteCode;
            op_size = _op_size;
            op = _op;
        }
    
        byte getByteCode ()
        {
            return byteCode;
        }
    
        int getOpSize ()
        {
            return op_size;
        }
        
        string getOp ()
        {
            return op;
        }        
};

class Local
{
    private:
        int number;
        int size;
        string type;
    
    public:
        int getNumber () const
        {
            return number;
        }
        
        int getSize () const
        {
            return size;
        }
    
        string getType () const
        {
            return type;
        }
    
        Local (int _n, int _size, string _type);
};

class MethodCode
{
    private:
        vector<Local*> vectorLocals;
        vector<Instruction*> vectorInstructions;
        
    public:
        MethodCode (vector<Local*>, vector<Instruction*>);
        //MethodCode ();
        void addLocal (Local& local);
        void addInstruction (Instruction& instruction);
        int getTotalLocals ()
        {
            return vectorLocals.size ();
        }
    
        int getTotalInstructions ()
        {
            return vectorInstructions.size ();
        }
    
        Local* getLocal (int i)
        {
            return vectorLocals [i];
        }
    
        Instruction* getInstruction (int i)
        {
            return vectorInstructions [i];
        }
};

class MethodInfo : public MemberInfo
{
    private:
        vector<string> vectorParamTypes;
        MethodCode* code;
    
    public:
        MethodInfo (string _name, string _type, bool _isStatic, AccessSpecifier _accessSpec, 
                    vector<string> paramTypes, MethodCode* code);
        void addParamType (string type);
        int getParamCount ()
        {
            return vectorParamTypes.size ();
        }
        MethodCode *getCode ()
        {
            return code;
        }
};

class ClassInfo
{
    private:
        string name;
        int size;
        string parent;
        int n_members;
        int n_methods;
        vector <MemberInfo*> vectorMembers;
        vector <MethodInfo*> vectorMethods;
    
    public:
        ClassInfo (string _name, int _size, string _parent, vector <MemberInfo*> vMemInfo, 
                   vector <MethodInfo*> vMethInfo);
        
        string getName ()
        {
            return name;
        }
        
        string getParent ()
        {
            return parent;
        }
    
        int getSize ()
        {
            return size;
        }
    
        int totalMembers ()
        {
            return vectorMembers.size ();
        }
    
        int totalMethods ()
        {
            return vectorMethods.size ();
        }
    
        MethodInfo* getMethodInfo (int index)
        {
            return vectorMethods [index];
        }
    
        MemberInfo* getMemberInfo (int index)
        {
            return vectorMembers [index];
        }
        
        int addMethodInfo (MethodInfo& methodInfo);
        int addMemberInfo (MemberInfo& memberInfo);        
};

#endif /* __CLASSINFO_H__ */