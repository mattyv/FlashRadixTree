/*
 *  C++ Program to Implement Splay Tree
 *  Adapted from https://www.sanfoundry.com/cpp-program-implement-value_type-tree/ by Matthew Varendorff on 26/2/2024.
 */
#ifndef SPLAYTREE_HPP
#define SPLAYTREE_HPP
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <algorithm>
#include "unique_ptr_alloc.hpp"

namespace Flash
{
using namespace std;

template <typename T>
concept ComparableKeyType = requires(T a, T b)
{
    {a < b} -> std::convertible_to<bool>;
    {a > b} -> std::convertible_to<bool>;
    {a == b} -> std::convertible_to<bool>;
};


template <ComparableKeyType key_type, typename mapped_type, typename Allocator = std::allocator<mapped_type>>
class SplayTree
{
public:
    class value_type
    {
    public:
        value_type()  = default;
        value_type( const key_type& key, mapped_type&& val)
        : first(key), second(std::move(val))
        {
        }
        
        value_type( const key_type& key, const mapped_type& val)
        : first(key), second(val)
        {}
        
        value_type(const std::pair<key_type, mapped_type>& pair)
        : first(pair.first), second(pair.second)
        {}
        
        value_type(std::pair<key_type, mapped_type>&& pair)
        : first(std::move(pair.first)), second(std::move(pair.second))
        {}
        
        ~value_type() = default;
        
    private:
        
        value_type(value_type&& other) noexcept
        {
            first = std::move(other.first);
            second = std::move(other.second);
            children[LEFT] = other.children[LEFT];
            children[RIGHT] = other.children[RIGHT];
        }
    public:
        key_type first;
        mapped_type  second;
        using value_type_smart_prt = std::unique_ptr<value_type>;
        value_type_smart_prt children[2];
        
        constexpr bool operator==(const value_type& rhs) const
        {
            return first == rhs.first;
            //not checking children and value
        }
        
        constexpr bool operator!=(const value_type& rhs) const
        {
            return !(*this == rhs);
        }
        
        friend SplayTree;
    };
    enum Child { LEFT = 0, RIGHT = 1};
    using size_type = size_t;
    
private:
    
    enum class IteratorDirection { FORWARD, REVERSE};
    enum class IteratorConstness { CONST, NON_CONST};
    template<IteratorDirection direction, IteratorConstness constness = IteratorConstness::NON_CONST>
    class Xiterator
    {
    private:
        using tree_pointer = std::conditional_t<constness == IteratorConstness::CONST, const SplayTree*, SplayTree*>;
        using value_type_pointer = std::conditional_t<constness == IteratorConstness::CONST, const value_type*, value_type*>;
        using value_type_reference = std::conditional_t<constness == IteratorConstness::CONST, const value_type&, value_type&>;
        using value = std::conditional_t<constness == IteratorConstness::CONST, const Xiterator, Xiterator>;
        using pointer = std::conditional_t<constness == IteratorConstness::CONST, const Xiterator*, Xiterator*>;
        using reference = std::conditional_t<constness == IteratorConstness::CONST, const Xiterator&, Xiterator&>;
        
        value_type_pointer _node = nullptr;
        tree_pointer _tree = nullptr;
        bool _isEnd = true;
        IteratorDirection _direction = direction;
        
        Xiterator(const value_type_pointer node, tree_pointer tree)
        : _node(node), _tree(tree), _isEnd(node == nullptr || tree == nullptr)
        {
        }
        
    public:
        Xiterator()  = default;
        // Default copy constructor - used for same type
        Xiterator(const Xiterator& other) noexcept = default;
        
        Xiterator(Xiterator&& other) noexcept
        {
            _node = std::move(other._node);
            _tree = std::move(other._tree);
            _isEnd = std::move(other._isEnd);
            _direction = std::move(other._direction);
        }
        
        // Default copy assignment operator - used for same type
        Xiterator& operator=(const Xiterator& other) noexcept = default;
        
        // Prevent cross-direction copying and assignment using a deleted function template
        template<IteratorDirection OtherDirection>
        Xiterator(const Xiterator<OtherDirection>&) = delete;
        
