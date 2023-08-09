#pragma once

#include <Dolphin/OS.h>
#include <Dolphin/types.h>
#include <Dolphin/math.h>

#include <JSystem/bits/functional_hash.h>
#include <JSystem/JGadget/Allocator.hxx>
#include "global_allocator.hxx"

namespace BetterSMS {

    template <class _CharT, class _Alloc = JGadget::TAllocator<_CharT>> class TBasicString {
    public:
        typedef _CharT value_type;
        typedef _Alloc allocator_type;
        typedef typename _Alloc::reference reference;
        typedef typename _Alloc::const_reference const_reference;
        typedef typename _Alloc::pointer pointer;
        typedef typename _Alloc::const_pointer const_pointer;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

        static constexpr size_type npos = -1;

        struct const_iterator;

        struct iterator {
            typedef _CharT value_type;
            typedef ptrdiff_t difference_type;
            typedef typename _Alloc::pointer pointer;
            typedef typename _Alloc::const_pointer const_pointer;

            friend class TBasicString;
            friend struct TBasicString::const_iterator;

            _GLIBCXX20_CONSTEXPR iterator(const iterator &iter) = default;

        private:
            explicit _GLIBCXX20_CONSTEXPR iterator(pointer node) : mCurrent(node) {}
            // For internal conversion (erase)
            _GLIBCXX20_CONSTEXPR iterator(const const_iterator &iter)
                : mCurrent(const_cast<pointer>(iter.mCurrent)) {}

        public:
            _GLIBCXX20_CONSTEXPR bool operator==(const iterator &rhs) const {
                return mCurrent == rhs.mCurrent;
            }
            _GLIBCXX20_CONSTEXPR bool operator!=(const iterator &rhs) const {
                return mCurrent != rhs.mCurrent;
            }

            _GLIBCXX20_CONSTEXPR iterator operator+(int i) {
                iterator temp{mCurrent};
                temp->mCurrent += i;
                return temp;
            }

            _GLIBCXX20_CONSTEXPR iterator &operator+=(int i) {
                mCurrent += i;
                return *this;
            }

            _GLIBCXX20_CONSTEXPR iterator &operator++() {
                ++mCurrent;
                return *this;
            }

            _GLIBCXX20_CONSTEXPR iterator operator++(int) {
                iterator temp{mCurrent};
                ++mCurrent;
                return temp;
            }

            _GLIBCXX20_CONSTEXPR iterator operator-(int i) {
                iterator temp{mCurrent};
                temp->mCurrent -= i;
                return temp;
            }

            _GLIBCXX20_CONSTEXPR iterator &operator-=(int i) {
                mCurrent -= i;
                return *this;
            }

            _GLIBCXX20_CONSTEXPR iterator &operator--() {
                --mCurrent;
                return *this;
            }

            _GLIBCXX20_CONSTEXPR iterator operator--(int) {
                iterator temp{mCurrent};
                --mCurrent;
                return temp;
            }

            pointer operator->() const { return mCurrent; }
            reference operator*() const { return *mCurrent; }

        private:
            pointer mCurrent;
        };

        struct const_iterator {
            typedef _CharT value_type;
            typedef ptrdiff_t difference_type;
            typedef typename _Alloc::pointer pointer;
            typedef typename _Alloc::const_pointer const_pointer;

            friend class TBasicString;
            friend struct TBasicString::iterator;

            _GLIBCXX20_CONSTEXPR const_iterator(const iterator &iter) : mCurrent(iter.mCurrent) {}
            _GLIBCXX20_CONSTEXPR const_iterator(const const_iterator &iter) = default;

        private:
            explicit _GLIBCXX20_CONSTEXPR const_iterator(pointer node) : mCurrent(node) {}

        public:
            _GLIBCXX20_CONSTEXPR bool operator==(const const_iterator &rhs) const {
                return mCurrent == rhs.mCurrent;
            }
            _GLIBCXX20_CONSTEXPR bool operator!=(const const_iterator &rhs) const {
                return mCurrent != rhs.mCurrent;
            }

            _GLIBCXX20_CONSTEXPR const_iterator operator+(int i) {
                const_iterator temp{mCurrent};
                temp->mCurrent += i;
                return temp;
            }

            _GLIBCXX20_CONSTEXPR const_iterator &operator+=(int i) {
                mCurrent += i;
                return *this;
            }

            _GLIBCXX20_CONSTEXPR const_iterator &operator++() {
                ++mCurrent;
                return *this;
            }

            _GLIBCXX20_CONSTEXPR const_iterator operator++(int) {
                const_iterator temp{mCurrent};
                ++mCurrent;
                return temp;
            }

            _GLIBCXX20_CONSTEXPR const_iterator operator-(int i) {
                const_iterator temp{mCurrent};
                temp->mCurrent -= i;
                return temp;
            }

            _GLIBCXX20_CONSTEXPR const_iterator &operator-=(int i) {
                mCurrent -= i;
                return *this;
            }

            _GLIBCXX20_CONSTEXPR const_iterator &operator--() {
                --mCurrent;
                return *this;
            }

            _GLIBCXX20_CONSTEXPR const_iterator operator--(int) {
                const_iterator temp{mCurrent};
                --mCurrent;
                return temp;
            }

            const_pointer operator->() const { return mCurrent; }
            const_reference operator*() const { return *mCurrent; }

        private:
            const_pointer mCurrent;
        };

        _GLIBCXX20_CONSTEXPR TBasicString()
            _GLIBCXX_NOEXCEPT_IF(_GLIBCXX_NOEXCEPT_IF(allocator_type()))
            : mAllocator(), mData(nullptr), mSize(0), mCapacity(15) {
            mData    = allocate_buffer(mCapacity);
            mData[0] = '\0';
        }

        explicit _GLIBCXX20_CONSTEXPR TBasicString(const allocator_type &alloc) _GLIBCXX_NOEXCEPT
            : mAllocator(alloc), mData(nullptr), mSize(0), mCapacity(120) {
            mData    = allocate_buffer(mCapacity);
            mData[0] = '\0';
        }

        _GLIBCXX20_CONSTEXPR TBasicString(size_type count, value_type ch,
                                          const allocator_type &alloc = allocator_type())
            : TBasicString(alloc) {
            assign(count, ch);
        }

        _GLIBCXX20_CONSTEXPR TBasicString(const TBasicString &other, size_type pos,
                                          const allocator_type &alloc = allocator_type())
            : TBasicString(alloc) {
            assign(other, pos);
        }

        _GLIBCXX20_CONSTEXPR TBasicString(const TBasicString &other, size_type pos, size_type count,
                                          const allocator_type &alloc = allocator_type())
            : TBasicString(alloc) {
            assign(other, pos, count);
        }

        _GLIBCXX20_CONSTEXPR TBasicString(const_pointer s, size_type count,
                                          const allocator_type &alloc = allocator_type())
            : TBasicString(alloc) {
            assign(s, count);
        }

        _GLIBCXX20_CONSTEXPR TBasicString(const_pointer s,
                                          const allocator_type &alloc = allocator_type())
            : TBasicString(alloc) {
            assign(s);
        }

        _GLIBCXX20_CONSTEXPR TBasicString(const TBasicString &other)
            : TBasicString(other, other.mAllocator) {}

        _GLIBCXX20_CONSTEXPR TBasicString(const TBasicString &other, const allocator_type &alloc)
            : TBasicString(alloc) {
            mCapacity = other.mCapacity;
            assign(other);
        }

#if __cplusplus >= 201103L
        _GLIBCXX20_CONSTEXPR TBasicString(JSystem::initializer_list<value_type> list,
                                          const allocator_type &alloc = allocator_type())
            : TBasicString(alloc), mSize(list.size()) {
            mCapacity = get_capacity(list.size());
            mData     = allocate_buffer(mCapacity);
            JSystem::copy(list.begin(), list.end(), mData);
            mData[list.size()] = '\0';
        }
#endif

        ~TBasicString() { deallocate_buffer(mData); }

        _GLIBCXX20_CONSTEXPR TBasicString &assign(size_type count, value_type ch) {
            mSize = count;
            if (count > mCapacity) {
                resize_buffer(get_capacity(count));
            }
            for (size_type i = 0; i < count; ++i) {
                mData[i] = ch;
            }
            return *this;
        }

        _GLIBCXX20_CONSTEXPR TBasicString &assign(const TBasicString &other, size_type pos) {
            size_t length = other.mSize - pos;
            mSize         = length;
            if (length > mCapacity) {
                resize_buffer(get_capacity(length));
            }
            JSystem::copy(other.mData + pos, other.mData + length, mData);
            mData[length] = '\0';
            return *this;
        }

        _GLIBCXX20_CONSTEXPR TBasicString &assign(const TBasicString &other, size_type pos,
                                                  size_type count) {
            mSize = count;
            if (count > mCapacity) {
                resize_buffer(get_capacity(count));
            }
            JSystem::copy(other.mData + pos, other.mData + count, mData);
            mData[count] = '\0';  // Null terminator
            return *this;
        }

        _GLIBCXX20_CONSTEXPR TBasicString &assign(const_pointer s, size_type count) {
            mSize = count;
            if (count > mCapacity) {
                resize_buffer(get_capacity(count));
            }
            JSystem::copy(s, s + count, mData);
            mData[count] = '\0';
            return *this;
        }

        _GLIBCXX20_CONSTEXPR TBasicString &assign(const_pointer s) {
            size_type _size = 0;
            for (; s[_size] != '\0'; ++_size) {
            }

            mSize = _size;
            if (_size > mCapacity) {
                resize_buffer(get_capacity(_size));
            }

            JSystem::copy(s, s + _size, mData);
            mData[_size] = '\0';
            return *this;
        }

        _GLIBCXX20_CONSTEXPR TBasicString &assign(const TBasicString &other) {
            mSize = other.size();
            if (mSize > mCapacity) {
                resize_buffer(get_capacity(mSize));
            }
            JSystem::copy(other.mData, other.mData + other.mSize, mData);
            mData[other.mSize] = '\0';
            return *this;
        }

        _GLIBCXX20_CONSTEXPR allocator_type get_allocator() const _GLIBCXX_NOEXCEPT {
            return mAllocator;
        }

        _GLIBCXX20_CONSTEXPR reference at(size_type index) {
            if (index >= mSize) {
                OSPanic(__FILE__, __LINE__,
                        "Attempted OOB access to string of size %lu (index %lu)!", mSize, index);
                __OSUnhandledException(6, OSGetCurrentContext(), 0);
            }
            return mData[index];
        }

        _GLIBCXX20_CONSTEXPR const_reference at(size_type index) const {
            if (index >= mSize) {
                OSPanic(__FILE__, __LINE__,
                        "Attempted OOB access to string of size %lu (index %lu)!", mSize, index);
                __OSUnhandledException(6, OSGetCurrentContext(), 0);
            }
            return mData[index];
        }

#if __cplusplus >= 201103L
        _GLIBCXX20_CONSTEXPR reference front() { return *mData; }
        _GLIBCXX20_CONSTEXPR const_reference front() const { return *mData; }

        _GLIBCXX20_CONSTEXPR reference back() { return *(mData + mSize); }
        _GLIBCXX20_CONSTEXPR const_reference back() const { return *(mData + mSize); }
#endif

        _GLIBCXX20_CONSTEXPR pointer data() _GLIBCXX_NOEXCEPT { return mData; }
        _GLIBCXX20_CONSTEXPR const_pointer data() const _GLIBCXX_NOEXCEPT { return mData; }

        _GLIBCXX20_CONSTEXPR const_pointer c_str() const _GLIBCXX_NOEXCEPT { return mData; }

        _GLIBCXX20_CONSTEXPR iterator begin() _GLIBCXX_NOEXCEPT { return iterator(mData); }
        _GLIBCXX20_CONSTEXPR const_iterator begin() const _GLIBCXX_NOEXCEPT {
            return const_iterator(mData);
        }
        _GLIBCXX20_CONSTEXPR iterator end() _GLIBCXX_NOEXCEPT {
            return iterator(mData + mSize + 1);
        }
        _GLIBCXX20_CONSTEXPR const_iterator end() const _GLIBCXX_NOEXCEPT {
            return const_iterator(mData + mSize + 1);
        }

#if __cplusplus >= 201103L
        _GLIBCXX20_CONSTEXPR const_iterator cbegin() const _GLIBCXX_NOEXCEPT {
            return const_iterator(mData);
        }
        _GLIBCXX20_CONSTEXPR const_iterator cend() const _GLIBCXX_NOEXCEPT {
            return const_iterator(mData + mSize);
        }
#endif

        _GLIBCXX20_CONSTEXPR bool empty() const _GLIBCXX_NOEXCEPT { return mSize == 0; }

        _GLIBCXX20_CONSTEXPR size_type size() const _GLIBCXX_NOEXCEPT { return mSize; }
        _GLIBCXX20_CONSTEXPR size_type length() const _GLIBCXX_NOEXCEPT { return mSize; }

        _GLIBCXX20_CONSTEXPR size_type max_size() const _GLIBCXX_NOEXCEPT {
            return size_type(-1) / sizeof(value_type);
        }

        _GLIBCXX20_CONSTEXPR void reserve(size_type n) {
            if (n <= mCapacity)
                return;
            resize_buffer(n);
            mCapacity = n;
        }

        _GLIBCXX20_CONSTEXPR size_type capacity() const _GLIBCXX_NOEXCEPT { return mCapacity; }

#if __cplusplus >= 201103L
        _GLIBCXX20_CONSTEXPR void shrink_to_fit() {}
#endif

        _GLIBCXX20_CONSTEXPR void clear() { erase(begin(), end()); }

        _GLIBCXX20_CONSTEXPR TBasicString &insert(size_type index, size_type count, value_type ch) {
            if (mSize + count > mCapacity) {
                resize_buffer(get_capacity(mSize + count));
            }
            JSystem::copy(begin() + index, end() + 1, begin() + index + count);
            for (size_type i = 0; i < count; ++i) {
                mData[index + i] = ch;
            }
            mSize += count;
            return *this;
        }

        _GLIBCXX20_CONSTEXPR TBasicString &insert(size_type index, const_pointer s) {
            size_type _size = 0;
            for (; s[_size] != '\0'; ++_size) {
            }

            if (mSize + _size > mCapacity) {
                resize_buffer(get_capacity(mSize + _size));
            }

            JSystem::copy(begin() + index, end() + 1, begin() + index + _size);
            JSystem::copy(s, s + _size, mData + index);
            mSize += _size;
            return *this;
        }

        _GLIBCXX20_CONSTEXPR TBasicString &insert(size_type index, const_pointer s,
                                                  size_type count) {
            if (mSize + count > mCapacity) {
                resize_buffer(get_capacity(mSize + count));
            }

            JSystem::copy(begin() + index, end() + 1, begin() + index + count);
            JSystem::copy(s, s + count, mData + index);
            mSize += count;
            return *this;
        }

        _GLIBCXX20_CONSTEXPR TBasicString &insert(size_type index, const TBasicString &other) {
            if (mSize + other.size() > mCapacity) {
                resize_buffer(get_capacity(mSize + other.size()));
            }

            JSystem::copy(begin() + index, end() + 1, begin() + index + other.size());
            JSystem::copy(other.begin(), other.end(), mData + index);
            mSize += other.size();
            return *this;
        }

#if __cplusplus >= 201402L
        _GLIBCXX20_CONSTEXPR TBasicString &insert(size_type index, const TBasicString &other,
                                                  size_type index_str, size_type count) {
            TBasicString substring = other.substr(index_str, count);

            if (mSize + substring.size() > mCapacity) {
                resize_buffer(get_capacity(mSize + substring.size()));
            }

            JSystem::copy(begin() + index, end() + 1, begin() + index + count);
            JSystem::copy(other.begin(), other.end(), mData + index);
            mSize += substring.size();
            return *this;
        }
#else
        _GLIBCXX20_CONSTEXPR TBasicString &insert(size_type index, const TBasicString &other,
                                                  size_type index_str, size_type count = npos) {
            TBasicString substring = other.substr(index_str, count);

            if (mSize + substring.size() > mCapacity) {
                resize_buffer(get_capacity(mSize + substring.size()));
            }

            JSystem::copy(begin() + index, end() + 1, begin() + index + count);
            JSystem::copy(other.begin(), other.end(), mData + index);
            mSize += substring.size();
            return *this;
        }
#endif

        _GLIBCXX20_CONSTEXPR iterator insert(const_iterator pos, value_type ch) {
            JSystem::copy(pos, end() + 1, pos + 1);
            *iterator(pos) = ch;
            mSize += 1;
            return pos;
        }

        _GLIBCXX20_CONSTEXPR iterator insert(const_iterator pos, size_type count, value_type ch) {
            if (mSize + count > mCapacity) {
                resize_buffer(get_capacity(mSize + count));
            }

            auto ip = iterator(pos);

            JSystem::copy(pos, end() + 1, pos + count);
            for (size_type i = 0; i < count; ++i) {
                *ip++ = ch;
            }
            mSize += count;

            return pos;
        }

#if __cplusplus >= 201103L
        _GLIBCXX20_CONSTEXPR iterator insert(const_iterator pos,
                                             JSystem::initializer_list<value_type> list) {
            if (mSize + list.size() > mCapacity) {
                resize_buffer(get_capacity(mSize + list.size()));
            }

            auto ip = iterator(pos);

            JSystem::copy(pos, end() + 1, pos + list.size());
            JSystem::copy(list.begin(), list.end(), pos);
            mSize += list.size();

            return pos;
        }
#endif

        _GLIBCXX20_CONSTEXPR TBasicString &erase(size_type index = 0, size_type count = npos) {
            count = Min(count, mSize - index);
            JSystem::copy(mData + index + count, mData + mSize + 1, mData + index);
            mSize -= count;
            return *this;
        }

        _GLIBCXX20_CONSTEXPR iterator erase(const_iterator pos) {
            JSystem::copy(pos + 1, mData + mSize + 1, pos);
            mSize -= 1;
            return *this;
        }

        _GLIBCXX20_CONSTEXPR iterator erase(const_iterator first, const_iterator last) {
            JSystem::copy(last, end(), first);
            while (first++ != last) {
                mSize -= 1;
            }
            return *this;
        }

        _GLIBCXX20_CONSTEXPR void push_back(value_type ch) {
            if (mSize + 1 > mCapacity) {
                mCapacity = get_capacity(mSize + 1);
                resize_buffer(mCapacity);
            }
            mData[mSize++] = ch;
        }

        _GLIBCXX20_CONSTEXPR void pop_back() { erase(end() - 1); }

        _GLIBCXX20_CONSTEXPR TBasicString &append(size_type count, value_type ch) {
            if (mSize + count > mCapacity) {
                mCapacity = get_capacity(mSize + count);
                resize_buffer(mCapacity);
            }

            for (size_type i = 0; i < count; ++i) {
                mData[mSize++] = ch;
            }
            return *this;
        }

        _GLIBCXX20_CONSTEXPR TBasicString &append(const_pointer s) {
            size_type _size = 0;
            for (; s[_size] != '\0'; ++_size) {
            }

            if (mSize + _size > mCapacity) {
                mCapacity = get_capacity(mSize + _size);
                resize_buffer(mCapacity);
            }

            JSystem::copy(s, s + _size + 1, mData + mSize);
            mSize += _size;
            return *this;
        }

        _GLIBCXX20_CONSTEXPR TBasicString &append(const TBasicString &other) {
            if (mSize + other.mSize > mCapacity) {
                mCapacity = get_capacity(mSize + other.mSize);
                resize_buffer(mCapacity);
            }

            JSystem::copy(other.begin(), other.end() + 1, end());
            mSize += other.mSize;
            return *this;
        }

#if __cplusplus >= 201103L
        _GLIBCXX20_CONSTEXPR TBasicString &append(JSystem::initializer_list<value_type> list) {
            if (mSize + list.size() > mCapacity) {
                mCapacity = get_capacity(mSize + list.size());
                resize_buffer(mCapacity);
            }

            JSystem::copy(list.begin(), list.end(), end());
            mSize += list.size();
            mData[mSize] = '\0';
            return *this;
        }
#endif

        _GLIBCXX20_CONSTEXPR TBasicString &operator+=(value_type ch) { return append(1, ch); }

        _GLIBCXX20_CONSTEXPR TBasicString &operator+=(const_pointer s) { return append(s); }

        _GLIBCXX20_CONSTEXPR TBasicString &operator+=(const TBasicString &other) {
            return append(other);
        }

#if __cplusplus >= 201103L
        _GLIBCXX20_CONSTEXPR TBasicString &operator+=(JSystem::initializer_list<value_type> list) {
            return append(list);
        }
#endif

        _GLIBCXX20_CONSTEXPR TBasicString substr(size_type pos = 0, size_type count = npos) const {
            count = Min(count, mSize - pos);
            return TBasicString(mData + pos, count);
        }

        _GLIBCXX20_CONSTEXPR size_type copy(pointer dest, size_type count,
                                            size_type pos = 0) const {
            count = Min(count, mSize - pos);
            JSystem::copy(mData + pos, mData + pos + count, dest);
            return count;
        }

        _GLIBCXX20_CONSTEXPR void resize(size_type count) {
            if (count == mSize)
                return;

            if (count < mSize) {
                erase(begin() + count, end());
            } else {
                append(count - mSize, '\0');
            }
        }

    private:
        _GLIBCXX20_CONSTEXPR size_type get_capacity(size_type size) { return (size - 1) * 2; }

        _GLIBCXX20_CONSTEXPR void resize_buffer(size_type size) {
            pointer p = allocate_buffer(size);
            if (!p)
                OSPanic(__FILE__, __LINE__,
                        "Realloc for TBasicString failed! (Attempted to allocate %lu bytes)", size);
            JSystem::copy(mData, mData + mSize + 1, p);
            deallocate_buffer(mData);
            mData = p;
            mCapacity = size;
        }

        _GLIBCXX20_CONSTEXPR pointer allocate_buffer(size_type size) {
            return mAllocator.allocate(size + 1);
        }

        _GLIBCXX20_CONSTEXPR void deallocate_buffer(pointer p) { mAllocator.deallocate(p, 1); }

        allocator_type mAllocator;
        pointer mData;
        size_type mSize;
        size_type mCapacity;
    };

