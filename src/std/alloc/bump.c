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

/*
 * ===================================================================
 * 1. 依赖 (Includes)
 * ===================================================================
 */

#include <std/alloc/bump.h> // L3 Impl Header (我们自己)

/*
 * 依赖的 "Impls" 和 "Traits"
 * 我们需要 system.h 来获取 SYSTEM_ALLOC/RELEASE 宏
 * (作为我们的支撑分配器)。
 */
#include <core/mem/allocer.h> // L1 Trait (用于 ALLOC 宏)
#include <core/mem/layout.h>  // L1 Layout
#include <core/mem/sysalc.h>  // L3 Backing Allocator

/*
 * 依赖的 "Core" 模块
 */
#include <core/msg/asrt.h> // L3 Assertions (asrt, asrt_msg)
#include <core/option.h>   // L1 Option (Some, None, .kind)
#include <core/type.h> // L0 Types (usize, byte, anyptr, ...)

/*
 * 依赖的 C 标准库
 */
#include <stdlib.h> // (SIZE_MAX)
#include <string.h> // (memset, memcpy)

/* --- 对齐和常量 --- */

static bool
is_power_of_two(usize n)
{
  return (n != 0) && ((n & (n - 1)) == 0);
}

static usize
round_up_to(usize n, usize divisor)
{
  asrt(is_power_of_two(divisor));
  return (n + divisor - 1) & ~(divisor - 1);
}

static uptr
round_down_to(uptr n, usize divisor)
{
  asrt(is_power_of_two(divisor));
  return n & ~(divisor - 1);
}

#define CHUNK_ALIGN 16
#define FOOTER_SIZE                                        \
  (round_up_to(sizeof(ChunkFooter), CHUNK_ALIGN))
#define DEFAULT_CHUNK_SIZE_WITHOUT_FOOTER                  \
  (4096 - FOOTER_SIZE)

/* --- 哨兵 (Sentinel) 空 Chunk --- */
static ChunkFooter EMPTY_CHUNK_SINGLETON;
static bool EMPTY_CHUNK_INITIALIZED = false;

static ChunkFooter *
get_empty_chunk()
{
  if (!EMPTY_CHUNK_INITIALIZED)
  {
    EMPTY_CHUNK_SINGLETON.data =
      (byte *)&EMPTY_CHUNK_SINGLETON;
    EMPTY_CHUNK_SINGLETON.chunk_size = 0;
    EMPTY_CHUNK_SINGLETON.prev = &EMPTY_CHUNK_SINGLETON;
    EMPTY_CHUNK_SINGLETON.ptr =
      (byte *)&EMPTY_CHUNK_SINGLETON;
    EMPTY_CHUNK_SINGLETON.allocated_bytes = 0;
    EMPTY_CHUNK_INITIALIZED = true;
  }
  return &EMPTY_CHUNK_SINGLETON;
}

static bool
chunk_is_empty(ChunkFooter *footer)
{
  return footer == get_empty_chunk();
}

/*
 * ===================================================================
 * 4. 内部 Chunk 管理 (Internal Chunk Management)
 * ===================================================================
 */

/**
 * @brief 释放一个 chunk 链表
 * @param bump 用来访问 backing_alloc
 * @param footer 要释放的链表的头部
 */
static void
dealloc_chunk_list(Bump *bump, ChunkFooter *footer)
{
  while (!chunk_is_empty(footer))
  {
    ChunkFooter *prev = footer->prev;

    // ** 关键改动: 使用 Backing Allocator **
    SYSTEM_RELEASE(bump->backing_alloc,
                   footer->data,
                   /* (我们必须重建 layout 来释放) */
                   layout_from_size_align(
                     footer->chunk_size, CHUNK_ALIGN));

    footer = prev;
  }
}

