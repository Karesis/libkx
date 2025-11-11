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

#include <core/msg/panic.h>
#include <core/type/ptr.h> // 引入 anyptr
#include <stdio.h>
#include <stdlib.h>

typedef enum OptionKind
{
  SOME,
  NONE
} OptionKind;

/**
 * @brief (升级) 定义一个 Option "类"。
 *
 * 现在接受一个 "Name" (用于 struct 类型) 和
 * 一个 "Type" (它所包装的 C 类型)。
 * 这允许我们安全地包装指针类型，例如:
 * DEFINE_OPTION(u64_ptr, u64*)
 *
 * @param Name 最终的 Option 类型的名称 (例如: u64, u64_ptr)
 * @param Type 它所包装的 C 类型 (例如: u64, u64*)
 */
#define DEFINE_OPTION(Name, Type)                                                                  \
  typedef struct Option_##Name                                                                     \
  {                                                                                                \
    OptionKind kind;                                                                               \
    union {                                                                                        \
      Type some;                                                                                   \
    } value;                                                                                       \
  } Option_##Name

/* --- 构造器 (核心) --- */

/**
 * @brief (升级) Some 构造器
 * @param Name Option 的名称 (例如: u64, u64_ptr)
 * @param ...  要包装的 'some' 值
 */
#define __OPTION_SOME_PASTE(Name, ...)                                                             \
  (Option_##Name)                                                                                  \
  {                                                                                                \
    .kind = SOME, .value = {.some = __VA_ARGS__ }                                                  \
  }
#define Some(Name, ...) __OPTION_SOME_PASTE(Name, __VA_ARGS__)

/**
 * @brief None 构造器
 * @param Name Option 的名称 (例如: u64, u64_ptr)
 */
#define __OPTION_NONE_PASTE(Name)                                                                  \
  (Option_##Name)                                                                                  \
  {                                                                                                \
    .kind = NONE                                                                                   \
  }
#define None(Name) __OPTION_NONE_PASTE(Name)

/* --- 检查与解包  --- */

#define oexpect(opt, msg)                                                                          \
  ({                                                                                               \
    auto __opt_tmp = (opt);                                                                        \
    if (ois_none(__opt_tmp))                                                                       \
    {                                                                                              \
      panic("Failed expectation (expected Some): {}", (msg));                                      \
    }                                                                                              \
    __opt_tmp.value.some;                                                                          \
  })

#define ois_some(opt) ((opt).kind == SOME)

#define ois_none(opt) ((opt).kind == NONE)

#define ounwrap_or(opt, default_val)                                                               \
  ({                                                                                               \
    auto __opt_tmp = (opt);                                                                        \
    ois_some(__opt_tmp) ? __opt_tmp.value.some : (default_val);                                    \
  })

#define ounwrap_or_else(opt, func)                                                                 \
  ({                                                                                               \
    auto __opt_tmp = (opt);                                                                        \
    ois_some(__opt_tmp) ? __opt_tmp.value.some : (func)();                                         \
  })

/* --- 适配器 --- */

/**
 * @brief omap 宏
 * @param Out_Name (以前是 out_type)
 * 目标 Option 的 *名称* (例如: u64)
 */
#define __OPTION_MAP_PASTE(Out_Name, opt_in, var, ...)                                             \
  ({                                                                                               \
    auto __opt_tmp = (opt_in);                                                                     \
    (ois_none(__opt_tmp)) ? None(Out_Name) : ({                                                    \
      typeof(__opt_tmp.value.some) var = __opt_tmp.value.some;                                     \
      Some(Out_Name, __VA_ARGS__);                                                                 \
    });                                                                                            \
  })
#define omap(Out_Name, opt_in, var, ...) __OPTION_MAP_PASTE(Out_Name, opt_in, var, __VA_ARGS__)

/**
 * @brief oand_then 宏
 * @param Out_Name (以前是 out_type)
 * 目标 Option 的 *名称* (例如: u64)
 */
#define __OPTION_AND_THEN_PASTE(Out_Name, opt_in, var, ...)                                        \
  ({                                                                                               \
    auto __opt_tmp = (opt_in);                                                                     \
    (ois_none(__opt_tmp)) ? None(Out_Name) : ({                                                    \
      typeof(__opt_tmp.value.some) var = __opt_tmp.value.some;                                     \
      (__VA_ARGS__);                                                                               \
    });                                                                                            \
  })
#define oand_then(Out_Name, opt_in, var, ...)                                                      \
  __OPTION_AND_THEN_PASTE(Out_Name, opt_in, var, __VA_ARGS__)

/* --- 常用的 option 类型 --- */
DEFINE_OPTION(anyptr, anyptr);
DEFINE_OPTION(str, str);