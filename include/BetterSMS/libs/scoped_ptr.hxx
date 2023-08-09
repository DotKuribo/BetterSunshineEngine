#pragma once

template <typename T> class scoped_ptr {
    scoped_ptr()                              = delete;
    scoped_ptr(const scoped_ptr &)            = delete;
    scoped_ptr(scoped_ptr &&)                 = delete;
    scoped_ptr &operator=(const scoped_ptr &) = delete;

public:
    explicit scoped_ptr(T *ptr) : ptr_(ptr) {}
    ~scoped_ptr() { delete ptr_; }

    T *get() const { return ptr_; }
    T *operator->() const { return ptr_; }
    T &operator*() const { return *ptr_; }

private:
    T *ptr_;
};
