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

/* src/std/test.h */
#pragma once

/*
 * ===================================================================
 * 1. 依赖 (全部来自 Core)
 * ===================================================================
 */
#include <core/color.h>      // L1: fg, reset, rgb
#include <core/fmt/tofile.h> // L2: vformat 引擎的 FILE* Sink
#include <core/fmt/vformat.h> // L1: 引擎宏 (ARGS_COUNT, ...)
#include <core/msg/asrt.h>    // L3: asrt (用于致命断言)
#include <stdio.h>            // (stdout, stderr)

/*
 * ===================================================================
 * 2. 全局状态 (在 test.c 中定义)
 * ===================================================================
 *
 * 这些是 *真正的* 全局变量，由 test.c 拥有。
 * .h 文件只是声明 'extern' 来访问它们。
 */

extern int g_kx_test_total_run;
extern int g_kx_test_total_failed;

extern int g_kx_test_suite_run;
extern int g_kx_test_suite_passed;
extern const char *g_kx_test_suite_name;

/*
 * ===================================================================
 * 3. 核心 API 宏
 * ===================================================================
 */

/**
 * @brief 标记一个测试套件的开始。
 */
#define SUITE_START(name)                                  \
  do                                                       \
  {                                                        \
    g_kx_test_suite_name = (name);                         \
    g_kx_test_suite_run = 0;                               \
    g_kx_test_suite_passed = 0;                            \
    format_to_file(stdout,                                 \
                   "\n--- Test Suite: {} ---\n",           \
                   g_kx_test_suite_name);                  \
  } while (0)

/**
 * @brief (非致命) 断言。
 * 失败时，打印错误并继续运行套件。
 */
#define TEST_ASSERT(cond, fmt, ...)                        \
  do                                                       \
  {                                                        \
    g_kx_test_suite_run++;                                 \
    if (cond)                                              \
    {                                                      \
      g_kx_test_suite_passed++;                            \
    }                                                      \
    else                                                   \
    {                                                      \
      /* [!!] 失败时打印 (使用 vformat) [!!] */            \
      format_to_file(stderr,                               \
                     "{}    [FAIL] {}() at line {}:{}\n",  \
                     fg(rgb(255, 80, 80)),                 \
                     __func__,                             \
                     __LINE__,                             \
                     reset());                             \
      format_to_file(                                      \
        stderr, "           Condition: {}\n", #cond);      \
      format_to_file(stderr, "           Message:   ");    \
      /* (再次调用 vformat 来打印用户消息) */              \
      format_to_file(stderr,                               \
                     fmt __VA_OPT__(, ) __VA_ARGS__);      \
      format_to_file(stderr, "\n");                        \
    }                                                      \
  } while (0)

/**
 * @brief (致命) 断言。
 * 用于检查测试环境 (例如 setup 失败)。
 * 失败时，打印错误并 *中止* (panic)。
 */
#define TEST_ASSERT_FATAL(cond, fmt, ...)                  \
  asrt_msg(cond, fmt __VA_OPT__(, ) __VA_ARGS__)

/**
 * @brief 标记一个测试套件的结束。
 *
 * (关键改动) 它不再 'return'。
 * 它只更新全局计数器。
 */
#define SUITE_END()                                        \
  do                                                       \
  {                                                        \
    g_kx_test_total_run++;                                 \
    format_to_file(                                        \
      stdout,                                              \
      "--- Summary ({}): {} / {} passed ---\n",            \
      g_kx_test_suite_name,                                \
      g_kx_test_suite_passed,                              \
      g_kx_test_suite_run);                                \
    if (g_kx_test_suite_run != g_kx_test_suite_passed)     \
    {                                                      \
      g_kx_test_total_failed++;                            \
      format_to_file(stderr,                               \
                     "{}*** Suite FAILED ***{}\n",         \
                     fg(rgb(255, 80, 80)),                 \
                     reset());                             \
    }                                                      \
  } while (0)

/*
 * ===================================================================
 * 4. 运行器宏 (Runner Macros for main())
 * ===================================================================
 */

/**
 * @brief (可选) 声明一个测试套件函数 (用于 main)。
 */
#define TEST_SUITE(name) void name(void)

/**
 * @brief (可选) 运行一个测试套件函数。
 * (这只是为了让 main 更易读)。
 */
#define RUN_SUITE(name)                                    \
  do                                                       \
  {                                                        \
    format_to_file(stdout, "Running {}...\n", #name);      \
    name();                                                \
  } while (0)

/**
 * @brief 在 main() 的末尾打印所有测试的最终摘要。
 *
 * (关键改动) 这现在是 *唯一* 负责 'return' 的宏。
 */
#define TEST_SUMMARY()                                     \
  do                                                       \
  {                                                        \
    format_to_file(stdout,                                 \
                   "\n============================\n");    \
    format_to_file(stdout,                                 \
                   "  Total Suites Run:    {}\n",          \
                   g_kx_test_total_run);                   \
    format_to_file(                                        \
      stdout,                                              \
      "  Total Suites Failed: {}{}{}\n",                   \
      (g_kx_test_total_failed > 0) ? fg(rgb(255, 0, 0))    \
                                   : "",                   \
      g_kx_test_total_failed,                              \
      (g_kx_test_total_failed > 0) ? reset() : "");        \
    format_to_file(stdout,                                 \
                   "============================\n\n");    \
    if (g_kx_test_total_failed == 0)                       \
    {                                                      \
      format_to_file(stdout,                               \
                     "{}[OK] All {} suites passed.{}\n",   \
                     fg(rgb(80, 255, 80)),                 \
                     g_kx_test_total_run,                  \
                     reset());                             \
      return 0;                                            \
    }                                                      \
    else                                                   \
    {                                                      \
      format_to_file(stderr,                               \
                     "{}[!!!] {} suite(s) FAILED.{}\n",    \
                     fg(rgb(255, 0, 0)),                   \
                     g_kx_test_total_failed,               \
                     reset());                             \
      return 1;                                            \
    }                                                      \
  } while (0)
