#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
private:
    T* own_;
    BaseBlock* block_;

    template <typename U>
    friend class SharedPtr;

    template <typename Y>
    friend class WeakPtr;

public:
    WeakPtr() : own_(nullptr), block_(nullptr) {
    }

    WeakPtr(const WeakPtr& other) : own_(other.own_), block_(other.block_) {
        if (block_) {
            block_->cntw_++;
        }
    }

    WeakPtr(WeakPtr&& other) : own_(other.own_), block_(other.block_) {
        other.own_ = nullptr;
        other.block_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) : own_(other.own_), block_(other.block_) {
        if (block_) {
            block_->cntw_++;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    template <class X>
    WeakPtr& operator=(const WeakPtr<X>& other) {
        if (other.block_ == block_) {
            return *this;
        }
        own_ = other.own_;
        auto x = block_;
        block_ = other.block_;
        if (block_) {
            block_->cntw_++;
        }

        if (x) {
            x->cntw_--;
            if (x->cnt_ + x->cntw_ == 0) {
                delete x;
            }
        }
        return *this;
    }

    WeakPtr& operator=(const WeakPtr& other) {
        if (other.block_ == block_) {
            return *this;
        }
        own_ = other.own_;
        auto x = block_;
        block_ = other.block_;
        if (block_) {
            block_->cntw_++;
        }

        if (x) {
            x->cntw_--;
            if (x->cnt_ + x->cntw_ == 0) {
                delete x;
            }
        }
        return *this;
    }

    template <class X>
    WeakPtr& operator=(WeakPtr<X>&& other) {
        if (other.block_ == block_) {
            return *this;
        }

        own_ = other.own_;
        auto x = block_;
        block_ = other.block_;

        if (x) {
            x->cntw_--;
            if (x->cnt_ + x->cntw_ == 0) {
                delete x;
            }
        }
        other.block_ = nullptr;
        other.own_ = nullptr;

        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
        if (other.block_ == block_) {
            return *this;
        }

        own_ = other.own_;
        auto x = block_;
        block_ = other.block_;

        if (x) {
            x->cntw_--;
            if (x->cnt_ + x->cntw_ == 0) {
                delete x;
                x = nullptr;
            }
        }
        other.block_ = nullptr;
        other.own_ = nullptr;

        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        if (block_) {
            block_->cntw_--;
            if (block_->cntw_ + block_->cnt_ == 0) {
                delete block_;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (!block_) {
            return;
        }
        block_->cntw_--;
        if (block_->cnt_ + block_->cntw_ == 0) {
            delete block_;
        }
        block_ = nullptr;
        own_ = nullptr;
    }

    void Swap(WeakPtr& other) {
        std::swap(other, *this);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (!block_) {
            return 0;
        }
        return block_->cnt_;
    }

    bool Expired() const {
        if (!block_) {
            return true;
        }
        if (block_->cnt_ == 0) {
            return true;
        }
        return false;
    }

    SharedPtr<T> Lock() const {
        SharedPtr<T> x;
        if (block_) {
            if (block_->cnt_ == 0) {
                x.block_ = nullptr;
            } else {
                x.block_ = block_;
                x.own_ = own_;
            }
        }
        if (x.block_) {
            x.block_->cnt_++;
        }
        return x;
    }
};

