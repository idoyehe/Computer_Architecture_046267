#ifndef COMPUTER_ARCHITECTURE_046267_TWOLEVELCACHE_H
#define COMPUTER_ARCHITECTURE_046267_TWOLEVELCACHE_H

#include "cacheLevel.h"
#include <string>
#include <iostream>

using std::string;
using std::cout;
using std::endl;

namespace cacheSim {
    typedef enum {WRITE_NO_ALLOCATE = 0, WRITE_ALLOCATE = 1} WritePolicy;

    class TwoLevelCache {
    private:
        unsigned int cyclesCounter;
        const unsigned int memoryAccCycles;
        const WritePolicy writePolicy;
        unsigned int countAccess;
        unsigned time;
        CacheLevel L1;
        CacheLevel L2;
        //access the cache first to L1 and than to L2 and than to memory
        void accessCache(unsigned long int address);
    public:
        //constructor
        TwoLevelCache(unsigned int memoryAccCycles, unsigned int log2BlockSize, WritePolicy writePolicy,
                      unsigned int log2L1size, unsigned int log2L1numWays,
                      unsigned int L1AccCycles,unsigned int log2L2size, unsigned int log2L2numWays,
                      unsigned int L2AccCycles);
        //read operation
        void readFromAddress(unsigned long int address);
        //write operation
        void writeToAddress(unsigned long int address);
        //calculating L1 MR
        double getL1MissRate() const;
        //calculating L1 MR
        double getL2MissRate() const;
        //calculating L1 AMAT
        double getAccTimeAvg () const;
        int getCountAccess() const ;
        int getCountCycle() const ;
















    };
}
#endif //COMPUTER_ARCHITECTURE_046267_TWOLEVELCACHE_H
