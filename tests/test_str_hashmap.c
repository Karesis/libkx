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

#include <core/mem/sysalc.h>
#include <core/option.h>
#include <std/hashmap.h>
#include <std/test/test.h>
#include <string.h> // For Test 5 (strcpy)

// 实例化... (保持不变)
DEFINE_HASHMAP(StrMap, str, u64, SystemAlloc, SYSTEM, hash_fn_str, cmp_fn_str);

int
main(void)
{
  SUITE_START("HashMap (StrMap)");

  SystemAlloc sys;
  StrMap *map = StrMap_new(&sys);
  TEST_ASSERT(map != NULL, "Map creation failed (OOM?)");

  // --- Test 1: Put & Get ---
  StrMap_put(map, "key1", 100);
  StrMap_put(map, "key2", 200);

  Option_StrMap_V val = StrMap_get(map, "key1");
  TEST_ASSERT(ois_some(val), "GET: 'key1' not found");
  TEST_ASSERT(oexpect(val, "GET: val was None") == 100, "GET: Value for 'key1' was not 100");

  val = StrMap_get(map, "key2");
  TEST_ASSERT(ois_some(val), "GET: 'key2' not found");
  TEST_ASSERT(oexpect(val, "GET: val was None") == 200, "GET: Value for 'key2' was not 200");
  TEST_ASSERT(map->count == 2, "Count was not 2");

  // --- Test 2: Get Non-Existent Key ---
  val = StrMap_get(map, "non-existent-key");
  TEST_ASSERT(ois_none(val), "GET: Non-existent key was found");

  // --- Test 3: Update Value ---
  StrMap_put(map, "key1", 999);
  TEST_ASSERT(map->count == 2, "Count changed after update");

  val = StrMap_get(map, "key1");
  TEST_ASSERT(ois_some(val), "UPDATE: 'key1' not found after update");
  TEST_ASSERT(oexpect(val, "UPDATE: val was None") == 999, "UPDATE: Value was not 999");

  // --- Test 4: Delete Key ---
  bool deleted = StrMap_delete(map, "key2");
  TEST_ASSERT(deleted, "DELETE: Delete returned false for 'key2'");
  TEST_ASSERT(map->count == 1, "Count was not 1 after delete");

  val = StrMap_get(map, "key2");
  TEST_ASSERT(ois_none(val), "DELETE: 'key2' was found after delete");

  // --- Test 5: 验证 cmp_fn_str (内容比较) ---
  char stack_key[20];
  strcpy(stack_key, "key1");

  const str p_stack_key = stack_key;
  val = StrMap_get(map, p_stack_key);

  TEST_ASSERT(ois_some(val), "CMP_FN: Get failed using stack_key 'key1'");
  TEST_ASSERT(oexpect(val, "CMP_FN: val was None") == 999, "CMP_FN: Value was not 999");

  StrMap_free(map);
  SUITE_END();
  TEST_SUMMARY();

  return 0;
}
