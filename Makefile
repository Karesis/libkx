CC = clang
C_STD = -std=c23

SRC_DIR   = src
TEST_DIR  = tests

BUILD_DIR = build
LIB_DIR   = $(BUILD_DIR)/lib
OBJ_DIR   = $(BUILD_DIR)/obj
TEST_BIN_DIR = $(BUILD_DIR)/tests
TEST_OBJ_DIR = $(BUILD_DIR)/obj/tests

CPPFLAGS = -I$(SRC_DIR) -MMD -MP
CFLAGS   = -g -Wall -Wextra -pedantic $(C_STD) -O2

CFLAGS   += -mavx2
CFLAGS   += -Wno-gnu-statement-expression
CFLAGS   += -Wno-gnu-alignof-expression

CPPFLAGS += -DXXH_VECTOR=XXH_AVX2
CPPFLAGS += -DXXH_STATIC_LINKING_ONLY=1
CPPFLAGS += -DXXH_INLINE_ALL=1

LDFLAGS  = -g
LDLIBS   =

TARGET_LIB = $(LIB_DIR)/libkx.a

LIB_SRCS = $(shell find $(SRC_DIR) -name '*.c')
TEST_RUNNER_SRCS = $(shell find $(TEST_DIR) -name '*.c')

LIB_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(LIB_SRCS))
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c, $(TEST_OBJ_DIR)/%.o, $(TEST_RUNNER_SRCS))
TEST_TARGETS = $(patsubst $(TEST_OBJ_DIR)/%.o, $(TEST_BIN_DIR)/%, $(TEST_OBJS))
DEPS = $(LIB_OBJS:.o=.d) $(TEST_OBJS:.o=.d)

.PHONY: all lib tests run_tests clean
all: lib tests
lib: $(TARGET_LIB)
tests: $(TEST_TARGETS)

$(TARGET_LIB): $(LIB_OBJS)
	@echo "AR  $@"
	@mkdir -p $(LIB_DIR)
	@ar rcs $@ $(LIB_OBJS)

BUMP_OBJ = $(OBJ_DIR)/std/alloc/bump.o

ifeq ($(OS),Windows_NT)
  $(BUMP_OBJ): CFLAGS := $(CFLAGS)
else
  $(BUMP_OBJ): CFLAGS := $(CFLAGS) -D_POSIX_C_SOURCE=200809L
endif

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "CC  $<"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	@echo "CC  (Test) $<"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

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

$(TEST_BIN_DIR)/%: $(TEST_OBJ_DIR)/%.o $(TARGET_LIB)
	@echo "LINK $@"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -o $@ \
		-Wl,--start-group $(TARGET_LIB) -Wl,--end-group \
		$(LDFLAGS) $(LDLIBS)

clean:
	@echo "CLEAN $(BUILD_DIR)"
	@rm -rf $(BUILD_DIR)

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif