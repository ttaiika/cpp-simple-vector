#pragma once
#include <cassert>
#include <cstdlib>

template <typename Type>
class ArrayPtr
{
public:
    ArrayPtr() = default;

    explicit ArrayPtr(size_t size)
    {
        if (size == 0)
        {
            raw_ptr_ = nullptr;
        }
        else
        {
            Type* numbers = new Type[size];
            raw_ptr_ = numbers;
        }
    }

    explicit ArrayPtr(Type* raw_ptr) noexcept
    {
        raw_ptr_ = raw_ptr;
    }

    ArrayPtr(const ArrayPtr&) = delete;

    ~ArrayPtr()
    {
        delete[] raw_ptr_;
    }

    ArrayPtr& operator=(const ArrayPtr&) = delete;

    ArrayPtr& operator=(ArrayPtr&& rhs)
    {
        ArrayPtr(std::move(rhs.Get()));
        return *this;
    }

    [[nodiscard]] Type* Release() noexcept
    {
        Type* copy = raw_ptr_;
        raw_ptr_ = nullptr;
        return copy;
    }

    Type& operator[](size_t index) noexcept
    {
        return raw_ptr_[index];
    }

    const Type& operator[](size_t index) const noexcept
    {
        return raw_ptr_[index];
    }


    explicit operator bool() const
    {
        return (raw_ptr_ != nullptr);
    }

    Type* Get() const noexcept
    {
        return raw_ptr_;
    }

    void swap(ArrayPtr& other) noexcept
    {
        Type* copy = raw_ptr_;
        raw_ptr_ = other.raw_ptr_;
        other.raw_ptr_ = copy;
    }

private:
    Type* raw_ptr_ = nullptr;
};