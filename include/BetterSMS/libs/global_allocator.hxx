#pragma once

#include <Dolphin/types.h>

#include <JSystem/bits/c++config.h>
#include <JSystem/memory.hxx>
#include <JSystem/type_traits.hxx>
#include <JSystem/utility.hxx>

#include <JSystem/JKernel/JKRHeap.hxx>

template <typename _T> class TGlobalAllocator {
public:
    typedef _T value_type;
    typedef _T *pointer;
    typedef _T &reference;
    typedef const _T *const_pointer;
    typedef const _T &const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    template <class U> struct rebind {
        typedef TGlobalAllocator<U> other;
    };

    _GLIBCXX20_CONSTEXPR TGlobalAllocator() _GLIBCXX_USE_NOEXCEPT = default;
    _GLIBCXX20_CONSTEXPR
    TGlobalAllocator(const TGlobalAllocator &other) _GLIBCXX_USE_NOEXCEPT = default;
    template <class _U>
    _GLIBCXX20_CONSTEXPR TGlobalAllocator(const TGlobalAllocator<_U> &other) _GLIBCXX_USE_NOEXCEPT
        : _00() {}

    _GLIBCXX20_CONSTEXPR pointer address(reference _x) const { return JSystem::addressof(_x); }
    const_pointer address(const_reference _x) const _GLIBCXX_NOEXCEPT {
        return JSystem::addressof(_x);
    }

#if __cplusplus < 201703L
    pointer allocate(size_type _n, const void *hint = 0) {
        if (_n > max_size())
            return nullptr;
        return static_cast<_T *>(::operator new(_n * sizeof(value_type), JKRHeap::sSystemHeap, 4));
    }
#elif __cplusplus >= 201703L
#if __cplusplus == 201703L
    pointer allocate(size_type _n, const void *hint) {
        if (_n > max_size())
            return nullptr;
        return static_cast<_T *>(::operator new(_n * sizeof(value_type), JKRHeap::sSystemHeap, 4));
    }
#endif

    _GLIBCXX_NODISCARD _GLIBCXX20_CONSTEXPR pointer allocate(size_t _n) {
        return static_cast<_T *>(::operator new(_n * sizeof(value_type), JKRHeap::sSystemHeap, 4));
    }
#endif

    _GLIBCXX20_CONSTEXPR void deallocate(pointer _p, size_type) { ::operator delete(_p); }

#if __cplusplus <= 201703L
    size_type max_size() const _GLIBCXX_USE_NOEXCEPT { return size_t(-1) / sizeof(value_type); }
#endif

#if __cplusplus < 201103L
    void construct(pointer _p, const value_type &_val) { ::new ((void *)_p) value_type(_val); }
#elif __cplusplus <= 201703L
    template <typename... _Args> void construct(pointer _p, _Args &&..._args) {
        ::new ((void *)_p) value_type(JSystem::forward<_Args>(_args)...);
    }
#else
    template <typename... _Args> constexpr void construct_at(pointer _p, _Args &&..._args) {
        ::new ((void *)_p) value_type(JSystem::forward<_Args>(_args)...);
    }
#endif

#if __cplusplus < 201103L
    void destroy(pointer _p) { _p->~_T(); }
#else
    template <class _U> _GLIBCXX20_CONSTEXPR void destroy(_U *_p) { _p->~_U(); }
#endif

private:
    u8 _00;
};

template <typename T>
inline bool operator==(const TGlobalAllocator<T> &, const TGlobalAllocator<T> &) {
    return true;
}

template <typename T>
inline bool operator!=(const TGlobalAllocator<T> &, const TGlobalAllocator<T> &) {
    return true;
}