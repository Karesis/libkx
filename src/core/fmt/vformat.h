/*
 * Copyright (C) 2025 Karesis
 *
 * This file is part of libkx.
 *
 * libkx is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General
 * Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * libkx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this program.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

/* include/std/fmt/format.h */
#pragma once

/*
 * ===================================================================
 * 1. 依赖
 * ===================================================================
 */

#include <core/type.h> // libkx: For i8, u8, ..., bool, etc.
#include <stdarg.h>    // C Standard: For va_list

/*
 * ===================================================================
 * 2. 类型定义 (The Contract)
 * ===================================================================
 *
 * 这些 `TYPE_*` 常量是 `TYPE_ID` 宏 (编译时) 和
 * `vformat_engine` (运行时) 之间的“契约”。
 */

#define TYPE_NONE 0

#define TYPE_I8 1
#define TYPE_I16 2
#define TYPE_I32 3
#define TYPE_I64 4

#define TYPE_U8 5
#define TYPE_U16 6
#define TYPE_U32 7
#define TYPE_U64 8

#define TYPE_FLOAT 9
#define TYPE_DOUBLE 10

/*
 * 注意: 我们统一使用 `str` (const char*) 作为字符串类型。
 * libkx 用户应使用 s_as_str() 将 sstring/bstring 转换为 str
 * 来打印。
 */
#define TYPE_STR 11 /* 对应 `str` (const char *) */
#define TYPE_MUT_STR                                       \
  12                 /* 对应 `char *` (可变 C 字符串)    \
                      */
#define TYPE_CHAR 13 /* 对应 `char` */
#define TYPE_ANY 14  /* 对应 `void *` (用于打印指针地址) */

/*
 * ===================================================================
 * 3. _Generic 类型 ID 宏 (编译时)
 * ===================================================================
 */

/**
 * @brief 在编译时将一个 C 表达式 (t) 转换为一个 `TYPE_*`
 * 常量。
 */
#define TYPE_ID(t)                                         \
  _Generic((t),                                            \
    i8: TYPE_I8,                                           \
    i16: TYPE_I16,                                         \
    i32: TYPE_I32,                                         \
    i64: TYPE_I64,                                         \
    u8: TYPE_U8,                                           \
    u16: TYPE_U16,                                         \
    u32: TYPE_U32,                                         \
    u64: TYPE_U64,                                         \
    float: TYPE_FLOAT,                                     \
    double: TYPE_DOUBLE,                                   \
    str: TYPE_STR, /* const char* */                       \
    char *: TYPE_MUT_STR,                                  \
    char: TYPE_CHAR,                                       \
    void *: TYPE_ANY,                                      \
    default: TYPE_NONE)

/*
 * ===================================================================
 * 4. 宏元编程 (Variadic Macro Magic)
 * ===================================================================
 */

/**
 * @brief (内部) 将一个变量 `var` 展开为 `TYPE_ID(var), var`
 * 对。
 */
#define TYPE_INFO(var) TYPE_ID(var), (var)

/* --- 递归宏展开 (最多 16 个参数) --- */
#define EXPAND_1(func, var) func(var)
#define EXPAND_2(func, var, ...)                           \
  func(var), EXPAND_1(func, __VA_ARGS__)
#define EXPAND_3(func, var, ...)                           \
  func(var), EXPAND_2(func, __VA_ARGS__)
#define EXPAND_4(func, var, ...)                           \
  func(var), EXPAND_3(func, __VA_ARGS__)
#define EXPAND_5(func, var, ...)                           \
  func(var), EXPAND_4(func, __VA_ARGS__)
#define EXPAND_6(func, var, ...)                           \
  func(var), EXPAND_5(func, __VA_ARGS__)
#define EXPAND_7(func, var, ...)                           \
  func(var), EXPAND_6(func, __VA_ARGS__)
#define EXPAND_8(func, var, ...)                           \
  func(var), EXPAND_7(func, __VA_ARGS__)
#define EXPAND_9(func, var, ...)                           \
  func(var), EXPAND_8(func, __VA_ARGS__)
#define EXPAND_10(func, var, ...)                          \
  func(var), EXPAND_9(func, __VA_ARGS__)
#define EXPAND_11(func, var, ...)                          \
  func(var), EXPAND_10(func, __VA_ARGS__)
#define EXPAND_12(func, var, ...)                          \
  func(var), EXPAND_11(func, __VA_ARGS__)
#define EXPAND_13(func, var, ...)                          \
  func(var), EXPAND_12(func, __VA_ARGS__)
#define EXPAND_14(func, var, ...)                          \
  func(var), EXPAND_13(func, __VA_ARGS__)
#define EXPAND_15(func, var, ...)                          \
  func(var), EXPAND_14(func, __VA_ARGS__)
#define EXPAND_16(func, var, ...)                          \
  func(var), EXPAND_15(func, __VA_ARGS__)

/* --- 参数选择器 (用于选择 EXPAND_N) --- */
#define SELECT(_1,                                         \
               _2,                                         \
               _3,                                         \
               _4,                                         \
               _5,                                         \
               _6,                                         \
               _7,                                         \
               _8,                                         \
               _9,                                         \
               _10,                                        \
               _11,                                        \
               _12,                                        \
               _13,                                        \
               _14,                                        \
               _15,                                        \
               _16,                                        \
               name,                                       \
               ...)                                        \
  name

/**
 * @brief (内部) 将 `func` (即 `TYPE_INFO`) 应用于 `...`
 * 中的每个参数。
 */
#define EXPAND_ALL(func, ...)                              \
  __VA_OPT__(SELECT(__VA_ARGS__,                           \
                    EXPAND_16,                             \
                    EXPAND_15,                             \
                    EXPAND_14,                             \
                    EXPAND_13,                             \
                    EXPAND_12,                             \
                    EXPAND_11,                             \
                    EXPAND_10,                             \
                    EXPAND_9,                              \
                    EXPAND_8,                              \
                    EXPAND_7,                              \
                    EXPAND_6,                              \
                    EXPAND_5,                              \
                    EXPAND_4,                              \
                    EXPAND_3,                              \
                    EXPAND_2,                              \
                    EXPAND_1)(func, __VA_ARGS__))

/* --- 参数计数器 --- */
#define COUNT_ARGS(...)                                    \
  __VA_OPT__(SELECT(__VA_ARGS__,                           \
                    16,                                    \
                    15,                                    \
                    14,                                    \
                    13,                                    \
                    12,                                    \
                    11,                                    \
                    10,                                    \
                    9,                                     \
                    8,                                     \
                    7,                                     \
                    6,                                     \
                    5,                                     \
                    4,                                     \
                    3,                                     \
                    2,                                     \
                    1))

/**
 * @brief (内部) 计算 `...` 中的参数数量, 如果为空则为 0。
 */
#define ARGS_COUNT(...)                                    \
  (0 __VA_OPT__(+COUNT_ARGS(__VA_ARGS__)))

/*
 * ===================================================================
 * 5. 公共 API
 * ===================================================================
 */

/*
 * 定义 sink 函数指针的类型别名, 以便 vformat_func
 * 签名更清晰
 */
typedef void (*sink_char_fn)(void *sink, char c);
typedef void (*sink_bytes_fn)(void *sink,
                              const char *bytes,
                              usize len);

/**
 * @brief (内部) C 可变参数函数 (vformat_func)
 *
 * 接收 sink (void*), 两个函数指针, 和格式化参数。
 */
void vformat_func(void *sink,
                  sink_char_fn push_char,
                  sink_bytes_fn push_bytes,
                  const char *fmt,
                  int count,
                  ...);
