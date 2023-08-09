#pragma once

template <typename T> class scoped_ptr {
    scoped_ptr()                              = delete;
    scoped_ptr(const scoped_ptr &)            = delete;
    scoped_ptr(scoped_ptr &&)                 = delete;
    scoped_ptr &operator=(const scoped_ptr &) = delete;

public:
    explicit scoped_ptr(T *ptr = nullptr) : ptr_(ptr) {}
    ~scoped_ptr() { delete ptr_; }

    void reset(T *p = nullptr) {
        if (ptr_ != p) {
            delete ptr_;
            ptr_ = p;
        }
    }

    T *get() const { return ptr_; }
    T *operator->() const { return ptr_; }
    T &operator*() const { return *ptr_; }
    T &operator[](size_t i) const { return ptr_[i]; }

    bool operator bool() const { return ptr_; }


private:
    T *ptr_;
};
