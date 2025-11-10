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

/* tests/test_vector.c */

#include <core/mem/sysalc.h> // 3. 包含依赖 (分配器)
#include <std/test/test.h>   // 1. 包含测试框架
#include <std/vector.h>      // 2. 包含你要测试的库

/*
 * 你可以把 SystemAlloc 实例设为全局，
 * 供这个测试文件中的所有套件使用。
 */
static SystemAlloc g_sys;

DEFINE_VECTOR(Vec_i32, int, SystemAlloc, SYSTEM);
/*
 * =s=======================================
 * 套件 1: Vector Init
 * =s=======================================
 */
TEST_SUITE(test_vector_init)
{
  SUITE_START("Vector Init");

  Vec_i32 vec;
  Vec_i32_init(&vec, &g_sys);

  TEST_ASSERT(vec.len == 0, "Length should be 0");
  TEST_ASSERT(vec.cap == 0, "Capacity should be 0");
  TEST_ASSERT(vec.data == NULL, "Data should be NULL");

  SUITE_END();
}

/*
 * =s=======================================
 * 套件 2: Vector Push
 * =s=======================================
 */
TEST_SUITE(test_vector_push)
{
  SUITE_START("Vector Push");
  Vec_i32 vec;
  Vec_i32_init(&vec, &g_sys);

  Vec_i32_push(&vec, 10);
  TEST_ASSERT(vec.len == 1, "Length should be 1");
  TEST_ASSERT(vec.cap >= 1, "Capacity should be >= 1");
  TEST_ASSERT(vec.data[0] == 10, "Value[0] should be 10");

  Vec_i32_push(&vec, 20);
  TEST_ASSERT(vec.len == 2, "Length should be 2");
  TEST_ASSERT(vec.data[1] == 20, "Value[1] should be 20");

  // (可选) 别忘了释放内存
  Vec_i32_deinit(&vec);

  SUITE_END();
}

/*
 * =s=======================================
 * 主测试运行器 (Test Runner Main)
 * =s=======================================
 */
int
main(void)
{
  RUN_SUITE(test_vector_init);
  RUN_SUITE(test_vector_push);

  // ... (运行 test_string.c 中的套件)
  // (你需要在 main 中调用所有测试函数)

  TEST_SUMMARY();
}
