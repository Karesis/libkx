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


/*
 * Copyright 2025 Karesis
 * (Apache License Header - 已移植到 libkx)
 */

#pragma once

#include <core/type.h> // 引入: usize

/*
 * ===================================================================
 * 1. 范围 (Range)
 * ===================================================================
 */

/**
 * @brief (结构体) 表示一个半开半闭区间 [start, end)
 */
typedef struct Range
{
  usize start;
  usize end;
} Range;

/**
 * @brief (构造函数) 创建一个 Range 结构体
 *
 * @note 如果 start > end, 结果会是一个空范围 (start ==
 * end)。
 */
static inline Range
range(usize start, usize end)
{
  return (Range){.start = start,
                 .end = (start > end ? start : end)};
}

/*
 * ===================================================================
 * 2. 迭代宏
 * ===================================================================
 */

/**
 * @brief (宏) 遍历一个字面量范围 [start, end)
 *
 * @example
 * for_range(i, 0, 10) { // i 将从 0 到 9
 * dbg("i = {}", i);
 * }
 */
#define for_range(var, start, end)                         \
  for (usize var = (start); var < (end); ++var)

/**
 * @brief (宏) 遍历一个 Range 结构体
 *
 * @example
 * Range r = range(5, 10);
 * for_range_in(i, r) { // i 将从 5 到 9
 * dbg("i = {}", i);
 * }
 */
#define for_range_in(var, range_obj)                       \
  for (usize var = (range_obj).start;                      \
       var < (range_obj).end;                              \
       ++var)
