#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <list>

#include "classinfo.h"

#ifndef __Allocated_Object_H__
#define __Allocated_Object_H__

class MemoryBlock
{
    private:
        unsigned long size; //Size is in Bytes
        unsigned long startPos;
        byte *memory;
    
    public:
        MemoryBlock (unsigned long size, unsigned long startPos, byte *memory)
        {
            this->size = size;
            this->startPos = startPos;
            this->memory = memory;
        }
    
        MemoryBlock (unsigned long size)
        {
            this->memory = new byte[size];
            this->size = size;
            this->startPos = (unsigned long)this->memory;
        }
        
        unsigned long getStartPos ()
        {
            return startPos;
        }
    
        byte *getMemory ()
        {
            return memory;
        }
};

class AllocatedVariable
{
    private:
        MemoryBlock *memBlock;
    
    protected:
        AllocatedVariable (MemoryBlock *memBlock)
        {
            this->memBlock = memBlock;
        }

    public:
        virtual string getType () = 0;

        MemoryBlock* getMemBlock () const
        {
            return memBlock;
        }
};

class AllocatedPrimitive : public AllocatedVariable
{
    private:
        string type;

    public:
        AllocatedPrimitive (string _type, MemoryBlock* _memBlock) :
            AllocatedVariable (_memBlock)
        {
            type = _type;
        }
    
        string getType ()
        {
            return type;
        }
};

class AllocatedObject : public AllocatedVariable
{
    public:
        AllocatedObject (ClassInfo *_classInfo, MemoryBlock* _memBlock) :
            AllocatedVariable (_memBlock)
        {
            classInfo = _classInfo;
        }

        void addChild (AllocatedObject *child)
        {
            listChildren.insert (listChildren.end (), child);
        }
        
        AllocatedObject* getChild (int i)
        {
            return listChildren[i];
        }
    
        string getType ()
        {
            return classInfo->getName ();
        }

        ClassInfo* getClassInfo ()
        {
            return classInfo;
        }

    private:
        ClassInfo *classInfo;
        vector<AllocatedObject*> listChildren;
};

#endif /* __Allocated_Object_H__ */