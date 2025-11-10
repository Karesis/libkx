#pragma once

#include <core/hash/hasher.h>
#include <core/type.h>

/**
 * @file
 * @brief (Trait) 定义 kx_hash() 静态分发宏。
 *
 * 本文件定义了 "Hash" Trait，即一个类型如何将自身喂给一个
 * Hasher。
 *
 * 它通过两个 _Generic 宏实现静态分发：
 * 1. kx_hash(val, state):  (公开) 根据 `val` 的类型分发到
 * "impl Hash for T" 函数 (例如 kx_hash_u64)。
 * 2. KX_HASHER_WRITE_...(state, ...): (内部)
 * 在 "impl" 函数内部，根据 `state` (Hasher) 的类型分发到
 * 正确的 Hasher Trait 实现 (例如
 * kx_DefaultHasher_write_u64)。
 *
 * 这种 "双重分发" 允许 "core" (本文件)
 * 定义类型的哈希逻辑 (kx_hash_u64)，
 * 而 "std" (例如 default_hasher.h)
 * 可以提供具体的 Hasher 实现，并 "注入"
 * 到 KX_HASHER_WRITE 宏中，从而实现完全的静态解耦。
 */

// --- 1. Hasher 状态机分发 (X-Macro) ---
// ----------------------------------------

/**
 * @brief (Internal) Hasher 状态机 X-Macro 注入点。
 *
 * 这是一个 X-Macro。 任何 `std` 中的 Hasher 实现
 * (如 DefaultHasher) 必须：
 * 1. #include <core/hash/hash.h>
 * 2. #undef KX_HASHER_DISPATCH_LIST
 * 3. #define KX_HASHER_DISPATCH_LIST(state, func_name, ...)
 * ... (包含它自己的类型)
 *
 * 示例 (在 std/default_hasher.h 中):
 * #define KX_HASHER_DISPATCH_LIST(state, func, ...) \
 * DefaultHasher*: kx_DefaultHasher_##func(state,
 * ##__VA_ARGS__)
 */
#ifndef KX_HASHER_DISPATCH_LIST
#define KX_HASHER_DISPATCH_LIST(state, func_name, ...)     \
  /* core 中默认是空的。如果用户没有实现任何 hasher， */   \
  /* _Generic 会在 default 分支失败。*/                    \
  default:                                                 \
    (void)0
#endif

/* * 根据上面的 X-Macro 列表，为 Hasher Trait 的每个函数
 * 生成一个 _Generic 分发宏。
 */

/** (Internal) _Generic 分发到 kx_T_Hasher_write() */
#define KX_HASHER_WRITE(state, bytes, len)                 \
  _Generic((state),                                        \
    KX_HASHER_DISPATCH_LIST(state, write, bytes, len))
/** (Internal) _Generic 分发到 kx_T_Hasher_write_u8() */
#define KX_HASHER_WRITE_U8(state, val)                     \
  _Generic((state),                                        \
    KX_HASHER_DISPATCH_LIST(state, write_u8, val))
/** (Internal) _Generic 分发到 kx_T_Hasher_write_u16() */
#define KX_HASHER_WRITE_U16(state, val)                    \
  _Generic((state),                                        \
    KX_HASHER_DISPATCH_LIST(state, write_u16, val))
/** (Internal) _Generic 分发到 kx_T_Hasher_write_u32() */
#define KX_HASHER_WRITE_U32(state, val)                    \
  _Generic((state),                                        \
    KX_HASHER_DISPATCH_LIST(state, write_u32, val))
/** (Internal) _Generic 分发到 kx_T_Hasher_write_u64() */
#define KX_HASHER_WRITE_U64(state, val)                    \
  _Generic((state),                                        \
    KX_HASHER_DISPATCH_LIST(state, write_u64, val))
/** (Internal) _Generic 分发到 kx_T_Hasher_write_i8() */
#define KX_HASHER_WRITE_I8(state, val)                     \
  _Generic((state),                                        \
    KX_HASHER_DISPATCH_LIST(state, write_i8, val))
/** (Internal) _Generic 分发到 kx_T_Hasher_write_i16() */
#define KX_HASHER_WRITE_I16(state, val)                    \
  _Generic((state),                                        \
    KX_HASHER_DISPATCH_LIST(state, write_i16, val))
/** (Internal) _Generic 分发到 kx_T_Hasher_write_i32() */
#define KX_HASHER_WRITE_I32(state, val)                    \
  _Generic((state),                                        \
    KX_HASHER_DISPATCH_LIST(state, write_i32, val))
/** (Internal) _Generic 分发到 kx_T_Hasher_write_i64() */
#define KX_HASHER_WRITE_I64(state, val)                    \
  _Generic((state),                                        \
    KX_HASHER_DISPATCH_LIST(state, write_i64, val))
/** (Internal) _Generic 分发到 kx_T_Hasher_write_f32() */
#define KX_HASHER_WRITE_F32(state, val)                    \
  _Generic((state),                                        \
    KX_HASHER_DISPATCH_LIST(state, write_f32, val))
/** (Internal) _Generic 分发到 kx_T_Hasher_write_f64() */
#define KX_HASHER_WRITE_F64(state, val)                    \
  _Generic((state),                                        \
    KX_HASHER_DISPATCH_LIST(state, write_f64, val))
