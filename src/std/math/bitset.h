/*
 * Copyright 2025 Karesis
 * (Apache License Header)
 */

#pragma once

#include <assert.h>           // 引入: assert
#include <std/debug/option.h> // 引入: oexpect
#include <std/mem/allocer.h>  // 引入: __VEC_ALLOC_MACRO 等
#include <std/mem/allocer/bump.h> // 引入: Bump, BUMP 宏
#include <std/mem/allocer/system.h> // 引入: SystemAlloc, SYSTEM 宏
#include <std/mem/layout.h> // 引入: LAYOUT_OF, LAYOUT_OF_ARRAY
#include <std/mem/malloc.h> // 引入: sys_malloc, sys_free
#include <std/type/cm.h>    // 引入: bool, usize, u64
#include <string.h>         // 引入: memset, memcpy, memcmp

/*
 * ===================================================================
 * 1. 内部辅助 (不变)
 * ===================================================================
 *
 * (这些辅助函数是内联的, 供下面的模板宏使用)
 */

static inline usize
bitset_words_for_bits(usize bits)
{
  return (bits + 63) >> 6;
}

static inline usize
bitset_bit_index(usize bit)
{
  return bit >> 6; // bit / 64
}

static inline u64
bitset_bit_mask(usize bit)
{
  return (u64)1 << (bit & 63);
}

/*
 * ===================================================================
 * 2. 核心 Bitset 模板
 * ===================================================================
 *
 * 定义一个 "Bitset 模板", 它遵循 `libkx` 的 "句柄" 模式:
 * 1. 句柄 (struct) 在堆上 (sys_malloc/sys_free)。
 * 2. 数据 (words) 在分配器上
 * (PREFIX_ALLOC/PREFIX_RELEASE)。
 */
