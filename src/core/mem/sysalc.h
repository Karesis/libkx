/* include/std/mem/allocer/system.h */
#pragma once

#include <core/mem/allocer.h> // L1 Trait
#include <core/msg/panic.h>   // L3 panic
#include <core/option.h>      // 引入 Option, Some, None
#include <core/type.h> // 引入 usize, (和我们刚加的 anyptr)
#include <stdlib.h>    // 引入 C 标准库的 malloc, free 等

/* 1. Impl 的具体类型 (ZST - 零大小类型) */
typedef struct SystemAlloc
{
} SystemAlloc;

/**
 * @brief 包装 C 的 malloc，返回一个 Option。
 *
 * @param size 请求的字节数。
 * @return Some(ptr) 成功时，
 * None(anyptr) 失败时 (OOM)。
 */
static inline Option_anyptr
sys_malloc(usize size)
{
  void *ptr = malloc(size);
  if (ptr == NULL)
  {
    return None(anyptr);
  }
  return Some(anyptr, ptr);
}

/**
 * @brief 包装 C 的 calloc (分配并清零)，返回一个 Option。
 *
 * @param num 元素数量。
 * @param size 每个元素的大小。
 * @return Some(ptr) 成功时，
 * None(anyptr) 失败时 (OOM)。
 */
static inline Option_anyptr
sys_calloc(usize num, usize size)
{
  void *ptr = calloc(num, size);
  if (ptr == NULL)
  {
    return None(anyptr);
  }
  return Some(anyptr, ptr);
}

/**
 * @brief 包装 C 的 realloc，返回一个 Option。
 *
 * @param ptr 之前分配的指针 (如果是 NULL，等同于
 * sys_malloc)。
 * @param new_size 新的字节数。
 * @return Some(new_ptr) 成功时
 * (可能是新地址也可能是原地址)。 None(anyptr) 失败时
 * (OOM)。
 * @note 失败时 (返回 None)，*旧的 ptr 仍然有效且未被释放*。
 * 调用者必须处理这种情况。
 */
static inline Option_anyptr
sys_realloc(anyptr ptr, usize new_size)
{
  void *new_ptr = realloc(ptr, new_size);
  if (new_ptr == NULL && new_size > 0)
  { // realloc(ptr, 0) 返回 NULL 是合法的
    return None(anyptr);
  }
  return Some(anyptr, new_ptr);
}

/**
 * @brief 包装 C 的 free。
 *
 * @note C 的 free(NULL) 是安全且无操作的，
 * 所以这个包装器不需要额外的检查。
 */
static inline void
sys_free(anyptr ptr)
{
  free(ptr);
}

/**
 * @brief 包装 C11/C23 的 aligned_alloc，返回一个 Option。
 *
 * @param alignment 请求的对齐 (必须是 2 的幂)。
 * @param size 请求的字节数 (必须是 alignment 的倍数)。
 * @return Some(ptr) 成功时，
 * None(anyptr) 失败时 (OOM 或参数无效)。
 */
static inline Option_anyptr
sys_aligned_alloc(usize alignment, usize size)
{
  // aligned_alloc 的参数有严格要求
  // (在 layout.h 中我们已经有了 is_power_of_two)
  // assert(_is_power_of_two(alignment));
  // assert(size % alignment == 0);

  void *ptr = aligned_alloc(alignment, size);
  if (ptr == NULL)
  {
    return None(anyptr);
  }
  return Some(anyptr, ptr);
}

/* * 2. Impl 的契约实现
 * (注意：我们现在必须尊重 'layout.align')
 */

#define SYSTEM_ALLOC(self_ptr, layout)                     \
  ({                                                       \
    Option_anyptr __opt =                                  \
      sys_aligned_alloc((layout).align, (layout).size);    \
    if (__opt.kind == NONE)                                \
    {                                                      \
      panic("System alloc failed");                        \
    }                                                      \
    __opt.value.some;                                      \
  })

/* (sys_realloc 不支持对齐，这是一个问题，但我们暂时先用它)
 */
#define SYSTEM_REALLOC(                                    \
  self_ptr, old_ptr, old_layout, new_layout)               \
  ({                                                       \
    Option_anyptr __opt =                                  \
      sys_realloc((old_ptr), (new_layout).size);           \
    if (__opt.kind == NONE)                                \
    {                                                      \
      panic("System realloc failed");                      \
    }                                                      \
    __opt.value.some;                                      \
  })

#define SYSTEM_RELEASE(self_ptr, ptr, layout) sys_free(ptr)

#define SYSTEM_ZALLOC(self_ptr, layout)                    \
  ({                                                       \
    anyptr __ptr = SYSTEM_ALLOC(self_ptr, layout);         \
    memset(__ptr, 0, (layout).size);                       \
    __ptr;                                                 \
  })

/* --- 扩展 Trait (空操作/Panic) --- */
#define SYSTEM_RESET(self_ptr) ((void)0) /* 空操作 */

#define SYSTEM_SET_LIMIT(self_ptr, limit)                  \
  ((void)0) /* 空操作 */

#define SYSTEM_GET_ALLOCATED(self_ptr)                     \
  ((usize)0) /* 总是返回 0 */
