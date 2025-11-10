#pragma once

#include <core/hash/hasher.h>
#include <core/type.h>

/**
 * @file
 * @brief (Trait) 定义 hash() 静态分发宏。
 *
 * 本文件定义了 "Hash" Trait，即一个类型如何将自身喂给一个
 * Hasher。
 *
 * 它通过两个 _Generic 宏实现静态分发：
 * 1. hash(val, state):  (公开) 根据 `val` 的类型分发到
 * "impl Hash for T" 函数 (例如 hash_u64)。
 * 2. HASHER_WRITE_...(state, ...): (内部)
 * 在 "impl" 函数内部，根据 `state` (Hasher) 的类型分发到
 * 正确的 Hasher Trait 实现 (例如
 * DefaultHasher_write_u64)。
 *
 * 这种 "双重分发" 允许 "core" (本文件)
 * 定义类型的哈希逻辑 (hash_u64)，
 * 而 "std" (例如 default_hasher.h)
 * 可以提供具体的 Hasher 实现，并 "注入"
 * 到 HASHER_WRITE 宏中，从而实现完全的静态解耦。
 */

// --- 1. Hasher 状态机分发 (X-Macro) ---
// ----------------------------------------

/**
 * @brief (Internal) Hasher 状态机 X-Macro 注入点。
 *
 * 这是一个 X-Macro。 任何 `std` 中的 Hasher 实现
 * (如 DefaultHasher) 必须：
 * 1. #include <core/hash/hash.h>
 * 2. #undef HASHER_DISPATCH_LIST
 * 3. #define HASHER_DISPATCH_LIST(state, func_name, ...)
 * ... (包含它自己的类型)
 *
 * 示例 (在 std/default_hasher.h 中):
 * #define HASHER_DISPATCH_LIST(state, func, ...) \
 * DefaultHasher*: DefaultHasher_##func(state,
 * ##__VA_ARGS__)
 */
#ifndef HASHER_DISPATCH_LIST
#define HASHER_DISPATCH_LIST(state, func_name, ...)        \
  /* core 中默认是空的。如果用户没有实现任何 hasher， */   \
  /* _Generic 会在 default 分支失败。*/                    \
  default:                                                 \
    (void)0
#endif

/* * 根据上面的 X-Macro 列表，为 Hasher Trait 的每个函数
 * 生成一个 _Generic 分发宏。
 */

/** (Internal) _Generic 分发到 Hasher_write() */
#define HASHER_WRITE(state, bytes, len)                    \
  _Generic((state),                                        \
    HASHER_DISPATCH_LIST(state, write, bytes, len))
/** (Internal) _Generic 分发到 Hasher_write_u8() */
#define HASHER_WRITE_U8(state, val)                        \
  _Generic((state),                                        \
    HASHER_DISPATCH_LIST(state, write_u8, val))
/** (Internal) _Generic 分发到 Hasher_write_u16() */
#define HASHER_WRITE_U16(state, val)                       \
  _Generic((state),                                        \
    HASHER_DISPATCH_LIST(state, write_u16, val))
/** (Internal) _Generic 分发到 Hasher_write_u32() */
#define HASHER_WRITE_U32(state, val)                       \
  _Generic((state),                                        \
    HASHER_DISPATCH_LIST(state, write_u32, val))
/** (Internal) _Generic 分发到 Hasher_write_u64() */
#define HASHER_WRITE_U64(state, val)                       \
  _Generic((state),                                        \
    HASHER_DISPATCH_LIST(state, write_u64, val))
/** (Internal) _Generic 分发到 Hasher_write_i8() */
#define HASHER_WRITE_I8(state, val)                        \
  _Generic((state),                                        \
    HASHER_DISPATCH_LIST(state, write_i8, val))
/** (Internal) _Generic 分发到 Hasher_write_i16() */
#define HASHER_WRITE_I16(state, val)                       \
  _Generic((state),                                        \
    HASHER_DISPATCH_LIST(state, write_i16, val))
/** (Internal) _Generic 分发到 Hasher_write_i32() */
#define HASHER_WRITE_I32(state, val)                       \
  _Generic((state),                                        \
    HASHER_DISPATCH_LIST(state, write_i32, val))
/** (Internal) _Generic 分发到 Hasher_write_i64() */
#define HASHER_WRITE_I64(state, val)                       \
  _Generic((state),                                        \
    HASHER_DISPATCH_LIST(state, write_i64, val))
/** (Internal) _Generic 分发到 Hasher_write_f32() */
#define HASHER_WRITE_F32(state, val)                       \
  _Generic((state),                                        \
    HASHER_DISPATCH_LIST(state, write_f32, val))
/** (Internal) _Generic 分发到 Hasher_write_f64() */
#define HASHER_WRITE_F64(state, val)                       \
  _Generic((state),                                        \
    HASHER_DISPATCH_LIST(state, write_f64, val))
/** (Internal) _Generic 分发到 Hasher_write_usize() */
#define HASHER_WRITE_USIZE(state, val)                     \
  _Generic((state),                                        \
    HASHER_DISPATCH_LIST(state, write_usize, val))