    template <class _CharT, class _Alloc = JGadget::TAllocator<_CharT>>
    _GLIBCXX20_CONSTEXPR bool operator==(const TBasicString<_CharT, _Alloc>& a,
                                         const TBasicString<_CharT, _Alloc> &b) {
        if (a.length() != b.length())
            return false;

        auto _a = a.data();
        auto _b = a.data();
        for (; *_a == *_b; _a++, _b++)
            if (*_a == '\0')
                return true;

        return false;
    }

#if __cplusplus <= 201703L
    template <class _CharT, class _Alloc = JGadget::TAllocator<_CharT>>
    _GLIBCXX20_CONSTEXPR bool operator!=(const TBasicString<_CharT, _Alloc> &a,
                                         const TBasicString<_CharT, _Alloc> &b) {
        if (a.length() != b.length())
            return true;

        auto _a = a.data();
        auto _b = a.data();
        for (; *_a == *_b; _a++, _b++)
            if (*_a == '\0')
                return false;

        return true;
    }
#endif

    using TString = TBasicString<char>;
    using TGlobalString = TBasicString<char, TGlobalAllocator<char>>;
    using TWString = TBasicString<wchar_t>;
#ifdef _GLIBCXX_USE_CHAR8_T
    using TU8String = TBasicString<u8>;
#endif
    using TU16String = TBasicString<u16>;
    using TU32String = TBasicString<u32>;
}  // namespace BetterSMS