static ChunkFooter *
new_chunk(Bump *bump,
          usize new_size_without_footer,
          usize align,
          ChunkFooter *prev)
{
  new_size_without_footer =
    round_up_to(new_size_without_footer, CHUNK_ALIGN);

  usize alloc_size;
  if (__builtin_add_overflow(
        new_size_without_footer, FOOTER_SIZE, &alloc_size))
  {
    return NULL; // OOM
  }

  alloc_size = round_up_to(alloc_size, align);
  if (alloc_size == 0)
  {
    return NULL;
  }

  // ** 关键改动: 使用 Backing Allocator **
  // (我们使用 sys_aligned_alloc，它在 system.h 中返回
  // Option)
  Option_anyptr opt = sys_aligned_alloc(align, alloc_size);

  if (opt.kind == NONE)
  {
    return NULL; // OOM
  }
  byte *data = (byte *)opt.value.some;

  ChunkFooter *footer_ptr =
    (ChunkFooter *)(data + new_size_without_footer);

  footer_ptr->data = data;
  footer_ptr->chunk_size = alloc_size;
  footer_ptr->prev = prev;
  footer_ptr->allocated_bytes =
    prev->allocated_bytes + new_size_without_footer;

  uptr ptr_start = (uptr)footer_ptr;
  footer_ptr->ptr =
    (byte *)round_down_to(ptr_start, bump->min_align);

  asrt(footer_ptr->ptr >= footer_ptr->data);

  return footer_ptr;
}

/*
 * ===================================================================
 * 5. 分配路径 (Allocation Paths)
 * ===================================================================
 */

static anyptr
alloc_layout_slow(Bump *bump, Layout layout)
{
  ChunkFooter *current_footer = bump->current_chunk_footer;

  usize prev_usable_size = 0;
  if (!chunk_is_empty(current_footer))
  {
    prev_usable_size =
      current_footer->chunk_size - FOOTER_SIZE;
  }

  usize new_size_without_footer;
  if (__builtin_mul_overflow(
        prev_usable_size, 2, &new_size_without_footer))
  {
    new_size_without_footer = SIZE_MAX;
  }

  if (new_size_without_footer <
      DEFAULT_CHUNK_SIZE_WITHOUT_FOOTER)
  {
    new_size_without_footer =
      DEFAULT_CHUNK_SIZE_WITHOUT_FOOTER;
  }

  usize requested_align = (layout.align > bump->min_align)
                            ? layout.align
                            : bump->min_align;
  usize requested_size =
    round_up_to(layout.size, requested_align);

  if (new_size_without_footer < requested_size)
  {
    new_size_without_footer = requested_size;
  }

  if (bump->allocation_limit != SIZE_MAX)
  {
    usize allocated = current_footer->allocated_bytes;
    usize limit = bump->allocation_limit;
    usize remaining =
      (limit > allocated) ? (limit - allocated) : 0;

    if (new_size_without_footer > remaining)
    {
      if (requested_size > remaining)
      {
        return NULL; // OOM
      }
      new_size_without_footer = requested_size;
    }
  }

  usize chunk_align = (layout.align > CHUNK_ALIGN)
                        ? layout.align
                        : CHUNK_ALIGN;
  chunk_align = (chunk_align > bump->min_align)
                  ? chunk_align
                  : bump->min_align;

  ChunkFooter *new_footer =
    new_chunk(bump,
              new_size_without_footer,
              chunk_align,
              current_footer);
  if (!new_footer)
  {
    return NULL; // OOM
  }

  bump->current_chunk_footer = new_footer;

  ChunkFooter *footer = new_footer;
  byte *ptr = footer->ptr;
  byte *start = footer->data;
  byte *result_ptr;
  usize aligned_size;

  asrt(((uptr)ptr % bump->min_align) == 0);

  if (layout.align <= bump->min_align)
  {
    if (__builtin_add_overflow(
          layout.size, bump->min_align - 1, &aligned_size))
    {
      return NULL;
    }
    aligned_size = aligned_size & ~(bump->min_align - 1);
    usize capacity = (usize)(ptr - start);
    asrt_msg(aligned_size <= capacity,
             "New chunk too small!");
    result_ptr = ptr - aligned_size;
  }
  else
  {
    if (__builtin_add_overflow(
          layout.size, layout.align - 1, &aligned_size))
    {
      return NULL;
    }
    aligned_size = aligned_size & ~(layout.align - 1);
    byte *aligned_ptr_end =
      (byte *)round_down_to((uptr)ptr, layout.align);
    asrt_msg(aligned_ptr_end >= start,
             "New chunk alignment failed!");
    usize capacity = (usize)(aligned_ptr_end - start);
    asrt_msg(aligned_size <= capacity,
             "New chunk too small!");
    result_ptr = aligned_ptr_end - aligned_size;
  }

  asrt(((uptr)result_ptr % layout.align) == 0);
  asrt(result_ptr >= start);
  footer->ptr = result_ptr;
  return (anyptr)result_ptr;
}

