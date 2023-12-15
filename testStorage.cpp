#include <gtest/gtest.h>
#include <algorithm>
#include <chrono>
#include <random>
#include "FastStorage.h"

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(StorageTest, AllInPlace) {
    FastStorage<int, 3> a;
    EXPECT_EQ(a.size(), 0);

    FastStorage<int, 3> b{1};
    EXPECT_EQ(b.size(), 1);
    EXPECT_EQ(b[0], 1);

    FastStorage<int, 3> c{1, 2, 3};
    EXPECT_EQ(c.size(), 3);
    EXPECT_EQ(c[0], 1);
    EXPECT_EQ(c[1], 2);
    EXPECT_EQ(c[2], 3);
}

TEST(StorageTest, Mixed) {
    FastStorage<int, 1> a{1, 2, 3};
    EXPECT_EQ(a.size(), 3);
    EXPECT_EQ(a[0], 1);
    EXPECT_EQ(a[1], 2);
    EXPECT_EQ(a[2], 3);

    const FastStorage<int, 2> b{1, 2, 3, 4};
    EXPECT_EQ(b.size(), 4);
    EXPECT_EQ(b[0], 1);
    EXPECT_EQ(b[1], 2);
    EXPECT_EQ(b[2], 3);
    EXPECT_EQ(b[3], 4);

    FastStorage<int, 3> c{1, 2, 3, 4};
    EXPECT_EQ(c.size(), 4);
    EXPECT_EQ(c[0], 1);
    EXPECT_EQ(c[1], 2);
    EXPECT_EQ(c[2], 3);
    EXPECT_EQ(c[3], 4);
}

TEST(StorageTest, NoInPlace) {
    FastStorage<int, 0> f{1, 2, 3, 4};
    EXPECT_EQ(f.size(), 4);
    EXPECT_EQ(f[0], 1);
    EXPECT_EQ(f[1], 2);
    EXPECT_EQ(f[2], 3);
    EXPECT_EQ(f[3], 4);
}

TEST(StorageTest, PushBack) {
    FastStorage<int, 2> a;
    a.push_back(1);
    a.push_back(2);
    a.push_back(3);
    a.push_back(4);
    EXPECT_EQ(a.size(), 4);
    EXPECT_EQ(a[0], 1);
    EXPECT_EQ(a[1], 2);
    EXPECT_EQ(a[2], 3);
    EXPECT_EQ(a[3], 4);
}

TEST(StorageTest, EmplaceBack) {
    FastStorage<std::pair<double,bool>, 2> a;
    a.emplace_back(1, false);
    a.emplace_back(2, true);
    a.emplace_back(3, true);
    a.emplace_back(4, false);
    EXPECT_EQ(a.size(), 4);
    EXPECT_EQ(a[0].first, 1);
    EXPECT_EQ(a[0].second, false);
    EXPECT_EQ(a[1].first, 2);
    EXPECT_EQ(a[1].second, true);
    EXPECT_EQ(a[2].first, 3);
    EXPECT_EQ(a[2].second, true);
    EXPECT_EQ(a[3].first, 4);
    EXPECT_EQ(a[3].second, false);
}

TEST(StorageTest, PopBack) {
    FastStorage<int, 2> a;
    a.push_back(1);
    a.push_back(2);
    a.push_back(3);
    a.push_back(4);

    a.pop_back();
    EXPECT_EQ(a.size(), 3);
    EXPECT_EQ(a[0], 1);
    EXPECT_EQ(a[1], 2);
    EXPECT_EQ(a[2], 3);

    a.pop_back();
    EXPECT_EQ(a.size(), 2);
    EXPECT_EQ(a[0], 1);
    EXPECT_EQ(a[1], 2);

    a.pop_back();
    EXPECT_EQ(a.size(), 1);
    EXPECT_EQ(a[0], 1);

    a.pop_back();
    EXPECT_EQ(a.size(), 0);

    // no throw although empty
    EXPECT_NO_THROW(a.pop_back());
    EXPECT_EQ(a.size(), 0);
}

