#pragma once

#include <array>
#include <memory>
#include <list>
#include <stdexcept>
#include <iterator>
#include <cstring>

///@brief FastStorage is a container that stores the first N elements in place and the rest in a std::vector.
/// Useful for small containers where the size has an upper bound most of the time.
template<class T, size_t N = 5>
class FastStorage {
    alignas(T) char mInPlace[sizeof(T) * N];
    std::unique_ptr<std::vector<T>> mOutOfPlace = nullptr; ///< out of place memory stored as vector

    size_t mSize = 0;

    T &getInPlace(size_t index) noexcept {
        return reinterpret_cast<T *>(mInPlace)[index];
    }

    const T &getInPlace(size_t index) const noexcept {
        return reinterpret_cast<const T *>(mInPlace)[index];
    }

    void setInPlace(size_t index, T &&value) noexcept {
        reinterpret_cast<T *>(mInPlace)[index] = std::move(value);
    }

    void createOutOfPlace() {
        if (!mOutOfPlace) {
            mOutOfPlace = std::make_unique<std::vector<T>>();
            mOutOfPlace->reserve(N);
        }
    }

public:
    FastStorage() = default;

    FastStorage(const FastStorage &other) = delete;

    FastStorage &operator=(const FastStorage &other) = delete;

    FastStorage(std::initializer_list<T> list) {
        for (auto &&i: list) {
            push_back(std::move(i));
        }
    }

    void clear() {
        if (mOutOfPlace) {
            mOutOfPlace->clear();
            mSize = std::min(N, mSize);
        }

        size_t numElements = std::min(N, mSize);
        for (size_t i = 0; i < numElements; ++i) {
            pop_back();
        }

        mSize = 0;
    }

    ///@brief adds an element to the end of the storage
    void push_back(const T &value) noexcept {
        if (mSize < N) {
            getInPlace(mSize) = value;
        } else {
            createOutOfPlace();
            mOutOfPlace->push_back(value);
        }
        ++mSize;
    }

    ///@brief creates and adds an element to the end of the storage
    template<class... Args>
    void emplace_back(Args &&... args) noexcept {
        if (mSize < N) {
            new(&getInPlace(mSize)) T(std::forward<Args>(args)...);
        } else {
            createOutOfPlace();
            mOutOfPlace->emplace_back(std::forward<Args>(args)...);
        }
        ++mSize;
    }

    ///@brief removes the last element from the storage
    void pop_back() noexcept {
        if (mSize > 0) {
            --mSize;
            if (mSize < N) {
                // call destructor here
                getInPlace(mSize).~T();
            } else {
                mOutOfPlace->pop_back();
            }
        }
    }

    ///@brief removes the element at the given @p mIndex
    bool erase(size_t index) noexcept {
        if (index >= mSize) {
            return false;
        }

        if (index < N) {
            getInPlace(index).~T();
            for (size_t i = index; i < std::min(N - 1, mSize - 1); ++i) {
                getInPlace(i) = std::move(getInPlace(i + 1));
            }
            if (mSize > N) {
                getInPlace(N - 1) = std::move((*mOutOfPlace)[0]);
                mOutOfPlace->erase(mOutOfPlace->begin());
            }
        } else {
            mOutOfPlace->erase(std::next(mOutOfPlace->begin(), index - N));
        }
        --mSize;
        return true;
    }

    ///@return the element at the given @p mIndex.
    T &operator[](const size_t index) noexcept {
        if (index < N) {
            return getInPlace(index);
        }
        return (*mOutOfPlace)[index - N];

    }

    ///@return the element at the given @p mIndex
    const T &operator[](const size_t index) const noexcept {
        if (index < N) {
            return getInPlace(index);
        }
        return (*mOutOfPlace)[index - N];

    }

    ///@return the element at the given @p mIndex. tho
    const T &at(const size_t index) const {
        if (mSize == 0) {
            throw std::out_of_range("Storage is empty");
        }
        if (index >= mSize) {
            throw std::out_of_range("Index out of range");
        }
        if (index < N) {
            return getInPlace(index);
        }
        return mOutOfPlace->at(index - N);

    }

    ///@return the element at the given @p mIndex. tho
    T &at(const size_t index) {
        if (mSize == 0) {
            throw std::out_of_range("Storage is empty");
        }
        if (index >= mSize) {
            throw std::out_of_range("Index out of range");
        }
        if (index < N) {
            return getInPlace(index);
        }
        return mOutOfPlace->at(index - N);

    }

    ///@return the current size of the container
    size_t size() const noexcept {
        return mSize;
    }

    /// @brief random access iterator for the FastStorage class
    class iterator {
        size_t mIndex = 0;
        FastStorage *mStorage;
    public:
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T *;  // or also value_type*
        using reference = T &;  // or also value_type&

        iterator() = delete;

        explicit iterator(FastStorage &storage) : mStorage(storage) {}

        explicit iterator(size_t index, FastStorage *storage) : mIndex(index), mStorage(storage) {}

        iterator erase() {
            if (mStorage->erase(mIndex)) {
                return iterator(--mIndex, mStorage);
            }

            return iterator(mIndex, mStorage);
        }

        // rule of three
        iterator(const iterator &other) : mIndex(other.mIndex), mStorage(other.mStorage) {}

        iterator &operator=(const iterator &other) {
            if (other == *this) {
                return *this;
            }
            mIndex = other.mIndex;
            mStorage = other.mStorage;
            return *this;
        }

        ~iterator() = default;

        //Pointer like operators
        reference operator*() const { return (*mStorage)[mIndex]; }

        const value_type *operator->() const { return mStorage[mIndex]; }

        reference operator[](difference_type off) const { return &(mStorage->operator[](mIndex)); }

        //Increment / Decrement
        iterator &operator++() {
            ++mIndex;
            return *this;
        }

        iterator operator++(int) {
            size_t tmp = mIndex;
            ++*this;
            return iterator(tmp, mStorage);
        }

        iterator &operator--() {
            --mIndex;
            return *this;
        }

        iterator operator--(int) {
            size_t tmp = mIndex;
            --*this;
            return iterator(tmp, mStorage);
        }

        //Arithmetic
        iterator &operator+=(difference_type off) {
            mIndex += off;
            return *this;
        }

        iterator operator+(difference_type off) const { return iterator(mIndex + off, mStorage); }

        friend iterator operator+(difference_type off, const iterator &right) {
            return iterator(off + right.mIndex, right.mStorage);
        }

        iterator &operator-=(difference_type off) {
            mIndex -= off;
            return *this;
        }

        iterator operator-(difference_type off) const { return iterator(mIndex - off, mStorage); }

        difference_type operator-(const iterator &right) const { return mIndex - right.mIndex; }

        //Comparison operators
        bool operator==(const iterator &r) const { return mIndex == r.mIndex; }

        bool operator!=(const iterator &r) const { return mIndex != r.mIndex; }

        bool operator<(const iterator &r) const { return mIndex < r.mIndex; }

        bool operator<=(const iterator &r) const { return mIndex <= r.mIndex; }

        bool operator>(const iterator &r) const { return mIndex > r.mIndex; }

        bool operator>=(const iterator &r) const { return mIndex >= r.mIndex; }
    };

    ///@return an iterator to the beginning of the container
    iterator begin() noexcept {
        return iterator(0, this);
    }

    ///@return an iterator to the end of the container
    iterator end() noexcept {
        return iterator(mSize, this);
    }

    iterator erase(iterator it) noexcept {
        return it.erase();
    }
};