static anyptr
try_alloc_layout_fast(Bump *bump, Layout layout)
{
  ChunkFooter *footer = bump->current_chunk_footer;
  byte *ptr = footer->ptr;
  byte *start = footer->data;
  usize min_align = bump->min_align;
  byte *result_ptr;
  usize aligned_size;

  asrt_msg((chunk_is_empty(footer) ||
            ((uptr)ptr % min_align) == 0),
           "Bump pointer invariant broken");

  if (layout.align <= min_align)
  {
    if (__builtin_add_overflow(
          layout.size, min_align - 1, &aligned_size))
    {
      return NULL;
    }
    aligned_size = aligned_size & ~(min_align - 1);
    usize capacity = (usize)(ptr - start);
    if (aligned_size > capacity)
    {
      return NULL;
    }
    result_ptr = ptr - aligned_size;
  }
  else
  {
    if (__builtin_add_overflow(
          layout.size, layout.align - 1, &aligned_size))
    {
      return NULL;
    }
    aligned_size = aligned_size & ~(layout.align - 1);
    byte *aligned_ptr_end =
      (byte *)round_down_to((uptr)ptr, layout.align);
    if (aligned_ptr_end < start)
    {
      return NULL;
    }
    usize capacity = (usize)(aligned_ptr_end - start);
    if (aligned_size > capacity)
    {
      return NULL;
    }
    result_ptr = aligned_ptr_end - aligned_size;
  }

  footer->ptr = result_ptr;
  return (anyptr)result_ptr;
}

/*
 * ===================================================================
 * 6. 公共 API 实现 (Public API Implementation)
 * ===================================================================
 */

/* --- 生命周期 --- */

void
bump_init_min_align(Bump *self,
                    SystemAlloc *backing_alloc,
                    usize min_align)
{
  asrt_msg(self != NULL, "Bump pointer cannot be NULL");
  asrt_msg(is_power_of_two(min_align),
           "min_align must be a power of two");
  asrt_msg(
    min_align <= CHUNK_ALIGN,
    "min_align cannot be larger than CHUNK_ALIGN (16)");

  self->current_chunk_footer = get_empty_chunk();
  self->allocation_limit = SIZE_MAX;
  self->min_align = min_align;
  self->backing_alloc =
    backing_alloc; // ** 关键: 保存支撑分配器 **
}

void
bump_init(Bump *self, SystemAlloc *backing_alloc)
{
  bump_init_min_align(self, backing_alloc, 1);
}

Option_BumpPtr
bump_new_min_align(SystemAlloc *backing_alloc,
                   usize min_align)
{
  // ** 关键改动: 使用 Backing Allocator 分配 Bump 自身 **
  // (我们使用 sys_malloc，它返回 Option)
  Option_anyptr opt = sys_malloc(sizeof(Bump));

  if (opt.kind == NONE)
  {
    return None(BumpPtr); // OOM
  }

  Bump *bump = (Bump *)opt.value.some;
  bump_init_min_align(bump, backing_alloc, min_align);
  return Some(BumpPtr, bump);
}

Option_BumpPtr
bump_new(SystemAlloc *backing_alloc)
{
  return bump_new_min_align(backing_alloc, 1);
}

