#pragma once

#include <array>
#include <memory>
#include <list>
#include <stdexcept>
#include <iterator>
#include <cstring>
#include <new>
#include <vector>

///@brief FastStorage is a container that stores the first N elements in a uninitialized array (in place) and additional elements
/// in a std::vector. This is useful for small containers where the size has an upper bound most of the time.
/// Note that N should stay small to avoid large amounts of data on the stack.
template<class T, size_t N = 5>
class FastStorage {
    std::aligned_storage_t<sizeof(T), alignof(T)> mInPlace[N]; ///< in place memory as a uninitialized array
    std::vector<T> mOutOfPlace; ///< out of place memory stored as vector TODO
    size_t mSize = 0; ///< the current number of elements in the container

    T &getInPlace(size_t index) noexcept {
        return *reinterpret_cast<T*>(&mInPlace[index]);
    }

    const T &getInPlace(size_t index) const noexcept {
        return *reinterpret_cast<const T*>(&mInPlace[index]);
    }

public:
    FastStorage() = default;

    // copy ctr
    FastStorage(const FastStorage& other) {
        if (this == &other) {
            return;
        }
        this->mOutOfPlace = other.mOutOfPlace;
        this->mSize = other.mSize;

        for (size_t i = 0; i < std::min(N, mSize); ++i) {
            new(&getInPlace(i)) T(other.getInPlace(i));
        }
    }

    // copy assign operator
    FastStorage &operator=(const FastStorage& other) {
        if (this == &other) {
            return *this;
        }

        clear();
        this->mSize = other.mSize;
        for (size_t i = 0; i < std::min(N, mSize); ++i) {
            new(&getInPlace(i)) T(other.getInPlace(i));
        }
        this->mOutOfPlace = other.mOutOfPlace;
        return *this;
    }

    ~FastStorage() {
        clear();
    }

    FastStorage(std::initializer_list<T> list) {
        for (auto &&i: list) {
            push_back(std::move(i));
        }
    }

    void clear() {
        mOutOfPlace.clear();
        mSize = std::min(N, mSize);
        for (size_t i = 0; i < mSize; ++i) {
            getInPlace(i).~T();
        }
        mSize = 0;
    }

    ///@brief adds an element to the end of the storage
    void push_back(const T &value) noexcept {
        if (mSize < N) {
            ::new(&mInPlace[mSize]) T(value);
        } else if (mSize == N) {
            mOutOfPlace.reserve(N); // reserve N elements because N elements are already in use
            mOutOfPlace.push_back(value);
        } else {
            mOutOfPlace.push_back(value);
        }
        ++mSize;
    }

    ///@brief creates and adds an element to the end of the storage
    template<typename ...Args> void emplace_back(Args&&... args)
    {
        if (mSize < N) {
            ::new(&mInPlace[mSize]) T(std::forward<Args>(args)...);
        } else if (mSize == N) {
            mOutOfPlace.reserve(N); // reserve N elements because N elements are already in use
            mOutOfPlace.emplace_back(std::forward<Args>(args)...);
        } else {
            mOutOfPlace.emplace_back(std::forward<Args>(args)...);
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
                mOutOfPlace.pop_back();
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
                getInPlace(N - 1) = std::move(mOutOfPlace[0]);
                mOutOfPlace.erase(mOutOfPlace.begin());
            }
        } else {
            mOutOfPlace.erase(std::next(mOutOfPlace.begin(), index - N));
        }
        --mSize;
        return true;
    }

    ///@return the element at the given @p mIndex.
    T &operator[](const size_t index) noexcept {
        if (index < N) {
            return getInPlace(index);
        }
        return mOutOfPlace[index - N];

    }

    ///@return the element at the given @p mIndex
    const T &operator[](const size_t index) const noexcept {
        if (index < N) {
            return getInPlace(index);
        }
        return mOutOfPlace[index - N];

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
        return mOutOfPlace.at(index - N);

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
        return mOutOfPlace.at(index - N);

    }

    ///@return the current size of the container
    size_t size() const noexcept {
        return mSize;
    }

    /// @brief random access iterator for the FastStorage class
    template <typename PointerType, typename StorageRef>
    class iterator {
        size_t mIndex = 0;
        StorageRef *mStorage;
    public:
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = PointerType;
        using pointer = PointerType *;  // or also value_type*
        using reference = PointerType &;  // or also value_type&

        iterator() = delete;

        explicit iterator(StorageRef &storage) : mStorage(storage) {}

        explicit iterator(size_t index, StorageRef *storage) : mIndex(index), mStorage(storage) {}

        iterator erase() {
            if (mStorage->erase(mIndex)) {
                return iterator(--mIndex, mStorage);
            }

            return iterator(mIndex, mStorage);
        }

        // rule of three
        iterator(const iterator &other) : mIndex(other.mIndex), mStorage(other.mStorage) {}

        iterator &operator=(const iterator &other) {
            if (this == &other) {
                return *this;
            }
            mIndex = other.mIndex;
            mStorage = other.mStorage;
            return *this;
        }

        ~iterator() = default;

        //Pointer like operators
        reference operator*() const noexcept { return (*mStorage)[mIndex]; }

        const value_type *operator->() const noexcept { return mStorage[mIndex]; }

        reference operator[](difference_type off) const noexcept { return &(mStorage->operator[](mIndex)); }

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

    typedef iterator<const T, const FastStorage<T, N>> const_iterator_type;
    typedef iterator<T, FastStorage<T, N>> iterator_type;

    ///@return an iterator to the beginning of the container
    iterator_type begin() noexcept {
        return iterator_type(0, this);
    }

    ///@return an iterator to the end of the container
    iterator_type end() noexcept {
        return iterator_type(mSize, this);
    }

    ///@return an iterator to the beginning of the container
    const_iterator_type begin() const noexcept {
        return const_iterator_type(0, this);
    }

    ///@return an iterator to the end of the container
    const_iterator_type end() const noexcept {
        return const_iterator_type(mSize, this);
    }

    iterator_type erase(iterator_type it) noexcept {
        return it.erase();
    }
};