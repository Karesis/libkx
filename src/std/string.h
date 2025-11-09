/*
 * Copyright 2025 Karesis
 * (Apache License Header)
 */

#pragma once

// ** 关键改动：包含 L4 集合原语 **
#include <std/vector.h> // 引入: DEFINE_VECTOR

#include <core/mem/sysalc.h> // 引入: SystemAlloc, SYSTEM 宏
#include <std/alloc/bump.h>  // 引入: Bump, BUMP 宏

// ** (不变) 包含 L1 Core 依赖 **
#include <core/type/str.h> // 引入: str, str_len
// 引入 "格式化引擎"
#include <core/fmt/vformat.h>
#include <string.h> // 引入: memcpy

/*
 * ===================================================================
 * 1. 核心向量 (Vector) 定义
 * ===================================================================
 *
 * DEFINE_VECTOR 提供了基础的 _new, _destroy, _push,
 * _reserve_more 等。
 */

/**
 * @brief (实例 1) "sstring" (SystemAlloc String)
 */
DEFINE_VECTOR(sstring, char, SystemAlloc, SYSTEM)

/**
 * @brief (实例 2) "bstring" (BumpAlloc String)
 */
DEFINE_VECTOR(bstring, char, Bump, BUMP)

/*
 * ===================================================================
 * 2. 字符串专用 API 模板
 * ===================================================================
 *
 * [优化]
 * 这是一个新的模板宏, 用于消除 `sstring_` 和 `bstring_`
 * 辅助函数之间的代码重复。
 *
 * 它依赖于 `DEFINE_VECTOR` 已经生成的 TypeName##_new,
 * TypeName##_reserve_to, TypeName##_reserve_more,
 * TypeName##_len 和 TypeName##_as_const_ptr 函数。
 *
 * @param TypeName  我们正在操作的类型 (例如: sstring,
 * bstring)
 * @param AllocType 构造函数所需的分配器类型 (例如:
 * SystemAlloc, Bump)
 */
#define DEFINE_STRING_API(TypeName, AllocType)                            \
                                                                          \
  /**                                                                     \
   * @brief (泛型模板) 从一个 C 字符串 (str) 创建一个新的 \
   * string。                                                            \
   */                                                                     \
  static inline TypeName *TypeName##_new_from_str(                        \
    AllocType *alloc, str s)                                              \
  {                                                                       \
    usize len = str_len(s);                                               \
    TypeName *self =                                                      \
      TypeName##_new(alloc); /* (来自 DEFINE_VECTOR) */                   \
    TypeName##_reserve_to(                                                \
      self, len + 1); /* (来自 DEFINE_VECTOR) */                          \
    memcpy(self->data, s, len);                                           \
    self->data[len] = '\0';                                               \
    self->len = len;                                                      \
    return self;                                                          \
  }                                                                       \
                                                                          \
  /**                                                                     \
   * @brief (泛型模板) 在 string 末尾追加一个 C 字符串      \
   * (str)。                                                             \
   */                                                                     \
  static inline void TypeName##_push_str(TypeName *self,                  \
                                         str s)                           \
  {                                                                       \
    usize len = str_len(s);                                               \
    if (len == 0)                                                         \
      return;                                                             \
    TypeName##_reserve_more(                                              \
      self, len); /* (来自 DEFINE_VECTOR) */                              \
    memcpy(self->data + self->len, s, len);                               \
    self->len += len;                                                     \
    self->data[self->len] = '\0';                                         \
  }                                                                       \
                                                                          \
  /**                                                                     \
   * @brief (泛型模板) 将 string 作为 C 字符串 (str)            \
   * 查看。                                                            \
   */                                                                     \
  static inline str TypeName##_as_str(                                    \
    const TypeName *self)                                                 \
  {                                                                       \
    if (TypeName##_len(self) ==                                           \
        0) /* (来自 DEFINE_VECTOR) */                                     \
    {                                                                     \
      return ""; /* 返回一个指向静态空字符串的指针 */                     \
    }                                                                     \
    return TypeName##_as_const_ptr(                                       \
      self); /* (来自 DEFINE_VECTOR) */                                   \
  }                                                                       \
                                                                          \
  /**                                                                     \
   * @brief (泛型模板) 在 string 末尾追加一段原始字节。   \
   * (注意: 假设 'rawstr' 是 'const char*' 或 'const                \
   * void*')                                                              \
   */                                                                     \
  static inline void TypeName##_push_bytes(                               \
    TypeName *self, const char *bytes, usize len)                         \
  {                                                                       \
    if (len == 0)                                                         \
      return;                                                             \
    TypeName##_reserve_more(                                              \
      self, len); /* (来自 DEFINE_VECTOR) */                              \
    memcpy(self->data + self->len, bytes, len);                           \
    self->len += len;                                                     \
    self->data[self->len] = '\0';                                         \
  }

