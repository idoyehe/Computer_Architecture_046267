#ifndef COMPUTER_ARCHITECTURE_046267_CACHEENTRY_H
#define COMPUTER_ARCHITECTURE_046267_CACHEENTRY_H

namespace cacheSim {
    class CacheEntry {
    private:
        unsigned long int tag;
        bool invalid;
        bool dirty;
        int timeStamp;
    public:
        CacheEntry();

        void setTag(unsigned long int newTag);

        void setInvalidBit(bool newInvalid);

        void setDirtyBit(bool newDirty);

        bool getDirtyBit() const;

        bool getInvalidBit() const;

        unsigned long int getTagBit() const;

        void setTimeStamp(int newTimeStamp);

        int  getTimeStamp() const;
    };
}
#endif //COMPUTER_ARCHITECTURE_046267_CACHEENTRY_H
