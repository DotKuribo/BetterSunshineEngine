#pragma once

#include <Dolphin/types.h>
#include <JSystem/JDrama/JDRNameRef.hxx>
#include <JSystem/JGadget/List.hxx>

template <typename _Tp> class optional;
/// Tag type to disengage optional objects.
struct nullopt_t {
    // Do not user-declare default constructor at all for
    // optional_value = {} syntax to work.
    // nullopt_t() = delete;
    // Used for constructing nullopt.
    enum class _Construct { _Token };
    // Must be constexpr for nullopt_t to be literal.
    explicit constexpr nullopt_t(_Construct) {}
};
/// Tag to disengage optional objects.
inline constexpr nullopt_t nullopt{nullopt_t::_Construct::_Token};

template <typename _T> class Optional {
public:
    Optional() : mValue(nullptr) {}
    Optional(_T value) : mValue(&value) {}

    Optional &operator=(const Optional &other) {
        if (other.hasValue())
            mValue = &other.value();
        else
            mValue = nullptr;

        return *this;
    }
    Optional &operator=(const _T &other) {
        mValue = &other;
        return *this;
    }
    Optional &operator=(nullopt_t) noexcept {
        mValue = nullptr;
        return *this;
    }

    bool operator==(const Optional &other) {
        if (!hasValue())
            return !other.hasValue();

        if (!other.hasValue())
            return false;

        return value() == other.value();
    }
    bool operator==(const _T &other) {
        if (!hasValue())
            return false;

        return value() == other;
    }
    bool operator==(nullopt_t) noexcept { return hasValue(); }

    bool operator!=(const Optional &other) {
        if (!hasValue())
            return other.hasValue();

        if (!other.hasValue())
            return true;

        return value() != other.value();
    }
    bool operator!=(const _T &other) {
        if (!hasValue())
            return true;

        return value() != other;
    }
    bool operator!=(nullopt_t) noexcept { return !hasValue(); }

    _T &operator*() { return value(); }
    _T &operator->() { return value(); }

    bool hasValue() const { return mValue != nullptr; }

    bool operator bool() { return hasValue(); }

    _T &value() const {
        SMS_ASSERT(hasValue(), "Bad access to optional value!\n");
        return *mValue;
    }
    _T &valueOr(const _T &default_) const { return hasValue() ? *mValue : default_; }

private:
    _T *mValue;
};

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
    constexpr size_t SurfaceSize = 128;

    struct Item {
        JDrama::TNameRef mKey;
        _V mValue;
    };

    TDictS() { mItemBuffer = new JGadget::TList<Item, JGadget::TAllocator>()[SurfaceSize]; }
    ~TDictS() {}

    Optional<_V> operator[](const char *key) { return get(key); }
    Optional<_V> operator[](const JDrama::TNameRef &key) { return get(key); }

    Optional<_V> get(const char *key) { get(JDrama::TNameRef(key)); }
    Optional<_V> get(const JDrama::TNameRef &key) {
        const u32 index = getIndex(getHash(key));

        JGadget::TList<Item, JGadget::TAllocator> itemList = mItemBuffer[index];
        for (auto item : itemList) {
            Item &keyValue = item.mItem;
            if (strcmp(keyValue.mKey.mKeyCode, key.mKeyCode) == 0) {
                return Optional<_V>(keyValue.mValue);
            }
        }

        return Optional<_V>();
    }

    void set(const char *key, _V value) { set(JDrama::TNameRef(key), value); }
    void set(const JDrama::TNameRef &key, _V value) {
        const u32 index = getIndex(getHash(key));

        JGadget::TList<Item, JGadget::TAllocator> itemList = mItemBuffer[index];
        for (auto item : itemList) {
            Item &keyValue = item.mItem;
            if (strcmp(keyValue.mKey.mKeyCode, key.mKeyCode) == 0) {
                keyValue.mValue = value;
                return;
            }
        }

        itemList.insert(itemList.end(), {key, value})
    }

    Optional<_V> pop(const char *key) { pop(JDrama::TNameRef(key)); }
    Optional<_V> pop(const JDrama::TNameRef &key) {
        const u32 index = getIndex(getHash(key));

        JGadget::TList<Item, JGadget::TAllocator> itemList = mItemBuffer[index];
        for (auto item : itemList) {
            Item &keyValue = item.mItem;
            if (strcmp(keyValue.mKey.mKeyCode, key.mKeyCode) == 0) {
                itemList.erase(item);
                return Optional<_V>(keyValue.mValue);
            }
        }

        return Optional<_V>();
    }

    _V &setDefault(const char *key, _V default_) { setDefault(JDrama::TNameRef(key), default_); }
    _V &setDefault(const JDrama::TNameRef &key, _V default_) {
        const u32 index = getIndex(getHash(key));

        for (auto item : mItemBuffer[index]) {
            Item &keyValue = item.mItem;
            if (strcmp(keyValue.mKey.mKeyCode, key.mKeyCode) == 0) {
                return keyValue.mValue;
            }
        }

        itemList.insert(itemList.end(), {key, default_});
        return default_;
    }

    JGadget::TList<Item, JGadget::TAllocator> items() const {
        if (!mItemBuffer)
            return JGadget::TList<Item, JGadget::TAllocator>();

        auto fullList = JGadget::TList<Item, JGadget::TAllocator>();
        for (u32 i = 0; i < SurfaceSize; ++i) {
            for (auto item : mItemBuffer[i]) {
                fullList.insert(fullList.end(), item.mItem);
            }
        }

        return fullList;
    }

    bool hasKey(const char *key) { return hasKey(JDrama::TNameRef(key)); }
    bool hasKey(const JDrama::TNameRef &key) {
        const u32 index = getIndex(getHash(key));

        JGadget::TList<Item, JGadget::TAllocator> itemList = mItemBuffer[index];
        for (auto item : itemList) {
            Item &keyValue = item.mItem;
            if (strcmp(keyValue.mKey.mKeyCode, key.mKeyCode) == 0) {
                return true;
            }
        }

        return false;
    }

