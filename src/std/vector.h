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

/* include/std/coll/vector.h */
#pragma once

/*
 * ===================================================================
 * 1. 依赖 (全都是 Core)
 * ===================================================================
 */

#include <core/mem/allocer.h> // L1 Trait (ALLOC, REALLOC, ...)
#include <core/mem/layout.h>  // L1 Layout
#include <core/msg/panic.h>   // L3 Panic
#include <core/type.h>        // L0 Types (usize, byte, ...)

/*
 * ===================================================================
 * 2. 核心模板：DEFINE_VECTOR
 * ===================================================================
 *
 * 这是你的 "静态 C++ 模板"：
 * 它是一个宏，用于 "实例化" 一个与特定分配器
 * (由 AllocPrefix 标识) 绑定的具体 Vector 类型。
 *
 * @param TypeName  我们正在操作的类型 (例如: sstring,
 * MyVec_int)
 * @param T         向量中存储的元素类型 (例如: char, int)
 * @param AllocType 构造函数所需的分配器类型 (例如: Bump,
 * SystemAlloc)
 * @param AllocPrefix 用于静态分发 Trait 调用的前缀 (BUMP,
 * SYSTEM)
 */
#define DEFINE_VECTOR(TypeName, T, AllocType, AllocPrefix)                                         \
                                                                                                   \
  /* --- 1. 具体的 struct 类型 --- */                                                              \
                                                                                                   \
  typedef struct TypeName                                                                          \
  {                                                                                                \
    T *data;                                                                                       \
    usize len;                                                                                     \
    usize cap;                                                                                     \
    AllocType *alloc_state; /* <-- 具体的分配器指针 */                                             \
  } TypeName;                                                                                      \
                                                                                                   \
  /* --- 2. 核心 API (Lifecycle) --- */                                                            \
                                                                                                   \
  /**                                                                                              \
   * @brief 初始化一个 *已存在* 的 Vector (例如在栈上)。                            \
   */                                                                                              \
  static inline void TypeName##_init(TypeName *self, AllocType *alloc)                             \
  {                                                                                                \
    self->data = NULL;                                                                             \
    self->len = 0;                                                                                 \
    self->cap = 0;                                                                                 \
    self->alloc_state = alloc;                                                                     \
  }                                                                                                \
                                                                                                   \
  /**                                                                                              \
   * @brief 在堆上创建并初始化一个新的 Vector。                                      \
   * (使用 'alloc' 分配器来分配 *结构体自身*)                                         \
   */                                                                                              \
  static inline TypeName *TypeName##_new(AllocType *alloc)                                         \
  {                                                                                                \
    /* * 静态分发：ALLOC(SYSTEM, alloc, ...)                                                  \
     * 或 ALLOC(BUMP, alloc, ...)                                                                 \
     */                                                                                            \
    TypeName *self = (TypeName *)ALLOC(AllocPrefix, alloc, LAYOUT_OF(TypeName));                   \
    TypeName##_init(self, alloc);                                                                  \
    return self;                                                                                   \
  }                                                                                                \
                                                                                                   \
  /**                                                                                              \
   * @brief 销毁 Vector，释放其 *内部* 数据。                                           \
   * (不释放 self 结构体本身)                                                              \
   */                                                                                              \
  static inline void TypeName##_deinit(TypeName *self)                                             \
  {                                                                                                \
    if (self->data)                                                                                \
    {                                                                                              \
      /* * 静态分发：RELEASE(SYSTEM, ...) */                                                       \
      RELEASE(AllocPrefix, self->alloc_state, self->data, LAYOUT_OF_ARRAY(T, self->cap));          \
    }                                                                                              \
    self->data = NULL;                                                                             \
    self->len = 0;                                                                                 \
    self->cap = 0;                                                                                 \
  }                                                                                                \
                                                                                                   \
  /**                                                                                              \
   * @brief 销毁并释放一个在堆上创建的 Vector。                                      \
   */                                                                                              \
  static inline void TypeName##_destroy(TypeName *self)                                            \
  {                                                                                                \
    if (self == NULL)                                                                              \
      return;                                                                                      \
    TypeName##_deinit(self);                                                                       \
    /* * 静态分发：RELEASE(SYSTEM, ...) 释放结构体自身 */                                          \
    RELEASE(AllocPrefix, self->alloc_state, self, LAYOUT_OF(TypeName));                            \
  }                                                                                                \
                                                                                                   \
  /* --- 3. 核心 API (Capacity) --- */                                                             \
                                                                                                   \
  static inline void TypeName##_reserve_to(TypeName *self, usize new_cap)                          \
  {                                                                                                \
    if (new_cap <= self->cap)                                                                      \
      return;                                                                                      \
    Layout old_layout = LAYOUT_OF_ARRAY(T, self->cap);                                             \
    Layout new_layout = LAYOUT_OF_ARRAY(T, new_cap);                                               \
                                                                                                   \
    /* * 静态分发：REALLOC(SYSTEM, ...) */                                                         \
    self->data = (T *)REALLOC(AllocPrefix, self->alloc_state, self->data, old_layout, new_layout); \
    /* (REALLOC 宏在 OOM 时会 panic) */                                                            \
    self->cap = new_cap;                                                                           \
  }                                                                                                \
                                                                                                   \
  static inline void TypeName##_reserve_more(TypeName *self, usize additional)                     \
  {                                                                                                \
    if (self->len + additional <= self->cap)                                                       \
      return;                                                                                      \
    usize required_cap = self->len + additional;                                                   \
    usize new_cap = (self->cap == 0) ? 8 : (self->cap * 2);                                        \
    if (new_cap < required_cap)                                                                    \
    {                                                                                              \
      new_cap = required_cap;                                                                      \
    }                                                                                              \
    TypeName##_reserve_to(self, new_cap);                                                          \
  }                                                                                                \
                                                                                                   \
  /* --- 4. 核心 API (Access) --- */                                                               \
                                                                                                   \
  static inline void TypeName##_push(TypeName *self, T element)                                    \
  {                                                                                                \
    TypeName##_reserve_more(self, 1);                                                              \
    self->data[self->len] = element;                                                               \
    self->len++;                                                                                   \
  }                                                                                                \
                                                                                                   \
  static inline void TypeName##_clear(TypeName *self)                                              \
  {                                                                                                \
    self->len = 0;                                                                                 \
  }                                                                                                \
                                                                                                   \
  static inline usize TypeName##_len(const TypeName *self)                                         \
  {                                                                                                \
    return self->len;                                                                              \
  }                                                                                                \
                                                                                                   \
  static inline usize TypeName##_cap(const TypeName *self)                                         \
  {                                                                                                \
    return self->cap;                                                                              \
  }                                                                                                \
                                                                                                   \
  static inline T *TypeName##_as_ptr(TypeName *self)                                               \
  {                                                                                                \
    return self->data;                                                                             \
  }                                                                                                \
                                                                                                   \
  static inline const T *TypeName##_as_const_ptr(const TypeName *self)                             \
  {                                                                                                \
    return self->data;                                                                             \
  }