namespace JSystem {
    template <typename _CharT, typename _Alloc,
              typename _StrT = BetterSMS::TBasicString<_CharT, _Alloc>>
    struct __str_hash_base : public __hash_base<size_t, _StrT> {
        [[__nodiscard__]] size_t operator()(const _StrT &__s) const noexcept {
            return _Hash_impl::hash(__s.data(), __s.length() * sizeof(_CharT));
        }
    };

#ifndef _GLIBCXX_COMPATIBILITY_CXX0X
    /// std::hash specialization for string.
    template <typename _Alloc>
    struct hash<BetterSMS::TBasicString<char, _Alloc>>
        : public __str_hash_base<char, _Alloc> {};

    /// std::hash specialization for wstring.
    template <typename _Alloc>
    struct hash<BetterSMS::TBasicString<wchar_t, _Alloc>>
        : public __str_hash_base<wchar_t, _Alloc> {};

    template <typename _Alloc>
    struct __is_fast_hash<hash<BetterSMS::TBasicString<wchar_t, _Alloc>>> : JSystem::false_type {};
#endif /* _GLIBCXX_COMPATIBILITY_CXX0X */

#ifdef _GLIBCXX_USE_CHAR8_T
    /// JSystem::hash specialization for u8string.
    template <typename _Alloc>
    struct hash<BetterSMS::TBasicString<char8_t, _Alloc>>
        : public __str_hash_base<char8_t, _Alloc> {};
#endif