/*
 * ===================================================================
 * 3. 模板实例化 (Template Instantiation)
 * ===================================================================
 */

// [优化]
// 不再有重复的代码, 只需调用模板宏

/**
 * @brief (实例 1) sstring API
 */
DEFINE_STRING_API(sstring, SystemAlloc)

/**
 * @brief (实例 2) bstring API
 */
DEFINE_STRING_API(bstring, Bump)

/*
 * ===================================================================
 * 5. [新增] 格式化 Sink 适配器
 * ===================================================================
 *
 * 这些是 vformat_func 引擎所需的回调函数。
 * 它们依赖于 DEFINE_VECTOR 和 DEFINE_STRING_API 已经
 * 生成的 _push 和 _push_bytes 函数。
 */

/* --- sstring Adapters --- */
static inline void
sstring_push_char_adapter(void *sink, char c)
{
  /* * 静态分发：调用 sstring_push (来自 DEFINE_VECTOR) */
  sstring_push((sstring *)sink, c);
}
static inline void
sstring_push_bytes_adapter(void *sink,
                           const char *bytes,
                           usize len)
{
  /* * 静态分发：调用 sstring_push_bytes (来自
   * DEFINE_STRING_API) */
  sstring_push_bytes((sstring *)sink, bytes, len);
}

/* --- bstring Adapters --- */
static inline void
bstring_push_char_adapter(void *sink, char c)
{
  /* * 静态分发：调用 bstring_push (来自 DEFINE_VECTOR) */
  bstring_push((bstring *)sink, c);
}
static inline void
bstring_push_bytes_adapter(void *sink,
                           const char *bytes,
                           usize len)
{
  /* * 静态分发：调用 bstring_push_bytes (来自
   * DEFINE_STRING_API) */
  bstring_push_bytes((bstring *)sink, bytes, len);
}

/*
 * ===================================================================
 * 4. 统一的泛型宏 (Generic Macros)
 * ===================================================================
 *
 * (此部分保持不变)
 *
 * 使用 C11 _Generic 在编译时选择正确的实现。
 * 这为 `sstring` 和 `bstring` 提供了统一的 API。
 */

/**
 * @brief (泛型) 在字符串末尾追加一个 C 字符串 (str)。
 */
#define s_push_str(self, s)                                \
  _Generic((self),                                         \
    sstring *: sstring_push_str,                           \
    bstring *: bstring_push_str)(self, s)

/**
 * @brief (泛型) 将字符串作为 C 字符串 (str) 查看。
 */
#define s_as_str(self)                                     \
  _Generic((self),                                         \
    sstring *: sstring_as_str,                             \
    const sstring *: sstring_as_str,                       \
    bstring *: bstring_as_str,                             \
    const bstring *: bstring_as_str)(self)

/**
 * @brief (泛型) 从一个 C 字符串 (str)
 * 创建一个新的动态字符串。
 */
#define s_new_from_str(alloc, s)                           \
  _Generic((alloc),                                        \
    SystemAlloc *: sstring_new_from_str,                   \
    Bump *: bstring_new_from_str)(alloc, s)

/**
 * @brief (泛型) 格式化内容并追加到 sstring 或 bstring。
 *
 * @example
 * sstring *s = sstring_new(...);
 * s_format(s, "Hello, {str}!", "world");
 */
#define s_format(sink, fmt, ...)                           \
  vformat_func(                                            \
    (void *)(sink),                                        \
                                                           \
    /* 2. 静态选择 "push_char" 适配器 */                   \
    _Generic((sink),                                       \
      sstring *: sstring_push_char_adapter,                \
      bstring *: bstring_push_char_adapter),               \
                                                           \
    /* 3. 静态选择 "push_bytes" 适配器 */                  \
    _Generic((sink),                                       \
      sstring *: sstring_push_bytes_adapter,               \
      bstring *: bstring_push_bytes_adapter),              \
                                                           \
    /* 4. 格式化字符串和其余参数 (来自 vformat.h) */       \
    (fmt),                                                 \
    ARGS_COUNT(__VA_ARGS__) __VA_OPT__(, )                 \
      EXPAND_ALL(TYPE_INFO, __VA_ARGS__))
