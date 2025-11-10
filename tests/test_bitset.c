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

#include <core/mem/layout.h> // KIB 宏在概念上属于这里
#include <core/mem/sysalc.h>
#include <std/alloc/bump.h>
#include <std/math/bitset.h>
#include <std/test/test.h>

/* --- 定义 KIB 宏 (修复 'KIB' aundeclared error) --- */
#define KIB(n) ((n) * 1024)

int
main(void)
{
  /*
   * ===================================================================
   * Test Suite 1: System Allocator (sbitset)
   * ===================================================================
   */
  SUITE_START("Bitset (SystemAlloc)");
  {
    SystemAlloc sys;
    /* 修复: 参数顺序 (num_bits, alloc) */
    sbitset *bs = bs_create(&sys, 100);
    TEST_ASSERT(bs != NULL, "bs_create (sys) failed");

    /* Test 1: Set / Test / Clear (sbitset) */
    TEST_ASSERT(bs_test(bs, 10) == false, "sbitset: Initial val not false");
    bs_set(bs, 10);
    TEST_ASSERT(bs_test(bs, 10) == true, "sbitset: set/test failed");
    bs_clear(bs, 10);
    TEST_ASSERT(bs_test(bs, 10) == false, "sbitset: clear/test failed");

    /* Test 2: Boundary conditions (sbitset) */
    bs_set(bs, 0);
    bs_set(bs, 99);
    TEST_ASSERT(bs_test(bs, 0) == true, "sbitset: set bit 0 failed");
    TEST_ASSERT(bs_test(bs, 99) == true, "sbitset: set bit 99 failed");

    /* Test 3: Clear All / Set All (sbitset) */
    bs_set_all(bs);
    TEST_ASSERT(bs_test(bs, 50) == true, "sbitset: set_all failed");
    bs_clear_all(bs);
    TEST_ASSERT(bs_test(bs, 50) == false, "sbitset: clear_all failed");

    /* Test 4: Create All (sbitset) */
    /* 修复: 参数顺序 (num_bits, alloc) */
    sbitset *bs_all = bs_create_all(&sys, 100);
    TEST_ASSERT(bs_all != NULL, "bs_create_all (sys) failed");
    TEST_ASSERT(bs_test(bs_all, 10) == true, "sbitset: create_all bit 10 not set");
    TEST_ASSERT(bs_test(bs_all, 99) == true, "sbitset: create_all bit 99 not set");

    /* Test 5: Copy / Equals (sbitset) */
    bs_copy(bs, bs_all);
    TEST_ASSERT(bs_equals(bs, bs_all) == true, "sbitset: copy/equals failed");
    bs_clear(bs, 10);
    TEST_ASSERT(bs_equals(bs, bs_all) == false, "sbitset: equals post-clear failed");

    /* Test 6: Logical Ops (sbitset) */
    sbitset *bs1 = bs_create(&sys, 64);
    sbitset *bs2 = bs_create(&sys, 64);
    sbitset *dest = bs_create(&sys, 64);
    bs_set(bs1, 1);
    bs_set(bs1, 2); /* bs1 = { 1, 2 } */
    bs_set(bs2, 2);
    bs_set(bs2, 3); /* bs2 = { 2, 3 } */

    /* Union: {1, 2} | {2, 3} = {1, 2, 3} */
    bs_union(dest, bs1, bs2);
    TEST_ASSERT(bs_test(dest, 1) && bs_test(dest, 2) && bs_test(dest, 3), "sbitset: union failed");
    TEST_ASSERT(bs_test(dest, 4) == false, "sbitset: union failed (bit 4)");

    /* Intersect: {1, 2} & {2, 3} = {2} */
    bs_intersect(dest, bs1, bs2);
    TEST_ASSERT(bs_test(dest, 2) == true, "sbitset: intersect failed (bit 2)");
    TEST_ASSERT(bs_test(dest, 1) == false, "sbitset: intersect failed (bit 1)");
    TEST_ASSERT(bs_test(dest, 3) == false, "sbitset: intersect failed (bit 3)");

    /* Difference: {1, 2} - {2, 3} = {1} */
    bs_difference(dest, bs1, bs2);
    TEST_ASSERT(bs_test(dest, 1) == true, "sbitset: difference failed (bit 1)");
    TEST_ASSERT(bs_test(dest, 2) == false, "sbitset: difference failed (bit 2)");

    bs_destroy(bs);
    bs_destroy(bs_all);
    bs_destroy(bs1);
    bs_destroy(bs2);
    bs_destroy(dest);
  }
  SUITE_END();

  /*
   * ===================================================================
   * Test Suite 2: Bump Allocator (bbitset)
   * ===================================================================
   */
  SUITE_START("Bitset (Bump)");
  {
    /* 修复: 根据 bump.h API, Bump 需要一个 SystemAlloc
     * 来工作 */
    SystemAlloc sys; /* 支撑分配器 */
    Bump bump_alloc;
    bump_init(&bump_alloc, &sys); /* 修复: 正确的初始化 */

    /* 修复: 参数顺序 (num_bits, alloc) */
    bbitset *bs = bs_create(&bump_alloc, 200);
    TEST_ASSERT(bs != NULL, "bs_create (bump) failed");

    /* 针对 bump 分配器进行简单的冒烟测试 */
    TEST_ASSERT(bs_test(bs, 10) == false, "bbitset: Initial val not false");
    bs_set(bs, 10);
    bs_set(bs, 199);
    TEST_ASSERT(bs_test(bs, 10) == true, "bbitset: set/test failed");
    TEST_ASSERT(bs_test(bs, 199) == true, "bbitset: set bit 199 failed");

    /* 修复: 参数顺序 (num_bits, alloc) */
    bbitset *bs_all = bs_create_all(&bump_alloc, 200);
    TEST_ASSERT(bs_all != NULL, "bs_create_all (bump) failed");
    TEST_ASSERT(bs_test(bs_all, 150) == true, "bbitset: create_all failed");

    /* 销毁 bs 和 bs_all (这只会释放堆上的句柄,
     * 'words' 数组在 bump_reset 时才会被回收)
     */
    bs_destroy(bs);
    bs_destroy(bs_all);

    /* 测试重置 (bump_reset 会释放所有 Chunks) */
    bump_reset(&bump_alloc);

    /* 修复: 参数顺序 (num_bits, alloc) */
    bbitset *bs3 = bs_create(&bump_alloc, 100);
    TEST_ASSERT(bs3 != NULL, "bbitset: create after reset failed");
    bs_set(bs3, 50);
    TEST_ASSERT(bs_test(bs3, 50), "bbitset: reset test failed");
    bs_destroy(bs3);

    /* 彻底销毁 bump 分配器 (释放它持有的所有 Chunks) */
    bump_destroy(&bump_alloc);
  }
  SUITE_END();

  TEST_SUMMARY();
  return 0;
}
