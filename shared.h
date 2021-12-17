#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
private:
    T* own_;
    BaseBlock* block_;

    template <typename Y>
    friend class SharedPtr;

    template <typename X, typename... Args>
    friend SharedPtr<X> MakeShared(Args&&... args);

    template <typename O>
    friend class WeakPtr;

public:
    SharedPtr() {
        block_ = nullptr;
        own_ = nullptr;
    }

    SharedPtr(std::nullptr_t) : own_(nullptr), block_(nullptr) {
    }

    template <class U>
    explicit SharedPtr(U* ptr) : own_(ptr), block_(new CleverBlock<U>(ptr)) {
        if (block_) {
            block_->cnt_ = 1;
        }
        CanConstruct(ptr);
    }
    explicit SharedPtr(T* ptr) : own_(ptr), block_(new CleverBlock<T>(ptr)) {
        if (block_) {
            block_->cnt_ = 1;
        }
        CanConstruct(ptr);
    }

    template <class U>
    SharedPtr(const SharedPtr<U>& other) : own_(other.own_), block_(other.block_) {
        if (block_) {
            block_->cnt_++;
        }
    }

    SharedPtr(const SharedPtr& other) : own_(other.own_), block_(other.block_) {
        if (block_) {
            block_->cnt_++;
        }
    }

    template <class U>
    SharedPtr(SharedPtr<U>&& other) : own_(other.own_), block_(other.block_) {
        other.own_ = nullptr;
        other.block_ = nullptr;
    }

    SharedPtr(SharedPtr&& other) : own_(other.own_), block_(other.block_) {
        other.own_ = nullptr;
        other.block_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) : own_(ptr), block_(other.block_) {
        if (block_) {
            block_->cnt_++;
        }
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) : own_(other.own_), block_(other.block_) {
        if (!block_) {
            throw BadWeakPtr();
        }
        if (block_->cnt_ == 0) {
            throw BadWeakPtr();
        }
        block_->cnt_++;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    template <class U>
    SharedPtr& operator=(const SharedPtr<U>& other) {
        if (block_ == other.block_) {
            return *this;
        }
        own_ = other.own_;
        auto x = block_;
        block_ = other.block_;
        if (block_) {
            block_->cnt_++;
        }

        if (x) {
            x->cnt_--;
            if (x->cnt_ + x->cntw_ == 0) {
                delete x;
            } else {
                if (x->cnt_ == 0) {
                    x->Del();
                }
            }
        }
        return *this;
    }

    SharedPtr& operator=(const SharedPtr& other) {
        if (block_ == other.block_) {
            return *this;
        }
        own_ = other.own_;
        auto x = block_;
        block_ = other.block_;
        if (block_) {
            block_->cnt_++;
        }

        if (x) {
            x->cnt_--;
            if (x->cnt_ + x->cntw_ == 0) {
                delete x;
            } else {
                if (x->cnt_ == 0) {
                    x->Del();
                }
            }
        }
        return *this;
    }

    template <class U>
    SharedPtr& operator=(SharedPtr<U>&& other) {
        if (other.block_ == block_) {
            return *this;
        }
        auto x = block_;
        block_ = other.block_;
        own_ = other.own_;
        other.own_ = nullptr;
        other.block_ = nullptr;

        if (x) {
            x->cnt_--;
            if (x->cnt_ + x->cntw_ == 0) {
                delete x;
            } else {
                if (x->cnt_ == 0) {
                    x->Del();
                }
            }
        }

        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        if (other.block_ == block_) {
            return *this;
        }
        auto x = block_;
        block_ = other.block_;
        own_ = other.own_;
        other.block_ = nullptr;
        other.own_ = nullptr;

        if (x) {
            x->cnt_--;
            if (x->cnt_ + x->cntw_ == 0) {
                delete x;
            } else {
                if (x->cnt_ == 0) {
                    x->Del();
                }
            }
        }

        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        if (!block_) {
            return;
        }
        block_->cnt_--;
        if (block_->cnt_ + block_->cntw_ == 0) {
            delete block_;
        } else {
            if (block_->cnt_ == 0) {
                // std::cout << this << ' ' << (this)->block_ << std::endl;
                block_->Del();
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (!block_) {
            own_ = nullptr;
            return;
        }
        block_->cnt_--;
        if (block_->cnt_ + block_->cntw_ == 0) {
            delete block_;
        } else {
            if (block_->cnt_ == 0) {
                block_->Del();
            }
        }
        block_ = nullptr;
        own_ = nullptr;
    }

    template <class U>
    void Reset(U* ptr) {
        own_ = ptr;
        if (block_) {
            block_->cnt_--;
            if (block_->cnt_ + block_->cntw_ == 0) {
                delete block_;
            } else {
                if (block_->cnt_ == 0) {
                    block_->Del();
                }
            }
        }
        block_ = new CleverBlock<U>(ptr);
        block_->cnt_++;
    }

    void Reset(T* ptr) {
        own_ = ptr;
        if (block_) {
            block_->cnt_--;
            if (block_->cnt_ + block_->cntw_ == 0) {
                delete block_;
            } else {
                if (block_->cnt_ == 0) {
                    block_->Del();
                }
            }
        }
        block_ = new CleverBlock<T>(ptr);
        block_->cnt_++;
    }

    void Swap(SharedPtr& other) {
        std::swap(other, *this);
    }

    T* Get() const {
        return own_;
    }

    auto GetBlock() const {
        return block_;
    }

    T& operator*() const {
        return *own_;
    }

    T* operator->() const {
        return own_;
    }

    size_t UseCount() const {
        if (!block_) {
            return 0;
        }
        return block_->cnt_;
    }

    explicit operator bool() const {
        return (own_);
    }
    bool Sravnenie(const SharedPtr& x) const {
        return block_ == x.block_;
    }

private:
    template <typename U>
    void AssignToSelf(EnableSharedFromThis<U>* esft) {
        WeakPtr<U> lol(*this);
        if (esft) {

            esft->self_ = lol;
            esft->self1_ = lol;
        }
    }
    template <class U>
    void CanConstruct(U* ptr) {
        if constexpr (std::is_convertible_v<U*, EnableSharedFromThisBase*>) {
            AssignToSelf(ptr);
        }
    }
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
}

template <typename X, typename... Args>
SharedPtr<X> MakeShared(Args&&... args) {
    SharedPtr<X> other;
    auto block = new SmartBlock<X>(std::forward<Args>(args)...);
    other.block_ = block;
    other.block_->cnt_++;
    other.own_ = block->Get();
    other.CanConstruct(block->Get());

    return other;
}


template <typename T>
class EnableSharedFromThis : public EnableSharedFromThisBase {
public:
    WeakPtr<T> self_;
    WeakPtr<const T> self1_;

    SharedPtr<T> SharedFromThis() {
        return self_.Lock();
    }

    SharedPtr<const T> SharedFromThis() const {
        return self1_.Lock();
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return self_;
    }
    WeakPtr<const T> WeakFromThis() const noexcept {
        return self1_;
    }
};