private:
    constexpr u32 getIndex(u16 hash) { return hash % SurfaceSize; }

    u16 getHash(const char *key) const { return JDrama::TNameRef::calcKeyCode(key); }
    u16 getHash(const JDrama::TNameRef &key) const {return key.mKeyCode}

    JGadget::TList<Item, JGadget::TAllocator> *mItemBuffer;
};

template <typename _V> class TDictI {
public:
    constexpr size_t SurfaceSize = 128;

    struct Item {
        u32 mKey;
        _V mValue;
    };

    TDictI() { mItemBuffer = new JGadget::TList<Item, JGadget::TAllocator>()[SurfaceSize]; }
    ~TDictI() {}

    Optional<_V> operator[](u32 key) { return get(key); }

    Optional<_V> get(u32 key) {
        const u32 index = getIndex(key);

        JGadget::TList<Item, JGadget::TAllocator> itemList = mItemBuffer[index];
        for (auto item : itemList) {
            Item &keyValue = item.mItem;
            if (keyValue.mKey == key) {
                return Optional<_V>(keyValue.mValue);
            }
        }

        return Optional<_V>();
    }

    void set(u32 key, _V value) {
        const u32 index = getIndex(key);

        JGadget::TList<Item, JGadget::TAllocator> itemList = mItemBuffer[index];
        for (auto item : itemList) {
            Item &keyValue = item.mItem;
            if (keyValue.mKey == key) {
                keyValue.mValue = value;
                return;
            }
        }

        itemList.insert(itemList.end(), {key, value})
    }

    Optional<_V> pop(u32 key) {
        const u32 index = getIndex(key);

        JGadget::TList<Item, JGadget::TAllocator> itemList = mItemBuffer[index];
        for (auto item : itemList) {
            Item &keyValue = item.mItem;
            if (keyValue.mKey == key) {
                itemList.erase(item);
                return Optional<_V>(keyValue.mValue);
            }
        }

        return Optional<_V>();
    }

    _V &setDefault(u32 key, _V default_) {
        const u32 index = getIndex(key);

        for (auto item : mItemBuffer[index]) {
            Item &keyValue = item.mItem;
            if (keyValue.mKey == key) {
                return keyValue.mValue;
            }
        }

        itemList.insert(itemList.end(), {key, default_});
        return default_;
    }

    JGadget::TList<Item, JGadget::TAllocator> items() const {
        if (!mItemBuffer)
            return JGadget::TList<Item, JGadget::TAllocator>();

        auto fullList = JGadget::TList<Item, JGadget::TAllocator>();
        for (u32 i = 0; i < SurfaceSize; ++i) {
            for (auto item : mItemBuffer[i]) {
                fullList.insert(fullList.end(), item.mItem);
            }
        }

        return fullList;
    }

    bool hasKey(u32 key) {
        const u32 index = getIndex(key);

        JGadget::TList<Item, JGadget::TAllocator> itemList = mItemBuffer[index];
        for (auto item : itemList) {
            Item &keyValue = item.mItem;
            if (keyValue.mKey == key) {
                return true;
            }
        }

        return false;
    }

private:
    constexpr u32 getIndex(u32 hash) { return hash % SurfaceSize; }

    JGadget::TList<Item, JGadget::TAllocator> *mItemBuffer;
};