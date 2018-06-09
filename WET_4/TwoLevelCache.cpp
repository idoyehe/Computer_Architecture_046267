#include <cassert>
#include "TwoLevelCache.h"
using namespace cacheSim;
cacheSim::TwoLevelCache::TwoLevelCache(unsigned int memoryAccCycles,
                                       unsigned int log2BlockSize,
                                       WritePolicy writePolicy,
                                       unsigned int log2L1size,
                                       unsigned int log2L1numWays,
                                       unsigned int L1AccCycles,
                                       unsigned int log2L2size,
                                       unsigned int log2L2numWays,
                                       unsigned int L2AccCycles):
cyclesCounter(0),memoryAccCycles(memoryAccCycles),writePolicy(writePolicy),
countAccess(0),
time(0),
L1(L1AccCycles,log2L1size,log2L1numWays,log2BlockSize),
L2(L2AccCycles,log2L2size,log2L2numWays,log2BlockSize){}

void TwoLevelCache::accessCache(unsigned long int address) {
    this->countAccess++;
    this->cyclesCounter += this->L1.getCycleAccess();
    this->L1.incCountAccess();
    if (!this->L1.isAddressExist(address)) {
        this->L1.incCountMiss();
        //failed to find in  L1 search in L2
        this->L2.incCountAccess();
        this->cyclesCounter += this->L2.getCycleAccess();
        if (!this->L2.isAddressExist(address)) {
            this->L2.incCountMiss();
            this->cyclesCounter += this->memoryAccCycles;

            //store from memory in L2 and then L1
            unsigned long int removed_address;
            bool wasDirty;
            if (this->L2.storeNewAddress(address, this->time, &removed_address,
                                         &wasDirty)) {
                this->L1.removeBlock(removed_address);
            }
            if (this->L1.storeNewAddress(address, this->time, &removed_address,
                                         &wasDirty)) {
                if (wasDirty) {
                    assert(this->L2.isAddressExist(removed_address));
                    this->L2.updateBlockTimeStamp(removed_address, this->time);
                }
            }
        } else {// it is in L2 but not in L1
            unsigned long int removed_address;
            bool wasDirty;
            this->L2.updateBlockTimeStamp(address, this->time);
            if (this->L1.storeNewAddress(address, this->time,
                                         &removed_address, &wasDirty)) {
                if (wasDirty) {
                    assert(this->L2.isAddressExist(removed_address));
                    this->L2.updateBlockTimeStamp(removed_address,
                                                  this->time);
                }
            }
        }
    } else {
        this->L1.updateBlockTimeStamp(address, this->time);
    }
    this->time++;
}

void TwoLevelCache::readFromAddress(unsigned long int address) {
    this->accessCache(address);
}

void TwoLevelCache::writeToAddress(unsigned long int address) {
    if(this->writePolicy == WRITE_ALLOCATE){
        this->accessCache(address);
        this->L1.markBlockDirty(address);
    }
    else{
        this->countAccess++;
        this->cyclesCounter += this->L1.getCycleAccess();
        this->L1.incCountAccess();
        if(!this->L1.isAddressExist(address)) {
            this->L1.incCountMiss();
            //failed to find in  L1 search in L2
            this->L2.incCountAccess();
            this->cyclesCounter += this->L2.getCycleAccess();
            if (!this->L2.isAddressExist(address)) {
                this->L2.incCountMiss();
                this->cyclesCounter += this->memoryAccCycles;
            }
            else{// it is in L2 but not in L1
                this->L2.updateBlockTimeStamp(address,this->time);
            }
        }
        else{
            this->L1.updateBlockTimeStamp(address,this->time);
        }
        this->time++;
    }
}

double TwoLevelCache::getL1MissRate() const {
    return (double)this->L1.getCountMiss() / (double) this->L1.getCountAccess();
}

double TwoLevelCache::getL2MissRate() const {
    return (double)this->L2.getCountMiss() / (double) this->L2.getCountAccess();
}

double TwoLevelCache::getAccTimeAvg() const {
    return (double)this->cyclesCounter / (double) this->countAccess;
}
