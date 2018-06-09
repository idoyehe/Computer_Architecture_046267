#ifndef COMPUTER_ARCHITECTURE_046267_CACHELEVEL_H
#define COMPUTER_ARCHITECTURE_046267_CACHELEVEL_H

#include "cacheEntry.h"
namespace cacheSim {

    class CacheLevel {
    private:
        const unsigned int cyclesAccess;
        const unsigned int log2Size;
        const unsigned int log2NumWays;
        unsigned int numWays;
        const unsigned int log2BlockSize;
        unsigned int log2WaySize;
        unsigned int waySizeEntries;
        unsigned int countAccess;
        unsigned int countMiss;
        CacheEntry **ways;
    public:
        CacheLevel(unsigned int cyclesAccess,unsigned int log2Size,unsigned int log2NumWays ,unsigned int log2BlockSize);
        ~CacheLevel();

        unsigned int tagCalculator(unsigned long int address) const;
        unsigned int setCalculator(unsigned long int address) const;
        int getWayToStore(unsigned long int address) const;//return which way was remove
        bool storeNewAddress(unsigned long int address, int time, unsigned long int* addressRemoved);
        bool isAddressExist(unsigned long int address);
        bool updateBlockTimeStamp(unsigned long int address, int time);

        void incCountMiss();
        void incCountAccess();
        unsigned int getCountMiss() const ;
        unsigned int getCountAccess() const ;
        unsigned int getCycleAccess() const ;
        bool removeBlock(unsigned long int address);
    };
}
#endif //COMPUTER_ARCHITECTURE_046267_CACHELEVEL_H
