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


/* include/std/debug/option.h */
#pragma once

#include <core/msg/panic.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum OptionKind
{
  SOME,
  NONE
} OptionKind;

/*
 * 注意: 这个宏只在 option_core.h 中定义
 */
#define DEFINE_OPTION(type)                                \
  typedef struct Option_##type                             \
  {                                                        \
    OptionKind kind;                                       \
    union {                                                \
      type some;                                           \
    } value;                                               \
  } Option_##type

/* --- 构造器 (核心) --- */

#define __OPTION_SOME_PASTE(type, ...)                     \
  (Option_##type)                                          \
  {                                                        \
    .kind = SOME, .value = {.some = __VA_ARGS__ }          \
  }

#define Some(type, ...)                                    \
  __OPTION_SOME_PASTE(type, __VA_ARGS__)

#define __OPTION_NONE_PASTE(type)                          \
  (Option_##type)                                          \
  {                                                        \
    .kind = NONE                                           \
  }

#define None(type) __OPTION_NONE_PASTE(type)

/*
 * 核心的 OptionKind, DEFINE_OPTION, Some, None
 * 已经从 option_core.h 导入。
 */

/* --- 检查与解包 (功能性) --- */

#define oexpect(opt, msg)                                  \
  ({                                                       \
    __auto_type __opt_tmp = (opt);                         \
    if (ois_none(__opt_tmp))                               \
    {                                                      \
      panic("Failed expectation (expected Some): {}",      \
            (msg));                                        \
    }                                                      \
    __opt_tmp.value.some;                                  \
  })

#define ois_some(opt) ((opt).kind == SOME)

#define ois_none(opt) ((opt).kind == NONE)

#define ounwrap_or(opt, default_val)                       \
  ({                                                       \
    __auto_type __opt_tmp = (opt);                         \
    ois_some(__opt_tmp) ? __opt_tmp.value.some             \
                        : (default_val);                   \
  })

#define ounwrap_or_else(opt, func)                         \
  ({                                                       \
    __auto_type __opt_tmp = (opt);                         \
    ois_some(__opt_tmp) ? __opt_tmp.value.some : (func)(); \
  })

/* --- 适配器 (功能性) --- */

#define __OPTION_MAP_PASTE(out_type, opt_in, var, ...)     \
  ({                                                       \
    __auto_type __opt_tmp = (opt_in);                      \
    (ois_none(__opt_tmp)) ? None(out_type) : ({            \
      typeof(__opt_tmp.value.some) var =                   \
        __opt_tmp.value.some;                              \
      Some(out_type, __VA_ARGS__);                         \
    });                                                    \
  })

#define omap(out_type, opt_in, var, ...)                   \
  __OPTION_MAP_PASTE(out_type, opt_in, var, __VA_ARGS__)

#define __OPTION_AND_THEN_PASTE(                           \
  out_type, opt_in, var, ...)                              \
  ({                                                       \
    __auto_type __opt_tmp = (opt_in);                      \
    (ois_none(__opt_tmp)) ? None(out_type) : ({            \
      typeof(__opt_tmp.value.some) var =                   \
        __opt_tmp.value.some;                              \
      (__VA_ARGS__);                                       \
    });                                                    \
  })

#define oand_then(out_type, opt_in, var, ...)              \
  __OPTION_AND_THEN_PASTE(                                 \
    out_type, opt_in, var, __VA_ARGS__)

// 常用的option类型
DEFINE_OPTION(anyptr);