        template<IteratorDirection OtherDirection>
        Xiterator& operator=(const Xiterator<OtherDirection>&) = delete;
        
        //converter functions to convert form forward to reverse and vice versa
        //template<IteratorDirection OtherDirection>
        auto get_other_direction() const  noexcept {
            if constexpr (direction == IteratorDirection::FORWARD) {
                return Xiterator<IteratorDirection::REVERSE, constness>(_node, _tree);
            } else {
                return Xiterator<IteratorDirection::FORWARD, constness>(_node, _tree);
            }
        }
        
        
        reference operator++()
        {
            if(_isEnd || _node == nullptr || _tree == nullptr)
            {
                _isEnd = true;
                return *this;
            }
            
            //value_type tree to traverse
            if (_direction == IteratorDirection::FORWARD)
            {
                if(_node->children[RIGHT] == nullptr )
                    _node.reset();
                else if(_tree->get(_node->first) != nullptr)//make sure node is at the root
                    _node = _tree->_rotateToNextLarger();
            }
            else
            {
                if(_node->children[LEFT] == nullptr )
                    _node = nullptr;
                else if(_tree->get(_node->first) != nullptr)
                    _node = _tree->_rotateToNextSmaller();
            }
            _isEnd = _node == nullptr;
            return *this;
        }
        reference operator--()
        {
            if(_isEnd || _node == nullptr || _tree == nullptr)
            {
                _isEnd = true;
                return *this;
            }
            
            //value_type tree to traverse
            if (_direction == IteratorDirection::FORWARD)
            {
                //make sure node is at the root
                if(_node->children[LEFT] == nullptr )
                    _node = nullptr;
                else if(_tree->get(_node->first) != nullptr)
                    _node = _tree->_rotateToNextSmaller();
            }
            else
            {
                if(_node->children[RIGHT] == nullptr )
                    _node = nullptr;
                else if(_tree->get(_node->first) != nullptr)
                    _node = _tree->_rotateToNextLarger();
            }
            _isEnd = _node == nullptr;
            return *this;
        }
        //pre-increment
        value operator++(int)
        {
            Xiterator tmp = *this;
            ++(*this);
            return tmp;
        }
        //pre-decrement
        value operator--(int)
        {
            Xiterator tmp = *this;
            --(*this);
            return tmp;
        }
        constexpr value_type_pointer operator->() const
        {
            return _node;
        }
        constexpr value_type_reference operator*()
        {
            return *_node;
        }
        constexpr bool operator==(const Xiterator& rhs) const
        {
            if(_tree != rhs._tree)
                return false;
            if(_isEnd == rhs._isEnd)
                return true;
            if(_node == nullptr && rhs._node == nullptr)
                return true;
            if(_node == nullptr || rhs._node == nullptr)
                return false;
            return *_node == *rhs._node;
        }
        constexpr bool operator!=(const Xiterator& rhs) const
        {
            if(_tree != rhs._tree)
                return true;
            if(_isEnd != rhs._isEnd)
                return true;
            if(_node == nullptr && rhs._node == nullptr)
                return false;
            if(_node == nullptr || rhs._node == nullptr)
                return true;
            return *_node != *rhs._node;
        }
        friend SplayTree;
    };
public:
    using allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<value_type>;
    using pointer = typename std::allocator_traits<allocator_type>::pointer;
    using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
    using reference = value_type&;
    using pair_type = std::pair<key_type, mapped_type>;
    using pair_reference = pair_type&;
    using const_value_type_reference = const value_type&;
    using value_type_reference = value_type&;
    using value_type_smart_ptr = unique_ptr<value_type>;
    using const_value_type_pointer = const value_type*;
    using value_type_pointer = value_type*;
    using iterator = Xiterator<IteratorDirection::FORWARD>;
    using const_iterator = Xiterator<IteratorDirection::FORWARD, IteratorConstness::CONST>;
    using reverse_iterator = Xiterator<IteratorDirection::REVERSE>;
    using const_reverse_iterator = Xiterator<IteratorDirection::REVERSE, IteratorConstness::CONST>;
    
    
    explicit SplayTree( const allocator_type& allocator = allocator_type())
    : _node_allocator(allocator), _root(make_unique_alloc<value_type>(allocator))
    {}
    ~SplayTree()
    {
        clear();
    }
    
