#pragma once

#include <core/mem/layout.h>
#include <core/type/ptr.h>
#include <core/type/size.h>

/*
 * ===================================================================
 * 1. Allocator Trait (分配器契约)
 * ===================================================================
 *
 * 任何 Allocator (Impl) 都 *必须* 提供核心契约 (Core
 * Contract)。 它可以 *可选* 提供扩展契约 (Extended
 * Contract)。
 *
 * --- 核心契约 (Core Contract) ---
 * (消费者如 Vector 依赖这些)
 *
 * - PREFIX_ALLOC(self_ptr, layout) -> anyptr (Panic on OOM)
 * - PREFIX_REALLOC(self_ptr, old_ptr, old_layout,
 * new_layout) -> anyptr (Panic on OOM)
 * - PREFIX_RELEASE(self_ptr, ptr, layout) -> void
 * - PREFIX_ZALLOC(self_ptr, layout) -> anyptr (Panic on
 * OOM)
 *
 * --- 扩展契约 (Extended Contract) ---
 * (高级用户或特定容器可能依赖这些)
 *
 * - PREFIX_RESET(self_ptr) -> void
 * - PREFIX_SET_LIMIT(self_ptr, limit) -> void
 * - PREFIX_GET_ALLOCATED(self_ptr) -> usize
 */

/*
 * ===================================================================
 * 2. Trait "Dispatcher" (静态分发器)
 * ===================================================================
 */

#define __ALLOC_CONCAT_IMPL(a, b) a##b
#define __ALLOC_CONCAT(a, b) __ALLOC_CONCAT_IMPL(a, b)

/* --- 核心 Trait 分发器 --- */

/**
 * @brief (Trait API) 静态分发到 PREFIX_ALLOC
 */
#define ALLOC(Prefix, self_ptr, layout)                    \
  __ALLOC_CONCAT(Prefix, _ALLOC)((self_ptr), (layout))

/**
 * @brief (Trait API) 静态分发到 PREFIX_REALLOC
 */
#define REALLOC(                                           \
  Prefix, self_ptr, old_ptr, old_layout, new_layout)       \
  __ALLOC_CONCAT(Prefix, _REALLOC)(                        \
    (self_ptr), (old_ptr), (old_layout), (new_layout))

/**
 * @brief (Trait API) 静态分发到 PREFIX_RELEASE
 */
#define RELEASE(Prefix, self_ptr, ptr, layout)             \
  __ALLOC_CONCAT(Prefix,                                   \
                 _RELEASE)((self_ptr), (ptr), (layout))

/**
 * @brief (Trait API) 静态分发到 PREFIX_ZALLOC
 * (分配清零的内存)
 */
#define ZALLOC(Prefix, self_ptr, layout)                   \
  __ALLOC_CONCAT(Prefix, _ZALLOC)((self_ptr), (layout))

/* --- 扩展 Trait 分发器 --- */

/**
 * @brief (Trait API) 静态分发到 PREFIX_RESET
 */
#define ALLOC_RESET(Prefix, self_ptr)                      \
  __ALLOC_CONCAT(Prefix, _RESET)((self_ptr))

/**
 * @brief (Trait API) 静态分发到 PREFIX_SET_LIMIT
 */
#define ALLOC_SET_LIMIT(Prefix, self_ptr, limit)           \
  __ALLOC_CONCAT(Prefix, _SET_LIMIT)((self_ptr), (limit))

/**
 * @brief (Trait API) 静态分发到 PREFIX_GET_ALLOCATED
 */
#define ALLOC_GET_ALLOCATED(Prefix, self_ptr)              \
  __ALLOC_CONCAT(Prefix, _GET_ALLOCATED)((self_ptr))
