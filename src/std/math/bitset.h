#pragma once

#include <core/mem/allocer.h>
#include <core/mem/layout.h>
#include <core/mem/sysalc.h>
#include <core/msg/asrt.h>
#include <core/option.h>
#include <core/type.h>
#include <std/alloc/bump.h>
#include <string.h>

static inline usize
bitset_words_for_bits(usize bits)
{
  return (bits + 63) >> 6;
}

static inline usize
bitset_bit_index(usize bit)
{
  return bit >> 6;
}

static inline u64
bitset_bit_mask(usize bit)
{
  return (u64)1 << (bit & 63);
}

#define DEFINE_BITSET(TypeName, AllocType, AllocPrefix)    \
                                                           \
  typedef struct TypeName                                  \
  {                                                        \
    usize num_bits;                                        \
    usize num_words;                                       \
    u64 *words;                                            \
    AllocType *alloc_state;                                \
  } TypeName;                                              \
                                                           \
  static inline TypeName *TypeName##_create(               \
    AllocType *alloc, usize num_bits)                      \
  {                                                        \
    asrt(alloc != NULL);                                   \
    usize num_words = bitset_words_for_bits(num_bits);     \
    Layout word_layout = LAYOUT_OF_ARRAY(u64, num_words);  \
                                                           \
    TypeName *bs = (TypeName *)oexpect(                    \
      sys_malloc(sizeof(TypeName)),                        \
      "Failed to allocate Bitset struct itself");          \
                                                           \
    u64 *words = ZALLOC(AllocPrefix, alloc, word_layout);  \
                                                           \
    if (words == NULL && num_bits > 0)                     \
    {                                                      \
      sys_free(bs);                                        \
      return NULL;                                         \
    }                                                      \
                                                           \
    bs->num_bits = num_bits;                               \
    bs->num_words = num_words;                             \
    bs->words = words;                                     \
    bs->alloc_state = alloc;                               \
    return bs;                                             \
  }                                                        \
                                                           \
  static inline void TypeName##_destroy(TypeName *bs)      \
  {                                                        \
    if (bs == NULL)                                        \
      return;                                              \
    Layout word_layout =                                   \
      LAYOUT_OF_ARRAY(u64, bs->num_words);                 \
                                                           \
    RELEASE(AllocPrefix,                                   \
            bs->alloc_state,                               \
            bs->words,                                     \
            word_layout);                                  \
                                                           \
    sys_free(bs);                                          \
  }                                                        \
                                                           \
  static inline TypeName *TypeName##_create_all(           \
    AllocType *alloc, usize num_bits)                      \
  {                                                        \
    asrt(alloc != NULL);                                   \
    usize num_words = bitset_words_for_bits(num_bits);     \
    Layout word_layout = LAYOUT_OF_ARRAY(u64, num_words);  \
                                                           \
    TypeName *bs = (TypeName *)oexpect(                    \
      sys_malloc(sizeof(TypeName)),                        \
      "Failed to allocate Bitset struct itself");          \
                                                           \
    u64 *words = ALLOC(AllocPrefix, alloc, word_layout);   \
                                                           \
    if (words == NULL && num_bits > 0)                     \
    {                                                      \
      sys_free(bs);                                        \
      return NULL;                                         \
    }                                                      \
                                                           \
    bs->num_bits = num_bits;                               \
    bs->num_words = num_words;                             \
    bs->words = words;                                     \
    bs->alloc_state = alloc;                               \
                                                           \
    memset(bs->words, 0xFF, bs->num_words * sizeof(u64));  \
    usize remaining_bits = bs->num_bits & 63;              \
    if (remaining_bits > 0)                                \
    {                                                      \
      u64 last_mask = ((u64)1 << remaining_bits) - 1;      \
      bs->words[bs->num_words - 1] &= last_mask;           \
    }                                                      \
    return bs;                                             \
  }                                                        \
                                                           \
  static inline void TypeName##_set(TypeName *bs,          \
                                    usize bit)             \
  {                                                        \
    asrt(bs != NULL && bs->words != NULL);                 \
    asrt_msg(bit < bs->num_bits,                           \
             "Bitset_set: index out of bounds");           \
    bs->words[bitset_bit_index(bit)] |=                    \
      bitset_bit_mask(bit);                                \
  }                                                        \
                                                           \
  static inline void TypeName##_clear(TypeName *bs,        \
                                      usize bit)           \
  {                                                        \
    asrt(bs != NULL && bs->words != NULL);                 \
    asrt_msg(bit < bs->num_bits,                           \
             "Bitset_clear: index out of bounds");         \
    bs->words[bitset_bit_index(bit)] &=                    \
      ~bitset_bit_mask(bit);                               \
  }                                                        \
                                                           \
  static inline bool TypeName##_test(const TypeName *bs,   \
                                     usize bit)            \
  {                                                        \
    asrt(bs != NULL && bs->words != NULL);                 \
    asrt_msg(bit < bs->num_bits,                           \
             "Bitset_test: index out of bounds");          \
    return (bs->words[bitset_bit_index(bit)] &             \
            bitset_bit_mask(bit)) != 0;                    \
  }                                                        \
                                                           \
  static inline void TypeName##_set_all(TypeName *bs)      \
  {                                                        \
    asrt(bs != NULL && bs->words != NULL);                 \
    if (bs->num_words == 0)                                \
      return;                                              \
    memset(bs->words, 0xFF, bs->num_words * sizeof(u64));  \
    usize remaining_bits = bs->num_bits & 63;              \
    if (remaining_bits > 0)                                \
    {                                                      \
      u64 last_mask = ((u64)1 << remaining_bits) - 1;      \
      bs->words[bs->num_words - 1] &= last_mask;           \
    }                                                      \
  }                                                        \
                                                           \
  static inline void TypeName##_clear_all(TypeName *bs)    \
  {                                                        \
    asrt(bs != NULL && bs->words != NULL);                 \
    if (bs->num_words == 0)                                \
      return;                                              \
    memset(bs->words, 0, bs->num_words * sizeof(u64));     \
  }                                                        \
                                                           \
  static inline bool TypeName##_equals(                    \
    const TypeName *bs1, const TypeName *bs2)              \
  {                                                        \
    asrt(bs1 != NULL && bs1->words != NULL);               \
    asrt(bs2 != NULL && bs2->words != NULL);               \
    asrt_msg(bs1->num_bits == bs2->num_bits,               \
             "Bitset_equals: mismatched sizes");           \
    if (bs1->num_words == 0)                               \
      return true;                                         \
    return memcmp(bs1->words,                              \
                  bs2->words,                              \
                  bs1->num_words * sizeof(u64)) == 0;      \
  }                                                        \
                                                           \
  static inline void TypeName##_copy(TypeName *dest,       \
                                     const TypeName *src)  \
  {                                                        \
    asrt(dest != NULL && dest->words != NULL);             \
    asrt(src != NULL && src->words != NULL);               \
    asrt_msg(dest->num_bits == src->num_bits,              \
             "Bitset_copy: mismatched sizes");             \
    if (src->num_words == 0)                               \
      return;                                              \
    memcpy(dest->words,                                    \
           src->words,                                     \
           dest->num_words * sizeof(u64));                 \
  }                                                        \
                                                           \
  static inline void TypeName##_intersect(                 \
    TypeName *dest,                                        \
    const TypeName *src1,                                  \
    const TypeName *src2)                                  \
  {                                                        \
    asrt(dest->num_bits == src1->num_bits &&               \
         dest->num_bits == src2->num_bits);                \
    for (usize i = 0; i < dest->num_words; ++i)            \
    {                                                      \
      dest->words[i] = src1->words[i] & src2->words[i];    \
    }                                                      \
  }                                                        \
                                                           \
  static inline void TypeName##_union(                     \
    TypeName *dest,                                        \
    const TypeName *src1,                                  \
    const TypeName *src2)                                  \
  {                                                        \
    asrt(dest->num_bits == src1->num_bits &&               \
         dest->num_bits == src2->num_bits);                \
    for (usize i = 0; i < dest->num_words; ++i)            \
    {                                                      \
      dest->words[i] = src1->words[i] | src2->words[i];    \
    }                                                      \
  }                                                        \
                                                           \
  static inline void TypeName##_difference(                \
    TypeName *dest,                                        \
    const TypeName *src1,                                  \
    const TypeName *src2)                                  \
  {                                                        \
    asrt(dest->num_bits == src1->num_bits &&               \
         dest->num_bits == src2->num_bits);                \
    for (usize i = 0; i < dest->num_words; ++i)            \
    {                                                      \
      dest->words[i] = src1->words[i] & ~src2->words[i];   \
    }                                                      \
  }                                                        \
                                                           \
  static inline usize TypeName##_count_slow(               \
    const TypeName *bs)                                    \
  {                                                        \
    asrt(bs != NULL && bs->words != NULL);                 \
    usize count = 0;                                       \
    for (usize i = 0; i < bs->num_bits; ++i)               \
    {                                                      \
      if (TypeName##_test(bs, i))                          \
      {                                                    \
        count++;                                           \
      }                                                    \
    }                                                      \
    return count;                                          \
  }

DEFINE_BITSET(sbitset, SystemAlloc, SYSTEM)

DEFINE_BITSET(bbitset, Bump, BUMP)

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