    SplayTree(const SplayTree& ) = delete;
    SplayTree& operator=(const SplayTree& ) = delete;
    
    //move constructor
    SplayTree( SplayTree&& other) noexcept
    {
        _root = other._root;
        _size = other._size;
        other._root = nullptr;
        other._size = 0;
        _node_allocator = std::move(other._node_allocator);
    }
    
    constexpr const allocator_type& get_allocator() const noexcept
    {
        return _node_allocator;
    }
    constexpr const_iterator root() const noexcept
    {
        return iterator(_root.get(), this);
    }
    constexpr iterator root() noexcept
    {
        return iterator(_root.get(), this);
    }
    constexpr bool empty() const  noexcept
    {
        return _root == nullptr;
    }
    constexpr size_type size() const  noexcept
    {
        return _size;
    }
    
    void clear()
    {
        while(_root != nullptr)
            erase(_root->first);
        _root = nullptr;
        _size = 0;
    }
    
    iterator insert(const key_type& key, mapped_type&& value)
    {
        value_type val(key, std::forward<mapped_type>(value));
        _root = std::move(_insert(_root.get(), std::move(val)));
        if(_root == nullptr)
            return end();
        return iterator(_root.get(), this);
    }
    
    //bit of a hack to take a pair but thats not what this class uses under the hood so we just convert.
    //will be perfectly compatablere
    std::pair<iterator, bool> insert(pair_type&& value) //untested
    {
        throw std::runtime_error("not tested");
        auto it = get(value.first);
        if(it != nullptr)
            return std::make_pair(it, false);
        else
        {
            value_type val(std::forward<value_type>(value));
            auto it = insert(std::move(val));
            return std::make_pair(insert(value.first, value.second), true);
        }
    }
    
    template<class... Args>
    std::pair<iterator, bool> emplace(Args&&... args) //untested
    {
        throw std::runtime_error("not tested");
        value_type val(std::forward<Args>(args)...);
        auto it = get(val.first);
        if(it != nullptr)
            return std::make_pair(it, false);
        else
        {
            _root = _insert(_root, std::move(val));
            if(_root == nullptr)
                return std::make_pair(end() ,false);
            return std::make_pair(it, true);
        }
    }
    
    const_iterator find(const key_type& key) const
    {
        _root = _splay(key, _root.get());
        if((_root == nullptr ) || (_root && _root->first != key))
            return _cendIt;
        return const_iterator(_root.get(), this);
    }
    
    iterator find(const key_type& key)
    {
        _root = _splay(key, _root.get());
        if((_root == nullptr ) || (_root && _root->first != key))
            return end();
        return iterator(_root.get(), this);
    }
    
    const value_type_pointer get(const key_type& key) const
    {
        _root = _splay(key, _root.get());
        if((_root == nullptr ) || (_root && _root->first != key))
            return nullptr;
        return _root.get();
    }
    
    value_type_pointer get(const key_type& key)
    {
        _root = _splay(key, _root.get());
        if((_root == nullptr ) || (_root && _root->first != key))
            return nullptr;
        return _root.get();
    }
    
    const value_type_pointer getMinimum() const
    {
        return _getMinimumAndSplay(_root.get());
    }
    
    value_type_pointer getMinimum()
    {
        return _getMinimumAndSplay(_root.get());
    }
    
    const value_type_pointer getMaximum() const
    {
        return _getMaximumAndSplay(_root.get());
    }
    
    const value_type_pointer getMaximum()
    {
        return _getMaximumAndSplay(_root.get());
    }
    
    bool contains(const key_type& key) const
    {
        return get(key) != nullptr;
    }
    
    iterator operator[]( key_type& key) //untested
    {
        throw std::runtime_error("not tested");
        auto it = get(key);
        if(it == nullptr)
            return insert(key, mapped_type());
        return it;
    }
    
    const_iterator operator[]( key_type& key) const //untested
    {
        throw std::runtime_error("not tested");
        auto it = get(key);
        if(it == nullptr)
            return insert(key, mapped_type());
        return it;
    }
    
    iterator find_predecessor(const key_type& key)
    {
        _root = _splay(key, _root.get());
        if(_root == nullptr)
            return end();
        return iterator(_root.get(), this);
    }
    
