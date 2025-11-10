# ===================================================================
# libkx Makefile (V4 - 2025-11-10)
#
# 架构: (同 V3)
# 目标 (Targets): (同 V3)
#
# (V4 升级):
# 1. 将 xxHash 标志添加到 CFLAGS 和 CPPFLAGS。
# 2. 将测试构建分为 (Compile .c -> .o) 和 (Link .o -> bin)。
# ===================================================================

# --- 1. 配置 (Configuration) ---

# 编译器和标准
CC = clang
C_STD = -std=c23

# 目录 (无变化)
SRC_DIR   = src
TEST_DIR  = tests

BUILD_DIR = build
LIB_DIR   = $(BUILD_DIR)/lib
OBJ_DIR   = $(BUILD_DIR)/obj
TEST_BIN_DIR = $(BUILD_DIR)/tests
#(V4) 新增: 测试的 .o 文件目录
TEST_OBJ_DIR = $(BUILD_DIR)/obj/tests

# 编译标志
# (V4) CPPFLAGS: 只放预处理器标志 (-I, -D, -M)
CPPFLAGS = -I$(SRC_DIR) -MMD -MP
# (V4) CFLAGS: 只放编译器标志 (-g, -O, -std, -W)
CFLAGS   = -g -Wall -Wextra -pedantic $(C_STD) -O2

# --- (V4) libkx 核心依赖标志 (xxHash) ---
# 这些标志会被 *所有* 编译 (库 和 测试) 所需
CFLAGS   += -mavx2
CPPFLAGS += -DXXH_VECTOR=XXH_AVX2
CPPFLAGS += -DXXH_STATIC_LINKING_ONLY=1
CPPFLAGS += -DXXH_INLINE_ALL=1
# ---

# 链接器标志 (LDFLAGS 通常用于 -L, -g 等)
LDFLAGS  = -g
# 链接库 (LDLIBS 通常用于 -l)
LDLIBS   =

# --- 2. 目标 (Targets) ---

# 目标静态库 (无变化)
TARGET_LIB = $(LIB_DIR)/libkx.a

# --- 3. 源文件发现 (Source File Discovery) ---

# (无变化)
LIB_SRCS = $(shell find $(SRC_DIR) -name '*.c')
TEST_RUNNER_SRCS = $(shell find $(TEST_DIR) -name '*.c')

# --- 4. 对象和依赖 (Object & Dependency Mapping) ---

# (无变化) .c -> .o 映射 (用于 libkx.a)
LIB_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(LIB_SRCS))

# (V4) .c -> .o 映射 (用于测试运行器)
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c, $(TEST_OBJ_DIR)/%.o, $(TEST_RUNNER_SRCS))

# (V4) .c (runner) -> 可执行文件 映射
TEST_TARGETS = $(patsubst $(TEST_OBJ_DIR)/%.o, $(TEST_BIN_DIR)/%, $(TEST_OBJS))

# (V4) .d (依赖文件) 映射 (现在包含 库 和 测试)
DEPS = $(LIB_OBJS:.o=.d) $(TEST_OBJS:.o=.d)

# --- 5. 核心构建目标 (Core Build Targets) ---

# (无变化)
all: lib tests
.PHONY: all lib tests run_tests clean
lib: $(TARGET_LIB)
tests: $(TEST_TARGETS)

# (无变化)
$(TARGET_LIB): $(LIB_OBJS)
	@echo "AR  $@"
	@mkdir -p $(LIB_DIR)
	@ar rcs $@ $(LIB_OBJS)

# --- 6. 编译规则 (Compilation Rules) ---

# (V4) 找到 bump.o 的正确路径 (无变化)
BUMP_OBJ = $(OBJ_DIR)/std/alloc/bump.o

# (V4) *覆盖* bump.o 的 CFLAGS (无变化)
ifeq ($(OS),Windows_NT)
  $(BUMP_OBJ): CFLAGS := $(CFLAGS)
else
  $(BUMP_OBJ): CFLAGS := $(CFLAGS) -D_POSIX_C_SOURCE=200809L
endif

# 模式规则: 如何从 src/ .c 编译 .o 文件
# (V4) 添加了 $(CPPFLAGS)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "CC  $<"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# (V4) 新增: 模式规则: 如何从 tests/ .c 编译 .o 文件
# 这将确保 tests/test_hashmap.c 
# 在编译时能获取到所有 $(CPPFLAGS) (包括 -DXXH...=1)
$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	@echo "CC  (Test) $<"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# --- 7. 测试目标 (Test Targets) ---

# (无变化)
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

# (V4) 模式规则: 如何 *链接* 一个测试可执行文件
#
# 依赖:
# 1. $^ (第一个依赖)  (e.g., build/obj/tests/test_hashmap.o)
# 2. $^ (第二个依赖)  (e.g., build/lib/libkx.a)
#
$(TEST_BIN_DIR)/%: $(TEST_OBJ_DIR)/%.o $(TARGET_LIB)
	@echo "LINK $@"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $^ -o $@ \
	  $(LDFLAGS) $(LDLIBS)

# --- 8. 清理 (Clean) ---

# (无变化)
clean:
	@echo "CLEAN $(BUILD_DIR)"
	@rm -rf $(BUILD_DIR)

# --- 9. 依赖包含 (Dependency Inclusion) ---

# (无变化, 但 DEPS 变量现在更大了)
ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif