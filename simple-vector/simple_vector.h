#pragma once

#include "array_ptr.h"

#include <array>
#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <stdexcept>

class ReserveProxy
{
public:
    ReserveProxy(size_t capacity)
        :capacity_(capacity)
    {
    }

    size_t capacity_;
};

ReserveProxy Reserve(size_t capacity_to_reserve)
{
    return ReserveProxy(capacity_to_reserve);
}

template <typename Type>
class SimpleVector
{
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size)
        : size_(size)
        , capacity_(size)
        , arr_(size)
    {
        std::fill(arr_.Get(), arr_.Get() + size_, 0);
    }

    SimpleVector(size_t size, const Type& value)
        : size_(size)
        , capacity_(size)
        , arr_(size)
    {
        std::fill(arr_.Get(), arr_.Get() + size, value);
    }

    SimpleVector(std::initializer_list<Type> init)
        :SimpleVector(init.size())
    {
        std::copy(init.begin(), init.end(), arr_.Get());
    }

    SimpleVector(ReserveProxy capacity)
        : size_(0)
        , capacity_(capacity.capacity_)
    {
    }

    size_t GetSize() const noexcept
    {
        return size_;
    }

    size_t GetCapacity() const noexcept
    {
        return capacity_;
    }

    bool IsEmpty() const noexcept
    {
        return (size_ == 0);
    }

    Type& operator[](size_t index) noexcept
    {
        assert(static_cast<int> (index) >= 0 && index < size_);
        return arr_[index];
    }

    const Type& operator[](size_t index) const noexcept
    {
        assert(static_cast<int> (index) >= 0 && index < size_);
        return arr_[index];
    }

    Type& At(size_t index)
    {
        if (index >= size_)
        {
            throw std::out_of_range("The index is out of range");
        }
        return arr_[index];
    }

    const Type& At(size_t index) const
    {
        if (index >= size_)
        {
            throw std::out_of_range("The index is out of range");
        }
        return arr_[index];
    }

    void Clear() noexcept
    {
        size_ = 0;
    }

    void Resize(size_t new_size)
    {
        if (new_size > size_)
        {
            ArrayPtr<Type> new_arr(new_size);
            std::move(this->begin(), this->end(), new_arr.Get());
            std::fill(new_arr.Get() + size_, new_arr.Get() + new_size, 0);
            std::generate(new_arr.Get() + size_, new_arr.Get() + new_size, []() {return Type(); });
            arr_.swap(new_arr);
            capacity_ = new_size;
        }
        else if (new_size == size_)
        {
            std::fill(arr_.Get(), arr_.Get() + size_, 0);
        }
        size_ = new_size;
    }

    Iterator begin() noexcept
    {
        return arr_.Get();
    }

    Iterator end() noexcept
    {
        return (arr_.Get() + size_);
    }

    ConstIterator begin() const noexcept
    {
        return arr_.Get();
    }

    ConstIterator end() const noexcept
    {
        return (arr_.Get() + size_);
    }

    ConstIterator cbegin() const noexcept
    {
        return (arr_.Get());
    }

    ConstIterator cend() const noexcept
    {
        return (arr_.Get() + size_);
    }

    SimpleVector(const SimpleVector& other)
        :size_(other.GetSize())
        , capacity_(other.GetCapacity())
        , arr_(other.GetCapacity())
    {
        std::copy(other.begin(), other.end(), arr_.Get());
    }

    SimpleVector(SimpleVector&& other)
        :size_(other.GetSize())
        , capacity_(other.GetCapacity())
        , arr_(other.GetCapacity())
    {
        std::move(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()), arr_.Get());
        other.Clear();
    }

    SimpleVector& operator=(const SimpleVector& rhs)
    {
        if (this == &rhs)
        {
            return *this;
        }
        SimpleVector rhs_copy(rhs);
        swap(rhs_copy);
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs)
    {
        if (this == &rhs)
        {
            return *this;
        }
        SimpleVector rhs_copy(std::move(rhs));
        swap(rhs_copy);
        return *this;
    }

    void PushBack(const Type& item)
    {
        Insert(this->end(), item);
    }

    void PushBack(Type&& item)
    {
        Insert(this->end(), std::move(item));
    }

    Iterator Insert(ConstIterator pos, Type&& value)
    {
        assert(pos >= begin() && pos <= end());

        Iterator it = const_cast<Iterator>(pos);
        if (size_ != capacity_)
        {
            Iterator result_it = std::move_backward(it, end(), end() + 1) - 1;
            *result_it = std::move(value);
            ++size_;
            return result_it;
        }
        else
        {
            if (capacity_ == 0)
            {
                capacity_ = 1;
            }
            else
            {
                capacity_ = capacity_ * 2;
            }
            SimpleVector new_arr(capacity_);
            new_arr.size_ = size_ + 1;
            Iterator result_it = std::move(begin(), it, new_arr.begin());
            *result_it = std::move(value);
            std::move_backward(it, end(), new_arr.end());
            swap(new_arr);
            return result_it;
        }
    }

    void PopBack() noexcept
    {
        assert(size_ != 0);
        arr_[size_] = 0;
        --size_;
    }

    Iterator Erase(ConstIterator pos)
    {
        assert(pos >= begin() && pos <= end());

        Iterator it = const_cast<Iterator>(pos);
        std::move(it + 1, end(), it);
        --size_;
        return it;
    }

    void swap(SimpleVector& other) noexcept
    {
        arr_.swap(other.arr_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity)
    {
        if (new_capacity < capacity_)
        {
            return;
        }
        SimpleVector copy(new_capacity);
        size_t size = size_;
        std::copy(this->begin(), this->end(), copy.begin());
        swap(copy);
        size_ = size;
    }

private:
    size_t size_ = 0;
    size_t capacity_ = 0;
    ArrayPtr<Type> arr_;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return (lhs.GetSize() == rhs.GetSize()) && std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return !(lhs < rhs);
}