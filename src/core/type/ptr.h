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

#pragma once

#include <stddef.h>
#include <stdint.h>

typedef intptr_t ptr;
typedef uintptr_t uptr;
typedef void *anyptr;
typedef const void *canyptr; // "const any pointer"

/**
 * @brief container_of 宏
 *
 * 通过一个结构体成员的指针，获取该结构体“容器”的指针。
 *
 * @param ptr 成员的指针
 * @param type 容器结构体的类型 (如 MyStruct)
 * @param member 成员在结构体中的名字 (如 list_node)
 */
#define container_of(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))
