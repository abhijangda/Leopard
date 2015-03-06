#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <list>

#include "classinfo.h"

#ifndef __Allocated_Object_H__
#define __Allocated_Object_H__

class AllocatedVariable;

class MemoryBlock
{
    private:
        unsigned long size; //Size is in Bytes
        unsigned long startPos;
        byte *memory;
        AllocatedVariable *allocVar;
    
    public:
        MemoryBlock (unsigned long size, unsigned long startPos, byte *memory)
        {
            this->size = size;
            this->startPos = startPos;
            this->memory = memory;
        }
    
        MemoryBlock (unsigned long size, byte *memory)
        {
            this->memory = memory;
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
        
        unsigned long getSize ()
        {
            return size;
        }
    
        void setAllocatedVariable (AllocatedVariable *_allocVar)
        {
            allocVar = _allocVar;
        }
    
        AllocatedVariable* getAllocatedVariable ()
        {
            return allocVar;
        }
};

class AllocatedVariable
{
    private:
        MemoryBlock *memBlock;
        /* This vector collects the addresses which contains address to this
         * memory block's data. In other words, the address of the reference
         * variable */
        vector<unsigned long *> vectorAddresses;

    protected:
        AllocatedVariable (MemoryBlock *memBlock)
        {
            this->memBlock = memBlock;
            memBlock->setAllocatedVariable (this);
        }

    public:
        virtual string getType () = 0;

        MemoryBlock* getMemBlock () const
        {
            return memBlock;
        }
        
        void setMemBlock (MemoryBlock* memBlock)
        {
            this->memBlock = memBlock;
        }
        
        void pushAddress (unsigned long* address)
        {
            vectorAddresses.push_back (address);
        }
        
        void clearAllAddress ()
        {
            vectorAddresses.clear ();
        }
    
        int totalAddresses ()
        {
            return vectorAddresses.size ();
        }
    
        unsigned long* popAddress ()
        {
            unsigned long* p = vectorAddresses.back ();
            vectorAddresses.pop_back ();
            return p;
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
            reachable = false;
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
    
        int getTotalChildren ()
        {
            return listChildren.size ();
        }

        string getType ()
        {
            return classInfo->getName ();
        }

        ClassInfo* getClassInfo ()
        {
            return classInfo;
        }
    
        void setReachable ()
        {
            reachable = true;
        }
    
        void unsetReachable ()
        {
            reachable = false;
        }
    
        bool getIsReachable ()
        {
            return reachable;
        }
        

    private:
        ClassInfo *classInfo;
        vector<AllocatedObject*> listChildren;
        
        bool reachable;
};

#endif /* __Allocated_Object_H__ */