    const_value_type_pointer get_predecessor(const key_type& key) const
    {
        return _splay(key, _root.get());
    }
    
    const_iterator find_predecessor(const key_type& key) const
    {
        _root = _find(key, _root.get());
        if(_root == nullptr)
            return cend();
        return const_iterator(_root.get(), this);
    }
    
    iterator lower_bound(key_type key)  //untested
    {
        throw std::runtime_error("not tested");
        auto it = find_predecessor(key);
        if(it == end())
            return it;
        else
            return ++it;;
    }
    
    iterator erase(key_type key)
    {
        _root = _erase(key, _root.get());
        if(_root == nullptr)
            return end();
        return iterator(_root.get(), this);;
    }
    
    iterator begin()  noexcept
    {
        if(_root == nullptr)
            return end();
        _root = _getMinimumAndSplay(_root.get());
        return iterator(_root.get(), this);
    }
    
    const_iterator begin() const  noexcept
    {
        if(_root == nullptr)
            return end();
        _root = _getMinimumAndSplay(_root.get());
        return const_iterator(_root.get(), this);
    }
    
    const_iterator cbegin() const  noexcept
    {
        if(_root == nullptr)
            return end();
        _root = _getMinimumAndSplay(_root.get());
        return const_iterator(_root.get(), this);
    }
    
    const_reverse_iterator crbegin() const  noexcept
    {
        if(_root == nullptr)
            return rend();
        _root = _getMaximumAndSplay(_root.get());
        return const_reverse_iterator(_root.get(), this);
    }
    
    reverse_iterator rbegin()  noexcept
    {
        if(_root == nullptr)
            return rend();
        _root = _getMaximumAndSplay(_root.get());
        return reverse_iterator(_root.get(), this);
    }
    
    const_reverse_iterator rbegin() const  noexcept
    {
        if(_root == nullptr)
            return rend();
        _root = _getMaximumAndSplay(_root.get());
        return const_reverse_iterator(_root.get(), this);
    }
    
    constexpr const iterator& end()  noexcept
    {
        return _endIt;
    }
    
    constexpr const const_iterator& end() const  noexcept
    {
        return _cendIt;
    }
    
    constexpr const const_iterator& cend() const  noexcept
    {
        return _cendIt;
    }
    
    constexpr const reverse_iterator& rend()  noexcept
    {
        return _rendIt;
    }
    
    constexpr const const_reverse_iterator& rend() const  noexcept
    {
        return _crendIt;
    }
    
    constexpr const const_reverse_iterator& crend() const  noexcept
    {
        return _crendIt;
    }
    
    void printInOrder()
    {
        _printInOrder(_root);
    }
    
    //move assignment
    SplayTree& operator=( SplayTree&& other) noexcept
    {
        if (this != &other)
        {
            _root = other._root;
            _size = other._size;
            other._root = nullptr;
            other._size = 0;
        }
        return *this;
    }
private:
    using UniquePtrAlloc = std::unique_ptr<value_type, std::function<void(value_type*)>>; //todo move me
    
    mutable UniquePtrAlloc _root;
    //sentinal node for end()
    iterator _endIt = iterator(nullptr, this);
    const_iterator _cendIt = const_iterator(nullptr, this);
    //sentinal node for rend()
    reverse_iterator _rendIt = reverse_iterator(nullptr, this);
    const_reverse_iterator _crendIt = const_reverse_iterator(nullptr, this);
    size_t _size = 0;
    allocator_type _node_allocator;
    
    template<typename... Args>
    value_type_pointer _insert(value_type_pointer root, Args&&... args)
    {
        if (!root) {
            // If there is no root, create a new node and return it.
            return _New_Node(std::forward<Args>(args)...);
        }
        
        // Create a new node as we are sure we need to insert it.
        auto new_node = _New_Node(std::forward<value_type>(args)...);
        // Splay the tree with the given key.
        root = _splay(new_node->first, root);
        
        // If the key is already in the tree, we return the splayed tree without inserting.
        if (new_node->first == root->first) {
            //delete new_node;
            return root;
        }
        //any bad alloc at this point have not made substantial changes so we're ok to let the go uncaught here
        if(! new_node)
            return nullptr;
        
        // Calculate direction: 0 for LEFT, 1 for RIGHT
        const Child dir = static_cast<Child>(new_node->first > root->first);
        const Child other_dir = static_cast<Child>(1 - dir);
        
        
        if (root != nullptr && new_node->first != root->first)
        {
            new_node->children[dir] = root->children[dir];
            root->children[dir] = nullptr;
            new_node->children[other_dir] = root;
        }
        
        return new_node;
    }
    
