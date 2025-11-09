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
