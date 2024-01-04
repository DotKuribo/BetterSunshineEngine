#pragma once

struct nullopt_t {};

template <typename T> class optional {
public:
    optional() noexcept {
        mData.mEmpty = nullopt_t();
        mHasValue    = false;
    }

    optional(nullopt_t _n) noexcept {
        mData.mEmpty = _n;
        mHasValue    = false;
    }

    optional(const optional &other) noexcept { *this = other; }

    optional(optional &&other) noexcept { *this = other; }

    optional(const T &data) noexcept {
        mData.mValue = data;
        mHasValue    = true;
    }

    optional &operator=(const optional &other) noexcept {
        mHasValue = other.mHasValue;
        if (other.mHasValue) {
            mData.mValue = other.mData.mValue;
        } else {
            mData.mEmpty = nullopt_t();
        }
    }

    optional &operator=(optional &&other) noexcept {
        mHasValue = other.mHasValue;
        if (other.mHasValue) {
            mData.mValue = other.mData.mValue;
        } else {
            mData.mEmpty = nullopt_t();
        }
    }

    const T *operator->() const noexcept { return &mData.mValue; }

    T *operator->() noexcept { return &mData.mValue; }

    //const T &operator*() const & noexcept { return mData.mValue; }

    T &operator*() noexcept { return mData.mValue; }

    //const T &&operator*() const && noexcept { return mData.mValue; }

    //T &&operator*() noexcept { return mData.mValue; }

    explicit operator bool() const noexcept { return mHasValue; }
    bool has_value() const noexcept { return mHasValue; }

    /*const T &value() const & {
        if (mHasValue)
            return mData.mValue;
        throw;
    }*/

    T &value() {
        return mData.mValue;
    }

    /*const T &&value() const && {
        if (mHasValue)
            return mData.mValue;
        throw;
    }*/

    /*T &&value() {
        if (mHasValue)
            return mData.mValue;
        throw;
    }*/

    template <class U>
    T value_or(U &&default_value) const& {
        if (mHasValue)
            return mData.mValue;
        return default_value;
    }

    /*template <class U>
    T value_or(U &&default_value) && {
        if (mHasValue)
            return mData.mValue;
        return default_value;
    }*/

    void reset() noexcept {
        if (mHasValue) {
            mData.mValue.~T();
        }
    }

private:
    union U {
        U() : mEmpty({}) {}
        T mValue;
        nullopt_t mEmpty;
    } mData;
    bool mHasValue;
};