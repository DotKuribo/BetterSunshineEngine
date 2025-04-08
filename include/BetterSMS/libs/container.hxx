#pragma once

#include <Dolphin/OS.h>
#include <Dolphin/string.h>
#include <Dolphin/types.h>
#include <JSystem/JDrama/JDRNameRef.hxx>
#include <JSystem/JGadget/List.hxx>
#include <SMS/assert.h>

template <typename _T> class TRingBuffer {
public:
    TRingBuffer(size_t _capacity, bool _garbageCollect = true)
        : mCapacity(_capacity), mIndex(0), mGarbageCollect(_garbageCollect) {
        mBuffer = new _T *[_capacity];
    }
    ~TRingBuffer();

    void push(_T *item) {
        if (mBuffer[mIndex] && mGarbageCollect) {
            delete mBuffer[mIndex];
        }
        mBuffer[mIndex] = item;
        mIndex          = (mIndex + 1) % mCapacity;
    }

    _T *pop() {
        _T *item        = mBuffer[mIndex];
        mBuffer[mIndex] = nullptr;
        mIndex          = mIndex > 0 ? mIndex - 1 : mCapacity - (1 + mIndex);
        return item;
    }

    _T *current() const { return mBuffer[mIndex]; }

    _T *at(int index, bool absolute) {
        if (absolute) {
            return mBuffer[index % mCapacity];
        }
        return mBuffer[(mIndex + index) % mCapacity];
    }

    _T *next() {
        mIndex   = (mIndex + 1) % mCapacity;
        _T *item = mBuffer[mIndex];
        return item;
    }

    _T *prev() {
        mIndex   = mIndex > 0 ? mIndex - 1 : mCapacity - (1 + mIndex);
        _T *item = mBuffer[mIndex];
        return item;
    }

    bool contains(_T *item) const {
        for (int i = 0; i < mCapacity; ++i) {
            if (mBuffer[i] == item) {
                return true;
            }
        }
        return false;
    }

    size_t capacity() const { return mCapacity; }

private:
    _T **mBuffer;
    int mIndex;
    size_t mCapacity;
    bool mGarbageCollect;
};
