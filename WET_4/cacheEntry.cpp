#include "cacheEntry.h"
#define INVALID -1

using namespace cacheSim;

cacheSim::CacheEntry::CacheEntry():tag(0),invalid(true), timeStamp(INVALID), dirty(
        false){}

void cacheSim::CacheEntry::setTag(unsigned long int newTag) {
    this->tag = newTag;
}

void CacheEntry::setInvalidBit(bool newInvalid) {
    this->invalid = newInvalid;
}

bool CacheEntry::getInvalidBit() const {
    return this->invalid;
}

unsigned long int CacheEntry::getTag() const {
    return this->tag;
}

int CacheEntry::getTimeStamp() const {
    return this->timeStamp;
}

void CacheEntry::setTimeStamp(int newTimeStamp) {
    this->timeStamp = newTimeStamp;
}

void CacheEntry::setDirtyBit(bool newDirtyBit) {
    this->dirty = newDirtyBit;
}

bool CacheEntry::getDirtyBit() const{
    return this->dirty;
}