TEST(StorageTest, Erase) {
    FastStorage<int, 2> a;
    a.push_back(1);
    a.push_back(2);
    a.push_back(3);
    a.push_back(4);

    EXPECT_TRUE(a.erase(2));
    EXPECT_EQ(a.size(), 3);
    EXPECT_EQ(a[0], 1);
    EXPECT_EQ(a[1], 2);
    EXPECT_EQ(a[2], 4);

    EXPECT_TRUE(a.erase(0));
    EXPECT_EQ(a.size(), 2);
    EXPECT_EQ(a[0], 2);
    EXPECT_EQ(a[1], 4);

    EXPECT_TRUE(a.erase(1));
    EXPECT_EQ(a.size(), 1);
    EXPECT_EQ(a[0], 2);

    EXPECT_TRUE(a.erase(0));
    EXPECT_EQ(a.size(), 0);

    // no throw although empty
    EXPECT_FALSE(a.erase(0));
}

TEST(StorageTest, TestDestructorCalls) {

    class TestDestructorCalls {
        std::vector<int>* mNumDestructorCalls;
        int mIdentifier = 0;
    public:
        TestDestructorCalls(std::vector<int>* num_destructor_calls, int identifier) : mNumDestructorCalls(num_destructor_calls), mIdentifier(identifier) {}
        ~TestDestructorCalls() {
            mNumDestructorCalls->push_back(mIdentifier);
        }
    };

    {
        std::vector<int> num_destructor_calls;

        FastStorage<TestDestructorCalls, 2> a;
        a.emplace_back(&num_destructor_calls, 1);
        a.emplace_back(&num_destructor_calls, 2);
        a.emplace_back(&num_destructor_calls, 3);
        a.pop_back();
        a.pop_back();
        a.pop_back();

        EXPECT_EQ(num_destructor_calls.size(), 3);
        EXPECT_EQ(num_destructor_calls[0], 3);
        EXPECT_EQ(num_destructor_calls[1], 2);
        EXPECT_EQ(num_destructor_calls[2], 1);
    }

    {
        std::vector<int> num_destructor_calls;

        FastStorage<TestDestructorCalls, 2> a;
        a.emplace_back(&num_destructor_calls, 1);
        a.emplace_back(&num_destructor_calls, 2);
        a.emplace_back(&num_destructor_calls, 3);
        a.erase(0);
        a.erase(0);
        a.erase(0);

        EXPECT_EQ(num_destructor_calls[0], 1);
        // this destructor call is done after moving from vector to in place memory
        EXPECT_EQ(num_destructor_calls[1], 3);
        EXPECT_EQ(num_destructor_calls[2], 2);
        EXPECT_EQ(num_destructor_calls[3], 3);
        EXPECT_EQ(num_destructor_calls.size(), 4);
    }

    {
        std::vector<int> num_destructor_calls;

        FastStorage<TestDestructorCalls, 2> a;
        a.emplace_back(&num_destructor_calls, 1);
        a.emplace_back(&num_destructor_calls, 2);
        a.emplace_back(&num_destructor_calls, 3);
        a.clear();

        EXPECT_EQ(num_destructor_calls[0], 3);
        EXPECT_EQ(num_destructor_calls[1], 1);
        EXPECT_EQ(num_destructor_calls[2], 2);
        EXPECT_EQ(num_destructor_calls.size(), 3);
    }

    {
        std::vector<int> num_destructor_calls;

        FastStorage<TestDestructorCalls, 5> a;
        a.emplace_back(&num_destructor_calls, 1);
        a.emplace_back(&num_destructor_calls, 2);
        a.emplace_back(&num_destructor_calls, 3);
        a.clear();

        EXPECT_EQ(num_destructor_calls[0], 1);
        EXPECT_EQ(num_destructor_calls[1], 2);
        EXPECT_EQ(num_destructor_calls[2], 3);
        EXPECT_EQ(num_destructor_calls.size(), 3);
    }

    {
        std::vector<int> num_destructor_calls;

        FastStorage<TestDestructorCalls, 0> a;
        a.emplace_back(&num_destructor_calls, 1);
        a.emplace_back(&num_destructor_calls, 2);
        a.emplace_back(&num_destructor_calls, 3);
        a.clear();

        // 6 because vector
        EXPECT_EQ(num_destructor_calls.size(), 6);
        EXPECT_EQ(a.size(), 0);
    }

}