/** (Internal) _Generic 分发到 Hasher_write_ptr() */
#define HASHER_WRITE_PTR(state, val)                       \
  _Generic((state),                                        \
    HASHER_DISPATCH_LIST(state, write_ptr, val))

// --- 3. 公开的 hash() 静态分发宏 ---
// ------------------------------------

/**
 * @brief (Public) 将一个值喂入 Hasher 状态机。
 *
 * 这是一个 _Generic 宏，它构成了 "Hash" Trait
 * 的主要入口点。 它会根据 `value` 的类型，静态分发到正确的
 * "impl Hash for T" 函数。
 *
 * @param value 要哈希的值的指针 (e.g., &my_u64, &my_str)
 * @param hasher_state 指向 Hasher 状态机的指针 (e.g.,
 * &my_default_hasher)
 */
#define hash(value, hasher_state)                           \
  do                                                        \
  {                                                         \
    _Generic((value),                                       \
      /* Unsigned Ints */ /* * 关键修复:                \
                           * 在每个分支内部，       \
                           * 显式地将 'value'           \
                           * 转换为该分支的类型， \
                           * 然后再解引用。          \
                           */                               \
                                                            \
      u8 *: HASHER_WRITE_U8(hasher_state, *((u8 *)value)),  \
      const u8 *: HASHER_WRITE_U8(hasher_state,             \
                                  *((const u8 *)value)),    \
      u16 *: HASHER_WRITE_U16(hasher_state,                 \
                              *((u16 *)value)),             \
      const u16 *: HASHER_WRITE_U16(                        \
               hasher_state, *((const u16 *)value)),        \
      u32 *: HASHER_WRITE_U32(hasher_state,                 \
                              *((u32 *)value)),             \
      const u32 *: HASHER_WRITE_U32(                        \
               hasher_state, *((const u32 *)value)),        \
      u64 *: HASHER_WRITE_U64(hasher_state,                 \
                              *((u64 *)value)),             \
      const u64 *: HASHER_WRITE_U64(                        \
               hasher_state, *((const u64 *)value)),        \
                                                            \
      /* Signed Ints */                                     \
      i8 *: HASHER_WRITE_I8(hasher_state, *((i8 *)value)),  \
      const i8 *: HASHER_WRITE_I8(hasher_state,             \
                                  *((const i8 *)value)),    \
      i16 *: HASHER_WRITE_I16(hasher_state,                 \
                              *((i16 *)value)),             \
      const i16 *: HASHER_WRITE_I16(                        \
               hasher_state, *((const i16 *)value)),        \
      i32 *: HASHER_WRITE_I32(hasher_state,                 \
                              *((i32 *)value)),             \
      const i32 *: HASHER_WRITE_I32(                        \
               hasher_state, *((const i32 *)value)),        \
      i64 *: HASHER_WRITE_I64(hasher_state,                 \
                              *((i64 *)value)),             \
      const i64 *: HASHER_WRITE_I64(                        \
               hasher_state, *((const i64 *)value)),        \
                                                            \
      /* Floats */                                          \
      f32 *: HASHER_WRITE_F32(hasher_state,                 \
                              *((f32 *)value)),             \
      const f32 *: HASHER_WRITE_F32(                        \
               hasher_state, *((const f32 *)value)),        \
      f64 *: HASHER_WRITE_F64(hasher_state,                 \
                              *((f64 *)value)),             \
      const f64 *: HASHER_WRITE_F64(                        \
               hasher_state, *((const f64 *)value)),        \
                                                            \
      /* Strings (特例) */                                  \
      str *: ({                                             \
               /* 修复: 强制转换 'value' */                 \
               str __v = *((str *)value);                   \
               if (__v == NULL)                             \
               {                                            \
                 HASHER_WRITE_U8(hasher_state, 0);          \
               }                                            \
               else                                         \
               {                                            \
                 usize len = str_len(__v);                  \
                 HASHER_WRITE((hasher_state), __v, len);    \
                 HASHER_WRITE_U64((hasher_state),           \
                                  (u64)len);                \
               }                                            \
             }),                                            \
      const str *: ({                                       \
               /* 修复: 强制转换 'value' */                 \
               const str __v = *((const str *)value);       \
               if (__v == NULL)                             \
               {                                            \
                 HASHER_WRITE_U8(hasher_state, 0);          \
               }                                            \
               else                                         \
               {                                            \
                 usize len = str_len(__v);                  \
                 HASHER_WRITE((hasher_state), __v, len);    \
                 HASHER_WRITE_U64((hasher_state),           \
                                  (u64)len);                \
               }                                            \
             }),                                            \
                                                            \
      /* Pointers (by address) */                           \
      anyptr *: HASHER_WRITE_PTR(hasher_state,              \
                                 *((anyptr *)value)),       \
      canyptr *: HASHER_WRITE_PTR(hasher_state,             \
                                  *((canyptr *)value)),     \
                                                            \
      default: (void)0);                                    \
  } while (0)
