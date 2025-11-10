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

/* src/std/fmt/format.c */

#include <core/fmt/vformat.h>
/* #include <std/io/sink.h> (不再需要) */
#include <core/type.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/*
 * 用于将数字/指针转换为字符串的临时缓冲区大小。
 * 64位整数(20位) + 浮点数(e.g., -1.234567e+300) +
 * 指针(0x...) 64 字节通常是安全的。
 */
#define TEMP_BUF_SIZE 64

/*
 * ===================================================================
 * 2. 核心格式化引擎 (私有)
 * ===================================================================
 */

/**
 * @brief 核心格式化引擎。
 *
 * 新签名: 接收 sink 和两个函数指针。
 */
static void
vformat_engine(void *sink,
               sink_char_fn push_c,  // 接收的函数指针
               sink_bytes_fn push_b, // 接收的函数指针
               const char *fmt,
               int count,
               va_list args)
{
  if (fmt == NULL || sink == NULL || push_c == NULL ||
      push_b == NULL)
  {
    return;
  }

  char temp_buf[TEMP_BUF_SIZE] = {0};
  int fmtlen = (int)strlen(fmt);
  int param_index = 0;

  for (int i = 0; i < fmtlen; i++)
  {
    bool match = false;
    // 仅匹配 "{}"
    if (fmt[i] == '{' && i + 1 < fmtlen &&
        fmt[i + 1] == '}')
    {
      i = i + 1; // 跳过 '}'
      match = true;
    }

    if (match)
    {
      // 匹配到 "{}", 尝试处理一个参数
      if (param_index >= count)
      {
        // 参数不足, 推送 "{}" 字面量
        push_c(sink, '{');
        push_c(sink, '}');
        continue;
      }

      param_index++;

      // 1. 从 va_list 读取 TYPE ID
      int type = va_arg(args, int);
      int len = 0;

      // 2. 根据 TYPE ID 读取值并推送到 Sink
      switch (type)
      {
      /* --- 字符串和字符 --- */
      case TYPE_STR:
      case TYPE_MUT_STR: {
        char *data = va_arg(args, char *);
        if (data == NULL)
        {
          data = "(null)"; // 安全处理
        }
        len = (int)strlen(data);
        push_b(sink, data, (usize)len);
        break;
      }
      case TYPE_CHAR: {
        // 'char' 被提升为 'int'
        int data = va_arg(args, int);
        push_c(sink, (char)data);
        break;
      }

      /* --- 有符号整数 --- */
      case TYPE_I8:
      case TYPE_I16:
      case TYPE_I32: {
        // 'i8', 'i16' 被提升为 'int'
        int data = va_arg(args, int);
        len = snprintf(temp_buf, TEMP_BUF_SIZE, "%d", data);
        push_b(sink, temp_buf, (usize)len);
        break;
      }
      case TYPE_I64: {
        i64 data = va_arg(args, i64);
        len = snprintf(
          temp_buf, TEMP_BUF_SIZE, "%" PRId64, data);
        push_b(sink, temp_buf, (usize)len);
        break;
      }

      /* --- 无符号整数 --- */
      case TYPE_U8:
      case TYPE_U16: {
        // 'u8', 'u16' 被提升为 'unsigned int'
        unsigned int data = va_arg(args, unsigned int);
        len = snprintf(temp_buf, TEMP_BUF_SIZE, "%u", data);
        push_b(sink, temp_buf, (usize)len);
        break;
      }
      case TYPE_U32: {
        u32 data = va_arg(args, u32);
        len = snprintf(temp_buf, TEMP_BUF_SIZE, "%u", data);
        push_b(sink, temp_buf, (usize)len);
        break;
      }
      case TYPE_U64: {
        u64 data = va_arg(args, u64);
        len = snprintf(
          temp_buf, TEMP_BUF_SIZE, "%" PRIu64, data);
        push_b(sink, temp_buf, (usize)len);
        break;
      }

      /* --- 浮点数 --- */
      case TYPE_FLOAT:
      case TYPE_DOUBLE: {
        // 'float' 被提升为 'double'
        double data = va_arg(args, double);
        len = snprintf(temp_buf, TEMP_BUF_SIZE, "%f", data);
        push_b(sink, temp_buf, (usize)len);
        break;
      }

      /* --- 指针 --- */
      case TYPE_ANY: // void*
      {
        void *data = va_arg(args, void *);
        len = snprintf(temp_buf, TEMP_BUF_SIZE, "%p", data);
        push_b(sink, temp_buf, (usize)len);
        break;
      }

      /* --- 错误处理 --- */
      case TYPE_NONE:
      default: {
        // 不支持的类型或 `default` 匹配
        // 尝试跳过这个未知参数。这有风险,
        // 因为不知道它的大小, 但 `void*` 通常是安全的。
        (void)va_arg(args, void *);
        const char *err_msg = "[?BAD_TYPE?]";
        push_b(sink, err_msg, strlen(err_msg));
        break;
      }
      } // end switch
    }
    else
    {
      // 非占位符, 只是一个普通字符
      push_c(sink, fmt[i]);
    }
  } // end for
}

/*
 * ===================================================================
 * 3. 公共 API 实现 (在 .h 中声明)
 * ===================================================================
 */

void
vformat_func(void *sink,
             sink_char_fn push_char,
             sink_bytes_fn push_bytes,
             const char *fmt,
             int count,
             ...)
{
  va_list args;
  va_start(args, count);

  // 将所有参数传递给核心引擎
  vformat_engine(
    sink, push_char, push_bytes, fmt, count, args);

  va_end(args);
}
