#ifndef SHARED_MAP_H
#define SHARED_MAP_H

#include <memory>
#include <utility>
#include <vector>

#ifndef SC_NOEXCEPT
#define SC_NOEXCEPT noexcept
#endif //!SC_NOEXCEPT

namespace SharedContainer {
/// unordered_map
///
/// 这是一个简单的共享hash_map实现，其实也不是map，而是一个具有 unordered_map 接口
/// 的 vector 。这个是因为我们在使用 json 的时候，一般对象的键值对都不是很多（<30），
/// 这时候使用 vector 也不会慢，而且有效利用缓存优势，并且很少有修改的情况，大部分时候
/// 都只是序列化过程中的简单插入。使用 shared_ptr 来避免复制的时候进行拷贝，进行共享
/// 使用，这样可以在内部使用的时候，有效进行缓存和自由组合，避免临时对象的拷贝。
template <typename Key,
    typename T,
    typename Hash = std::hash<Key>,
    typename KeyEqual = std::equal_to<Key>,
    typename Allocator = std::allocator<std::pair<const Key, T>>>
class unordered_map {
public:
    typedef std::vector<std::pair<Key, T>, Allocator> base_type;
    typedef unordered_map<Key, T, Hash, KeyEqual, Allocator> this_type;

    typedef typename base_type::size_type size_type;
    typedef /*typename*/ Key key_type;
    typedef T mapped_type;
    typedef typename base_type::value_type value_type; // NOTE: 'value_type = pair<const key_type, mapped_type>'.
    typedef typename base_type::allocator_type allocator_type;
    // typedef typename base_type::node_type node_type;
    typedef typename base_type::iterator iterator;
    typedef typename base_type::const_iterator const_iterator;
    typedef /*typename*/ std::pair<iterator, bool> insert_return_type;

private:
    std::shared_ptr<base_type> _m;

public:
    /// unordered_map
    ///
    /// Default constructor.
    ///
    explicit unordered_map(const allocator_type& allocator = Allocator())
        : _m(std::make_shared<base_type>())
    {
        // Empty
    }

    /// unordered_map
    ///
    /// Constructor which creates an empty container, but start with nBucketCount buckets.
    /// We default to a small nBucketCount value, though the user really should manually
    /// specify an appropriate value in order to prevent memory from being reallocated.
    ///
    explicit unordered_map(size_type nBucketCount, const Hash& hashFunction = Hash(), const KeyEqual& predicate = KeyEqual(), const allocator_type& allocator = Allocator())
        : _m(std::make_shared<base_type>())
    {
        // Empty
    }

    unordered_map(const this_type& x)
        : _m(x._m)
    {
    }

    unordered_map(this_type&& x)
        : _m(x._m)
    {
    }

    unordered_map(this_type&& x, const allocator_type& allocator)
        : _m(x._m)
    {
    }

    /// unordered_map
    ///
    /// initializer_list-based constructor.
    /// Allows for initializing with brace values (e.g. unordered_map<int, char*> hm = { {3,"c"}, {4,"d"}, {5,"e"} }; )
    ///
    unordered_map(std::initializer_list<value_type> ilist, size_type nBucketCount = 0, const Hash& hashFunction = Hash(), const KeyEqual& predicate = KeyEqual(), const allocator_type& allocator = Allocator())
        : _m(std::make_shared<base_type>(ilist))
    {
        // Empty
    }

    /// unordered_map
    ///
    /// An input bucket count of <= 1 causes the bucket count to be equal to the number of
    /// elements in the input range.
    ///
    template <typename ForwardIterator>
    unordered_map(ForwardIterator first, ForwardIterator last, size_type nBucketCount = 0, const Hash& hashFunction = Hash(), const KeyEqual& predicate = KeyEqual(), const allocator_type& allocator = Allocator())
        : _m(std::make_shared<base_type>(first, last))
    {
        // Empty
    }

    // 替换容器内容
    inline this_type& operator=(const this_type& other)
    {
        _m = other._m;
        return *this;
    }
    inline this_type& operator=(this_type&& other)
    {
        _m = other._m;
        return *this;
    }
    inline this_type& operator=(std::initializer_list<value_type> ilist)
    {
        _m = std::make_shared<base_type>(ilist);
        return *this;
    }

