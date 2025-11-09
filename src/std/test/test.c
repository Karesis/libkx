/* src/std/test.c */

#include <std/test/test.h> // (包含我们自己的头文件)

/*
 * ===================================================================
 * 全局状态 (Private Implementation)
 * ===================================================================
 *
 * 在 .h 文件中被声明为 'extern'。
 * 在 .c 文件中被 *定义*。
 *
 * 这确保了整个程序中 *只有一组* 全局计数器。
 */

int g_kx_test_total_run = 0;
int g_kx_test_total_failed = 0;

int g_kx_test_suite_run = 0;
int g_kx_test_suite_passed = 0;
const char *g_kx_test_suite_name = "";