void
bump_destroy(Bump *self)
{
  if (self)
  {
    dealloc_chunk_list(self, self->current_chunk_footer);
    self->current_chunk_footer = get_empty_chunk();
  }
}

void
bump_free(Bump *self)
{
  if (self)
  {
    SystemAlloc *backing_alloc = self->backing_alloc;
    bump_destroy(self);

    // ** 关键改动: 使用 Backing Allocator 释放 Bump 自身 **
    // (我们使用 sys_free，它在 system.h 中)
    sys_free(self);
  }
}

void
bump_reset(Bump *self)
{
  asrt_msg(self != NULL, "Bump 'self' cannot be NULL");
  ChunkFooter *current_footer = self->current_chunk_footer;
  if (chunk_is_empty(current_footer))
  {
    return;
  }

  // ** 关键改动: dealloc_chunk_list 现在需要 Bump* **
  // 我们只释放 'prev' 列表
  dealloc_chunk_list(self, current_footer->prev);

  // 重置 *当前* chunk
  current_footer->prev = get_empty_chunk();
  uptr ptr_start = (uptr)current_footer;
  current_footer->ptr =
    (byte *)round_down_to(ptr_start, self->min_align);
  usize usable_size =
    (usize)((byte *)current_footer - current_footer->data);
  current_footer->allocated_bytes = usable_size;
}

/* --- [私有] 核心实现函数 --- */

Option_anyptr
bump_alloc_impl(Bump *self, Layout layout)
{
  asrt_msg(self != NULL, "Bump 'self' cannot be NULL");

  if (layout.size == 0)
  {
    uptr ptr = (uptr)self->current_chunk_footer->ptr;
    return Some(anyptr,
                (anyptr)round_down_to(ptr, layout.align));
  }
  if (layout.align == 0 || !is_power_of_two(layout.align))
  {
    layout.align = 1; // 默认对齐
  }

  anyptr alloc = try_alloc_layout_fast(self, layout);
  if (alloc)
  {
    return Some(anyptr, alloc);
  }
  alloc = alloc_layout_slow(self, layout);
  if (alloc)
  {
    return Some(anyptr, alloc);
  }

  return None(anyptr);
}

Option_anyptr
bump_realloc_impl(Bump *self,
                  anyptr old_ptr,
                  Layout old_layout,
                  Layout new_layout) // <-- 签名已更新
{
  asrt_msg(self != NULL, "Bump 'self' cannot be NULL");

  if (old_ptr == NULL)
  {
    return bump_alloc_impl(self, new_layout);
  }

  if (new_layout.size == 0)
  {
    return Some(
      anyptr,
      (anyptr)round_down_to(
        (uptr)self->current_chunk_footer->ptr,
        new_layout.align)); // 使用 new_layout.align
  }

  // Arena realloc 总是 alloc + copy
  Option_anyptr new_opt = bump_alloc_impl(self, new_layout);
  if (new_opt.kind == NONE)
  {
    return None(anyptr); // OOM
  }

  anyptr new_ptr = new_opt.value.some;

  usize copy_size = (old_layout.size < new_layout.size)
                      ? old_layout.size
                      : new_layout.size;
  if (copy_size > 0)
  {
    memcpy(new_ptr, old_ptr, copy_size);
  }
  return Some(anyptr, new_ptr);
}

/* --- (扩展 API 实现) --- */
void
bump_set_allocation_limit(Bump *self, usize limit)
{
  asrt_msg(self != NULL, "Bump 'self' cannot be NULL");
  self->allocation_limit = limit;
}

usize
bump_get_allocated_bytes(const Bump *self)
{
  asrt_msg(self != NULL, "Bump 'self' cannot be NULL");
  return self->current_chunk_footer->allocated_bytes;
}

void
bump_release_impl(Bump *self, anyptr ptr, Layout layout)
{
  (void)self;
  (void)ptr;
  (void)layout;
}