TEST(StorageTest, TestFastVectorDestructor) {
    class TestDestructorCalls {
        std::vector<int>* mNumDestructorCalls;
        int mIdentifier = 0;
    public:
        TestDestructorCalls(std::vector<int>* num_destructor_calls, int identifier) : mNumDestructorCalls(num_destructor_calls), mIdentifier(identifier) {}
        ~TestDestructorCalls() {
            mNumDestructorCalls->push_back(mIdentifier);
        }
    };

    {
        std::vector<int> num_destructor_calls;

        {
            FastStorage<TestDestructorCalls, 2> a;
            a.emplace_back(&num_destructor_calls, 1);
            a.emplace_back(&num_destructor_calls, 2);
            a.emplace_back(&num_destructor_calls, 3);
        }

        EXPECT_EQ(num_destructor_calls.size(), 3);

    }
}

TEST(StorageTest, TestIterator) {
    // test on some library algorithms
    {
        FastStorage<int, 2> a{2, 4, 1, 3};
        std::sort(a.begin(), a.end());
        EXPECT_EQ(a.size(), 4);
        EXPECT_EQ(a[0], 1);
        EXPECT_EQ(a[1], 2);
        EXPECT_EQ(a[2], 3);
        EXPECT_EQ(a[3], 4);
    }

    {
        FastStorage<int, 2> a{2, 4, 1, 3};
        std::reverse(a.begin(), a.end());
        EXPECT_EQ(a.size(), 4);
        EXPECT_EQ(a[0], 3);
        EXPECT_EQ(a[1], 1);
        EXPECT_EQ(a[2], 4);
        EXPECT_EQ(a[3], 2);
    }

    {
        // test erase during iteration
        FastStorage<int, 2> a{2, 4, 1, 3};

        for (auto i = a.begin(); i != a.end(); ++i) {
            i = a.erase(i);
        }
        EXPECT_EQ(a.size(), 0);
    }

    {
        // test erase with find
        FastStorage<int, 2> a{2, 4, 1, 3, 4};

        auto it = std::find(a.begin(), a.end(), 4);

        a.erase(it);
        EXPECT_EQ(a.size(), 4);
        EXPECT_EQ(a[0], 2);
        EXPECT_EQ(a[1], 1);
        EXPECT_EQ(a[2], 3);
        EXPECT_EQ(a[3], 4);
    }

    // const it
    {
        FastStorage<int, 2> a{1,2,3,4};

        for (const auto& i: a) {
            EXPECT_TRUE(std::find(a.begin(), a.end(),i) != a.end());
        }
    }

    // const it
    {
        class MyInt {
            int mValue;
        public:
            MyInt(int value) : mValue(value) {}
            bool operator==(const MyInt& other) const {
                return mValue == other.mValue;
            }
        };
        FastStorage<MyInt, 2> a{1,2,3,4};

        for (const auto& i: a) {
            EXPECT_TRUE(std::find(a.begin(), a.end(), i) != a.end());
        }
    }
}

TEST(StorageTest, TestAt) {
    {
        FastStorage<int, 2> a{2, 4, 1, 3, 4};

        EXPECT_EQ(a.at(0), 2);
        EXPECT_EQ(a.at(1), 4);
        EXPECT_EQ(a.at(2), 1);
        EXPECT_EQ(a.at(3), 3);
        EXPECT_EQ(a.at(4), 4);

        EXPECT_THROW(a.at(5), std::out_of_range);
        EXPECT_THROW(a.at(-1), std::out_of_range);
        EXPECT_THROW(a.at(-1100), std::out_of_range);
        EXPECT_THROW(a.at(6), std::out_of_range);
    }
}

