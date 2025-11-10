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

/* include/std/fmt/color.h */
#pragma once

/*
 * ===================================================================
 * 1. 依赖
 * ===================================================================
 */

#include <core/type.h> // 依赖: u8, str, usize
#include <stdio.h>     // 依赖: snprintf
#include <string.h>    // 依赖: strlen, memcpy

/*
 * ===================================================================
 * 2. 颜色类型定义
 * ===================================================================
 *
 * Note: 也可以考虑将 Color24 和 rgb 移至 'std/type/color.h'
 * 或 'std/type/cmm.h' 中。
 */

/**
 * @brief 24-bit (True Color) RGB 颜色。
 */
typedef struct color24_t
{
  u8 r;
  u8 g;
  u8 b;
} Color24;

/**
 * @brief 构造一个 Color24 结构体。
 */
#define rgb(r, g, b) ((Color24){(u8)(r), (u8)(g), (u8)(b)})

/**
 * @brief (内部) ANSI 重置码
 */
#define ANSI_RESET "\033[0m"

/*
 * ===================================================================
 * 3. 线程局部 (Thread-Local) 暂存缓冲区
 * ===================================================================
 *
 * C23 `thread_local` 保证每个线程有其独立的暂存缓冲区。
 * format() 会从这个缓冲区读取 'str' (TYPE_STR) 并将其
 * 推送到 Sink。
 *
 * 这是零分配 (No-malloc) 且线程安全的。
 */

/**
 * @brief 暂存缓冲区大小 (字节)
 *
 * (e.g., "\[38;2;255;255;255m" (19 字节) + \0)
 * 128 字节足够容纳多次连续的颜色调用。
 */
#define COLOR_SCRATCH_SIZE 128

/**
 * @brief (内部) 线程局部暂存缓冲区。
 */
static thread_local char g_color_scratch_buf[COLOR_SCRATCH_SIZE];

/**
 * @brief (内部) 缓冲区中当前的位置。
 */
static thread_local usize g_color_scratch_pos = 0;

/**
 * @brief (内部) 在暂存缓冲区中分配空间并返回指针
 *
 * 这不是一个真正的 "alloc"，它只是将 ANSI 码写入缓冲区
 * 并返回一个指向它的 `str`。
 */
static inline str
color_scratch_alloc(const char *ansi_code)
{
  usize len = strlen(ansi_code);

  // 检查是否会溢出。如果溢出, 简单地从头开始重置缓冲区。
  if (g_color_scratch_pos + len + 1 > COLOR_SCRATCH_SIZE)
  {
    g_color_scratch_pos = 0; // 重置
  }

  // 再次检查 (如果单个 ANSI 码 > 128, 这不太可能)
  if (len + 1 > COLOR_SCRATCH_SIZE)
  {
    return (str) "[COLOR_ERR]";
  }

  // 复制 ANSI 码到暂存区
  char *ptr = g_color_scratch_buf + g_color_scratch_pos;
  memcpy(ptr, ansi_code, len + 1); // +1 复制 '\0'

  // 推进位置
  g_color_scratch_pos += (len + 1);

  return (str)ptr;
}

/*
 * ===================================================================
 * 4. 公共 API
 * ===================================================================
 */

/**
 * @brief (公共 API) 返回一个指向 24-bit 前景色 (FG) ANSI
 * 码的 str。
 * @param c Color24 颜色。
 * @return `str` (const char*).
 * 该指针指向一个线程局部缓冲区, 在后续调用中可能会失效。
 * 应立即被 `format` 等函数使用。
 */
static inline str
fg(Color24 c)
{
  // 栈上临时缓冲区
  char temp_buf[32];
  snprintf(temp_buf, 32, "\033[38;2;%u;%u;%um", c.r, c.g, c.b);

  // "分配" 到暂存区并返回 `str`
  return color_scratch_alloc(temp_buf);
}

/**
 * @brief (公共 API) 返回一个指向 24-bit 背景色 (BG) ANSI
 * 码的 str。
 * @param c Color24 颜色。
 * @return `str` (const char*). 立即使用。
 */
static inline str
bg(Color24 c)
{
  char temp_buf[32];
  snprintf(temp_buf, 32, "\033[48;2;%u;%u;%um", c.r, c.g, c.b);
  return color_scratch_alloc(temp_buf);
}

/**
 * @brief (公共 API) 返回一个指向重置码的 str。
 * @return `str` (const char*).
 */
static inline str
reset(void)
{
  // `ANSI_RESET` 是一个静态字符串字面量,
  // 它不需要暂存缓冲区, 可以直接返回。
  return (str)ANSI_RESET;
}