    // 获取首位迭代器
    inline iterator begin() SC_NOEXCEPT
    {
        return _m->begin();
    }
    inline const_iterator begin() const SC_NOEXCEPT
    {
        return _m->begin();
    }
    inline const_iterator cbegin() const SC_NOEXCEPT
    {
        return _m->cbegin();
    }
    inline iterator end() SC_NOEXCEPT
    {
        return _m->end();
    }
    inline const_iterator end() const SC_NOEXCEPT
    {
        return _m->end();
    }
    inline const_iterator cend() const SC_NOEXCEPT
    {
        return _m->cend();
    }
    // 检查容器是否为空(公开成员函数)
    inline bool empty() const SC_NOEXCEPT
    {
        return _m->empty();
    }
    // 返回容纳的元素数(公开成员函数)
    inline size_type size() const SC_NOEXCEPT
    {
        return _m->size();
    }
    // 返回可容纳的最大元素数(公开成员函数)
    inline size_type max_size() const SC_NOEXCEPT
    {
        return _m->max_size();
    }
    // 从容器擦除所有元素。此调用后 size() 返回零。
    // 非法化任何指代所含元素的引用、指针或迭代器。可能亦非法化尾后迭代器。
    inline void clear() SC_NOEXCEPT
    {
        if (_m.use_count() == 1) {
            _m->clear();
        }
    }
    // 将内容与 other 的交换。不在单个元素上调用任何移动、复制或交换操作
    void swap(unordered_map& other) SC_NOEXCEPT
    {
        _m.swap(other._m);
    }
    // 寻找键等于 key 的的元素
    inline iterator find(const Key& key)
    {
        iterator it = begin();
        for (; it != end(); ++it) {
            if (it->first == key) {
                break;
            }
        }
        return it;
    }
    inline const_iterator find(const Key& key) const
    {
        const_iterator it = cbegin();
        for (; it != cend(); ++it) {
            if (it->first == key) {
                break;
            }
        }
        return it;
    }

    // 返回拥有比较等于指定参数 key 的关键的元素数，因为此容器不允许重复故为 1 或 0
    size_type count(const Key& key) const
    {
        size_type c = 0;
        for (const_iterator it = cbegin(); it != cend(); ++it) {
            if (it->first == key) {
                ++c;
            }
        }
        return c;
    }
    // 从容器移除指定的元素。
    //    1) 移除位于 pos 的元素。
    //    2) 移除范围 [first; last) 中的元素，它必须是 *this 中的合法范围。
    //    3) 移除关键等于 key 的元素（若存在一个）。
    // 到被擦除元素的引用和迭代器被非法化。其他迭代器和引用不被非法化。
    // 迭代器 pos 必须合法且可解引用。从而 end() 迭代器（合法，但不可解引用）不能用作 pos 所用的值。
    // 返回值  后随最后被移除的元素的迭代器。
    inline iterator erase(const_iterator pos)
    {
        //if (pos < end()) {
        //    std::swap(_m->back(), *const_cast<iterator>(pos));
        //    _m->pop_back();
        //}
        //return begin();
        return _m->erase(pos);
    }
    inline iterator erase(const_iterator first, const_iterator last)
    {
        return _m->erase(first, last);
    }
    //返回值   被移除的元素数
    inline size_type erase(const key_type& key)
    {
        size_type sz = size();
        for (iterator pos = begin(); pos != end(); ++pos) {
            if (pos->first == key) {
                std::swap(_m->back(), *pos);
                _m->pop_back();
            }
        }
        return sz - size();
    }

