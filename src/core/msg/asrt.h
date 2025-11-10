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

/**
 * @brief (公共 API) 断言一个条件必须为真。
 *
 * 如果条件为假, 将调用 panic 并打印自定义消息。
 *
 * @param cond (bool) 必须为真的条件
 * @param fmt  (const char *) panic 时的格式化字符串
 * @param ...  (可选) 格式化参数
 *
 * @example
 * asrt_msg(user_id > 0, "Invalid user ID: {}",
 * user_id);
 */
#define asrt_msg(cond, fmt, ...)                           \
  do                                                       \
  {                                                        \
    if (!(cond))                                           \
    {                                                      \
      panic("Assertion failed (`{}`): " fmt,               \
            #cond __VA_OPT__(, ) __VA_ARGS__);             \
    }                                                      \
  } while (0)

/**
 * @brief (公共 API) 断言一个条件必须为真 (使用默认消息)。
 *
 * @param cond (bool) 必须为真的条件
 *
 * @example
 * asrt(ptr != NULL);
 * // panic 时: Assertion failed: `ptr != NULL`
 */
#define asrt(cond)                                         \
  do                                                       \
  {                                                        \
    if (!(cond))                                           \
    {                                                      \
      panic("Assertion failed: {}", #cond);                \
    }                                                      \
  } while (0)