#define DEFINE_BITSET(TypeName, AllocType, AllocPrefix)             \
                                                                    \
  /**                                                               \
   * @brief (模板) Bitset 结构体定义                         \
   */                                                               \
  typedef struct TypeName                                           \
  {                                                                 \
    usize num_bits;                                                 \
    usize num_words;                                                \
    u64 *words;                                                     \
    AllocType *alloc_state; /*< 强类型分配器指针 */                 \
  } TypeName;                                                       \
                                                                    \
  /**                                                               \
   * @brief (模板) 创建一个所有位为 0 的 Bitset (空集) \
   */                                                               \
  static inline TypeName *TypeName##_create(                        \
    usize num_bits, AllocType *alloc)                               \
  {                                                                 \
    assert(alloc != NULL);                                          \
    usize num_words = bitset_words_for_bits(num_bits);              \
    Layout word_layout = LAYOUT_OF_ARRAY(u64, num_words);           \
                                                                    \
    /* 1. 句柄在堆上 (libkx 模式) */                                \
    TypeName *bs = (TypeName *)oexpect(                             \
      sys_malloc(sizeof(TypeName)),                                 \
      "Failed to allocate Bitset struct itself");                   \
                                                                    \
    /* 2. 数据在分配器上 (清零) */                                  \
    u64 *words =                                                    \
      ALLOC_ZEROED(AllocPrefix, alloc, u64, num_words);             \
                                                                    \
    if (words == NULL && num_bits > 0)                              \
    {                                                               \
      sys_free(bs); /* 分配失败, 清理句柄 */                        \
      return NULL;                                                  \
    }                                                               \
                                                                    \
    bs->num_bits = num_bits;                                        \
    bs->num_words = num_words;                                      \
    bs->words = words;                                              \
    bs->alloc_state = alloc;                                        \
    return bs; /* 不变性 (已清零) 成立 */                           \
  }                                                                 \
                                                                    \
  /**                                                               \
   * @brief (模板) 销毁 Bitset 句柄                           \
   */                                                               \
  static inline void TypeName##_destroy(TypeName *bs)               \
  {                                                                 \
    if (bs == NULL)                                                 \
      return;                                                       \
                                                                    \
    /* 1. 释放分配器上的数据 (PREFIX_RELEASE) */                    \
    RELEASE(AllocPrefix,                                            \
            bs->alloc_state,                                        \
            u64,                                                    \
            bs->words,                                              \
            bs->num_words);                                         \
                                                                    \
    /* 2. 释放堆上的句柄 */                                         \
    sys_free(bs);                                                   \
  }                                                                 \
                                                                    \
  /**                                                               \
   * @brief (模板) 创建一个所有位为 1 的 Bitset (全集) \
   */                                                               \
  static inline TypeName *TypeName##_create_all(                    \
    usize num_bits, AllocType *alloc)                               \
  {                                                                 \
    assert(alloc != NULL);                                          \
    usize num_words = bitset_words_for_bits(num_bits);              \
    Layout word_layout = LAYOUT_OF_ARRAY(u64, num_words);           \
                                                                    \
    TypeName *bs = (TypeName *)oexpect(                             \
      sys_malloc(sizeof(TypeName)),                                 \
      "Failed to allocate Bitset struct itself");                   \
                                                                    \
    /* 2. 数据在分配器上 (未初始化) */                              \
    u64 *words =                                                    \
      ALLOC(AllocPrefix, alloc, u64, num_words);                    \
                                                                    \
    if (words == NULL && num_bits > 0)                              \
    {                                                               \
      sys_free(bs);                                                 \
      return NULL;                                                  \
    }                                                               \
                                                                    \
    bs->num_bits = num_bits;                                        \
    bs->num_words = num_words;                                      \
    bs->words = words;                                              \
    bs->alloc_state = alloc;                                        \
                                                                    \
    /* 3. 手动设置为 1 并维护不变性 */                              \
    memset(bs->words, 0xFF, bs->num_words * sizeof(u64));           \
    usize remaining_bits = bs->num_bits & 63;                       \
    if (remaining_bits > 0)                                         \
    {                                                               \
      u64 last_mask = ((u64)1 << remaining_bits) - 1;               \
      bs->words[bs->num_words - 1] &= last_mask;                    \
    }                                                               \
    return bs;                                                      \
  }                                                                 \
                                                                    \
  /* --- (所有其他 API 都是通用的, 只需要替换 TypeName)      \
   * --- */                                                         \
                                                                    \
  static inline void TypeName##_set(TypeName *bs,                   \
                                    usize bit)                      \
  {                                                                 \
    assert(bs != NULL && bs->words != NULL);                        \
    assert(bit < bs->num_bits &&                                    \
           "Bitset_set: index out of bounds");                      \
    bs->words[bitset_bit_index(bit)] |=                             \
      bitset_bit_mask(bit);                                         \
  }                                                                 \
                                                                    \
  static inline void TypeName##_clear(TypeName *bs,                 \
                                      usize bit)                    \
  {                                                                 \
    assert(bs != NULL && bs->words != NULL);                        \
    assert(bit < bs->num_bits &&                                    \
           "Bitset_clear: index out of bounds");                    \
    bs->words[bitset_bit_index(bit)] &=                             \
      ~bitset_bit_mask(bit);                                        \
  }                                                                 \
                                                                    \
  static inline bool TypeName##_test(const TypeName *bs,            \
                                     usize bit)                     \
  {                                                                 \
    assert(bs != NULL && bs->words != NULL);                        \
    assert(bit < bs->num_bits &&                                    \
           "Bitset_test: index out of bounds");                     \
    return (bs->words[bitset_bit_index(bit)] &                      \
            bitset_bit_mask(bit)) != 0;                             \
  }                                                                 \
                                                                    \
  static inline void TypeName##_set_all(TypeName *bs)               \
  {                                                                 \
    assert(bs != NULL && bs->words != NULL);                        \
    if (bs->num_words == 0)                                         \
      return;                                                       \
    memset(bs->words, 0xFF, bs->num_words * sizeof(u64));           \
    /* 维护不变性 */                                                \
    usize remaining_bits = bs->num_bits & 63;                       \
    if (remaining_bits > 0)                                         \
    {                                                               \
      u64 last_mask = ((u64)1 << remaining_bits) - 1;               \
      bs->words[bs->num_words - 1] &= last_mask;                    \
    }                                                               \
  }                                                                 \
                                                                    \
  static inline void TypeName##_clear_all(TypeName *bs)             \
  {                                                                 \
    assert(bs != NULL && bs->words != NULL);                        \
    if (bs->num_words == 0)                                         \
      return;                                                       \
    memset(bs->words, 0, bs->num_words * sizeof(u64));              \
  }                                                                 \
                                                                    \
  static inline bool TypeName##_equals(                             \
    const TypeName *bs1, const TypeName *bs2)                       \
  {                                                                 \
    assert(bs1 != NULL && bs1->words != NULL);                      \
    assert(bs2 != NULL && bs2->words != NULL);                      \
    assert(bs1->num_bits == bs2->num_bits &&                        \
           "Bitset_equals: mismatched sizes");                      \
    if (bs1->num_words == 0)                                        \
      return true;                                                  \
    return memcmp(bs1->words,                                       \
                  bs2->words,                                       \
                  bs1->num_words * sizeof(u64)) == 0;               \
  }                                                                 \
                                                                    \
  static inline void TypeName##_copy(TypeName *dest,                \
                                     const TypeName *src)           \
  {                                                                 \
    assert(dest != NULL && dest->words != NULL);                    \
    assert(src != NULL && src->words != NULL);                      \
    assert(dest->num_bits == src->num_bits &&                       \
           "Bitset_copy: mismatched sizes");                        \
    if (src->num_words == 0)                                        \
      return;                                                       \
    memcpy(dest->words,                                             \
           src->words,                                              \
           dest->num_words * sizeof(u64));                          \
  }                                                                 \
                                                                    \
  static inline void TypeName##_intersect(                          \
    TypeName *dest,                                                 \
    const TypeName *src1,                                           \
    const TypeName *src2)                                           \
  {                                                                 \
    assert(dest->num_bits == src1->num_bits &&                      \
           dest->num_bits == src2->num_bits);                       \
    for (usize i = 0; i < dest->num_words; ++i)                     \
    {                                                               \
      dest->words[i] = src1->words[i] & src2->words[i];             \
    }                                                               \
  }                                                                 \
                                                                    \
  static inline void TypeName##_union(                              \
    TypeName *dest,                                                 \
    const TypeName *src1,                                           \
    const TypeName *src2)                                           \
  {                                                                 \
    assert(dest->num_bits == src1->num_bits &&                      \
           dest->num_bits == src2->num_bits);                       \
    for (usize i = 0; i < dest->num_words; ++i)                     \
    {                                                               \
      dest->words[i] = src1->words[i] | src2->words[i];             \
    }                                                               \
  }                                                                 \
                                                                    \
  static inline void TypeName##_difference(                         \
    TypeName *dest,                                                 \
    const TypeName *src1,                                           \
    const TypeName *src2)                                           \
  {                                                                 \
    assert(dest->num_bits == src1->num_bits &&                      \
           dest->num_bits == src2->num_bits);                       \
    for (usize i = 0; i < dest->num_words; ++i)                     \
    {                                                               \
      dest->words[i] = src1->words[i] & ~src2->words[i];            \
    }                                                               \
  }                                                                 \
                                                                    \
  static inline usize TypeName##_count_slow(                        \
    const TypeName *bs)                                             \
  {                                                                 \
    assert(bs != NULL && bs->words != NULL);                        \
    usize count = 0;                                                \
    for (usize i = 0; i < bs->num_bits; ++i)                        \
    {                                                               \
      if (TypeName##_test(bs, i))                                   \
      {                                                             \
        count++;                                                    \
      }                                                             \
    }                                                               \
    return count;                                                   \
  }

/*
 * ===================================================================
 * 3. 模板实例化
 * ===================================================================
 *
 * (假设 SYSTEM_ALLOC_ZEROED 和 BUMP_ALLOC_ZEROED 存在)
 */

/**
 * @brief (实例 1) "sbitset" (SystemAlloc Bitset)
 */
DEFINE_BITSET(sbitset, SystemAlloc, SYSTEM)

/**
 * @brief (实例 2) "bbitset" (BumpAlloc Bitset)
 */
DEFINE_BITSET(bbitset, Bump, BUMP)

/*
 * ===================================================================
 * 4. 统一的泛型宏 (Generic Macros)
 * ===================================================================
 *
 * (为所有函数提供 _Generic 包装器)
 */

#define bs_create(alloc, num_bits)                         \
  _Generic((alloc),                                        \
    SystemAlloc *: sbitset_create,                         \
    Bump *: bbitset_create)(alloc, num_bits)

#define bs_destroy(self)                                   \
  _Generic((self),                                         \
    sbitset *: sbitset_destroy,                            \
    bbitset *: bbitset_destroy)(self)

#define bs_create_all(alloc, num_bits)                     \
  _Generic((alloc),                                        \
    SystemAlloc *: sbitset_create_all,                     \
    Bump *: bbitset_create_all)(alloc, num_bits)

#define bs_set(self, bit)                                  \
  _Generic((self),                                         \
    sbitset *: sbitset_set,                                \
    bbitset *: bbitset_set)(self, bit)

#define bs_clear(self, bit)                                \
  _Generic((self),                                         \
    sbitset *: sbitset_clear,                              \
    bbitset *: bbitset_clear)(self, bit)

#define bs_test(self, bit)                                 \
  _Generic((self),                                         \
    const sbitset *: sbitset_test,                         \
    sbitset *: sbitset_test,                               \
    const bbitset *: bbitset_test,                         \
    bbitset *: bbitset_test)(self, bit)

#define bs_set_all(self)                                   \
  _Generic((self),                                         \
    sbitset *: sbitset_set_all,                            \
    bbitset *: bbitset_set_all)(self)

#define bs_clear_all(self)                                 \
  _Generic((self),                                         \
    sbitset *: sbitset_clear_all,                          \
    bbitset *: bbitset_clear_all)(self)

#define bs_equals(bs1, bs2)                                \
  _Generic((bs1),                                          \
    const sbitset *: sbitset_equals,                       \
    sbitset *: sbitset_equals,                             \
    const bbitset *: bbitset_equals,                       \
    bbitset *: bbitset_equals)(bs1, bs2)

#define bs_copy(dest, src)                                 \
  _Generic((dest),                                         \
    sbitset *: sbitset_copy,                               \
    bbitset *: bbitset_copy)(dest, src)

#define bs_intersect(dest, src1, src2)                     \
  _Generic((dest),                                         \
    sbitset *: sbitset_intersect,                          \
    bbitset *: bbitset_intersect)(dest, src1, src2)

#define bs_union(dest, src1, src2)                         \
  _Generic((dest),                                         \
    sbitset *: sbitset_union,                              \
    bbitset *: bbitset_union)(dest, src1, src2)

#define bs_difference(dest, src1, src2)                    \
  _Generic((dest),                                         \
    sbitset *: sbitset_difference,                         \
    bbitset *: bbitset_difference)(dest, src1, src2)

#define bs_count_slow(self)                                \
  _Generic((self),                                         \
    const sbitset *: sbitset_count_slow,                   \
    sbitset *: sbitset_count_slow,                         \
    const bbitset *: bbitset_count_slow,                   \
    bbitset *: bbitset_count_slow)(self)
