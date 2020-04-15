#ifndef JSON_TYPES_H
#define JSON_TYPES_H

#include <jmespath/shared_map.h>
#include <jmespath/shared_vector.h>
#include <nlohmann/json.hpp>

namespace jmespath {
typedef nlohmann::basic_json<SharedContainer::unordered_map, SharedContainer::vector, std::string> JSONType;
} /* ! namespace jmespath*/

// 必须特化 destroy 函数，不然 object/array 里面的元素会被 move 走
template <>
void jmespath::JSONType::json_value::destroy(value_t t) noexcept
{
    switch (t) {
    case value_t::object: {
        std::allocator<object_t> alloc;
        std::allocator_traits<decltype(alloc)>::destroy(alloc, object);
        std::allocator_traits<decltype(alloc)>::deallocate(alloc, object, 1);
        break;
    }

    case value_t::array: {
        std::allocator<array_t> alloc;
        std::allocator_traits<decltype(alloc)>::destroy(alloc, array);
        std::allocator_traits<decltype(alloc)>::deallocate(alloc, array, 1);
        break;
    }

    case value_t::string: {
        std::allocator<string_t> alloc;
        std::allocator_traits<decltype(alloc)>::destroy(alloc, string);
        std::allocator_traits<decltype(alloc)>::deallocate(alloc, string, 1);
        break;
    }

    default: {
        break;
    }
    }
}

// 特化 std::swap 和 std::hash
// specialization of std::swap, and std::hash
namespace std {

/// hash value for JSON objects
template <>
struct hash<jmespath::JSONType> {
    /*!
    @brief return a hash value for a JSON object
    @since version 1.0.0
    */
    std::size_t operator()(const jmespath::JSONType& j) const
    {
        // 通过字符串表示法进行的散列
        // a naive hashing via the string representation
        const auto& h = hash<jmespath::JSONType::string_t>();
        return h(j.dump());
    }
};
/*!
@brief exchanges the values of two JSON objects
@since version 1.0.0
*/
template <>
inline void
swap<jmespath::JSONType>(jmespath::JSONType& j1, jmespath::JSONType& j2) noexcept(
    is_nothrow_move_constructible<jmespath::JSONType>::value and
        is_nothrow_move_assignable<jmespath::JSONType>::value)
{
    OUTFUNCINFO
    j1.swap(j2);
}

/// specialization for std::less<value_t>
/// @note: do not remove the space after '<',
///        see https://github.com/nlohmann/json/pull/679
//template <>
//struct less<jmespath::JSONType::value_t> {
//    /*!
//    @brief compare two value_t enum values
//    @since version 3.0.0
//    */
//    bool operator()(jmespath::JSONType::value_t lhs,
//        jmespath::JSONType::value_t rhs) const noexcept
//    {
//        return nlohmann::detail::operator<(lhs, rhs);
//    }
//};

} // namespace std

#endif /* !JSON_TYPES_H */
