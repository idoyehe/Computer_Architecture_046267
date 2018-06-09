#include "cacheEntry.h"
#define INVALID -1

using namespace cacheSim;

cacheSim::CacheEntry::CacheEntry():tag(0),invalid(true),dirty(false), timeStamp(INVALID){}

void cacheSim::CacheEntry::setTag(unsigned long int newTag) {
    this->tag = newTag;
}

void CacheEntry::setInvalidBit(bool newInvalid) {
    this->invalid = newInvalid;
}

void CacheEntry::setDirtyBit(bool newDirty) {
    this->dirty = newDirty;
}

bool CacheEntry::getDirtyBit() const {
    return this->dirty;
}

bool CacheEntry::getInvalidBit() const {
    return this->invalid;
}

unsigned long int CacheEntry::getTagBit() const {
    return this->tag;
}

int CacheEntry::getTimeStamp() const {
    return this->timeStamp;
}

void CacheEntry::setTimeStamp(int newTimeStamp) {
    this->timeStamp = newTimeStamp;
}
