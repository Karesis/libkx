/*
 * Copyright (C) 2025 Karesis
 * (License header as in other files)
 */

#include <core/mem/sysalc.h>
#include <core/option.h>
#include <std/sintern/sintern.h> // 包含我们要测试的模块
#include <std/test/test.h>
#include <string.h> // For strcpy

int
main(void)
{
  SUITE_START("SIntern (String Interner)");

  SystemAlloc sys;
  Option_SInternPtr opt = sintern_new(&sys);
  TEST_ASSERT(ois_some(opt), "SIntern creation failed (OOM?)");
  SIntern *interner = oexpect(opt, "SIntern creation failed (OOM?)");

  // --- Test 1: Intern a simple string ---
  str s1 = sintern_intern(interner, "hello");
  TEST_ASSERT(s1 != NULL, "intern() returned NULL");
  TEST_ASSERT(str_cmp(s1, "hello") == EQUAL, "s1 is not 'hello'");

  // --- Test 2: Intern the *same* string ---
  str s2 = sintern_intern(interner, "hello");
  TEST_ASSERT(str_cmp(s2, "hello") == EQUAL, "s2 is not 'hello'");

  // ** 核心断言：指针必须相等 **
  TEST_ASSERT(s1 == s2, "s1 and s2 do not point to the same memory!");

  // --- Test 3: Intern a *different* string ---
  str s3 = sintern_intern(interner, "world");
  TEST_ASSERT(str_cmp(s3, "world") == EQUAL, "s3 is not 'world'");

  // ** 核心断言：指针必须不相等 **
  TEST_ASSERT(s1 != s3, "s1 and s3 point to the same memory!");

  // --- Test 4: Intern a string from a different stack location ---
  char stack_str[10];
  strcpy(stack_str, "hello");

  str s4 = sintern_intern(interner, (str)stack_str);
  TEST_ASSERT(str_cmp(s4, "hello") == EQUAL, "s4 is not 'hello'");

  // ** 核心断言：s4 必须等于 s1 (来自 "hello" 字面量) **
  TEST_ASSERT(s1 == s4, "s4 (from stack) does not match s1 (from literal)");

  // --- Test 5: Intern bytes (from lexer) ---
  const char *source_code = "let x = 10; let y = 20;";
  // 模拟词法分析器提取了 "let" (长度 3)
  str let_kw_1 = sintern_intern_bytes(interner, source_code, 3);
  TEST_ASSERT(str_cmp(let_kw_1, "let") == EQUAL, "let_kw_1 is not 'let'");

  // --- Test 6: Intern bytes (same token, different location) ---
  // 模拟词法分析器提取了第二个 "let" (在索引 12 处, 长度 3)
  str let_kw_2 = sintern_intern_bytes(interner, source_code + 12, 3);
  TEST_ASSERT(str_cmp(let_kw_2, "let") == EQUAL, "let_kw_2 is not 'let'");

  // ** 核心断言：两个 'let' 必须是同一个指针 **
  TEST_ASSERT(let_kw_1 == let_kw_2, "Both 'let' tokens are not the same interned pointer");

  // --- Test 7: Intern bytes (different token) ---
  // 模拟词法分析器提取了 "y" (在索引 16 处, 长度 1)
  str y_ident = sintern_intern_bytes(interner, source_code + 16, 1);
  TEST_ASSERT(str_cmp(y_ident, "y") == EQUAL, "y_ident is not 'y'");

  TEST_ASSERT(let_kw_1 != y_ident, "'let' and 'y' have the same pointer!");

  // --- Cleanup ---
  sintern_free(interner);
  SUITE_END();
  TEST_SUMMARY();

  return 0;
}
