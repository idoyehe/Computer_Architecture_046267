#ifndef COMPUTER_ARCHITECTURE_046267_CACHEENTRY_H
#define COMPUTER_ARCHITECTURE_046267_CACHEENTRY_H

namespace cacheSim {
    class tagEntry {
    private:
        unsigned long int tag;
        bool invalid;
        int timeStamp;
        bool dirty;
    public:
        tagEntry();

        void setTag(unsigned long int newTag);

        void setInvalidBit(bool newInvalid);

        bool getInvalidBit() const;

        unsigned long int getTag() const;

        void setTimeStamp(int newTimeStamp);

        int  getTimeStamp() const;

        void setDirtyBit(bool newDirtyBit);

        bool getDirtyBit() const;
    };
}
#endif //COMPUTER_ARCHITECTURE_046267_CACHEENTRY_H