    void _erase(const key_type& key, value_type_smart_ptr& root)
    {
        _splay(key, root);
        if(!root)
            return;
        if (key != root->first)
            return;
        
        if (!root->children[LEFT])
        {
            root = root->children[RIGHT];
            //implicitly deletes old root.
            --_size;
        }
        else
        {
            /*Note: Since key == root->first,
             so after Splay(key, root->children[LEFT]),
             the tree we get will have no right child tree.*/
            //todo: come back to this
            root = _splay(key, root->children[LEFT]);
            root->children[RIGHT] = temp->children[RIGHT];
        }
        
        //_destroy_node( temp);
        //return root;
    }
    // RR(Y rotates to the right)
    void _RR_Rotate(value_type_smart_ptr& k2) const  noexcept
    {
        //auto k1 = k2->children[LEFT];
        auto k1 = std::move(k2->children[LEFT]);
        k2->children[LEFT] = std::move(k1->children[RIGHT]);
        k1->children[RIGHT] = std::move(k2);
#ifdef __GNUC__
        //prefetch the left child of k2
        if(k2->children[LEFT])
        {
            //expect to read to this locaitoin with low locality
            __builtin_prefetch(k2->children[LEFT].get(), 0, 1);
        }
#endif
        //return k1;
        k2 = std::move(k1);
    }
    
    // LL(Y rotates to the left)
    value_type_pointer _LL_Rotate(value_type_pointer k2) const  noexcept
    {
        //auto k1 = k2->children[RIGHT];
        auto k1 = std::move(k2->children[RIGHT]);
        k2->children[RIGHT] = std::move(k1->children[LEFT]);
        k1->children[LEFT] = std::move(k2);
#ifdef __GNUC__
        //prefetch the right child of k2
        if(k2->children[RIGHT])
        {
            //expect to read to this locaitoin with low locality
            __builtin_prefetch(k2->children[RIGHT].get(), 0, 1);
        }
#endif
        //return k1;
        k2 = std::move(k1);
    }
    
    // An implementation of top-down value_type tree
    void _splay(const key_type& key, value_type_smart_ptr& root) const //todo: come back to this
    {
        if (!root)
            return;
        
        value_type header;
        /* header.children[RIGHT] points to L tree;
         header.children[LEFT] points to R Tree */
        //header.children[LEFT] = header.children[RIGHT] = nullptr;
        value_type_smart_ptr treesMinOrMax[2] = {&header, &header};
        
        while (true) {
            // Calculate direction: 0 for LEFT, 1 for RIGHT
            const Child dir = static_cast<Child>(key > root->first);
            const Child other_dir = static_cast<Child>(1 - dir);
            if (key == root->first) {
                break;
            }
            
            if(!root->children[dir] ) {
                break;
            }
            typedef void (SplayTree::*RotationFunc)(value_type_smart_ptr&) const;
            const RotationFunc rotate[2] = { &SplayTree::_RR_Rotate, &SplayTree::_LL_Rotate };
            // Check for zig-zig or zag-zag case
            if ((key < root->children[dir]->first && dir == LEFT) ||
                (key > root->children[dir]->first && dir == RIGHT))
            {
                (this->*rotate[dir])(root);
                if (!root->children[dir]) {
                    break;
                }
            }
            
            // Link the current root to the opposite tree
            //treesMinOrMax[other_dir]->children[dir] = std::move(root);
            //treesMinOrMax[other_dir] = std::move(treesMinOrMax[other_dir]->children[dir]);
            treesMinOrMax[other_dir] = std::move(root);
            root = root->children[dir];
            treesMinOrMax[other_dir]->children[dir] = nullptr;
            
        }
        
        // Assemble left and right trees with the new root
        treesMinOrMax[LEFT]->children[RIGHT] = root->children[LEFT];
        treesMinOrMax[RIGHT]->children[LEFT] = root->children[RIGHT];
        root->children[LEFT] = header->children[RIGHT];
        root->children[RIGHT] = header->children[LEFT];
        
        return root;
    }
    