/** (Internal) _Generic 分发到 kx_T_Hasher_write_usize() */
#define KX_HASHER_WRITE_USIZE(state, val)                  \
  _Generic((state),                                        \
    KX_HASHER_DISPATCH_LIST(state, write_usize, val))
/** (Internal) _Generic 分发到 kx_T_Hasher_write_ptr() */
#define KX_HASHER_WRITE_PTR(state, val)                    \
  _Generic((state),                                        \
    KX_HASHER_DISPATCH_LIST(state, write_ptr, val))

// --- 2. "impl Hash for T" 函数实现 ---
// ------------------------------------

// "impl Hash for u8"
static inline void
kx_hash_u8(const u8 *val, void *hasher_state)
{
  KX_HASHER_WRITE_U8(hasher_state, *val);
}
// "impl Hash for u16"
static inline void
kx_hash_u16(const u16 *val, void *hasher_state)
{
  KX_HASHER_WRITE_U16(hasher_state, *val);
}
// "impl Hash for u32"
static inline void
kx_hash_u32(const u32 *val, void *hasher_state)
{
  KX_HASHER_WRITE_U32(hasher_state, *val);
}
// "impl Hash for u64"
static inline void
kx_hash_u64(const u64 *val, void *hasher_state)
{
  KX_HASHER_WRITE_U64(hasher_state, *val);
}

// "impl Hash for i8"
static inline void
kx_hash_i8(const i8 *val, void *hasher_state)
{
  KX_HASHER_WRITE_I8(hasher_state, *val);
}
// "impl Hash for i16"
static inline void
kx_hash_i16(const i16 *val, void *hasher_state)
{
  KX_HASHER_WRITE_I16(hasher_state, *val);
}
// "impl Hash for i32"
static inline void
kx_hash_i32(const i32 *val, void *hasher_state)
{
  KX_HASHER_WRITE_I32(hasher_state, *val);
}
// "impl Hash for i64"
static inline void
kx_hash_i64(const i64 *val, void *hasher_state)
{
  KX_HASHER_WRITE_I64(hasher_state, *val);
}

// "impl Hash for f32"
static inline void
kx_hash_f32(const f32 *val, void *hasher_state)
{
  //
  KX_HASHER_WRITE_F32(hasher_state, *val);
}
// "impl Hash for f64"
static inline void
kx_hash_f64(const f64 *val, void *hasher_state)
{
  KX_HASHER_WRITE_F64(hasher_state, *val);
}

// "impl Hash for usize"
static inline void
kx_hash_usize(const usize *val, void *hasher_state)
{
  KX_HASHER_WRITE_USIZE(hasher_state, *val);
}

// "impl Hash for str"
static inline void
kx_hash_str(const str *val, void *hasher_state)
{
  usize len = str_len(*val);
  KX_HASHER_WRITE(hasher_state, *val, len);
  // 关键: 也要哈希
  // "abc" 和 "abc\0d" 应该不同
  KX_HASHER_WRITE_U64(hasher_state, (u64)len);
}

// "impl Hash for anyptr" (哈希指针地址)
static inline void
kx_hash_ptr(const void **val, void *hasher_state)
{
  KX_HASHER_WRITE_PTR(hasher_state, *val);
}

// --- 3. 公开的 kx_hash() 静态分发宏 ---
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
#define kx_hash(value, hasher_state)                       \
  _Generic((value), /* Unsigned Ints */                    \
    u8 *: kx_hash_u8,                                      \
    const u8 *: kx_hash_u8,                                \
    u16 *: kx_hash_u16,                                    \
    const u16 *: kx_hash_u16,                              \
    u32 *: kx_hash_u32,                                    \
    const u32 *: kx_hash_u32,                              \
    u64 *: kx_hash_u64,                                    \
    const u64 *: kx_hash_u64,                              \
                                                           \
    /* Signed Ints */                                      \
    i8 *: kx_hash_i8,                                      \
    const i8 *: kx_hash_i8,                                \
    i16 *: kx_hash_i16,                                    \
    const i16 *: kx_hash_i16,                              \
    i32 *: kx_hash_i32,                                    \
    const i32 *: kx_hash_i32,                              \
    i64 *: kx_hash_i64,                                    \
    const i64 *: kx_hash_i64,                              \
                                                           \
    /* Floats */                                           \
    f32 *: kx_hash_f32,                                    \
    const f32 *: kx_hash_f32,                              \
    f64 *: kx_hash_f64,                                    \
    const f64 *: kx_hash_f64,                              \
                                                           \
    /* Size/Arch */                                        \
    usize *: kx_hash_usize,                                \
    const usize *: kx_hash_usize,                          \
                                                           \
    /* Strings */                                          \
    str *: kx_hash_str,                                    \
    const str *: kx_hash_str,                              \
    rawstr *: kx_hash_str, /* rawstr 也是 const char* */   \
    const rawstr *: kx_hash_str,                           \
                                                           \
    /* Pointers (by address) */                            \
    anyptr *: kx_hash_ptr, /* void** */                    \
    canyptr *: (void (*)(const void **, void *))           \
      kx_hash_ptr, /* const void** */                      \
    ptr *: (void (*)(const void **,                        \
                     void *))kx_hash_ptr, /* intptr_t* */  \
    uptr *: (void (*)(                                     \
      const void **, void *))kx_hash_ptr, /* uintptr_t* */ \
                                                           \
    /* 默认情况，你可以添加一个 panic */                   \
    default: (void)0                                       \
                                                           \
           )((value), (hasher_state))
