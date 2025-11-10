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

/**
 * @file
 * @brief (Impl) 开放寻址、线性探测的哈希表宏模板。
 *
 * 实现了你设计的虚拟代码中的 HashMap。
 * - 算法: 开放寻址
 * - 探测: 线性探测
 * - 删除: 墓碑 (Tombstones)
 */
#include <std/hash/default.h> // 包含默认 Hasher 和 hash.h

#include <core/math/ordering.h> // EQUAL
#include <core/mem/allocer.h>   // 分配器 Trait
#include <core/mem/layout.h>    // 布局
#include <core/mem/sysalc.h>
#include <core/msg/asrt.h> // asrt!
#include <core/msg/dbg.h>
#include <core/option.h> // Option<T>
#include <core/result.h>
#include <core/type.h> // 基础类型

// --- 默认的 Hash/Compare 函数 ---
// 用户可以传递这些预置函数给 DEFINE_HASHMAP
// ------------------------------------

/** (Helper) u64 键的默认哈希函数 */
static inline u64
hash_fn_u64(const u64 *key)
{
  DefaultHasher h = DefaultHasher_new();
  hash(key, &h); // 静态分发
  return DefaultHasher_finish(&h);
}
/** (Helper) u64 键的默认比较函数 */
static inline bool
cmp_fn_u64(const u64 *a, const u64 *b)
{
  return *a == *b;
}

/** (Helper) str 键的默认哈希函数 */
static inline u64
hash_fn_str(const str *key)
{
  DefaultHasher h = DefaultHasher_new();
  hash(key, &h); // 静态分发
  return DefaultHasher_finish(&h);
}
/** (Helper) str 键的默认比较函数 */
static inline bool
cmp_fn_str(const str *a, const str *b)
{
  // --- 新增的 NULL 检查 ---
  // (假设两个 NULL 键是相等的)
  if (*a == NULL && *b == NULL)
  {
    return true;
  }
  // (假设 NULL 不等于任何非 NULL 键)
  if (*a == NULL || *b == NULL)
  {
    return false;
  }
  return str_cmp(*a, *b) == EQUAL;
}

// ------------------------------------
// ---    哈希表模板定义
// ------------------------------------

/**
 * @brief (Template) 定义一个 HashMap "类"。
 *
 * @param T_Name
 * 要生成的哈希表类型的名称 (例如: StrMap)。
 * @param K_Type
 * 键 (Key) 的类型 (例如: str)。
 * @param V_Type
 * 值 (Value) 的类型 (例如: u64)。
 * @param A_Type
 * 分配器 "Trait" 类型 (例如: SystemAlloc, Bump)。
 * @param FN_HASH
 * 一个函数签名为 `u64 (*)(const K_Type*)` 的哈希函数。
 * @param FN_CMP
 * 一个函数签名为 `bool (*)(const K_Type*, const K_Type*)`
 * 的比较函数。
 */