    // 插入一个值
    inline insert_return_type insert(const value_type& value)
    {
        return std::make_pair(_m->insert(_m->end(), value), true);
    }
    template <class P>
    inline std::pair<iterator, bool> insert(P&& value) SC_NOEXCEPT(std::is_constructible<value_type, P&&>::value)
    {
        // 确保 value 可以构造一个 value_type 对象
        // static_assert(std::is_constructible<value_type, P&&>::value == true);
        return insert(value_type(value));
    }
    /// insert
    /// 这是一个扩展，用于若 key 不存在则创建一个键值对，值使用默认值。
    inline insert_return_type insert(const key_type& key)
    {
        return insert(value_type(key, T()));
    }
    inline insert_return_type insert(key_type&& key)
    {
        return insert(value_type(std::move(key), T()));
    }
    // 若容器中无拥有该关键的元素，则插入以给定的 args 原位构造的新元素到容器。
    template <class... Args>
    inline insert_return_type emplace(Args&&... args)
    {
        return insert(value_type(std::forward<Args>(args)...));
    }

    // 返回位于指定位置 pos 的元素的引用，有边界检查。
    // 若 pos 不在容器范围内，则抛出 std::out_of_range 类型的异常。
    inline T& at(const key_type& k)
    {
        iterator it = find(k);

        if (it == end()) {
            // throw exeption if exceptions enabled
            throw std::out_of_range("invalid unordered_map<K, T> key");
        }
        // undefined behaviour if exceptions and asserts are disabled and it == end()
        return it->second;
    }

    inline const T& at(const key_type& k) const
    {
        const_iterator it = find(k);

        if (it == end()) {
            // throw exeption if exceptions enabled
            throw std::out_of_range("invalid unordered_map<K, T> key");
        }
        // undefined behaviour if exceptions and asserts are disabled and it == end()
        return it->second;
    }
    // 返回到映射到等于 key 的键的值的引用，若这种键不存在则进行插入。
    inline mapped_type& operator[](const key_type& key)
    {
        iterator it = find(key);
        if (it != end()) {
            return (*it).second;
        }
        _m->emplace_back(std::move(key), mapped_type());
        return _m->back().second;
    }

    inline mapped_type& operator[](key_type&& key)
    {
        iterator it = find(key);
        if (it != end()) {
            return (*it).second;
        }

        _m->emplace_back(key, mapped_type());
        return _m->back().second;
    }

}; // unordered_map
} // namespace SharedContainer

namespace std {

/// unordered_map erase_if
/// 擦除所有满足特定判别标准的元素
/// https://en.cppreference.com/w/cpp/container/unordered_map/erase_if
template <typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename UserPredicate>
void erase_if(SharedContainer::unordered_map<Key, T, Hash, KeyEqual, Allocator>& c, UserPredicate predicate)
{
    // Erases all elements that satisfy the predicate from the container.
    for (auto i = c.begin(), last = c.end(); i != last;) {
        if (predicate(*i)) {
            i = c.erase(i);
        } else {
            ++i;
        }
    }
}

///////////////////////////////////////////////////////////////////////
// global operators  比较 unordered_map 中的值
///////////////////////////////////////////////////////////////////////

template <typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
inline bool operator==(const SharedContainer::unordered_map<Key, T, Hash, KeyEqual, Allocator>& a,
    const SharedContainer::unordered_map<Key, T, Hash, KeyEqual, Allocator>& b)
{
    typedef typename SharedContainer::unordered_map<Key, T, Hash, KeyEqual, Allocator>::const_iterator const_iterator;

    // We implement branching with the assumption that the return value is usually false.
    if (a.size() != b.size())
        return false;

    // For map (with its unique keys), we need only test that each element in a can be found in b,
    // as there can be only one such pairing per element. multimap needs to do a something more elaborate.
    for (const_iterator ai = a.begin(), aiEnd = a.end(), biEnd = b.end(); ai != aiEnd; ++ai) {
        const_iterator bi = b.find(ai->first);

        if ((bi == biEnd) || !(*ai == *bi)) // We have to compare the values, because lookups are done by keys alone but the full value_type of a map is a key/value pair.
            return false; // It's possible that two elements in the two containers have identical keys but different values.
    }

    return true;
}

template <typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
inline bool
operator!=(const SharedContainer::unordered_map<Key, T, Hash, KeyEqual, Allocator>& a,
    const SharedContainer::unordered_map<Key, T, Hash, KeyEqual, Allocator>& b)
{
    return !(a == b);
}

} /* ! namespace std*/

#endif //!SHARED_MAP_H