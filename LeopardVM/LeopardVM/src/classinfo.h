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

enum LocalType
{
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
    protected:
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
    
        virtual ~Local () {}
};

class LocalArray : public Local
{
    private:
        int n_elements;
    
    public:
        LocalArray (int _n, int _size, string _type_element, int _n_elements) : Local (_n, _size, _type_element)
        {
            n_elements = _n_elements;
        }

        int getElements () const
        {
            return n_elements;
        }
    
        virtual ~LocalArray () {}
};

class LocalReference
{
};

class MethodCode
{
    private:
        vector<Local*> vectorLocals;
        vector<Instruction*> vectorInstructions;
        
    public:
        MethodCode (vector<Local*>, vector<Instruction*>);
        ~MethodCode ()
        {
            for (int i = 0; i < vectorLocals.size (); i++)
            {
                delete vectorLocals[i];
            }
        
            for (int i = 0; i < vectorInstructions.size (); i++)
            {
                delete vectorInstructions[i];
            }
        }

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
        string getParamType (int index)
        {
            return vectorParamTypes [index];
        }

        int getParamCount ()
        {
            return vectorParamTypes.size ();
        }

        MethodCode *getCode ()
        {
            return code;
        }
    
        ~MethodInfo ()
        {
            delete code;
        }
};

class ClassInfo
{
    private:
        string name;
        int size;
        int n_members;
        int n_methods;
        vector <MemberInfo*> vectorMembers;
        vector <MethodInfo*> vectorMethods;
        vector <ClassInfo*> listChildren;
        ClassInfo* parent;
        
    
    public:
        ClassInfo (string _name, int _size, ClassInfo* _parent, vector <MemberInfo*> vMemInfo,
                   vector <MethodInfo*> vMethInfo);
        
        ~ClassInfo ()
        {
            for (int i = 0; i < vectorMembers.size (); i++)
            {
                delete vectorMembers[i];
            }
            
            for (int i = 0; i < vectorMethods.size (); i++)
            {
                delete vectorMethods[i];
            }
        }

        void addChildClass (ClassInfo* child)
        {
            listChildren.push_back (child);
        }

        string getName ()
        {
            return name;
        }
        
        string getParent ()
        {
            return parent->getName ();
        }
    
        ClassInfo* getParentClassInfo ()
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