    template <typename... Args>
    value_type_smart_ptr _New_Node(Args&&... args)
    {
        value_type* p_node = _node_allocator.allocate(1);
        new (p_node) value_type(std::forward<Args>(args)...);
        ++_size;
        //return std::make_shared<value_type>(p_node);
        return value_type_smart_ptr(p_node);
    }
    
    void _destroy_node(value_type* node)
    {
        if (node != nullptr)
        {
            node->~value_type();                     // Call the destructor for the node
            _node_allocator.deallocate(node, 1); // Deallocate the node's memory
            --_size;
        }
    }
    
    
    void _printInOrder(value_type* root)
    {
        if (root)
        {
            _printInOrder(root->children[LEFT]);
            cout<< "key: " <<root->first;
            if(root->children[LEFT])
                cout<< " | left child: "<< root->children[LEFT]->first;
            if(root->children[RIGHT])
                cout << " | right child: " << root->children[RIGHT]->first;
            cout<< "\n";
            _printInOrder(root->children[RIGHT]);
        }
    }
    
    
    value_type_pointer _getMinimumAndSplay(value_type_pointer root) const  noexcept {
        //perfom zig-zig or zig to move the left most node to the root
        if(root == nullptr) return nullptr;
        
        auto x = root;
        while (x->children[LEFT] != nullptr) {
            if (x->children[LEFT]->children[LEFT] != nullptr) {
                // "Zig-zig" step: make two right rotations
                x->children[LEFT] = _RR_Rotate(x->children[LEFT]);
                if (x->children[LEFT] != nullptr)
                    x = _RR_Rotate(x);
            } else {
                // "Zig" step: a single rotation is enough when there is no left child for the left child of x
                x = _RR_Rotate(x);
            }
        }
        root = x;
        return root;
    }
    
    
    value_type_pointer _getMaximumAndSplay(value_type_pointer root) const  noexcept {
        //perfom zag-zag or zag to move the right most node to the root
        if(root == nullptr) return nullptr;
        
        auto x = root;
        while (x->children[RIGHT] != nullptr) {
            if (x->children[RIGHT]->children[RIGHT] != nullptr) {
                // "Zig-zig" step: make two left rotations
                x->children[RIGHT] = _LL_Rotate(x->children[RIGHT]);
                if (x->children[RIGHT] != nullptr)
                    x = _LL_Rotate(x);
            } else {
                // "Zig" step: a single rotation is enough when there is no right child for the right child of x
                x = _LL_Rotate(x);
            }
        }
        root = x;
        return root;
    }
    
    // Rotate the tree so that the next larger element becomes the root
    value_type_pointer _rotateToNextLarger() const  noexcept {
        // If there's no right subtree, there's no next larger element
        if (!_root || !_root->children[RIGHT])
            return _root;
        
        // The right child with no left child is the next larger element
        if (!_root->children[RIGHT]->children[LEFT]) {
            _root = _LL_Rotate(_root); // Perform the left rotation
        } else { // The next larger element is in the left subtree of the right child
            _root->children[RIGHT] = _RR_Rotate(_root->children[RIGHT]);
            _root = _LL_Rotate(_root);
        }
        return _root;
    }
    
    // Rotate the tree so that the next smaller element becomes the root
    value_type_pointer _rotateToNextSmaller() const noexcept {
        // If there's no left subtree, there's no next smaller element
        if (!_root || !_root->children[LEFT])
            return _root;
        
        // The left child with no right child is the next smaller element
        if (!_root->children[LEFT]->children[RIGHT]) {
            _root = _RR_Rotate(_root); // Perform the right rotation
        } else { // The next smaller element is in the right subtree of the left child
            _root->children[LEFT] = _LL_Rotate(_root->children[LEFT]);
            _root = _RR_Rotate(_root);
        }
        return _root;
    }
};
}
#endif /* SplayTree_h */

