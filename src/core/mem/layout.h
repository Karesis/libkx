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

#pragma once

#include <assert.h>
#include <core/type.h>
#include <stdalign.h>
/**
 * @brief 内存请求描述符 (Memory Request Descriptor)
 *
 * 描述了一个内存块的大小和对齐要求。
 * 这等同于 Rust 的 `std::alloc::Layout`。
 */
typedef struct Layout
{
  usize size;
  usize align;
} Layout;

/**
 * @brief 辅助函数：检查 align 是否是 2 的幂
 */
static inline bool
_is_power_of_two(usize n)
{
  return (n > 0) && ((n & (n - 1)) == 0);
}

/**
 * @brief 从显式的大小和对齐创建一个 Layout (Rust:
 * `Layout::from_size_align`)
 *
 * @param size 内存块的大小（字节）。
 * @param align 内存块的对齐（字节）。必须是 2 的幂。
 * @return kx_Layout 描述符。
 * @panic 如果 align 不是 2 的幂，触发 assert。
 */
static inline Layout
layout_from_size_align(usize size, usize align)
{
  assert(_is_power_of_two(align) &&
         "Layout alignment must be a power of two");
  return (Layout){.size = size, .align = align};
}

/**
 * @brief 宏：为单个类型 T 创建 Layout (Rust:
 * `Layout::new::<T>`)
 *
 * @param T 要为其创建布局的类型。
 * @return Layout 描述符。
 */
#define LAYOUT_OF(T)                                       \
  (layout_from_size_align(sizeof(T), alignof(T)))

/**
 * @brief 宏：为一个包含 N 个 T 元素的数组创建 Layout
 * (Rust: `Layout::array::<T>(N)`)
 *
 * @param T 元素类型。
 * @param N 元素数量。
 * @return Layout 描述符。
 */
#define LAYOUT_OF_ARRAY(T, N)                              \
  (layout_from_size_align(sizeof(T) * (N), alignof(T)))
