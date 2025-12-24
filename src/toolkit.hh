// ( =< C++23 ) Dev toolkit

#include <cstdint>
#include <memory>
#include <string>
#include <concepts>
#include <array>
#include <cassert>
#include <print>
#include <type_traits>

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

// MAP STUFF
template<typename Map>
concept isMap = requires {
    typename Map::key_type;
    typename Map::mapped_type;
    typename Map::value_type;
} && std::same_as<typename Map::value_type, std::pair<const typename Map::key_type, typename Map::mapped_type>>;

template<isMap Map>
constexpr inline auto get_or_create( Map& map, const typename Map::key_type& key) noexcept
-> Map::value_type&& {
    auto [it, inserted] = map.try_emplace(key);
    return it->second;
}

// Meta

template<typename ...T>
struct TypeList {};

template<template<typename> class F, typename List>
struct Map;

template<template<typename> class F, typename ...Ts>
struct Map<F, TypeList<Ts...>> {
    using type = TypeList<F<Ts>...>;
};

// --------------
// ---- Pool ----
// --------------

// Inherit from:
struct PoolObj {
    bool active = false;
};

template<typename T>
concept isPoolable = requires (T t) {
    { t.active } -> std::convertible_to<bool>;
};

template<isPoolable T, size_t CAPACITY>
class Pool {
public:
    using Handle = size_t;
    static constexpr Handle INVALID_HANDLE = static_cast<Handle>(-1);

    Pool() noexcept {
        for(size_t i = 0; i < CAPACITY; ++i) {
            _free_list[i] = static_cast<Handle>((CAPACITY - 1) - i);
        }
    }

    ~Pool() {
        for(auto& elem : _data) {
            if(elem.active) {
                elem.~T();
            }
        }
    }

    template<typename... Args>
    [[nodiscard]] auto create( Args&&... args ) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    -> Handle {
        if(UNLIKELY(_num_free == 0)) {
            return INVALID_HANDLE;
        }

        const Handle index = _free_list[--_num_free];
        
        // perfect forwarding
        std::construct_at(&_data[index], args... ); // new (&_data[index]) T {std::forward<Args>(args)...};
        _data[index].active = true;

        return index;
    }

    auto destroy( Handle handle ) noexcept
    -> void {
        if(UNLIKELY(handle >= CAPACITY || !_data[handle].active)) {
            return;
        }

        std::destroy_at(&_data[handle]); // _data[handle].~T();
        _data[handle].active = false;
        _free_list[_num_free++] = handle;
    }

    [[nodiscard]] auto get( Handle handle ) noexcept
    -> T* {
        if(UNLIKELY(handle >= CAPACITY || !_data[handle].active)) {
            std::println("UNABLE TO RETRIEVE POOL OBJ (id: {}) ", handle);
            return nullptr;
        }
        return &_data[handle];
    }

    [[nodiscard]] auto operator[]( Handle handle ) noexcept
    -> T& {
        assert(handle < CAPACITY && _data[handle].active);
        return _data[handle];
    }

    template<typename Func>
    auto for_each( Func&& func )
    -> void {
        for(auto& elem : _data) {
            if(LIKELY(elem.active)) {
                func(elem);
            }
        }
    }

    [[nodiscard]] size_t size    () const noexcept { return CAPACITY - _num_free; }
    [[nodiscard]] size_t capacity() const noexcept { return CAPACITY; }
    [[nodiscard]] bool   full    () const noexcept { return _num_free == 0; }
    [[nodiscard]] bool   empty   () const noexcept { return _num_free == CAPACITY; }

private:
    std::array<T, CAPACITY>      _data;
    std::array<Handle, CAPACITY> _free_list;
    size_t                       _num_free = CAPACITY;
};