#define DEFINE_HASHMAP(T_Name,                              \
                       K_Type,                              \
                       V_Type,                              \
                       A_Type,                              \
                       A_Prefix,                            \
                       FN_HASH,                             \
                       FN_CMP)                              \
                                                            \
  /* 1. 内部结构体和枚举 (来自你的虚拟代码) */              \
                                                            \
  /** (Internal) 槽位状态 (EMPTY, OCCUPIED, DELETED) */     \
  typedef enum                                              \
  {                                                         \
    HM_STATE_EMPTY,                                         \
    HM_STATE_OCCUPIED,                                      \
    HM_STATE_DELETED /* 墓碑 */                             \
  } T_Name##_EntryState;                                    \
                                                            \
  /** (Internal) 哈希表槽位 (Entry) */                      \
  typedef struct                                            \
  {                                                         \
    K_Type key;                                             \
    V_Type value;                                           \
    T_Name##_EntryState state;                              \
  } T_Name##_Entry;                                         \
                                                            \
  /** (Public) HashMap 结构体本身 */                        \
  typedef struct                                            \
  {                                                         \
    T_Name##_Entry *entries;                                \
    usize capacity;                                         \
    usize count;                                            \
    A_Type *allocer; /* 指向分配器实例的指针 */             \
  } T_Name;                                                 \
                                                            \
  /* 2. 为 Option<V_Type> 和 Option<V_Type*> 生成定义 */    \
  /* 这使得 _get() 和 _get_ptr() 可以工作 */                \
  DEFINE_OPTION(T_Name##_V, V_Type);                        \
  DEFINE_OPTION(T_Name##_V_Ptr, V_Type *);                  \
                                                            \
  /* 3. 内部辅助函数 */                                     \
                                                            \
  /** (Internal) 负载因子 (Load Factor) */                  \
  static const f64 T_Name##_LOAD_FACTOR = 0.75;             \
  /** (Internal) 默认初始容量 */                            \
  static const usize T_Name##_DEFAULT_CAPACITY = 64;        \
                                                            \
  /** (Internal) 初始化槽位 (entries) 内存 */               \
  static inline void T_Name##_init_entries(                 \
    T_Name##_Entry *entries, usize capacity)                \
  {                                                         \
    for (usize i = 0; i < capacity; i++)                    \
    {                                                       \
      entries[i].state = HM_STATE_EMPTY;                    \
    }                                                       \
  }                                                         \
                                                            \
  /**                                                       \
   * (Internal) 核心函数: 查找槽位。               \
   * 完全复刻了你的 find_entry 逻辑。             \
   */                                                       \
  typedef struct                                            \
  {                                                         \
    usize index;                                            \
    bool found;                                             \
  } T_Name##_FindResult;                                    \
                                                            \
  static inline T_Name##_FindResult T_Name##_find_entry(    \
    T_Name##_Entry *entries,                                \
    usize capacity,                                         \
    const K_Type *key)                                      \
  {                                                         \
                                                            \
    if (capacity == 0)                                      \
    {                                                       \
      return (T_Name##_FindResult){0, false};               \
    }                                                       \
    u64 hash = FN_HASH(key);                                \
    usize base_index = (usize)(hash % (u64)capacity);       \
    usize first_tombstone =                                 \
      capacity; /* "哨兵"值, 表示未找到 */                  \
                                                            \
    for (usize i = 0; i < capacity; i++)                    \
    {                                                       \
      usize index = (base_index + i) % capacity;            \
      T_Name##_Entry *entry = &entries[index];              \
                                                            \
      switch (entry->state)                                 \
      {                                                     \
      case HM_STATE_EMPTY:                                  \
        /* 找到空槽, 搜索结束 */                            \
        if (first_tombstone != capacity)                    \
        {                                                   \
          /* 之前遇到过墓碑, 用那个墓碑槽 */                \
          return (T_Name##_FindResult){first_tombstone,     \
                                       false};              \
        }                                                   \
        else                                                \
        {                                                   \
          /* 没遇到墓碑, 用这个空槽 */                      \
          return (T_Name##_FindResult){index, false};       \
        }                                                   \
      case HM_STATE_OCCUPIED:                               \
        /* 槽被占用, 比较 key */                            \
        if (FN_CMP(&entry->key, key))                       \
        {                                                   \
          /* 找到了! */                                     \
          return (T_Name##_FindResult){index, true};        \
        }                                                   \
        /* Key 不匹配, 继续线性探测 */                      \
        break;                                              \
      case HM_STATE_DELETED:                                \
        /* 遇到墓碑 */                                      \
        if (first_tombstone == capacity)                    \
        {                                                   \
          /* 这是我们遇到的第一个墓碑, 记录它 */            \
          first_tombstone = index;                          \
        }                                                   \
        /* 继续探测 (key 可能在后面) */                     \
        break;                                              \
      }                                                     \
    }                                                       \
                                                            \
    /* 表已满 (或全是墓碑) 且未找到 key。 */                \
    /* 返回第一个墓碑的位置 (如果找到过) */                 \
    /* 或返回 'capacity' 作为"未找到插入槽"的信号 */        \
    return (T_Name##_FindResult){first_tombstone, false};   \
  }                                                         \
                                                            \
  /** (Internal) 声明 _resize 以便 _put 可以调用它 */       \
  static inline bool T_Name##_resize(T_Name *self);         \
                                                            \
  /** (Internal) 写入数据 (你的 write_at) */                \
  static inline void T_Name##_write_at(T_Name *self,        \
                                       usize index,         \
                                       K_Type key,          \
                                       V_Type value,        \
                                       bool is_new)         \
  {                                                         \
    if (is_new)                                             \
    {                                                       \
      asrt_msg(self->entries[index].state !=                \
                 HM_STATE_OCCUPIED,                         \
               "Writing new key to occupied slot");         \
      self->count++;                                        \
    }                                                       \
    else                                                    \
    {                                                       \
      asrt_msg(self->entries[index].state ==                \
                 HM_STATE_OCCUPIED,                         \
               "Updating value of non-occupied slot");      \
    }                                                       \
    self->entries[index].key = key;                         \
    self->entries[index].value = value;                     \
    self->entries[index].state = HM_STATE_OCCUPIED;         \
  }                                                         \
                                                            \
  /* 4. 公开 API (Public API) */                            \
                                                            \
  /** (Public) 创建一个新的 HashMap (你的 new) */           \
  static inline T_Name *T_Name##_new(A_Type *allocer)       \
  {                                                         \
    /* 1. 分配 HashMap 结构体本身 */                        \
    /* <<< FIX: 使用 ZALLOC 和 LAYOUT_OF 代替 _alloc_type \
     */                                                     \
    T_Name *self =                                          \
      ZALLOC(A_Prefix, allocer, LAYOUT_OF(T_Name));         \
    if (self == NULL)                                       \
    {                                                       \
      return NULL; /* OOM */                                \
    }                                                       \
                                                            \
    /* 2. 分配槽位 (entries) 数组 */                        \
    usize init_cap = T_Name##_DEFAULT_CAPACITY;             \
    /* <<< FIX: 使用 LAYOUT_OF_ARRAY 代替 layout_array */   \
    Layout layout =                                         \
      LAYOUT_OF_ARRAY(T_Name##_Entry, init_cap);            \
    /* <<< FIX: 使用 ALLOC Trait 代替 _alloc */             \
    void *ptr = ALLOC(A_Prefix, allocer, layout);           \
    if (ptr == NULL)                                        \
    {                                                       \
      /* <<< FIX: 使用 RELEASE 和 LAYOUT_OF 代替       \
       * _delete_type */                                    \
      RELEASE(A_Prefix, allocer, self, LAYOUT_OF(T_Name));  \
      return NULL; /* OOM */                                \
    }                                                       \
                                                            \
    self->entries = (T_Name##_Entry *)ptr;                  \
    self->capacity = init_cap;                              \
    self->count = 0;                                        \
    self->allocer = allocer;                                \
    T_Name##_init_entries(self->entries, init_cap);         \
                                                            \
    return self;                                            \
  }                                                         \
                                                            \
  /** (Public) 释放 HashMap */                              \
  static inline void T_Name##_free(T_Name *self)            \
  {                                                         \
    if (self == NULL)                                       \
    {                                                       \
      return;                                               \
    }                                                       \
    /* 1. 释放 entries 数组 */                              \
    /* <<< FIX: 使用 LAYOUT_OF_ARRAY 代替 layout_array */   \
    Layout layout = LAYOUT_OF_ARRAY(                        \
      sizeof(T_Name##_Entry), self->capacity);              \
    /* <<< FIX: 使用 RELEASE Trait 代替 _release */         \
    RELEASE(                                                \
      A_Prefix, self->allocer, self->entries, layout);      \
                                                            \
    /* 2. 释放 HashMap 结构体 */                            \
    /* <<< FIX: 使用 RELEASE 和 LAYOUT_OF 代替         \
     * _delete_type */                                      \
    RELEASE(                                                \
      A_Prefix, self->allocer, self, LAYOUT_OF(T_Name));    \
  }                                                         \
                                                            \
  /** (Public) 插入或更新一个键值对 (你的 put) */           \
  static inline void T_Name##_put(                          \
    T_Name *self, K_Type key, V_Type value)                 \
  {                                                         \
    T_Name##_FindResult res = T_Name##_find_entry(          \
      self->entries, self->capacity, &key);                 \
                                                            \
    if (res.found)                                          \
    {                                                       \
      /* Key 已存在, 更新 value */                          \
      T_Name##_write_at(                                    \
        self, res.index, key, value, false);                \
      return;                                               \
    }                                                       \
                                                            \
    /* Key 不存在, 检查是否需要 resize */                   \
    bool needs_resize =                                     \
      (res.index == self->capacity) ||                      \
      ((f64)(self->count + 1) >                             \
       (f64)self->capacity * T_Name##_LOAD_FACTOR);         \
                                                            \
    if (needs_resize)                                       \
    {                                                       \
      if (!T_Name##_resize(self))                           \
      {                                                     \
        asrt_msg(false, "HashMap resize failed (OOM)");     \
        return; /* 无法 resize (OOM) */                     \
      }                                                     \
      /* Resize 后, 必须重新查找槽位 */                     \
      res = T_Name##_find_entry(                            \
        self->entries, self->capacity, &key);               \
      asrt_msg(!res.found,                                  \
               "Key found immediately after resize");       \
      asrt_msg(res.index < self->capacity,                  \
               "No insert slot found after resize");        \
    }                                                       \
                                                            \
    /* 在新槽位插入 */                                      \
    T_Name##_write_at(self, res.index, key, value, true);   \
  }                                                         \
                                                            \
  /** (Internal) 扩容 (你的 resize) */                      \
  static inline bool T_Name##_resize(T_Name *self)          \
  {                                                         \
    T_Name##_Entry *old_entries = self->entries;            \
    usize old_capacity = self->capacity;                    \
    usize new_capacity = (old_capacity == 0)                \
                           ? T_Name##_DEFAULT_CAPACITY      \
                           : old_capacity * 2;              \
                                                            \
    /* <<< FIX: 使用 LAYOUT_OF_ARRAY 代替 layout_array */   \
    Layout new_layout = LAYOUT_OF_ARRAY(                    \
      sizeof(T_Name##_Entry), new_capacity);                \
    /* <<< FIX: 使用 ALLOC Trait 代替 _alloc */             \
    T_Name##_Entry *new_entries = (T_Name##_Entry *)ALLOC(  \
      A_Prefix, self->allocer, new_layout);                 \
    if (new_entries == NULL)                                \
    {                                                       \
      return false; /* OOM */                               \
    }                                                       \
                                                            \
    T_Name##_init_entries(new_entries, new_capacity);       \
                                                            \
    /* 更新 self 状态 (在新表上 re-hash) */                 \
    self->entries = new_entries;                            \
    self->capacity = new_capacity;                          \
    self->count = 0; /* _write_at 会把它加回来 */           \
                                                            \
    /* Re-hash: 遍历旧表, 把所有 OCCUPIED 的槽插入新表 */   \
    for (usize i = 0; i < old_capacity; i++)                \
    {                                                       \
      T_Name##_Entry *entry = &old_entries[i];              \
      if (entry->state == HM_STATE_OCCUPIED)                \
      {                                                     \
        /* 不能调用 _put, _put 会再次触发 resize! */        \
        T_Name##_FindResult res = T_Name##_find_entry(      \
          self->entries, self->capacity, &entry->key);      \
                                                            \
        asrt_msg(!res.found && res.index < self->capacity,  \
                 "Resize re-hash failed");                  \
                                                            \
        /* 直接写入 (true: is_new) */                       \
        T_Name##_write_at(self,                             \
                          res.index,                        \
                          entry->key,                       \
                          entry->value,                     \
                          true);                            \
      }                                                     \
    }                                                       \
                                                            \
    /* 释放旧表 (注意: 你的原代码这里是正确的!) */          \
    Layout old_layout = LAYOUT_OF_ARRAY(                    \
      sizeof(T_Name##_Entry), old_capacity);                \
    /* <<< FIX: 使用 RELEASE Trait 代替 _release */         \
    RELEASE(                                                \
      A_Prefix, self->allocer, old_entries, old_layout);    \
    return true;                                            \
  }                                                         \
                                                            \
  /** (Public) 获取 V 的指针 (你的 get_ptr) */              \
  /* <<< FIX: 返回类型应该是 Option_T_Name##_V_Ptr, */      \
  /* 而不是 T_Name##_V_Ptr (那只是个宏名字)。     */        \
  static inline Option_##T_Name##_V_Ptr T_Name##_get_ptr(   \
    T_Name *self, const K_Type key)                         \
  {                                                         \
    T_Name##_FindResult res = T_Name##_find_entry(          \
      self->entries, self->capacity, &key);                 \
    if (res.found)                                          \
    {                                                       \
      /* (这里的 Some(T_Name##_V_Ptr,...) 是正确的) */      \
      return Some(T_Name##_V_Ptr,                           \
                  &self->entries[res.index].value);         \
    }                                                       \
    else                                                    \
    {                                                       \
      return None(T_Name##_V_Ptr);                          \
    }                                                       \
  }                                                         \
                                                            \
  /** (Public) 获取 V (你的 get) */                         \
  static inline Option_##T_Name##_V T_Name##_get(           \
    T_Name *self, const K_Type key)                         \
  {                                                         \
    T_Name##_FindResult res = T_Name##_find_entry(          \
      self->entries, self->capacity, &key);                 \
    if (res.found)                                          \
    {                                                       \
      return Some(T_Name##_V,                               \
                  self->entries[res.index].value);          \
    }                                                       \
    else                                                    \
    {                                                       \
      return None(T_Name##_V);                              \
    }                                                       \
  }                                                         \
                                                            \
  /** (Public) 删除一个键 (你的 delete) */                  \
  static inline bool T_Name##_delete(T_Name *self,          \
                                     const K_Type key)      \
  {                                                         \
    T_Name##_FindResult res = T_Name##_find_entry(          \
      self->entries, self->capacity, &key);                 \
    if (!res.found)                                         \
    {                                                       \
      return false; /* 未找到 */                            \
    }                                                       \
                                                            \
    /* 找到了, 放置墓碑 */                                  \
    self->entries[res.index].state = HM_STATE_DELETED;      \
    self->count--;                                          \
    return true;                                            \
  }
