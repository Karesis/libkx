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
