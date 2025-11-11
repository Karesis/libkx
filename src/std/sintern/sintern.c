#include <std/sintern/sintern.h>

// --- 包含实现所需的 libkx 模块 ---

#include <core/mem/layout.h> // For LAYOUT_OF, layout_of_array
#include <core/msg/panic.h>  // For panic!
#include <std/alloc/bump.h>  // Bump (Arena) 分配器
#include <string.h>          // For memcpy, str_len
/*
 * ===================================================================
 * 2. API 函数实现
 * ===================================================================
 */

Option_SInternPtr
sintern_new(SystemAlloc *backing_alloc)
{
  // 1. 分配 SIntern 结构体本身 (使用 ZALLOC 清零)
  SIntern *self = ZALLOC(SYSTEM, backing_alloc, LAYOUT_OF(SIntern));
  if (self == NULL)
  {
    return None(SInternPtr); // OOM
  }

  self->backing_alloc = backing_alloc;

  // 2. 创建 Bump (Arena) 分配器
  Option_BumpPtr arena_opt = bump_new(backing_alloc);
  if (ois_none(arena_opt))
  {
    // OOM, 释放 'self' 结构体
    RELEASE(SYSTEM, backing_alloc, self, LAYOUT_OF(SIntern));
    return None(SInternPtr);
  }
  self->arena = oexpect(arena_opt, "Failed to create Bump arena for SIntern");

  // 3. 创建 HashMap
  self->map = SInternMap_new(backing_alloc);
  if (self->map == NULL)
  {
    // OOM, 释放 arena 和 'self' 结构体
    bump_free(self->arena);
    RELEASE(SYSTEM, backing_alloc, self, LAYOUT_OF(SIntern));
    return None(SInternPtr);
  }

  return Some(SInternPtr, self);
}

void
sintern_free(SIntern *self)
{
  if (self == NULL)
  {
    return;
  }

  // 释放顺序与创建顺序相反
  // 1. 释放 HashMap (使用 backing_alloc)
  SInternMap_free(self->map);

  // 2. 释放 Bump Arena (使用 backing_alloc, 这会释放所有字符串)
  bump_free(self->arena);

  // 3. 释放 SIntern 结构体本身 (使用 backing_alloc)
  RELEASE(SYSTEM, self->backing_alloc, self, LAYOUT_OF(SIntern));
}

str
sintern_intern(SIntern *self, str s)
{
  // [!!] 将输入的 'str' 转换为 'vstr' 以进行查找
  vstr key_view = vstr_from_str(s);

  // 1. 检查 'vstr' 键是否已经存在
  Option_SInternMap_V val_opt = SInternMap_get(self->map, key_view);

  if (ois_some(val_opt))
  {
    // 找到了! 返回 Arena 中的 'str' 指针
    return oexpect(val_opt, "SInternMap_get failed unexpectedly after ois_some");
  }

  // 2. 未找到。将 's' 复制到 Arena 中。
  //    (我们使用 key_view.len 而不是重新计算 str_len(s))
  usize len = key_view.len;
  Layout layout = LAYOUT_OF_ARRAY(char, len + 1); // +1 for '\0'
  char *new_ptr = (char *)BUMP_ALLOC(self->arena, layout);

  // 3. 复制字符串内容
  memcpy(new_ptr, s, len);
  new_ptr[len] = '\0';

  // 4. [!!] 将 *新* 的 (ptr, len) 存入 Map
  //    Key: 指向 Arena 的 vstr
  //    Value: 指向 Arena 的 str
  vstr new_key_view = vstr_new(new_ptr, len);
  str new_val_str = (str)new_ptr;

  SInternMap_put(self->map, new_key_view, new_val_str);

  // 5. 返回指向 Arena 的新指针
  return new_val_str;
}

str
sintern_intern_bytes(SIntern *self, const char *bytes, usize len)
{
  // 从 (ptr, len) 创建一个临时的 vstr 键
  vstr key_view = vstr_new(bytes, len);

  // 使用 vstr 键查找
  Option_SInternMap_V val_opt = SInternMap_get(self->map, key_view);

  if (ois_some(val_opt))
  {
    // 找到了! 零拷贝查找成功。
    return oexpect(val_opt, "SInternMap_get (bytes) failed unexpectedly after ois_some");
  }

  // 2. 未找到。从 Arena 分配
  Layout layout = LAYOUT_OF_ARRAY(char, len + 1);
  char *new_ptr = (char *)BUMP_ALLOC(self->arena, layout);

  // 3. 复制数据
  memcpy(new_ptr, bytes, len);
  new_ptr[len] = '\0'; // 确保它在 Arena 中是 '\0' 结尾的

  // 4. 存入 Map
  //    Key: 指向 Arena 的 vstr
  //    Value: 指向 Arena 的 str
  vstr new_key_view = vstr_new(new_ptr, len);
  str new_val_str = (str)new_ptr;

  SInternMap_put(self->map, new_key_view, new_val_str);

  // 5. 返回新指针
  return new_val_str;
}
