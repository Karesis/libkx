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
