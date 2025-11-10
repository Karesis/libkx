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

/* include/std/mem/allocer/bump.h */
#pragma once

/*
 * ===================================================================
 * 1. 依赖 (现在全部是 'core' 或 L3 'std')
 * ===================================================================
 */

#include <core/mem/allocer.h> // L1 Trait (ALLOC, REALLOC, ...)
#include <core/mem/layout.h> // L1 Layout
#include <core/msg/panic.h>  // L3 Panic
#include <core/option.h>     // L1 Option
#include <core/type.h>       // L0 Types (usize, anyptr)
#include <string.h>          // L0 memset (用于 ZALLOC)

// 我们需要 SystemAlloc 的 *类型*，作为 bump_new 的参数
#include <core/mem/sysalc.h>

/*
 * ===================================================================
 * 2. 核心类型定义 (不透明类型)
 * ===================================================================
 */

/**
 * @brief ChunkFooter (内部实现)
 * (这是 bump.h 中 'typedef struct Bump Bump' 的私有定义)
 */
typedef struct ChunkFooter ChunkFooter;
struct ChunkFooter
{
  byte *data;
  usize chunk_size;
  ChunkFooter *prev;
  byte *ptr;
  usize allocated_bytes;
};

/**
 * @brief Bump (内部实现)
 */
typedef struct Bump Bump;
struct Bump
{
  ChunkFooter *current_chunk_footer;
  usize allocation_limit;
  usize min_align;
  /**
   * @brief 支撑分配器。
   * Bump 本身也需要内存，它使用这个分配器来获取 Chunk。
   */
  SystemAlloc *backing_alloc;
};

/**
 * @brief 指向 Bump 的指针 (用于 Option)。
 */
DEFINE_OPTION(BumpPtr, Bump *);

/*
 * ===================================================================
 * 3. 生命周期管理 (Lifecycle)
 * ===================================================================
 *
 * 这些是 bump.c 中实现的 "public" 函数。
 */

/**
 * @brief 在堆上创建一个新的 Bump Arena (最小对齐为 1)。
 * @param backing_alloc 用于分配 Bump 结构体和 Chunks
 * 的支撑分配器。
 * @return Some(BumpPtr) 成功时, None 失败时 (OOM)。
 */
Option_BumpPtr bump_new(SystemAlloc *backing_alloc);

/**
 * @brief 在堆上创建一个新的 Bump Arena (指定最小对齐)。
 */
Option_BumpPtr bump_new_min_align(
  SystemAlloc *backing_alloc, usize min_align);

/**
 * @brief 初始化一个已分配的 Bump 结构 (例如在栈上)。
 * @param backing_alloc 用于分配 Chunks 的支撑分配器。
 */
void bump_init(Bump *self, SystemAlloc *backing_alloc);

/**
 * @brief 初始化一个已分配的 Bump 结构 (指定最小对齐)。
 */
void bump_init_min_align(Bump *self,
                         SystemAlloc *backing_alloc,
                         usize min_align);

/**
 * @brief 销毁 Arena，释放其分配的所有内存块。
 * @note 这 *不会* 释放 self 结构体本身。
 */
void bump_destroy(Bump *self);

/**
 * @brief 销毁 Arena 并释放 Bump 结构体本身。
 * @note 只能用于由 bump_new() 创建的 Arena。
 */
void bump_free(Bump *self);

/**
 * @brief 重置 Arena。释放所有 Chunk 并重置指针。
 */
void bump_reset(Bump *self);

/**
 * @brief (扩展 API) 设置分配限制。
 */
void bump_set_allocation_limit(Bump *self, usize limit);

/**
 * @brief (扩展 API) 获取已分配的总字节数。
 */
usize bump_get_allocated_bytes(const Bump *self);

/*
 * ===================================================================
 * 4. [私有] 核心实现函数 (Internal Prototypes)
 * ===================================================================
 *
 * 这些是在 bump.c 中定义的函数，由下面的契约宏调用。
 * 它们返回 Option，宏则负责 panic。
 */
Option_anyptr bump_alloc_impl(Bump *self, Layout layout);
Option_anyptr bump_realloc_impl(Bump *self,
                                anyptr old_ptr,
                                Layout old_layout,
                                Layout new_layout);
void bump_release_impl(Bump *self,
                       anyptr ptr,
                       Layout layout);

/*
 * ===================================================================
 * 5. [公共] 分配器宏契约 (The Trait Impl)
 * ===================================================================
 */

/* --- 核心 Trait Impl --- */

#define BUMP_ALLOC(self_ptr, layout)                       \
  ({                                                       \
    Option_anyptr __opt =                                  \
      bump_alloc_impl(self_ptr, layout);                   \
    if (__opt.kind == NONE)                                \
    {                                                      \
      panic("Bump allocation failed");                     \
    }                                                      \
    __opt.value.some;                                      \
  })

#define BUMP_REALLOC(                                      \
  self_ptr, old_ptr, old_layout, new_layout)               \
  ({                                                       \
    Option_anyptr __opt = bump_realloc_impl(               \
      self_ptr, old_ptr, old_layout, new_layout);          \
    if (__opt.kind == NONE)                                \
    {                                                      \
      panic("Bump reallocation failed");                   \
    }                                                      \
    __opt.value.some;                                      \
  })

#define BUMP_RELEASE(self_ptr, ptr, layout)                \
  bump_release_impl(self_ptr, ptr, layout) /* (no-op) */

#define BUMP_ZALLOC(self_ptr, layout)                      \
  ({                                                       \
    anyptr __ptr = BUMP_ALLOC(self_ptr, layout);           \
    memset(__ptr, 0, (layout).size);                       \
    __ptr;                                                 \
  })

/* --- 扩展 Trait Impl --- */

#define BUMP_RESET(self_ptr) bump_reset(self_ptr)

#define BUMP_SET_LIMIT(self_ptr, limit)                    \
  bump_set_allocation_limit(self_ptr, limit)

#define BUMP_GET_ALLOCATED(self_ptr)                       \
  bump_get_allocated_bytes(self_ptr)
