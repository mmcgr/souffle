/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2015, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file symbol_table_test.h
 *
 * Tests souffle's symbol table.
 *
 ***********************************************************************/

#include "tests/test.h"

#include "SymbolTable.h"
#include "utility/MiscUtil.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

namespace souffle::test {

// TEST(SymbolTable, Basics) {
//    SymbolTable table;
//
//    table.insert("Hello");
//
//    EXPECT_STREQ("Hello", table.resolve(table.lookup(table.resolve(table.lookup("Hello")))));
//
//    EXPECT_EQ(table.lookup("Hello"), table.lookup(table.resolve(table.lookup("Hello"))));
//
//    EXPECT_STREQ("Hello", table.resolve(table.lookup(table.resolve(table.lookup("Hello")))));
//
//    EXPECT_EQ(table.lookup("Hello"),
//            table.lookup(table.resolve(table.lookup(table.resolve(table.lookup("Hello"))))));
//}
//
// TEST(SymbolTable, Copy) {
//    auto* a = new SymbolTable();
//    a->insert("Hello");
//
//    auto* b = new SymbolTable(*a);
//
//    size_t a_idx = a->lookup("Hello");
//    size_t b_idx = b->lookup("Hello");
//
//    // hash should be the same
//    EXPECT_EQ(a_idx, b_idx);
//
//    EXPECT_STREQ("Hello", a->resolve(a_idx));
//    EXPECT_STREQ("Hello", b->resolve(b_idx));
//
//    // should be the same actual string
//    EXPECT_STREQ(a->resolve(a_idx), b->resolve(b_idx));
//
//    // b should survive
//    delete a;
//    EXPECT_STREQ("Hello", b->resolve(b_idx));
//
//    delete b;
//}
//
// TEST(SymbolTable, Assign) {
//    auto* a = new SymbolTable();
//    a->insert("Hello");
//
//    SymbolTable b = *a;
//    SymbolTable c;
//
//    c = *a;
//
//    size_t a_idx = a->lookup("Hello");
//    size_t b_idx = b.lookup("Hello");
//    size_t c_idx = c.lookup("Hello");
//
//    // hash should be the same
//    EXPECT_EQ(a_idx, b_idx);
//    EXPECT_EQ(b_idx, c_idx);
//
//    EXPECT_STREQ("Hello", a->resolve(a_idx));
//    EXPECT_STREQ("Hello", b.resolve(b_idx));
//    EXPECT_STREQ("Hello", c.resolve(c_idx));
//
//    // b and c should survive
//    delete a;
//    EXPECT_STREQ("Hello", b.resolve(b_idx));
//    EXPECT_STREQ("Hello", c.resolve(c_idx));
//}

TEST(SymbolTable, Inserts1) {
    // whether to print the recorded times to stdout
    // should be false unless developing
    const bool ECHO_TIME = true;

    // type for very big number
    using T = unsigned long long;
    time_point start, end;

    T n = 0;        // counter
    T N = 1000000;  // number of symbols to insert
    // T N = 1000000;   // larger tables for debugging/timing
    // T N = 1000000;  // larger tables for debugging/timing

    SymbolTable X;
    std::string x;

    std::vector<std::string> A;
    A.reserve(N);

    if (ECHO_TIME) {
        std::cout << "Testing with " << N << " strings of form <1234string>" << std::endl;
    }
    for (T i = 0; i < N; ++i) {
        x = std::to_string(i) + "string";
        A.push_back(x);
    }
    start = now();
    for (T i = 0; i < N; ++i) {
        X.insert(A[i]);  // insert one at a time
    }
    end = now();
    n = duration_in_us(start, end);  // record the time

    if (ECHO_TIME) {
        std::cout << "Inserting new elements:      " << n << " us" << std::endl;
    }

    // try inserting all the elements that were just inserted
    start = now();
    X.insert(A);
    end = now();
    n = duration_in_us(start, end);

    if (ECHO_TIME) {
        std::cout << "Inserting existing elements: " << n << " us" << std::endl;
    }
}

TEST(SymbolTable, Inserts2) {
    // whether to print the recorded times to stdout
    // should be false unless developing
    const bool ECHO_TIME = true;

    // type for very big number
    using T = unsigned long long;
    time_point start, end;

    T n = 0;        // counter
    T N = 1000000;  // number of symbols to insert
    // T N = 1000000;   // larger tables for debugging/timing
    // T N = 1000000;  // larger tables for debugging/timing

    SymbolTable X;
    std::string x;

    std::vector<std::string> A;
    A.reserve(N);

    if (ECHO_TIME) {
        std::cout << "Testing with " << N << " strings of form <long string to get things started:1234>"
                  << std::endl;
    }
    for (T i = 0; i < N; ++i) {
        x = "long string to get things started:" + std::to_string(i);
        A.push_back(x);
    }
    start = now();
    for (T i = 0; i < N; ++i) {
        X.insert(A[i]);  // insert one at a time
    }
    end = now();
    n = duration_in_us(start, end);  // record the time

    if (ECHO_TIME) {
        std::cout << "Inserting new elements:      " << n << " us" << std::endl;
    }

    // try inserting all the elements that were just inserted
    start = now();
    X.insert(A);
    end = now();
    n = duration_in_us(start, end);

    if (ECHO_TIME) {
        std::cout << "Inserting existing elements: " << n << " us" << std::endl;
    }
}

TEST(SymbolTable, Inserts3) {
    // whether to print the recorded times to stdout
    // should be false unless developing
    const bool ECHO_TIME = true;

    // type for very big number
    using T = unsigned long long;
    time_point start, end;

    T n = 0;       // counter
    T N = 100000;  // number of symbols to insert
    // T N = 1000000;   // larger tables for debugging/timing
    // T N = 1000000;  // larger tables for debugging/timing

    SymbolTable X;
    std::string x;

    std::vector<std::string> A;
    A.reserve(N);

    if (ECHO_TIME) {
        std::cout << "Testing with " << N << " strings of form <1234:long string to get things started>"
                  << std::endl;
    }
    for (T i = 0; i < N; ++i) {
        x = std::to_string(i) + "long string to get things started:";
        A.push_back(x);
    }
    start = now();
    for (T i = 0; i < N; ++i) {
        X.insert(A[i]);  // insert one at a time
    }
    end = now();
    n = duration_in_us(start, end);  // record the time

    if (ECHO_TIME) {
        std::cout << "Inserting new elements:      " << n << " us" << std::endl;
    }

    // try inserting all the elements that were just inserted
    start = now();
    X.insert(A);
    end = now();
    n = duration_in_us(start, end);

    if (ECHO_TIME) {
        std::cout << "Inserting existing elements: " << n << " us" << std::endl;
    }
}

TEST(SymbolTable, Inserts4) {
    // whether to print the recorded times to stdout
    // should be false unless developing
    const bool ECHO_TIME = true;

    // type for very big number
    using T = unsigned long long;
    time_point start;
    time_point end;

    T n = 0;        // counter
    T N = 1000000;  // number of symbols to insert
    // T N = 1000000;   // larger tables for debugging/timing
    // T N = 1000000;  // larger tables for debugging/timing

    SymbolTable X;
    std::string x;

    std::vector<std::string> A;
    A.reserve(N);

    if (ECHO_TIME) {
        std::cout << "Testing with " << N << " strings of form <1234>" << std::endl;
    }
    for (T i = 0; i < N; ++i) {
        x = std::to_string(i);
        A.push_back(x);
    }
    start = now();
    for (T i = 0; i < N; ++i) {
        X.insert(A[i]);  // insert one at a time
    }
    end = now();
    n = duration_in_us(start, end);  // record the time

    if (ECHO_TIME) {
        std::cout << "Inserting new elements:      " << n << " us" << std::endl;
    }

    // try inserting all the elements that were just inserted
    start = now();
    X.insert(A);
    end = now();
    n = duration_in_us(start, end);

    if (ECHO_TIME) {
        std::cout << "Inserting existing elements: " << n << " us" << std::endl;
    }
}

TEST(SymbolTable, Inserts5) {
    // whether to print the recorded times to stdout
    // should be false unless developing
    const bool ECHO_TIME = true;

    // type for very big number
    using T = unsigned long long;
    time_point start, end;

    T n = 0;        // counter
    T N = 1000000;  // number of symbols to insert
    // T N = 1000000;   // larger tables for debugging/timing
    // T N = 1000000;  // larger tables for debugging/timing

    SymbolTable X;
    std::string x;

    std::vector<std::string> A;
    A.reserve(N);

    if (ECHO_TIME) {
        std::cout << "Testing with " << N << " strings of form <\\1\\2\\3\\4>" << std::endl;
    }
    A.push_back("\0");
    for (T i = 1; i < N; ++i) {
        T j = i;
        x = "";
        while (j > 0) {
            x += (uint8_t)(j % 256);
            j /= 256;
        }
        A.push_back(x);
    }
    start = now();
    for (T i = 0; i < N; ++i) {
        X.insert(A[i]);  // insert one at a time
    }
    end = now();
    n = duration_in_us(start, end);  // record the time

    if (ECHO_TIME) {
        std::cout << "Inserting new elements:      " << n << " us" << std::endl;
    }

    // try inserting all the elements that were just inserted
    start = now();
    X.insert(A);
    end = now();
    n = duration_in_us(start, end);

    if (ECHO_TIME) {
        std::cout << "Inserting existing elements: " << n << " us" << std::endl;
    }
}

TEST(SymbolTable, InsertsParallel) {
    // whether to print the recorded times to stdout
    // should be false unless developing
    const bool ECHO_TIME = true;

    // type for very big number
    using T = unsigned long long;
    time_point start, end;

    T n = 0;        // counter
    T N = 1000000;  // number of symbols to insert
    // T N = 1000000;   // larger tables for debugging/timing
    // T N = 1000000;  // larger tables for debugging/timing

    SymbolTable X;
    std::string x;

    std::vector<std::string> A;
    A.reserve(N);
    A.push_back("");

    if (ECHO_TIME) {
        std::cout << "Testing with " << N << " strings of form <1234> in parallel" << std::endl;
    }
    for (T i = 0; i < N; ++i) {
        x = std::to_string(i);
        A.push_back(x);
    }
    start = now();

#pragma omp parallel for  //  schedule(static,1)
    for (auto it = A.begin(); it < A.end(); ++it) {
        X.insert(*it);
    }
    end = now();
    n = duration_in_us(start, end);  // record the time

    if (ECHO_TIME) {
        std::cout << "Inserting new elements:      " << n << " us" << std::endl;
    }
    EXPECT_EQ(A.size(), X.size());

    // try inserting all the elements that were just inserted
    start = now();
#pragma omp parallel for  //  schedule(static,1)
    for (auto it = A.begin(); it < A.end(); ++it) {
        X.insert(*it);
    }
    end = now();
    n = duration_in_us(start, end);

    if (ECHO_TIME) {
        std::cout << "Inserting existing elements: " << n << " us" << std::endl;
    }
    EXPECT_EQ(A.size(), X.size());
}

}  // namespace souffle::test
