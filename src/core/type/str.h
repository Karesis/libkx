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

#pragma once

#include <assert.h>
#include <core/math/ordering.h>
#include <core/type/size.h>
#include <stdbool.h>
#include <string.h>

typedef const char *rawstr;
typedef const char *str;

/**
 * @brief 获取字符串的长度 (不包括 '\0')。
 * @panic 如果 self 为 NULL，触发 assert。
 */
static inline usize
str_len(str self)
{
  assert(self != NULL && "str_len() received a NULL pointer");

  usize len = 0;
  while (self[len] != '\0')
  {
    len++;
  }
  return len;
}

/**
 * @brief 比较两个 str, 返回统一的 Ordering (LESS, EQUAL,
 * GREATER)
 * @panic 如果 s1 或 s2 为 NULL，触发 assert。
 */
static inline Ordering
str_cmp(str s1, str s2)
{
  assert(s1 != NULL && "str_cmp() received NULL for s1");
  assert(s2 != NULL && "str_cmp() received NULL for s2");

  while (*s1 != '\0' && *s2 != '\0')
  {
    if (*s1 < *s2)
    {
      return LESS;
    }
    if (*s1 > *s2)
    {
      return GREATER;
    }
    s1++;
    s2++;
  }

  // 检查是否一个字符串是另一个的前缀
  if (*s1 == '\0' && *s2 != '\0')
  {
    return LESS; // s1 比较短
  }
  if (*s1 != '\0' && *s2 == '\0')
  {
    return GREATER; // s2 比较短
  }

  return EQUAL;
}

/**
 * @brief 检查字符串 self 是否以 prefix 开头。
 * @panic 如果 self 或 prefix 为 NULL，触发 assert。
 */
static inline bool
str_starts_with(str self, str prefix)
{
  assert(self != NULL && "str_starts_with() received NULL for self");
  assert(prefix != NULL && "str_starts_with() received NULL for prefix");

  while (*prefix != '\0')
  {
    if (*self == '\0' || *self != *prefix)
    {
      return false;
    }
    self++;
    prefix++;
  }

  return true;
}

/**
 * @brief 检查字符串 self 是否以 suffix 结尾。
 * @panic 如果 self 或 suffix 为 NULL，触发 assert。
 */
static inline bool
str_ends_with(str self, str suffix)
{
  assert(self != NULL && "str_ends_with() received NULL for self");
  assert(suffix != NULL && "str_ends_with() received NULL for suffix");

  usize self_len = str_len(self);
  usize suffix_len = str_len(suffix);

  if (suffix_len == 0)
  {
    return true;
  }

  if (self_len < suffix_len)
  {
    return false;
  }

  str self_start_point = self + (self_len - suffix_len);

  while (*suffix != '\0')
  {
    if (*self_start_point != *suffix)
    {
      return false;
    }
    self_start_point++;
    suffix++;
  }

  return true;
}

/**
 * @brief 在 self 中查找子字符串 needle。
 * @panic 如果 self 或 needle 为 NULL，触发 assert。
 * @note strstr() 的行为是，如果 needle 是空字符串
 * ("")，它返回 self。
 */
static inline str
str_find(str self, str needle)
{
  assert(self != NULL && "str_find() received NULL for self");
  assert(needle != NULL && "str_find() received NULL for needle");
  return (str)strstr(self, needle);
}

/*
 * ===================================================================
 * vstr (String View / Slice)
 * ===================================================================
 */

/**
 * @brief (vstr) 字符串视图 (Slice)，一个非拥有的 (ptr, len) 对。
 *
 * 'ptr' 不保证以 '\0' 结尾。
 */
typedef struct vstr
{
  const char *ptr;
  usize len;
} vstr;

/**
 * @brief (vstr) 构造器：从指针和长度创建一个 vstr。
 * @panic 如果 ptr 为 NULL (除非 len 也为 0)。
 */
