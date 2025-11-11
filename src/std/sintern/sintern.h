#pragma once

#include <core/mem/sysalc.h> // For SystemAlloc
#include <core/option.h>     // For Option
#include <core/type.h>
#include <std/alloc/bump.h> // For Bump
#include <std/hashmap.h>    // DEFINE_HASHMAP 宏

/*
 * ===================================================================
 * 1. 字符串驻留器 (Symbol Table)
 * ===================================================================
 *
 * SIntern 是一种用于编译器的数据结构，它为每个唯一的字符串
 * 只存储一个副本。
 *
 * 这使得比较两个标识符是否相等的操作，从 O(N) 的 strcmp
 * 变成了 O(1) 的指针比较。
 *
 * 它在内部使用一个 Bump (Arena) 分配器来存储字符串数据，
 * 并使用一个 HashMap 来快速查找现有的字符串。
 */

/*
 * ===================================================================
 * 1. vstr 键的 Hash/Compare 辅助函数
 * ===================================================================
 */

/**
 * @brief (Internal) vstr 键的哈希函数
 *
 * 为 SInternMap 的 K_Type (vstr) 提供哈希实现。
 */
static inline u64
hash_fn_vstr(const vstr *v)
{
  DefaultHasher h = DefaultHasher_new();
  // 使用 vstr 的 ptr 和 len 来计算哈希值
  // (DefaultHasher_write 最终调用 xxhash)
  DefaultHasher_write(&h, v->ptr, v->len);
  return DefaultHasher_finish(&h);
}

/**
 * @brief (Internal) vstr 键的比较函数
 *
 * 为 SInternMap 的 K_Type (vstr) 提供比较实现。
 */
static inline bool
cmp_fn_vstr(const vstr *a, const vstr *b)
{
  // 使用我们新添加的 vstr_cmp 核心函数
  return vstr_cmp(*a, *b) == EQUAL;
}

/*
 * ===================================================================
 * 1. 内部 HashMap 实例化
 * ===================================================================
 *
 * 定义 SInternMap 类型，这是 SIntern 的核心查找表。
 * - K_Type: str (const char*)
 * - V_Type: str (const char*) (指向 Arena 中的唯一副本)
 * - A_Type: SystemAlloc (用于 HashMap 自己的 'entries' 数组)
 */
DEFINE_HASHMAP(SInternMap, vstr, str, SystemAlloc, SYSTEM, hash_fn_vstr, cmp_fn_vstr)

/**
 * @brief 字符串驻留器 (Symbol Table) 结构体。
 */
typedef struct SIntern
{
  /** (L3) 存储实际字符串数据的 Arena */
  Bump *arena;

  /** (L4) 查找表: HashMap<str, str> */
  SInternMap *map;

  /** (L2) 用于 arena 和 map 内部结构的支撑分配器 */
  SystemAlloc *backing_alloc;

} SIntern;

/**
 * @brief Option 类型: Option<SIntern*>
 */
DEFINE_OPTION(SInternPtr, SIntern *);

/*
 * ===================================================================
 * 2. 公共 API (Public API)
 * ===================================================================
 */

/**
 * @brief 创建一个新的字符串驻留器。
 *
 * @param backing_alloc
 * 支撑分配器 (例如 SystemAlloc)，用于创建内部的 Arena 和
 * HashMap。
 * @return Some(SIntern*) 成功, None 失败 (OOM)。
 */
Option_SInternPtr sintern_new(SystemAlloc *backing_alloc);

/**
 * @brief 释放驻留器及其存储的所有字符串。
 *
 * 这将释放整个 Arena 和 HashMap，使所有之前返回的 'str' 指针失效。
 */
void sintern_free(SIntern *self);

/**
 * @brief 驻留一个以 '\0' 结尾的 C 字符串 (str)。
 *
 * 检查字符串 's' 是否已经被驻留。
 * - 如果是，返回指向 Arena 中**现有**副本的指针。
 * - 如果否，将 's' 复制到 Arena 中，并返回指向**新**副本的指针。
 *
 * @param self 驻留器实例。
 * @param s 要驻留的、以 '\0' 结尾的字符串。
 * @return 唯一的、指向 Arena 内部的 'str' 指针。
 * @panic 如果 Arena 分配失败 (OOM)。
 */
str sintern_intern(SIntern *self, str s);

/**
 * @brief 驻留一个非 '\0' 结尾的字节缓冲区。
 *
 * 这对于词法分析器特别有用，它可以直接驻留一个 (ptr, len)
 * 的词素 (lexeme)， 而无需先在栈上创建临时字符串。
 *
 * @param self 驻留器实例。
 * @param bytes 指向要驻留的数据的指针。
 * @param len 数据的长度 (in bytes)。
 * @return 唯一的、指向 Arena 内部的 'str' 指针 (保证以 '\0'
 * 结尾)。
 * @panic 如果 Arena 分配失败 (OOM) 或 len > 1024 (栈缓冲区保护)。
 */
str sintern_intern_bytes(SIntern *self, const char *bytes, usize len);
