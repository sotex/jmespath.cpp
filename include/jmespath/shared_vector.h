#ifndef SHARED_VECTOR_H
#define SHARED_VECTOR_H

#include <memory>
#include <vector>

#ifndef SC_NOEXCEPT
#define SC_NOEXCEPT noexcept
#endif //!SC_NOEXCEPT

namespace SharedContainer {

/// \class vector uvector.h ustl.h
/// \ingroup Sequences
///
/// \brief STL vector equivalent.
///
/// Provides a typed array-like interface to a managed memory block, including
/// element access, iteration, modification, resizing, and serialization. In
/// this design elements frequently undergo bitwise move, so don't put it in
/// here if it doesn't support it. This mostly means having no self-pointers.
///
template <typename T, typename Allocator = std::allocator<T>>
class vector {
public:
    typedef std::vector<T, Allocator> base_type;
    typedef vector<T, Allocator> this_type;

    typedef typename base_type::value_type value_type;
    typedef typename base_type::allocator_type allocator_type;
    typedef typename base_type::size_type size_type;
    typedef typename base_type::difference_type difference_type;
    typedef typename base_type::reference reference;
    typedef typename base_type::const_reference const_reference;
    typedef typename base_type::pointer pointer;
    typedef typename base_type::const_pointer const_pointer;
    typedef typename base_type::iterator iterator;
    typedef typename base_type::const_iterator const_iterator;
    typedef typename base_type::reverse_iterator reverse_iterator;
    typedef typename base_type::const_reverse_iterator const_reverse_iterator;

private:
    std::shared_ptr<base_type> _m;

public:
    // 从各种数据源构造新容器
    // 使用传入参数，调用std::vector 的构造
    // template<class... Args>
    // inline vector(Args&&... args)
    //   : _m(std::make_shared<base_type>(std::forward<Args>(args)...))
    // {
    //     // Empty
    // }
    inline vector() SC_NOEXCEPT(SC_NOEXCEPT(Allocator()))
        : _m(std::make_shared<base_type>())
    {
    }
    inline explicit vector(const allocator_type& allocator) SC_NOEXCEPT
        : _m(std::make_shared<base_type>())
    {
    }
    inline explicit vector(size_type n, const allocator_type& allocator = Allocator())
        : _m(std::make_shared<base_type>(n))
    {
    }
    inline vector(size_type n, const value_type& value, const allocator_type& allocator = Allocator())
        : _m(std::make_shared<base_type>(n, value))
    {
    }
    inline vector(const this_type& x)
        : _m(x._m)
    {
    }
    inline vector(const this_type& x, const allocator_type& allocator)
        : _m(x._m)
    {
    }
    inline vector(this_type&& x) SC_NOEXCEPT
        : _m(x._m)
    {
    }
    inline vector(this_type&& x, const allocator_type& allocator)
        : _m(x._m)
    {
    }
    inline vector(std::initializer_list<value_type> ilist, const allocator_type& allocator = Allocator())
        : _m(std::make_shared<base_type>(ilist))
    {
    }
    template <typename InputIterator>
    inline vector(InputIterator first, InputIterator last, const allocator_type& allocator = Allocator())
        : _m(std::make_shared<base_type>(first, last))
    {
    }

    inline ~vector() {}

    // 赋值给容器
    this_type& operator=(const this_type& x)
    {
        _m = x._m;
    }
    this_type& operator=(std::initializer_list<value_type> ilist)
    {
        _m = std::make_shared<base_type>(ilist);
    }
    // 此处 C++17 使用
    this_type& operator=(this_type&& x) SC_NOEXCEPT(
        std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value || std::allocator_traits<Allocator>::is_always_equal::value)
    {
        _m = x._m;
    }
    // 此处 C++17 使用
    void swap(this_type& x) SC_NOEXCEPT(
        std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value || std::allocator_traits<Allocator>::is_always_equal::value)
    {
        _m = x._m;
    }

    // 将值赋给容器
    // 下面所有的都可以使用这种方式来转发，但是这会影响到代码补全提示，所以这里写全了每一个
    template <class... Args>
    inline void assign(Args&&... args)
    {
        return _m->assign(std::forward<Args>(args)...);
    }
    void assign(size_type n, const value_type& value)
    {
        return _m->assign(n, value);
    }

    template <typename InputIterator>
    void assign(InputIterator first, InputIterator last)
    {
        return _m->assign(first, last);
    }

    void assign(std::initializer_list<value_type> ilist)
    {
        return _m->assign(ilist);
    }

    inline iterator begin() SC_NOEXCEPT { return _m->begin(); }
    inline const_iterator begin() const SC_NOEXCEPT { return _m->begin(); }
    inline const_iterator cbegin() const SC_NOEXCEPT { return _m->cbegin(); }

