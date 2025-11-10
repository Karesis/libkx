/*
 * tests/minimal_hash_test.c
 * 目的：只测试 std/hash/default.h (L3) 是否成功注入
 * 并导致 core/hash/hash.h (L1) 被正确展开。
 */

// 1. 包含 L3 的实现头文件。
// 这是关键：它必须在 *最前面*，以确保 X-Macro
// (HASHER_DISPATCH_LIST) 在 hash.h 被包含 *之前* 就绪。
#include <std/hash/default.h>

// 2. default.h 应该已经包含了 hash.h。
//    我们现在直接测试 hash.h 中定义的宏是否"存在"。

// 3. 测试 "impl Hash for T" 宏 (例如 hash_u8)
//    如果 hash.h 没有被正确包含/展开，这一行就会失败。
#define MY_MACRO_TEST hash_u8

// 4. 测试 "HASHER_WRITE_..." 宏 (例如 HASHER_WRITE_U8)
//    如果 hash.h 没有被正确包含/展开，这一行也会失败。
#define MY_MACRO_TEST_2 HASHER_WRITE_U8

// 5. 尝试使用 hash() 宏
void
my_test_fn(DefaultHasher *h)
{
  u64 key = 123;
  // 如果 hash_u64, HASHER_WRITE_U64 等任何一个环节失败，
  // 这一行就会在编译时报错。
  hash(&key, h);
}

int
main(void)
{
  // 只是为了让 C 文件能被编译
  return 0;
}
