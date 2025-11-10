#include <core/mem/sysalc.h>
#include <core/option.h>
#include <std/hashmap.h>
#include <std/test/test.h>

// 1. 实例化 HashMap "模板"
DEFINE_HASHMAP(U64Map,
               u64,
               u64,
               SystemAlloc,
               SYSTEM,
               hash_fn_u64,
               cmp_fn_u64);

// 2. 编写测试套件
int
main(void)
{
  SUITE_START("HashMap (U64Map)");

  SystemAlloc sys;
  U64Map *map = U64Map_new(&sys);
  TEST_ASSERT(map != NULL, "Map creation failed (OOM?)");
  TEST_ASSERT(map->count == 0, "Initial count not 0");

  // --- Test 1: Put & Get ---
  U64Map_put(map, 100, 42);

  Option_U64Map_V val = U64Map_get(map, 100);

  TEST_ASSERT(ois_some(val), "GET: Key 100 not found");

  TEST_ASSERT(oexpect(val, "GET: val was None") == 42,
              "GET: Value for 100 was not 42");
  TEST_ASSERT(map->count == 1,
              "Count was not 1 after 1st insert");

  // --- Test 2: Get Non-Existent Key ---
  val = U64Map_get(map, 200);

  TEST_ASSERT(ois_none(val),
              "GET: Key 200 was found (should be absent)");

  // --- Test 3: Update Value ---
  U64Map_put(map, 100, 999); // 覆盖 key 100
  val = U64Map_get(map, 100);

  TEST_ASSERT(ois_some(val), "UPDATE: Key 100 not found");
  TEST_ASSERT(oexpect(val, "UPDATE: val was None") == 999,
              "UPDATE: Value was not updated to 999");
  TEST_ASSERT(map->count == 1,
              "Count changed after update (should be 1)");

  // --- Test 4: Delete Key ---
  bool deleted = U64Map_delete(map, 100);
  TEST_ASSERT(deleted, "DELETE: Delete returned false");
  TEST_ASSERT(map->count == 0,
              "Count was not 0 after delete");

  // 验证它确实被删除了
  val = U64Map_get(map, 100);
  TEST_ASSERT(ois_none(val),
              "DELETE: Key 100 was found after delete");

  // --- Test 5: Delete Non-Existent Key ---
  deleted = U64Map_delete(map, 999);
  TEST_ASSERT(
    !deleted,
    "DELETE: Deleting non-existent key returned true");

  U64Map_free(map);
  SUITE_END();
  TEST_SUMMARY();

  return 0;
}
