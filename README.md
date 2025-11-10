# libkx

> **libkx** is a modern C23 foundation library designed for high-performance systems programming. It provides a set of zero-cost, type-safe abstractions heavily inspired by Rust's `core`/`std` architecture.
>
> The core philosophy of this library is to fully embrace **static dispatch**, using C11/C23 `_Generic`, macros, and inline functions to completely avoid C++-style dynamic vtables and runtime overhead.

## 🏛️ Core Philosophy

  * **Static Dispatch: Zero runtime overhead.** This library does not use dynamic vtables. All "generics" are resolved at compile-time via macros (C-style Templates) and `_Generic` (static trait selection).
  * **`core` / `std` Architecture:**
      * **`src/core`**: The project's pure foundation. It is (almost) header-only, has no dependencies on `std`, and performs no allocations (except for `sysalc.h`). It defines "traits" (like `core/mem/allocer.h`) and core primitives (like `core/option.h`, `core/result.h`, `core/fmt/vformat.h`).
      * **`src/std`**: Implementations and consumers of `core`. It provides concrete implementations of `core` traits (like `std/alloc/bump.h`) and high-level data structures built upon them (like `std/vector.h`, `std/string.h`).
  * **Rust-flavored C:** Provides the comfort of a modern language (`Result<T, E>`, `Option<T>`, `panic!`, `dbg!`) while retaining the full control and transparency of C.

## ✨ Features

  * **`core/option.h`** & **`core/result.h`**: Rust-style `Option<T>` and `Result<T, E>` core data structures (`Some`, `None`, `Ok`, `Err`).
  * **`core/msg/` - Messaging**:
      * `panic.h`: A `panic!(...)` macro for unrecoverable errors.
      * `dbg.h`: A `dbg!(...)` macro for debug-printing (like `eprintln!`).
      * `asrt.h`: `asrt!(...)` and `asrt_msg!(...)` for assertions.
  * **`core/fmt/` - Type-Safe Formatting**:
      * `vformat.h`: The `_Generic`-driven type-safe formatting engine.
      * `tofile.h`: A low-level `FILE*` Sink for the engine (used by `panic` and `dbg`).
  * **`core/mem/` - Memory Traits & Primitives**:
      * `allocer.h`: The static allocator Trait (Contract). Defines `ALLOC`, `REALLOC`, etc., for static dispatch.
      * `sysalc.h`: The `SystemAlloc` Impl. The first implementation of the `allocer.h` trait, wrapping `malloc`/`free`.
      * `layout.h`: A `Layout` struct for describing memory blocks.
  * **`core/hash/` - Hashing Traits**:
      * `hasher.h`: The static hasher Trait (Contract).
      * `hash.h`: The `_Generic` engine for hashing types.
  * **`std/alloc/` - Allocator Implementations**:
      * `bump.h` & `bump.c`: A fast Bump (Arena) Allocator. The second impl of the `allocer.h` trait, which itself is backed by another allocator (like `SystemAlloc`).
  * **`std/hash/` - Hashing Implementation**:
      * `xxhash.c`: A concrete impl of the `hasher.h` trait using XXH64.
      * `default.h`: Provides the `DefaultHasher` used by the library.
  * **`std/` - Data Structures (Generic "Templates")**:
      * `vector.h`: `DEFINE_VECTOR` macro for instantiating a `Vector` type.
      * `string.h`: `sstring` (SystemAlloc) and `bstring` (BumpAlloc) implementations, built from the `Vector` macro. Provides the `s_format` sink.
      * `hashmap.h`: `DEFINE_HASHMAP` macro for instantiating an open-addressing, linear-probing `HashMap` type.
  * **`math/bitset.h`**: `DEFINE_BITSET` macro for instantiating a `Bitset` type bound to an allocator.
  * **`std/test/` - Built-in Test Framework**:
      * `test.h` and `test.c` are provided as part of the library.
      * Offers `SUITE_START`, `SUITE_END`, `TEST_ASSERT`, and `TEST_SUMMARY` macros for building test runners.

