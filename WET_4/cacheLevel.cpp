#include <cassert>
#include "cacheLevel.h"
#define INVALID -1
using namespace cacheSim;

cacheSim::CacheLevel::CacheLevel(unsigned int cyclesAccess,unsigned int log2Size,unsigned int log2NumWays ,unsigned int log2BlockSize):
        cyclesAccess(cyclesAccess),
        log2LevelSize(log2Size),
        log2NumWays(log2NumWays),
        numWays(1),
        log2BlockSize(log2BlockSize),
        log2WaySize(0),
        numOfSets(1),
        countAccess(0),
        countMiss(0),
        ways(nullptr)
{
    this->log2WaySize = (this->log2LevelSize - this->log2BlockSize) - this->log2NumWays;
    this->numOfSets = this->numOfSets << this->log2WaySize;
    this->numWays = this->numWays << this->log2NumWays;

    this->ways = new tagEntry*[numWays];
    for (unsigned int i = 0; i < this->numWays; ++i)
        this->ways[i] = new tagEntry[this->numOfSets];

}

cacheSim::CacheLevel::~CacheLevel() {
    for (unsigned int i = 0; i < this->numWays; ++i){
        delete (this->ways[i]);
    }
    delete ( this->ways);
}

unsigned int cacheSim::CacheLevel::tagCalculator(unsigned long int address) const{
    return address >> (log2BlockSize + log2WaySize);

}

unsigned int cacheSim::CacheLevel::setCalculator(unsigned long int address) const{
    unsigned int setWithTag = address >> (log2BlockSize);
    unsigned int setMask = this->numOfSets -1;
    return setWithTag & setMask;
}

int cacheSim::CacheLevel::getWayToStore(unsigned long int address) const{
    unsigned int set = this->setCalculator(address);
    int minTime = INVALID;
    int wayToEdit = INVALID;
    for(unsigned int currentWay = 0; currentWay < this->numWays; currentWay++) {
        if (this->ways[currentWay][set].getInvalidBit()) {
            return currentWay;
        }
    }
    for(unsigned int currentWay = 0; currentWay < this->numWays; currentWay++) {
        int currentTimeStamp = this->ways[currentWay][set].getTimeStamp();//LRU policy
        if(minTime == INVALID || minTime > currentTimeStamp){
            assert(!this->ways[currentWay][set].getInvalidBit());
            minTime = currentTimeStamp;
            wayToEdit = currentWay;
        }
    }
    assert(wayToEdit != INVALID);
    return wayToEdit;
}

bool cacheSim::CacheLevel::storeNewAddress(unsigned long int address, int time, unsigned long int* addressRemoved, bool* wasDirty) {
    assert(!this->isAddressExist(address));
    int wayToStore = this->getWayToStore(address);
    unsigned int tag = this->tagCalculator(address);
    unsigned int set = this->setCalculator(address);
    bool removed = false;
    (*wasDirty) = false;
    if(!this->ways[wayToStore][set].getInvalidBit()){
        (*wasDirty) = this->ways[wayToStore][set].getDirtyBit();
        unsigned long int addressRemoved_temp = this->ways[wayToStore][set].getTag();
        addressRemoved_temp = addressRemoved_temp << this->log2WaySize;
        addressRemoved_temp += set;
        addressRemoved_temp = addressRemoved_temp << this->log2BlockSize;
        (*addressRemoved) = addressRemoved_temp;
        removed = true;
    }
    this->ways[wayToStore][set].setTag(tag);
    this->ways[wayToStore][set].setInvalidBit(false);
    this->ways[wayToStore][set].setDirtyBit(false);
    this->ways[wayToStore][set].setTimeStamp(time);
    return removed;
}

bool cacheSim::CacheLevel::isAddressExist(unsigned long int address){
    unsigned int tag = this->tagCalculator(address);
    unsigned int set = this->setCalculator(address);
    for(unsigned int i = 0; i < this->numWays; i++){
        if(!this->ways[i][set].getInvalidBit() && this->ways[i][set].getTag() == tag){
            return true;
        }
    }
    return false;
}

bool cacheSim::CacheLevel::updateBlockTimeStamp(unsigned long int address, int time) {
    unsigned int tag = this->tagCalculator(address);
    unsigned int set = this->setCalculator(address);
    for(unsigned int i = 0; i < this->numWays; i++){
        if(!this->ways[i][set].getInvalidBit() && this->ways[i][set].getTag() == tag){
            this->ways[i][set].setTimeStamp(time);
            return true;
        }
    }
    return false;
}

void CacheLevel::incCountMiss() {
    this->countMiss++;
}

void CacheLevel::incCountAccess() {
    this->countAccess++;
}

unsigned int CacheLevel::getCountMiss() const {
    return this->countMiss;
}

unsigned int CacheLevel::getCountAccess() const {
    return this->countAccess;
}

unsigned int CacheLevel::getCycleAccess() const {
    return this->cyclesAccess;
}

bool CacheLevel::removeBlock(unsigned long int address) {
    if(!this->isAddressExist(address)){
        return false;
    }
    unsigned int tag = this->tagCalculator(address);
    unsigned int set = this->setCalculator(address);
    for(unsigned int i = 0; i < this->numWays; i++) {
        if(!this->ways[i][set].getInvalidBit() && this->ways[i][set].getTag() == tag){
            this->ways[i][set].setInvalidBit(true);
            return true;
        }
    }
    return false;
}

void CacheLevel::markBlockDirty(unsigned long int address) {
    if(!this->isAddressExist(address)){
        return;
    }
    unsigned int tag = this->tagCalculator(address);
    unsigned int set = this->setCalculator(address);
    for(unsigned int i = 0; i < this->numWays; i++) {
        if(!this->ways[i][set].getInvalidBit() && this->ways[i][set].getTag() == tag){
            this->ways[i][set].setDirtyBit(true);
        }
    }
}
