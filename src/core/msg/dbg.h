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

/* include/std/debug/dbg.h */
#pragma once

#include <core/color.h>
#include <core/fmt/tofile.h>
#include <stdio.h> // 依赖 stderr

/**
 * @brief (内部) 定义 dbg 宏使用的默认颜色。
 *
 * (一个明亮的青蓝色, 在暗色背景上非常易读)
 */
#define DBG_COLOR rgb(100, 210, 255)

/**
 * @brief (公共 API) 打印带有上下文的调试格式化消息。
 *
 * 这是一个 `println` 风格的调试宏, 它会自动:
 * 1. 打印到 `stderr`。
 * 2. [升级] 自动将整行输出着色 (使用 DBG_COLOR)。
 * 3. [升级] 自动在末尾重置颜色。
 * 4. 添加 `[DEBUG]` 标签。
 * 5. 自动包含 文件名 和 行号。
 * 6. 自动在末尾添加换行符 `\n`。
 *
 * @param fmt 格式化字符串, 使用 `{}` 作为占位符。
 * @param ... (可选) 要格式化的参数。
 *
 * @example
 * // 示例 1: 简单的消息
 * dbg("Boot process started.");
 * // 输出: (in bright cyan) [DEBUG] (src/main.c:10) Boot
 * process started. (color reset)
 *
 * // 示例 2: 带参数的消息
 * int user_id = 42;
 * dbg("Processing user: {}", user_id);
 * // 输出: (in bright cyan) [DEBUG] (src/main.c:11)
 * Processing user: 42 (color reset)
 */
#define dbg(fmt, ...)                                      \
  do                                                       \
  {                                                        \
    format_to_file(                                        \
      stderr, /* [升级] 1. 添加颜色 {占位符} 和 重置     \
         {占位符} */                                    \
      "{}[DEBUG] ({}:{}) " fmt "{}\n",                     \
                                                           \
      /* [升级] 2. 传入 fg() 和 reset() 作为参数 */        \
      fg(DBG_COLOR),                                       \
      __FILE__,                                            \
      __LINE__ __VA_OPT__(, ) __VA_ARGS__,                 \
      reset());                                            \
  } while (0)