TEST(StorageTest, NoDefaultConstructor) {
    class NoDefaultConstructor {
        int a = 0;
    public:
        NoDefaultConstructor() = delete;
        NoDefaultConstructor(int a) : a(a) {}
    };

    FastStorage<NoDefaultConstructor, 2> a{2, 4, 1, 3, 4};
}

TEST(StorageTest, VeryLargeContainer) {
    {
        FastStorage<int, 500> a;
        for (int i = 0; i < 100000; ++i) {
            a.push_back(i);
        }
        EXPECT_EQ(a.size(), 100000);
        for (int i = 0; i < 100000; ++i) {
            EXPECT_EQ(a[i], i);
        }
    }
}

TEST(StorageTest, InPlaceSize) {
    class BigMyType {
        int mValue;
        char mBigArray[1000];
    };

    FastStorage<BigMyType, 100> a;
    EXPECT_GT( sizeof (a), 100*sizeof (BigMyType));
    EXPECT_LT( sizeof (a), 101*sizeof (BigMyType));

    auto& b = a[0];
}

TEST(StorageTest, TestConstIt) {
    class BigMyType {
        int mValue;
        char mBigArray[1000];
    };

    const FastStorage<std::pair<BigMyType, int>, 100> a{std::make_pair(BigMyType(), 1)};
    // test if it compiles
    for (const auto& i : a) {
        EXPECT_EQ(i.second, 1);
    }

}

TEST(StorageTest, TestCopyConstructor) {
    class MyInt {
        int mValue;
    public:
        MyInt(int value) : mValue(value) {}
        bool operator==(const MyInt& other) const {
            return mValue == other.mValue;
        }
    };

    const FastStorage<MyInt, 2> a{1, 2, 3, 4};
    FastStorage<MyInt, 2> b = a;
    b[0] = 1;
    b[1] = 1;
    b[2] = 1;
    b[3] = 1;

    EXPECT_EQ(a[0], 1);
    EXPECT_EQ(a[1], 2);
    EXPECT_EQ(a[2], 3);
    EXPECT_EQ(a[3], 4);
}

TEST(StorageTest, RandomAccess) {
    for (size_t size = 0; size < 1000; size++) {
        FastStorage<int, 500> a;
        for (size_t i = 0; i < size; ++i) {
            a.push_back(i);
        }
        for (size_t i = 0; i < size; ++i) {
            EXPECT_EQ(a[i], i);
        }
    }

    for (size_t size = 1000; size > 0; size--) {
        FastStorage<int, 500> a;
        for (size_t i = 0; i < size; ++i) {
            a.push_back(i);
        }
        for (size_t i = 0; i < size; ++i) {
            EXPECT_EQ(a[i], i);
        }
    }
}

TEST(StorageTest, TestSpeed) {

    int num_runs = 1000000;
    int frequency = 4;

    auto start_faststorage = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_runs; ++i) {
        FastStorage<int, 4> a{1, 2, 3, 4};
        if (i % frequency == 0)
            a.push_back(5);
    }
    auto end_faststorage = std::chrono::high_resolution_clock::now();
    auto duration_faststorage = std::chrono::duration_cast<std::chrono::milliseconds>(end_faststorage - start_faststorage);
    std::cout << "FastStorage duration: " << duration_faststorage.count() << " milliseconds\n";

    // Timing for std::vector
    auto start_vector = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_runs; ++i) {
        std::vector<int> a{1, 2, 3, 4};
        if (i % frequency == 0)
            a.push_back(5);
    }
    auto end_vector = std::chrono::high_resolution_clock::now();
    auto duration_vector = std::chrono::duration_cast<std::chrono::milliseconds>(end_vector - start_vector);
    std::cout << "std::vector duration: " << duration_vector.count() << " milliseconds\n";
}