    inline iterator end() SC_NOEXCEPT { return _m->end(); }
    inline const_iterator end() const SC_NOEXCEPT { return _m->end(); }
    inline const_iterator cend() const SC_NOEXCEPT { return _m->cend(); }

    inline reverse_iterator rbegin() SC_NOEXCEPT { return _m->rbegin(); }
    inline const_reverse_iterator rbegin() const SC_NOEXCEPT { return _m->rbegin(); }
    inline const_reverse_iterator crbegin() const SC_NOEXCEPT { return _m->cbegin(); }

    inline reverse_iterator rend() SC_NOEXCEPT { return _m->rend(); }
    inline const_reverse_iterator rend() const SC_NOEXCEPT { return _m->rend(); }
    inline const_reverse_iterator crend() const SC_NOEXCEPT { return _m->crend(); }

    inline bool empty() const SC_NOEXCEPT { return _m->empty(); }
    inline size_type size() const SC_NOEXCEPT { return _m->size(); }
    inline size_type max_size() const SC_NOEXCEPT { return _m->max_size(); }
    inline size_type capacity() const SC_NOEXCEPT { return _m->capacity(); }

    inline void resize(size_type n, const value_type& value) { return _m->resize(n, value); }
    inline void resize(size_type n) { return _m->resize(n); }
    inline void reserve(size_type n) { return _m->reserve(n); }
    inline void set_capacity(size_type n = base_type::npos) { return _m->set_capacity(n); }
    inline void shrink_to_fit() { return _m->shrink_to_fit(); }

    inline pointer data() SC_NOEXCEPT { return _m->data(); }
    inline const_pointer data() const SC_NOEXCEPT { return _m->data(); }

    inline reference operator[](size_type n) { return _m->operator[](n); }
    inline const_reference operator[](size_type n) const { return _m->operator[](n); }

    inline reference at(size_type n) { return _m->at(n); }
    inline const_reference at(size_type n) const { return _m->at(n); }

    inline reference front() { return _m->front(); }
    inline const_reference front() const { return _m->front(); }

    inline reference back() { return _m->back(); }
    inline const_reference back() const { return _m->back(); }

    inline void push_back(const value_type& value) { return _m->push_back(value); }
    inline reference push_back() { return _m->push_back(); }
    inline void push_back(value_type&& value) { return _m->push_back(value); }
    inline void pop_back() { return _m->pop_back(); }

    template <class... Args>
    inline iterator emplace(const_iterator pos, Args&&... args)
    {
        return _m->emplace(pos, std::forward<Args>(args)...);
    }

    // C++17 之前返回 void，C++17开始返回 reference
    #if __cplusplus < 201703L
    template<class... Args>
    inline void emplace_back(Args&&... args)
    {
        return _m->emplace_back(std::forward<Args>(args)...);
    }
    #else
    template<class... Args>
    inline reference emplace_back(Args&&... args)
    {
        return _m->emplace_back(std::forward<Args>(args)...);
    }
    #endif

    inline iterator insert(const_iterator pos, const value_type& value) { return _m->insert(pos, value); }
    inline iterator insert(const_iterator pos, size_type n, const value_type& value) { return _m->insert(pos, n, value); }
    inline iterator insert(const_iterator pos, value_type&& value) { return _m->insert(pos, value); }
    inline iterator insert(const_iterator pos, std::initializer_list<value_type> ilist) { return _m->insert(pos, ilist); }

    template <typename InputIterator>
    inline iterator insert(const_iterator pos, InputIterator first, InputIterator last)
    {
        return _m->insert(pos, first, last);
    }

    inline iterator erase(const_iterator pos) { return _m->erase(pos); }
    inline iterator erase(const_iterator first, const_iterator last) { return _m->erase(first, last); }

    inline reverse_iterator erase(const_reverse_iterator pos) { return _m->erase(pos); }
    inline reverse_iterator erase(const_reverse_iterator first, const_reverse_iterator last) { return _m->erase(first, last); }

    inline void clear() SC_NOEXCEPT
    {
        if (_m.use_count() == 1) {
            _m->clear();
        }
    }
};
} // namespace SharedContainer

namespace std {
// 为 std::vector 特化 std::swap 算法。交换 lhs 与 rhs 的内容。调用 lhs.swap(rhs) 。
// C++17 添加 SC_NOEXCEPT
template <class T, class Alloc>
void swap(SharedContainer::vector<T, Alloc>& lhs, SharedContainer::vector<T, Alloc>& rhs) SC_NOEXCEPT
{
    lhs.swap(rhs);
}
}

#endif //!SHARED_VECTOR_H