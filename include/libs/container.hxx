#pragma once

#include <Dolphin/OS.h>
#include <Dolphin/string.h>
#include <Dolphin/types.h>
#include <JSystem/JDrama/JDRNameRef.hxx>
#include <JSystem/JGadget/List.hxx>
#include <SMS/assert.h>

template <typename _T> class TRingBuffer {
public:
    TRingBuffer(size_t _capacity, bool _garbageCollect)
        : mCapacity(_capacity), mIndex(0), mGarbageCollect(_garbageCollect) {
        mBuffer = new _T *[_capacity];
    }
    virtual ~TRingBuffer();

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

template <typename _V> class TDictS {
public:
    static constexpr size_t SurfaceSize = 64;

    struct Item {
        const char *mKey;
        _V mValue;

        bool operator!=(const Item &other) { return mKey != other.mKey; }
    };

    using ItemList = JGadget::TList<Item>;

    TDictS() { mItemBuffer = new ItemList[SurfaceSize]; }
    ~TDictS() { delete[] mItemBuffer; }

    _V *operator[](const char *key) { return get(key); }

    _V get(const char *key) const {
        const u32 index = getIndex(getHash(key));

        auto &itemList = mItemBuffer[index];
        for (auto &item : itemList) {
            Item &keyValue = item.mItem;
            if (strcmp(keyValue.mKey, key) == 0) {
                return keyValue.mValue;
            }
        }

        return nullptr;
    }

    void set(const char *key, _V value) {
        const u32 index = getIndex(getHash(key));

        auto &itemList = mItemBuffer[index];
        for (auto &item : itemList) {
            Item &keyValue = item.mItem;
            if (strcmp(keyValue.mKey, key) == 0) {
                keyValue.mValue = value;
                return;
            }
        }
        itemList.insert(itemList.end(), {key, value});
    }

    _V *pop(const char *key) {
        const u32 index = getIndex(getHash(key));

        auto &itemList = mItemBuffer[index];
        for (auto i = itemList.begin(); i != itemList.end(); ++i) {
            Item &keyValue = i->mItem;
            if (strcmp(keyValue.mKey, key) == 0) {
                itemList.erase(i);
                return &keyValue.mValue;
            }
        }

        return nullptr;
    }

    _V &setDefault(const char *key, const _V &default_) {
        const u32 index = getIndex(getHash(key));

        auto &itemList = mItemBuffer[index];
        for (auto &item : itemList) {
            Item &keyValue = item.mItem;
            if (strcmp(keyValue.mKey, key) == 0) {
                return &keyValue.mValue;
            }
        }

        itemList.insert(itemList.end(), {key, default_});
        return default_;
    }

    _V &setDefault(const char *key, _V *&&default_) {
        const u32 index = getIndex(getHash(key));

        auto &itemList = mItemBuffer[index];
        for (auto &item : itemList) {
            Item &keyValue = item.mItem;
            if (strcmp(keyValue.mKey, key) == 0) {
                delete default_;
                return &keyValue.mValue;
            }
        }

        itemList.insert(itemList.end(), {key, *default_});
        return *default_;
    }

    void items(ItemList &out) const {
        if (!mItemBuffer)
            return;

        for (u32 i = 0; i < SurfaceSize; ++i) {
            for (auto &item : mItemBuffer[i]) {
                out.insert(out.end(), item.mItem);
            }
        }
    }

    bool hasKey(const char *key) const {
        const u32 index = getIndex(getHash(key));

        auto &itemList = mItemBuffer[index];
        for (auto &item : itemList) {
            Item &keyValue = item.mItem;
            if (strcmp(keyValue.mKey, key) == 0) {
                return true;
            }
        }

        return false;
    }

private:
    constexpr u32 getIndex(u16 hash) const { return hash % SurfaceSize; }

    u16 getHash(const char *key) const { return JDrama::TNameRef::calcKeyCode(key); }
    u16 getHash(const JDrama::TNameRef &key) const { return key.mKeyCode; }

    ItemList *mItemBuffer;
};

template <typename _V> class TDictI {
public:
    static constexpr size_t SurfaceSize = 64;

    struct Item {
        u32 mKey;
        _V mValue;
    };

    using ItemList = JGadget::TList<Item>;

    TDictI() { mItemBuffer = new ItemList[SurfaceSize]; }
    ~TDictI() { delete[] mItemBuffer; }

    _V *operator[](u32 key) { return get(key); }

    _V *get(u32 key) {
        const u32 index = getIndex(key);

        auto &itemList = mItemBuffer[index];
        for (auto &item : itemList) {
            Item &keyValue = item.mItem;
            if (keyValue.mKey == key) {
                return &keyValue.mValue;
            }
        }

        return nullptr;
    }

    void set(u32 key, _V value) {
        const u32 index = getIndex(key);

        auto &itemList = mItemBuffer[index];
        for (auto &item : itemList) {
            Item &keyValue = item.mItem;
            if (keyValue.mKey == key) {
                keyValue.mValue = value;
                return;
            }
        }

        itemList.insert(itemList.end(), {key, value});
    }

    _V *pop(u32 key) {
        const u32 index = getIndex(key);

        auto &itemList = mItemBuffer[index];
        for (auto i = itemList.begin(); i != itemList.end(); ++i) {
            Item &keyValue = i->mItem;
            if (keyValue.mKey == key) {
                itemList.erase(i);
                return &keyValue.mValue;
            }
        }

        return nullptr;
    }

    _V &setDefault(u32 key, const _V &default_) {
        const u32 index = getIndex(key);

        auto &itemList = mItemBuffer[index];
        for (auto &item : itemList) {
            Item &keyValue = item.mItem;
            if (keyValue.mKey == key) {
                return &keyValue.mValue;
            }
        }

        itemList.insert(itemList.end(), {key, default_});
        return default_;
    }

    _V &setDefault(u32 key, _V *&&default_) {
        OSReport("Default value at 0x%X\n", &default_);
        const u32 index = getIndex(key);

        auto &itemList = mItemBuffer[index];
        for (auto &item : itemList) {
            Item &keyValue = item.mItem;
            if (keyValue.mKey == key) {
                delete default_;
                return &keyValue.mValue;
            }
        }

        itemList.insert(itemList.end(), {key, *default_});
        return *default_;
    }

    void items(ItemList &out) const {
        if (!mItemBuffer)
            return;

        for (u32 i = 0; i < SurfaceSize; ++i) {
            for (auto &item : mItemBuffer[i]) {
                out.insert(out.end(), item.mItem);
            }
        }
    }

    bool hasKey(u32 key) {
        const u32 index = getIndex(key);

        auto &itemList = mItemBuffer[index];
        for (auto &item : itemList) {
            Item &keyValue = item.mItem;
            if (keyValue.mKey == key) {
                return true;
            }
        }

        return false;
    }

private:
    constexpr u32 getIndex(u32 hash) { return hash % SurfaceSize; }

    ItemList *mItemBuffer;
};