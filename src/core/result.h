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

#pragma once

#include <core/msg/panic.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum ResultKind
{
  OK,
  ERR
} ResultKind;

/*
 * 注意: DEFINE_RESULT 只在 result_core.h 中定义
 */
#define DEFINE_RESULT(ok_type, err_type)                                                           \
  typedef struct Result_##ok_type##_##err_type                                                     \
  {                                                                                                \
    ResultKind kind;                                                                               \
    union {                                                                                        \
      ok_type ok;                                                                                  \
      err_type err;                                                                                \
    } value;                                                                                       \
  } Result_##ok_type##_##err_type;

/* --- 构造器 (核心) --- */
#define Ok(ok_type, err_type, ...)                                                                 \
  (Result_##ok_type##_##err_type)                                                                  \
  {                                                                                                \
    .kind = OK, .value = {.ok = __VA_ARGS__ }                                                      \
  }

#define Err(ok_type, err_type, ...)                                                                \
  (Result_##ok_type##_##err_type)                                                                  \
  {                                                                                                \
    .kind = ERR, .value = {.err = __VA_ARGS__ }                                                    \
  }

/* --- APIs --- */
#define ris_ok(res) ((res).kind == OK)

#define ris_err(res) ((res).kind == ERR)

#define rexpect(res, msg)                                                                          \
  ({                                                                                               \
    __auto_type __res_tmp = (res);                                                                 \
    if (ris_err(__res_tmp))                                                                        \
    {                                                                                              \
      panic("Failed expectation (expected Ok): {}", (msg));                                        \
    }                                                                                              \
    __res_tmp.value.ok; /* 表达式的返回值 */                                                       \
  })

#define rexpect_err(res, msg)                                                                      \
  ({                                                                                               \
    __auto_type __res_tmp = (res);                                                                 \
    if (ris_ok(__res_tmp))                                                                         \
    {                                                                                              \
      panic("Failed expectation (expected Err): {}", (msg));                                       \
    }                                                                                              \
    __res_tmp.value.err; /* 表达式的返回值 */                                                      \
  })

#define runwrap_or(res, default_val)                                                               \
  ({                                                                                               \
    __auto_type __res_tmp = (res);                                                                 \
    ris_ok(__res_tmp) ? __res_tmp.value.ok : (default_val);                                        \
  })

#define runwrap_or_else(res, func)                                                                 \
  ({                                                                                               \
    __auto_type __res_tmp = (res);                                                                 \
    ris_ok(__res_tmp) ? __res_tmp.value.ok : (func)();                                             \
  })

/* --- map --- */
/*
 * API 变更:
 * 旧: rmap((out_ok, err), res, var, ...)
 * 新: rmap(out_ok, err, res, var, ...)
 */
#define rmap(out_ok_type, err_type, res_in, var, ...)                                              \
  ({                                                                                               \
    __auto_type __res_tmp = (res_in);                                                              \
    (ris_err(__res_tmp)) ? Err(out_ok_type, err_type, __res_tmp.value.err) : ({                    \
      typeof(__res_tmp.value.ok) var = __res_tmp.value.ok;                                         \
      Ok(out_ok_type, err_type, __VA_ARGS__);                                                      \
    });                                                                                            \
  })

/* --- map_err --- */
/*
 * API 变更:
 * 旧: rmap_err((ok, out_err), res, var, ...)
 * 新: rmap_err(ok, out_err, res, var, ...)
 */
#define rmap_err(ok_type, out_err_type, res_in, var, ...)                                          \
  ({                                                                                               \
    __auto_type __res_tmp = (res_in);                                                              \
    (ris_ok(__res_tmp)) ? Ok(ok_type, out_err_type, __res_tmp.value.ok) : ({                       \
      typeof(__res_tmp.value.err) var = __res_tmp.value.err;                                       \
      Err(ok_type, out_err_type, __VA_ARGS__);                                                     \
    });                                                                                            \
  })

/* --- and_then --- */
/*
 * API 变更:
 * 旧: rand_then((out_ok, err), res, var, ...)
 * 新: rand_then(out_ok, err, res, var, ...)
 */
#define rand_then(out_ok_type, err_type, res_in, var, ...)                                         \
  ({                                                                                               \
    __auto_type __res_tmp = (res_in);                                                              \
    (ris_err(__res_tmp)) ? Err(out_ok_type, err_type, __res_tmp.value.err) : ({                    \
      typeof(__res_tmp.value.ok) var = __res_tmp.value.ok;                                         \
      (__VA_ARGS__); /* 必须返回一个 Result */                                                     \
    });                                                                                            \
  })
