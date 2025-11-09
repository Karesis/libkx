# ===================================================================
# libkx Makefile (V3 - 2025-11-10)
#
# 架构:
#   src/core/       (L0-L3) 纯头文件核心
#   src/std/        (L4+)   .c 实现 (包括 std/test/test.c)
#   tests/          (Runner) .c 测试运行器
#
# 目标 (Targets):
#   all         (默认) 构建库 (libkx.a) 和所有测试程序。
#   lib         只构建静态库 libkx.a (包含测试框架)。
#   tests       只构建所有测试程序 (依赖 lib)。
#   run_tests   构建并运行所有测试。
#   clean       清除所有构建产物 (build 目录)。
#
# ===================================================================

# --- 1. 配置 (Configuration) ---

# 编译器和标准
CC = clang
C_STD = -std=c23

# 目录
SRC_DIR   = src
TEST_DIR  = tests

BUILD_DIR = build
LIB_DIR   = $(BUILD_DIR)/lib
OBJ_DIR   = $(BUILD_DIR)/obj
TEST_BIN_DIR = $(BUILD_DIR)/tests

# 编译标志
# -I$(SRC_DIR): 允许 #include <core/type.h> 和 #include <std/vector.h>
# -MMD -MP:     自动生成 .d 依赖文件 (用于头文件)
CPPFLAGS = -I$(SRC_DIR)
CFLAGS   = -g -Wall -Wextra -pedantic $(C_STD) -MMD -MP $(CPPFLAGS)
LDFLAGS  =
LDLIBS   =

# --- 2. 目标 (Targets) ---

# 目标静态库 (包含所有 src/ c 文件)
TARGET_LIB = $(LIB_DIR)/libkx.a

# --- 3. 源文件发现 (Source File Discovery) ---

# [!!] 关键改动: 查找 *所有* src/ 下的 .c 文件
# (这现在 *包括* src/std/test/test.c)
LIB_SRCS = $(shell find $(SRC_DIR) -name '*.c')

# 查找 tests/ 目录下的所有 "runner" .c 文件
TEST_RUNNER_SRCS = $(shell find $(TEST_DIR) -name '*.c')

# --- 4. 对象和依赖 (Object & Dependency Mapping) ---

# .c -> .o 映射
LIB_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(LIB_SRCS))

# .c -> .d (依赖文件) 映射
DEPS = $(LIB_OBJS:.o=.d)

# .c (runner) -> 可执行文件 映射
TEST_TARGETS = $(patsubst $(TEST_DIR)/%.c, $(TEST_BIN_DIR)/%, $(TEST_RUNNER_SRCS))

# --- 5. 核心构建目标 (Core Build Targets) ---

# 默认目标 (第一个目标)
all: lib tests

# 声明非文件目标
.PHONY: all lib tests run_tests clean

# 目标：只构建库
lib: $(TARGET_LIB)

# 目标：只构建测试
tests: $(TEST_TARGETS)

# 目标：构建静态库 libkx.a
# 它依赖 *所有* 的 .o 文件 (现在包含 test.o)
$(TARGET_LIB): $(LIB_OBJS)
	@echo "AR  $@"
	@mkdir -p $(LIB_DIR)
	@ar rcs $@ $(LIB_OBJS)

# --- 6. 编译规则 (Compilation Rules) ---

# [!!] 为 bump.c 添加平台特定的 CFLAGS (用于 posix_memalign)
# 1. 找到 bump.o 的正确路径
BUMP_OBJ = $(OBJ_DIR)/std/alloc/bump.o

# 3. *覆盖* bump.o 的 CFLAGS
ifeq ($(OS),Windows_NT)
  # Windows: 不需要额外标志
  $(BUMP_OBJ): CFLAGS := $(CFLAGS)
else
  # Linux/macOS: 添加 _POSIX_C_SOURCE
  $(BUMP_OBJ): CFLAGS := $(CFLAGS) -D_POSIX_C_SOURCE=200809L
endif

# 模式规则：如何从 .c 文件编译 .o 文件
# (这条规则会自动为 bump.o 使用它专属的 CFLAGS)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "CC  $<"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@

# --- 7. 测试目标 (Test Targets) ---

# 目标：构建并运行所有测试
run_tests: tests
	@echo "--- Running libkx Test Suite ---"
	@for test in $(TEST_TARGETS); do \
		echo "RUN $$test"; \
		if ! ./$$test; then \
			echo ""; \
			echo "--- [FATAL] $$test FAILED ---"; \
			exit 1; \
		fi; \
	done
	@echo "--------------------------------"

# 模式规则：如何编译一个测试 "runner"
#
# [!!] 关键改动: 链接两个部分:
# 1. $<                 (测试 "runner" e.g., tests/test_vector.c)
# 2. $(TARGET_LIB)       (库 e.g., libkx.a, *已包含 test.o*)
#
$(TEST_BIN_DIR)/%: $(TEST_DIR)/%.c $(TARGET_LIB)
	@echo "LINK $@"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -o $@ \
	  -L$(LIB_DIR) -lkx \
	  $(LDFLAGS) $(LDLIBS)

# --- 8. 清理 (Clean) ---

# 目标：清理所有构建产物
clean:
	@echo "CLEAN $(BUILD_DIR)"
	@rm -rf $(BUILD_DIR)

# --- 9. 依赖包含 (Dependency Inclusion) ---

# 包含所有自动生成的 .d 依赖文件 (用于头文件)
ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif