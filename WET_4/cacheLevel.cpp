#include <cassert>
#include "cacheLevel.h"
#define INVALID -1

cacheSim::CacheLevel::CacheLevel(unsigned int cyclesAccess,unsigned int log2Size,unsigned int log2NumWays ,unsigned int log2BlockSize):
        cyclesAccess(cyclesAccess),
        log2Size(log2Size),
        log2NumWays(log2NumWays),
        numWays(1),
        log2BlockSize(log2BlockSize),
        waySizeEntries(1),
        countAccess(0),
        countMiss(0),
        ways(nullptr)
{
    int log2WaySize = (this->log2Size - this->log2BlockSize) - this->log2NumWays;
    this->waySizeEntries = this->waySizeEntries << log2WaySize;
    this->numWays = this->numWays << this->log2NumWays;

    this->ways = new CacheEntry*[numWays];
    for (int i = 0; i < this->numWays; ++i)
        this->ways[i] = new CacheEntry[this->waySizeEntries];

}

cacheSim::CacheLevel::~CacheLevel() {
    for (int i = 0; i < this->numWays; ++i){
        delete (this->ways[i]);
    }
    delete ( this->ways);
}

unsigned int cacheSim::CacheLevel::tagCalculator(unsigned long int address) const{
    return address >> (log2BlockSize + log2NumWays);

}

unsigned int cacheSim::CacheLevel::setCalculator(unsigned long int address) const{
    unsigned int setWithTag = address >> (log2BlockSize);
    unsigned int setMask = this->waySizeEntries -1;
    return setWithTag & setMask;
}

int cacheSim::CacheLevel::getWayToStore(unsigned long int address) const{
    unsigned int set = this->setCalculator(address);
    int minTime = INVALID;
    int wayToEdit = INVALID;
    for(int i = 0; i < this->numWays; i++){
        int currentTimeStamp = this->ways[i][set].getTimeStamp();
        if(minTime == INVALID || minTime > currentTimeStamp){
            minTime = currentTimeStamp;
            wayToEdit = i;
        }
    }
    assert(wayToEdit != INVALID);
    return wayToEdit;
}

void cacheSim::CacheLevel::storeNewAddress(unsigned long int address, int time) {
    int wayToStore = this->getWayToStore(address);
    unsigned int tag = this->tagCalculator(address);
    unsigned int set = this->setCalculator(address);
    this->ways[wayToStore][set].setTag(tag);
    this->ways[wayToStore][set].setInvalidBit(false);
    this->ways[wayToStore][set].setDirtyBit(false);
    this->ways[wayToStore][set].setTimeStamp(time);
}

bool cacheSim::CacheLevel::isAddressExist(unsigned long int address){
    unsigned int tag = this->tagCalculator(address);
    unsigned int set = this->setCalculator(address);
    for(int i = 0; i < this->numWays; i++){
        if(this->ways[i][set].getTagBit() == tag){
            return true;
        }
    }
    return false;
}

bool cacheSim::CacheLevel::updateBlockTimeStamp(unsigned long int address,
                                                int time) {
    unsigned int tag = this->tagCalculator(address);
    unsigned int set = this->setCalculator(address);
    for(int i = 0; i < this->numWays; i++){
        if(this->ways[i][set].getTagBit() == tag){
            this->ways[i][set].setTimeStamp(time);
            return true;
        }
    }
    return false;
}
