/*
 * Copyright (C) 2025 Karesis
 *
 * This file is part of libkx.
 *
 * libkx is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libkx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


/* include/std/debug/panic.h */
#pragma once

#include <core/color.h> // 依赖颜色库
#include <core/fmt/tofile.h>
#include <stdio.h>  // 依赖 stderr
#include <stdlib.h> // 依赖 abort()

/**
 * @brief (内部) 定义 panic 宏使用的默认颜色。
 *
 * (一个明亮的红色, 警告意味十足)
 */
#define PANIC_COLOR rgb(255, 80, 80)

/**
 * @brief (公共 API) 打印带有上下文的 panic 消息并终止程序。
 *
 * 这是一个 `println` 风格的 panic 宏, 它会自动:
 * 1. 打印到 `stderr`。
 * 2. 自动将整行输出着色 (使用 PANIC_COLOR)。
 * 3. 自动在末尾重置颜色。
 * 4. 添加 `[PANIC]` 标签。
 * 5. 自动包含 文件名 和 行号。
 * 6. 自动在末尾添加换行符 `\n`。
 * 7. 调用 `abort()` 终止程序 (这比 exit(1) 更好,
 * 因为它能被调试器捕获并生成 coredump)。
 *
 * @param fmt 格式化字符串, 使用 `{}` 作为占位符。
 * @param ... (可选) 要格式化的参数。
 *
 * @example
 * int *ptr = NULL;
 * if (ptr == NULL) {
 * panic("Pointer was null!");
 * }
 * // 输出: (in bright red) [PANIC] (src/main.c:10) Pointer
 * was null! (color reset)
 * // [程序终止]
 */
#define panic(fmt, ...)                                    \
  do                                                       \
  {                                                        \
    format_to_file(stderr,                                 \
                   "{}[PANIC] ({}:{}) " fmt "{}\n",        \
                                                           \
                   fg(PANIC_COLOR),                        \
                   __FILE__,                               \
                   __LINE__ __VA_OPT__(, ) __VA_ARGS__,    \
                   reset());                               \
    abort(); /* 终止程序 */                                \
  } while (0)
