#ifndef COMPUTER_ARCHITECTURE_046267_CACHELEVEL_H
#define COMPUTER_ARCHITECTURE_046267_CACHELEVEL_H

#include "tagEntry.h"
namespace cacheSim {

    class CacheLevel {
    private:
        const unsigned int cyclesAccess;
        const unsigned int log2LevelSize;
        const unsigned int log2NumWays;
        unsigned int numWays;
        const unsigned int log2BlockSize;
        unsigned int log2WaySize;
        unsigned int numOfSets;
        unsigned int countAccess;
        unsigned int countMiss;
        tagEntry **ways;
        unsigned int tagCalculator(unsigned long int address) const;
        unsigned int setCalculator(unsigned long int address) const;
        //return which way to store new tag
        int getWayToStore(unsigned long int address) const;//return which way was remove

    public:
        //constructor
        CacheLevel(unsigned int cyclesAccess,unsigned int log2Size,unsigned int log2NumWays ,unsigned int log2BlockSize);
        //destructor
        ~CacheLevel();
        //storing new tag and return the removed address an if it was dirty
        bool storeNewAddress(unsigned long int address, int time, unsigned long int* addressRemoved, bool* wasDirty);
        //check if address exist in this level
        bool isAddressExist(unsigned long int address);
        //update block time stamp for LRU policy
        bool updateBlockTimeStamp(unsigned long int address, int time);
        //adding one to misses counter
        void incCountMiss();
        //adding one to access counter
        void incCountAccess();
        unsigned int getCountMiss() const ;
        unsigned int getCountAccess() const ;
        unsigned int getCycleAccess() const ;
        //remove block from chache without checking if it is dirty
        bool removeBlock(unsigned long int address);
        //mark block as dirty
        void markBlockDirty(unsigned long int address);
    };
}
#endif //COMPUTER_ARCHITECTURE_046267_CACHELEVEL_H
