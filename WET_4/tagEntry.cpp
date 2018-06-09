#include "tagEntry.h"
#define INVALID -1

using namespace cacheSim;

cacheSim::tagEntry::tagEntry():tag(INVALID), invalid(true), timeStamp(INVALID), dirty(
        false){}

void cacheSim::tagEntry::setTag(unsigned long int newTag) {
    this->tag = newTag;
}

void tagEntry::setInvalidBit(bool newInvalid) {
    this->invalid = newInvalid;
}

bool tagEntry::getInvalidBit() const {
    return this->invalid;
}

unsigned long int tagEntry::getTag() const {
    return this->tag;
}

int tagEntry::getTimeStamp() const {
    return this->timeStamp;
}

void tagEntry::setTimeStamp(int newTimeStamp) {
    this->timeStamp = newTimeStamp;
}

void tagEntry::setDirtyBit(bool newDirtyBit) {
    this->dirty = newDirtyBit;
}

bool tagEntry::getDirtyBit() const{
    return this->dirty;
}