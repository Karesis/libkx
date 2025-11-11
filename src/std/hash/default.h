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
 * @brief (Impl) 默认的 Hasher (DefaultHasher) 实现。
 *
 * 1. 使用 xxHash (XXH64) 作为哈希算法。
 * 2. 实现了 <core/hash/hasher.h> 定义的 Hasher Trait。
 * 3. 使用 X-Macro "注入" 实现到 <core/hash/hash.h> 的
 * _Generic 分发器中。
 */

#include <std/hash/xxhash.h>

// 2. 包含 Hasher Trait 定义，我们需要它来“实现”
#include <core/hash/hasher.h>
#include <core/type.h>

// --- 定义 DefaultHasher ---

/**
 * @brief libkx 的默认 Hasher 状态机。
 *
 * 包装了 XXH64 的流式状态 `XXH64_state_t`。
 * (我们能访问这个结构体，因为 Makefile 中定义了
 * XXH_STATIC_LINKING_ONLY=1)
 */
typedef struct DefaultHasher
{
  XXH64_state_t state;
} DefaultHasher;

// --- "impl DefaultHasher" (固有方法) ---

/**
 * @brief (Impl) 创建一个新的 DefaultHasher 实例。
 * @param seed 哈希种子。
 * @return 已初始化的 Hasher 实例。
 */
static inline DefaultHasher
DefaultHasher_new_with_seed(u64 seed)
{
  DefaultHasher hasher;
  // XXH64_reset 完成初始化
  (void)XXH64_reset(&hasher.state, seed);
  return hasher;
}

/**
 * @brief (Impl) 使用默认种子 (0) 创建一个新的 DefaultHasher
 * 实例。
 */
static inline DefaultHasher
DefaultHasher_new(void)
{
  return DefaultHasher_new_with_seed(0);
}

// --- "impl Hasher for DefaultHasher" (Trait 实现) ---
// 我们必须提供 DEFINE_HASHER_TRAIT 所需的两个核心函数：

/**
 * @brief (Impl) Hasher Trait - write
 * 映射到 xxHash 的 update 函数。
 */
static inline void
DefaultHasher_write(DefaultHasher *self, const void *bytes, usize len)
{
  (void)XXH64_update(&self->state, bytes, len);
}

/**
 * @brief (Impl) Hasher Trait - finish
 * 映射到 xxHash 的 digest 函数。
 */
static inline u64
DefaultHasher_finish(const DefaultHasher *self)
{
  return XXH64_digest(&self->state);
}

// --- 实例化 Trait ---

// 这将自动为我们生成 DefaultHasher_write_u8, _u16, _u64
// 等所有便捷函数。
DEFINE_HASHER_TRAIT(DefaultHasher)

// --- 3. X-Macro 注入 ---

/**
 * @brief (Internal) X-Macro 注入
 *
 * 我们在这里 #undef <core/hash/hash.h>
 * 中定义的空列表，并提供我们自己的实现。
 *
 * 当 hash_u64() 调用 HASHER_WRITE_U64(state, val)
 * 时， 那个 _Generic 宏将会包含 "DefaultHasher*: ..."
 * 这一分支， 从而静态分发到 DefaultHasher_write_u64()。
 */
#define HASHER_DISPATCH_LIST(state, func, ...)                                                     \
  DefaultHasher * : DefaultHasher_##func(state, __VA_ARGS__)

#include <core/hash/hash.h>