static inline vstr
vstr_new(const char *ptr, usize len)
{
  assert(ptr != NULL || len == 0);
  return (vstr){.ptr = ptr, .len = len};
}

/**
 * @brief (vstr) 构造器：从一个以 '\0' 结尾的 str 创建一个 vstr。
 * @panic 如果 s 为 NULL。
 */
static inline vstr
vstr_from_str(str s)
{
  assert(s != NULL && "vstr_from_str received NULL");
  return (vstr){.ptr = s, .len = str_len(s)};
}

/**
 * @brief (vstr) 比较两个 vstr。
 */
static inline Ordering
vstr_cmp(vstr v1, vstr v2)
{
  // 1. 确定最小比较长度
  usize min_len = (v1.len < v2.len) ? v1.len : v2.len;

  // 2. 比较公共部分
  if (min_len > 0)
  {
    int cmp = memcmp(v1.ptr, v2.ptr, min_len);
    if (cmp < 0)
    {
      return LESS;
    }
    if (cmp > 0)
    {
      return GREATER;
    }
  }

  // 3. 公共部分相同，比较长度
  if (v1.len < v2.len)
  {
    return LESS; // v1 是 v2 的前缀
  }
  if (v1.len > v2.len)
  {
    return GREATER; // v2 是 v1 的前缀
  }

  return EQUAL;
}

/**
 * @brief (vstr) 检查 vstr v 是否以 prefix 开头。
 */
static inline bool
vstr_starts_with(vstr v, vstr prefix)
{
  if (prefix.len > v.len)
  {
    return false; // 前缀比字符串长
  }
  if (prefix.len == 0)
  {
    return true; // 空前缀总是匹配
  }

  // 比较 v 的前 prefix.len 个字节
  return memcmp(v.ptr, prefix.ptr, prefix.len) == 0;
}

/**
 * @brief (vstr) 检查 vstr v 是否以 suffix 结尾。
 */
static inline bool
vstr_ends_with(vstr v, vstr suffix)
{
  if (suffix.len > v.len)
  {
    return false; // 后缀比字符串长
  }
  if (suffix.len == 0)
  {
    return true; // 空后缀总是匹配
  }

  // 比较 v 的后 suffix.len 个字节
  const char *start_ptr = v.ptr + (v.len - suffix.len);
  return memcmp(start_ptr, suffix.ptr, suffix.len) == 0;
}

/**
 * @brief (vstr) 比较 vstr 和 str 是否相等。
 *
 * 这是一个常见的辅助函数，用于比较一个切片和一个
 * '\0' 结尾的字符串。
 * @panic 如果 s 为 NULL。
 */
static inline bool
vstr_equals_str(vstr v, str s)
{
  assert(s != NULL && "vstr_equals_str received NULL for s");

  usize s_len = str_len(s);
  if (v.len != s_len)
  {
    return false; // 长度不同
  }
  if (v.len == 0)
  {
    return true; // 两者都是空的
  }

  return memcmp(v.ptr, s, v.len) == 0;
}

/**
 * @brief (vstr) 在 vstr v 中查找子视图 needle。
 *
 * @return 如果找到，返回指向 v.ptr 内部子字符串开头的指针；
 * 如果未找到，返回 NULL。
 */
static inline const char *
vstr_find(vstr v, vstr needle)
{
  if (needle.len == 0)
  {
    return v.ptr; // 空字符串总是能找到
  }
  if (needle.len > v.len)
  {
    return NULL; // needle 比 v 长
  }

  // (KMP 的简单替代方案：Boyer-Moore-Horspool 或 朴素 memcmp)
  // 我们使用朴素的滑动窗口 memcmp

  // 最大的起始索引
  usize max_start = v.len - needle.len;

  for (usize i = 0; i <= max_start; i++)
  {
    // 检查 v[i..i+needle.len] 是否等于 needle
    if (memcmp(v.ptr + i, needle.ptr, needle.len) == 0)
    {
      return v.ptr + i; // 找到了
    }
  }

  return NULL; // 未找到
}
