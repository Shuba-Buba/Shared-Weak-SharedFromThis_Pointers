#pragma once

#include <cstddef>  // std::nullptr_t
#include <utility>
#include <iostream>
#include <type_traits>
#include <exception>

class EnableSharedFromThisBase {};

class BaseBlock {
public:
    size_t cnt_;
    size_t cntw_;

    BaseBlock() {
        cnt_ = 0;
        cntw_ = 0;
    }

    virtual void Del() {
    }

    virtual ~BaseBlock() = default;
};

template <class U>
class CleverBlock : public BaseBlock {
public:
    U* ptr;
    bool ok_ = false;

    CleverBlock() {
        ptr = nullptr;
        ok_ = false;
    }

    CleverBlock(U* other) {
        ptr = other;
    }

    size_t& Get() {
        return cnt_;
    }

    void Del() override {
        if (!ok_) {
            ok_ = true;
            if (ptr) {
                ok_ = true;
                delete ptr;
            }
        }
    }

    ~CleverBlock() {
        if (!ok_) {
            ok_ = true;
            if (ptr) {
                delete ptr;
            }
        }
    }
};

template <class V>
class SmartBlock : public BaseBlock {
private:
    std::aligned_storage_t<sizeof(V), alignof(V)> object_;
    bool ok_ = false;

public:
    template <class... Args>
    SmartBlock(Args&&... args) {
        new (&object_) V(std::forward<Args>(args)...);
    }

    V* Get() {
        return reinterpret_cast<V*>(&object_);
    }

    void Del() override {
        ok_ = true;
        reinterpret_cast<V*>(&object_)->~V();
    }

    ~SmartBlock() {
        if (!ok_) {
            reinterpret_cast<V*>(&object_)->~V();
        }
    }
};

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

template <typename T>
class EnableSharedFromThis;

