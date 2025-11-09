/* include/std/fmt/sink_file.h */
#pragma once

#include <core/fmt/vformat.h> // L1 引擎
#include <core/type.h>        // L0 类型
#include <stdio.h>            // C 库: FILE*

/* --- FILE* 专属适配器 --- */
static inline void
file_sink_push_char_adapter(void *sink, char c)
{
  fputc(c, (FILE *)sink);
}
static inline void
file_sink_push_bytes_adapter(void *sink,
                             const char *bytes,
                             usize len)
{
  fwrite(bytes, 1, len, (FILE *)sink);
}

/* --- 底层格式化宏 (只支持 FILE*) --- */

/**
 * @brief (底层 API) 格式化到一个 FILE* (例如 stderr)。
 *
 * 这是 panic 和 dbg 应该使用的宏。
 * 它不依赖 sstring/bstring。
 */
#define format_to_file(sink, fmt, ...)                     \
  vformat_func((void *)(sink),                             \
               file_sink_push_char_adapter,                \
               file_sink_push_bytes_adapter,               \
               (fmt),                                      \
               ARGS_COUNT(__VA_ARGS__) __VA_OPT__(, )      \
                 EXPAND_ALL(TYPE_INFO, __VA_ARGS__))
