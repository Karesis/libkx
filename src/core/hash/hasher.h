#pragma once

#include <core/type.h>

/**
 * @brief (Trait) Hasher 状态机接口。
 *
 * 这是一个 "Trait" 宏，它定义了任何 Hasher 实现（例如
 * "DefaultHasher"）
 * 必须提供的函数接口，以及自动派生的便捷函数。
 *
 * 任何实现了此 Trait 的类型 Hasher 都会获得：
 * 1. 必须实现的 (由 Impl 提供):
 * - void Hasher_write(Hasher* self, const void*
 * bytes, usize len)
 * - u64  Hasher_finish(const Hasher* self)
 *
 * 2. 自动派生的 (本宏提供):
 * - void Hasher_write_u8(Hasher* self, u8 val)
 * - void Hasher_write_u16(Hasher* self, u16 val)
 * - ... (以及所有其他你提供的基本类型)
 *
 * @param Hasher Hasher 的具体类型 (例如: DefaultHasher)
 */
#define DEFINE_HASHER_TRAIT(Hasher)                                        \
  /* --- 核心 Trait 函数 (必须由 Impl 提供) --- */                         \
                                                                           \
  /**                                                                      \
   * @brief (Impl) 向 Hasher 状态机喂入一块原始字节数据。   \
   * @param self Hasher 实例指针                                       \
   * @param bytes 指向要哈希的数据的指针                        \
   * @param len 要哈希的数据长度 (字节)                          \
   */                                                                      \
  void Hasher##_write(                                                     \
    Hasher *self, const void *bytes, usize len);                           \
                                                                           \
  /**                                                                      \
   * @brief (Impl) 完成哈希计算并返回最终的 64 位哈希值。 \
   * @note                                                                 \
   * 这是一个"完成"操作。在 finish                               \
   * 之后，Hasher 状态通常被认为是已消耗或需要重置。   \
   * @param self Hasher 实例指针 (通常为 const)                     \
   * @return 64 位的哈希结果                                         \
   */                                                                      \
  u64 Hasher##_finish(const Hasher *self);                                 \
                                                                           \
  /* --- 派生的 Trait 函数 (自动提供) --- */                               \
                                                                           \
  /** (Trait) 向 Hasher 喂入一个 u8。 */                                   \
  static inline void Hasher##_write_u8(Hasher *self,                       \
                                       u8 val)                             \
  {                                                                        \
    Hasher##_write(self, &val, sizeof(val));                               \
  }                                                                        \
                                                                           \
  /** (Trait) 向 Hasher 喂入一个 u16。 */                                  \
  static inline void Hasher##_write_u16(Hasher *self,                      \
                                        u16 val)                           \
  {                                                                        \
    Hasher##_write(self, &val, sizeof(val));                               \
  }                                                                        \
                                                                           \
  /** (Trait) 向 Hasher 喂入一个 u32。 */                                  \
  static inline void Hasher##_write_u32(Hasher *self,                      \
                                        u32 val)                           \
  {                                                                        \
    Hasher##_write(self, &val, sizeof(val));                               \
  }                                                                        \
                                                                           \
  /** (Trait) 向 Hasher 喂入一个 u64。 */                                  \
  static inline void Hasher##_write_u64(Hasher *self,                      \
                                        u64 val)                           \
  {                                                                        \
    Hasher##_write(self, &val, sizeof(val));                               \
  }                                                                        \
                                                                           \
  /** (Trait) 向 Hasher 喂入一个 i8。 */                                   \
  static inline void Hasher##_write_i8(Hasher *self,                       \
                                       i8 val)                             \
  {                                                                        \
    Hasher##_write(self, &val, sizeof(val));                               \
  }                                                                        \
                                                                           \
  /** (Trait) 向 Hasher 喂入一个 i16。 */                                  \
  static inline void Hasher##_write_i16(Hasher *self,                      \
                                        i16 val)                           \
  {                                                                        \
    Hasher##_write(self, &val, sizeof(val));                               \
  }                                                                        \
                                                                           \
  /** (Trait) 向 Hasher 喂入一个 i32。 */                                  \
  static inline void Hasher##_write_i32(Hasher *self,                      \
                                        i32 val)                           \
  {                                                                        \
    Hasher##_write(self, &val, sizeof(val));                               \
  }                                                                        \
                                                                           \
  /** (Trait) 向 Hasher 喂入一个 i64。 */                                  \
  static inline void Hasher##_write_i64(Hasher *self,                      \
                                        i64 val)                           \
  {                                                                        \
    Hasher##_write(self, &val, sizeof(val));                               \
  }                                                                        \
                                                                           \
  /** (Trait) 向 Hasher 喂入一个 f32。 */                                  \
  static inline void Hasher##_write_f32(Hasher *self,                      \
                                        f32 val)                           \
  {                                                                        \
    /* 注意: Hashing 浮点数通常通过其                            \
     * (IEEE 754) 字节表示来完成。*/                               \
    /* 这意味着 -0.0 和 0.0                                           \
     * 可能会有不同的哈希值，这取决于实现。*/            \
    Hasher##_write(self, &val, sizeof(val));                               \
  }                                                                        \
                                                                           \
  /** (Trait) 向 Hasher 喂入一个 f64。 */                                  \
  static inline void Hasher##_write_f64(Hasher *self,                      \
                                        f64 val)                           \
  {                                                                        \
    Hasher##_write(self, &val, sizeof(val));                               \
  }                                                                        \
                                                                           \
  /** (Trait) 向 Hasher 喂入一个 usize。 */                                \
  static inline void Hasher##_write_usize(Hasher *self,                    \
                                          usize val)                       \
  {                                                                        \
    Hasher##_write(self, &val, sizeof(val));                               \
  }                                                                        \
                                                                           \
  /** (Trait) 向 Hasher 喂入一个指针地址。*/                               \
  static inline void Hasher##_write_ptr(Hasher *self,                      \
                                        const void *ptr)                   \
  {                                                                        \
    /* 哈希指针地址, 而不是它指向的数据 */                                 \
    Hasher##_write(self, &ptr, sizeof(ptr));                               \
  }
