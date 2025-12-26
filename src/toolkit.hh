// ( =< C++23 ) Dev toolkit

#include <bitset>
#include <cstdint>
#include <memory>
#include <string>
#include <array>
#include <cassert>
#include <print>

constexpr static bool DEBUG = false;

using size_type = size_t;

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8  = uint8_t;

using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;
using i8  = int8_t;

using f64 = double;
using f32 = float;

using sstring      = std::string;
using sstring_view = std::string_view;

// Not for the optimization, but for the readibility
#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

// --------------
// ---- Pool ----
// --------------

template <typename T, size_type Capacity>
struct FixedFreelist {
    static_assert(sizeof(T) >= sizeof(T*),
                  "T must be at least as large as a pointer for freelist storage");
    static_assert(alignof(T) >= alignof(T*),
                  "T must have at least pointer alignment for freelist storage");

    struct Slot {
        alignas(T) std::byte storage[sizeof(T)];
    };

    Slot  slots_[Capacity];
    T*    free_head_;

    constexpr FixedFreelist()
        : free_head_(nullptr)
    {
        for (size_type i = Capacity; i > 0; --i) {
            size_type idx = i - 1;
            T*        p   = reinterpret_cast<T*>(slots_[idx].storage);
           *reinterpret_cast<T**>(p) = free_head_;
            free_head_                = p;
        }
    }

    [[nodiscard]] constexpr auto allocate() -> T* {
        if (UNLIKELY(!free_head_)) return nullptr;
        T* p        = free_head_;
        free_head_  = *reinterpret_cast<T**>(p);
        return p;
    }

    constexpr auto deallocate(T* p) -> void {
       *reinterpret_cast<T**>(p) = free_head_;
        free_head_               = p;
    }
};
