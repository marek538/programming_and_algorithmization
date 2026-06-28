# Advanced C++ Programming Projects

This repository contains a collection of advanced C++ assignments focusing on memory management, algorithm optimization, and object-oriented design.

---

## 1. CPatchStr: Optimized String Manipulation

### Problem
Standard string operations (append, insert, substring) require heavy memory reallocation and copying, making them highly inefficient for very large text structures.

### Solution
Implemented an optimized string class that utilizes reference counting (`std::shared_ptr`) and stores arrays of references to underlying base strings. This approach avoids redundant character copying, drastically reducing the memory footprint and speeding up manipulations.

---

## 2. CBigInt: Arbitrary-Precision Arithmetic

### Problem
Native integer types (`int`, `long long`) have a fixed size and overflow during large-scale mathematical computations.

### Solution
Created a custom class for handling infinite-precision integers. The implementation uses dynamic arrays operating in base $2^{32}$ (utilizing bitwise operations instead of standard division/modulo) to perform highly optimized addition, multiplication, and comparisons.

---

## 3. CLandRegister: Custom Database Indexing

### Problem
Managing a land registry where plots must be rapidly queryable by two distinct compound keys (Region+ID or City+Address), with standard associative containers (like `std::map` or `std::set`) strictly forbidden.

### Solution
Built a custom database relying on dynamically reallocated arrays and binary search algorithms. The solution ensures logarithmic lookup times and highly efficient sub-linear counting operations without standard STL maps.

---

## 4. Student Database: Complex Filtering & Sorting

### Problem
A university system needs to efficiently store, filter, sort, and autocomplete student records from massive datasets, where naive linear filtering becomes a severe performance bottleneck.

### Solution
Implemented a robust database engine utilizing advanced sorting mechanisms ($O(n \log n)$) and optimized multi-criteria filtering. The solution smartly structures the filtering conditions to minimize the execution time of queries.

---

## 5. CSpreadsheet: Cell Evaluation & Cycle Detection

### Problem
Designing a functional spreadsheet processor capable of evaluating complex formulas, updating dependent cells, and preventing infinite loops caused by circular dependencies.

### Solution
Developed a spreadsheet engine heavily relying on **Polymorphism** and Object-Oriented Programming (OOP) to represent and evaluate different cell types. Implemented graph traversal algorithms to effectively detect and reject cyclic references in cell dependencies.
