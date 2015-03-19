#ifndef __BASIC_BLOCK_H__
#define __BASIC_BLOCK_H__

class BasicBlock
{
    public:
        int startInstrIndex; /* The first instruction of BasicBlock */
        int length; /* lengthof BasicBlock */

    public:
        BasicBlock (int _start, int _length)
        {
            startInstrIndex = _start;
            length = _length;
            executionCount = 0;
        }
        
        int executionCount;
};

#endif /*__BASIC_BLOCK_H__*/