## 🏗️ Architecture

libkx's architecture is its core feature, strictly following dependencies to prevent cycles.

```plaintext
[L5+] Application (e.g., tests/test_vector.c)
|
v
[L4] std/string.h, std/vector.h, std/hashmap.h, std/math/bitset.h (Collections)
|      (Consumes L1 Trait, depends on L3 Impl)
v
[L3] std/alloc/bump.h, std/hash/default.h, std/test/test.h (Std Impls)
|      (Depends on L2 Sink & L2 Impl)
v
[L2] core/msg/panic.h, core/mem/sysalc.h (Core Impls)
|      (Depends on L1 Engine/Trait)
v
[L1] core/mem/allocer.h, core/fmt/vformat.h, core/hash/hasher.h (Core Trait/Engine)
|
v
[L0] core/type.h, core/option.h, core/result.h (Core Primitives)
```

## 🚀 Quick Start

This example demonstrates all core concepts of libkx:

1.  Create a `SystemAlloc` (L2 Impl).
2.  Use it to create a `Bump` allocator (L3 Impl).
3.  Use the `Bump` allocator to create a `bstring` (L4 Collection).
4.  Use `s_format` (L4 High-Level Sink) to write into it.
5.  Use `dbg!` (L3 Message) to print the result.

<!-- end list -->

```c
#include <core/msg/dbg.h>      // L3 Messaging
#include <core/option.h>     // L1 (for oexpect)
#include <std/alloc/bump.h>    // L3 Allocator Impl
#include <std/string.h>    // L4 Collection
#include <core/mem/sysalc.h> // L2 Allocator Impl

int main(void) {
    // 1. Create a SystemAlloc (on the stack, it's a ZST)
    SystemAlloc sys_alloc;

    // 2. Use SystemAlloc as a backing allocator to create a Bump arena
    //    (bump_new returns Option<Bump*>, we use oexpect to unwrap)
    Bump *arena = oexpect(
      bump_new(&sys_alloc), "Failed to create arena");

    // 3. Use the arena to create a bstring (Bump-allocated string)
    //    (s_new_from_str is a _Generic macro)
    sstring *str = s_new_from_str(arena, "Hello ");

    // 4. Use s_format (also _Generic) to append formatted text
    int user_id = 123;
    s_format(str, "User ID: {}", user_id);

    // 5. Use dbg! (L3) to print the bstring (L4)
    //    (s_as_str is _Generic, casting bstring to str)
    dbg("Final string: {}", s_as_str(str));

    // 6. Destroy the arena (this automatically frees the string's memory)
    bump_free(arena);
    
    return 0;
}
```

**Output:**

```bash
[DEBUG] (src/main.c:25) Final string: Hello User ID: 123
```

## 🛠️ Building

This project is built with `make` and `clang` (or `gcc`).

  * **Build the library (`libkx.a`):**
    ```bash
    make lib
    ```
  * **Build the tests:**
    ```bash
    make tests
    ```
  * **Build everything:**
    ```bash
    make all
    ```
  * **Run the tests:**
    ```bash
    make run_tests
    ```
  * **Clean build artifacts:**
    ```bash
    make clean
    ```

## 📜 License

libkx is distributed under the **GNU Lesser General Public License v3.0 or later** (`LGPL-3.0-or-later`).

The LGPL 3.0 is built upon the GPL 3.0. As such, this library includes both license files, which must be distributed together:

  * `COPYING`: The full text of the GNU General Public License (GPL) v3.0.
  * `COPYING.LESSER`: The additional permissions of the GNU Lesser General Public License (LGPL) v3.0.

In short, you are free to link libkx (dynamically or statically) with a closed-source application. However, if you modify the `libkx` library itself, you must release those modifications under an LGPL 3.0-compatible license.