    /// JSystem::hash specialization for u16string.
    template <typename _Alloc>
    struct hash<BetterSMS::TBasicString<char16_t, _Alloc>>
        : public __str_hash_base<char16_t, _Alloc> {};

    /// JSystem::hash specialization for u32string.
    template <typename _Alloc>
    struct hash<BetterSMS::TBasicString<char32_t, _Alloc>>
        : public __str_hash_base<char32_t, _Alloc> {};

#if !_GLIBCXX_INLINE_VERSION
    // PR libstdc++/105907 - __is_fast_hash affects unordered container ABI.
    template <> struct __is_fast_hash<hash<BetterSMS::TString>> : JSystem::false_type {};
    template <> struct __is_fast_hash<hash<BetterSMS::TWString>> : JSystem::false_type {};
    template <> struct __is_fast_hash<hash<BetterSMS::TU16String>> : JSystem::false_type {};
    template <> struct __is_fast_hash<hash<BetterSMS::TU32String>> : JSystem::false_type {};
#ifdef _GLIBCXX_USE_CHAR8_T
    template <> struct __is_fast_hash<hash<BetterSMS::TU8String>> : JSystem::false_type {};
#endif
#else
    // For versioned namespace, assume every std::hash<BetterSMS::TBasicString<>> is slow.
    template <typename _CharT, typename _Alloc>
    struct __is_fast_hash<hash<BetterSMS::TBasicString<_CharT, _Alloc>>> : JSystem::false_type {};
#endif
}  // namespace JSystem