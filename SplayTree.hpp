/*
 *  C++ Program to Implement Splay Tree
 *  Adapted from https://www.sanfoundry.com/cpp-program-implement-splay-tree/ by Matthew Varendorff on 26/2/2024.
 */
#ifndef SPLAYTREE_HPP
#define SPLAYTREE_HPP
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <algorithm>
using namespace std;

#ifndef MAX_ALIGNMENT
#define MAX_ALIGNMENT
template <typename... Args>
constexpr size_t max_alignment() {
    return std::max({alignof(Args)...});
}
#endif

template <typename T>
concept ComparableKeyType = requires(T a, T b)
{
    {a < b} -> std::convertible_to<bool>;
    {a > b} -> std::convertible_to<bool>;
    {a == b} -> std::convertible_to<bool>;
};


template <ComparableKeyType KeyType, typename ValueType>
class SplayTree
{
    enum class Sentinal { END, REND, NONE };
    static constexpr size_t max_alignment_value = max_alignment<KeyType, ValueType, void*[2], Sentinal>();
public:
    struct alignas(max_alignment_value) splay
    {
        splay() noexcept = default;
        splay(KeyType key, ValueType value) noexcept
        : key(key), value(value), children{nullptr, nullptr}, sentinal(Sentinal::NONE)
        {}
        ~splay() = default;
        
        KeyType key;
        ValueType value;
        splay* children[2] = {nullptr, nullptr};
        Sentinal sentinal = Sentinal::NONE;
        
        constexpr bool operator==(const splay& rhs) const noexcept
        {
            return key == rhs.key
            && sentinal == rhs.sentinal;
            //not checking children and value
        }
        
        constexpr bool operator!=(const splay& rhs) const noexcept
        {
            return !(*this == rhs);
        }
    private:
        splay(Sentinal sentinal) noexcept : key(), value(), children{nullptr, nullptr} , sentinal(sentinal) {}
        friend SplayTree;
    };
    enum Child { LEFT = 0, RIGHT = 1};
    

private:
    static constexpr size_t max_alignment_value_iterator = max_alignment<void*>();
    enum class IteratorDirection { FORWARD, REVERSE};
    template<IteratorDirection direction>
    class alignas( max_alignment_value_iterator) Xiterator
    {
    private:
        splay* _node = nullptr;
        const SplayTree* _tree = nullptr;
        
        Xiterator(splay* node, const SplayTree* tree) noexcept
        : _node(node), _tree(tree)
        {}
    public:
        Xiterator() noexcept = default;  //todo: shold probaly initalize to end() or rend()
        Xiterator& operator++() noexcept
        {
            if(_node == nullptr || _tree == nullptr || *_node == _tree->_end || *_node == _tree->_rend)
                return *this;
            
            //splay tree to traverse
            if constexpr (direction == IteratorDirection::FORWARD)
            {
                if(_node->children[RIGHT] == nullptr )
                    _node = const_cast<splay*>(&_tree->_end);
                //make sure node is at the root
                else if(_tree->find(_node->key) != _tree->end())
                    _node = _tree->_rotateToNextLarger();
            }
            else
            {
                if(_node->children[LEFT] == nullptr )
                    _node = const_cast<splay*>(&_tree->_rend);
                //make sure node is at the root
                else if(_tree->find(_node->key) != _tree->end())
                    _node = _tree->_rotateToNextSmaller();
            }
            return *this;
        }
        Xiterator& operator--() noexcept
        {
            if(_node == nullptr || _tree == nullptr || *_node == _tree->_end || *_node == _tree->_rend)
                return *this;
            
            //splay tree to traverse
            if constexpr (direction == IteratorDirection::FORWARD)
            {
                if(_node->children[LEFT] == nullptr )
                    _node = const_cast<splay*>(&_tree->_rend);
                //make sure node is at the root
                else if(_tree->find(_node->key) != _tree->end())
                    _node = _tree->_rotateToNextSmaller();
            }
            else
            {
                if(_node->children[RIGHT] == nullptr )
                    _node = const_cast<splay*>(&_tree->_end);
                //make sure node is at the root
                else if(_tree->find(_node->key) != _tree->end())
                    _node = _tree->_rotateToNextLarger();
            }
            return *this;
        }
        //pre-increment
        Xiterator operator++(int) noexcept
        {
            Xiterator tmp = *this;
            ++(*this);
            return tmp;
        }
        //pre-decrement
        Xiterator operator--(int) noexcept
        {
            Xiterator tmp = *this;
            --(*this);
            return tmp;
        }
        constexpr splay* operator->() const noexcept
        {
            return _node;
        }
        constexpr splay* operator*() const noexcept
        {
            return _node;
        }
        constexpr bool operator==(const Xiterator& rhs) const noexcept
        {
            if(_node == nullptr && rhs._node == nullptr)
                return true;
            return *_node == *rhs._node;
        }
        constexpr bool operator!=(const Xiterator& rhs) const noexcept
        {
            if(_node == nullptr && rhs._node == nullptr)
                return false;
            return *_node != *rhs._node;
        }
        friend SplayTree;
    };
public:
    using iterator = Xiterator<IteratorDirection::FORWARD>;
    using reverse_iterator = Xiterator<IteratorDirection::REVERSE>;
    
    
    SplayTree() noexcept = default;
    ~SplayTree() noexcept
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
    }
    constexpr splay* root() const noexcept
    {
        return _root;
    }
    constexpr bool empty() const noexcept
    {
        return _root == nullptr;
    }
    constexpr size_t size() const noexcept
    {
        return _size;
    }
    
    void clear() noexcept
    {
        while(_root != nullptr)
            erase(_root->key);
        _root = nullptr;
        _size = 0;
    }
    
    iterator insert(KeyType key, ValueType value) noexcept
    {
        _root = _insert(key, value, _root);
        return iterator(_root, this);
    }
    
    iterator find(KeyType key) const noexcept
    {
        _root = _find(key, _root);
        if(_root && _root->key != key)
            return _endIt;
        return iterator(_root, this);
    }
    
    iterator erase(KeyType key) noexcept
    {
        _root = _erase(key, _root);
        return iterator(_root, this);;
    }
    
    iterator begin() const noexcept
    {
        if(_root == nullptr)
            return end();
        _root = _getMinimumAndSplay(_root);
        return iterator(_root, this);
    }
    
    reverse_iterator rbegin() const noexcept
    {
        if(_root == nullptr)
            return end();
        _root = _getMaximumAndSplay(_root);
        return iterator(_root, this);
    }
    
    constexpr const iterator& end() const noexcept
    {
        return _endIt;
    }
    
    constexpr const reverse_iterator& rend() const noexcept
    {
        return _rendIt;
    }
    
    void printInOrder() noexcept
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
    mutable splay* _root = nullptr;
    //sentinal node for end()
    const splay _end = splay(Sentinal::END);
    const iterator _endIt = iterator(const_cast<splay*>(&_end), this);
    //sentinal node for rend()
    const splay _rend = splay(Sentinal::REND);
    const reverse_iterator _rendIt = reverse_iterator(const_cast<splay*>(&_rend), this);
    size_t _size = 0;
    
    splay* _insert(KeyType key, ValueType value, splay* root) noexcept
    {
        static splay* p_node = nullptr; //TODO: maybe get rid of this static variable
        if (!p_node)
            p_node = _New_Node(key, value);
        else
            p_node->key = key;
        if (!root)
        {
            root = p_node;
            p_node = nullptr;
            return root;
        }
        root = _splay(key, root);
        /* This is BST that, all keys <= root->key is in root->children[LEFT], all keys >
         root->key is in root->children[RIGHT]. */
        
        // Calculate direction: 0 for LEFT, 1 for RIGHT
        const Child dir = static_cast<Child>(key > root->key);
        const Child other_dir = static_cast<Child>(1 - dir);

        
        if (root != nullptr && key != root->key)
        {
            p_node->children[dir] = root->children[dir];
            p_node->children[other_dir] = root;
            root->children[dir] = nullptr;
            root = p_node;
        }
        p_node = nullptr;
        return root;
    }
    
    splay* _erase(KeyType key, splay* root) noexcept
    {
        if (!root)
            return nullptr;
        if(!(root = _splay(key, root)))
            return nullptr;
        if (key != root->key)
            return root;
        
        splay* temp = root;
        if (!root->children[LEFT])
            root = root->children[RIGHT];
        else
        {
            /*Note: Since key == root->key,
             so after Splay(key, root->children[LEFT]),
             the tree we get will have no right child tree.*/
            root = _splay(key, root->children[LEFT]);
            root->children[RIGHT] = temp->children[RIGHT];
        }
        --_size;
        delete temp;
        return root;
    }
    splay* _find(KeyType key, splay* root) const noexcept
    {
        return _splay(key, root);
    }
    // RR(Y rotates to the right)
    splay* _RR_Rotate(splay* k2) const noexcept
    {
        splay* k1 = k2->children[LEFT];
        k2->children[LEFT] = k1->children[RIGHT];
        k1->children[RIGHT] = k2;
        return k1;
    }
    
    // LL(Y rotates to the left)
    splay* _LL_Rotate(splay* k2) const noexcept
    {
        splay* k1 = k2->children[RIGHT];
        k2->children[RIGHT] = k1->children[LEFT];
        k1->children[LEFT] = k2;
        return k1;
    }
    
    // An implementation of top-down splay tree
    splay* _splay(KeyType key, splay* root) const noexcept
    {
        if (!root)
            return nullptr;
        splay header;
        /* header.children[RIGHT] points to L tree;
         header.children[LEFT] points to R Tree */
        //header.children[LEFT] = header.children[RIGHT] = nullptr;
        splay* treesMinOrMax[2] = { &header, &header };
        
        while (true) {
            // Calculate direction: 0 for LEFT, 1 for RIGHT
            const Child dir = static_cast<Child>(key > root->key);
            const Child other_dir = static_cast<Child>(1 - dir);
            if (key == root->key) {
                break;
            }
            
            if(!root->children[dir] ) {
                break;
            }
            typedef splay* (SplayTree::*RotationFunc)(splay*) const;
            const RotationFunc rotate[2] = { &SplayTree::_RR_Rotate, &SplayTree::_LL_Rotate };
            // Check for zig-zig or zag-zag case
            if ((key < root->children[dir]->key && dir == LEFT) ||
                (key > root->children[dir]->key && dir == RIGHT))
            {
                root = (this->*rotate[dir])(root);
                if (!root->children[dir]) {
                    break;
                }
            }
            
            // Link the current root to the opposite tree
            treesMinOrMax[other_dir]->children[dir] = root;
            treesMinOrMax[other_dir] = treesMinOrMax[other_dir]->children[dir];
            treesMinOrMax[other_dir] = root;
            root = root->children[dir];
            treesMinOrMax[other_dir]->children[dir] = nullptr;
        }
        
        // Assemble left and right trees with the new root
        treesMinOrMax[LEFT]->children[RIGHT] = root->children[LEFT];
        treesMinOrMax[RIGHT]->children[LEFT] = root->children[RIGHT];
        root->children[LEFT] = header.children[RIGHT];
        root->children[RIGHT] = header.children[LEFT];
        
        return root;
    }
    
    

    splay* _New_Node(KeyType key, ValueType value) noexcept
    {
        splay* p_node = new splay(key, value);
        p_node->children[LEFT] = p_node->children[RIGHT] = nullptr;
        ++_size;
        return p_node;
    }
    

    void _printInOrder(splay* root) noexcept
    {
        if (root)
        {
            _printInOrder(root->children[LEFT]);
            cout<< "key: " <<root->key;
            if(root->children[LEFT])
                cout<< " | left child: "<< root->children[LEFT]->key;
            if(root->children[RIGHT])
                cout << " | right child: " << root->children[RIGHT]->key;
            cout<< "\n";
            _printInOrder(root->children[RIGHT]);
        }
    }

    
    splay* _getMinimumAndSplay(splay* root) const  noexcept{
        //perfom zig-zig or zig to move the left most node to the root
        if(root == nullptr) return nullptr;
        
        splay* x = root;
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
    
    
    splay* _getMaximumAndSplay(splay* root) const  noexcept{
        //perfom zag-zag or zag to move the right most node to the root
        if(root == nullptr) return nullptr;
        
        splay* x = root;
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
    splay* _rotateToNextLarger() const noexcept {
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
    splay* _rotateToNextSmaller() const noexcept {
        if (!_root || !_root->children[LEFT]) return _root; // No left subtree, nothing to do
        
        splay* parent = _root;
        splay* child = _root->children[LEFT];
        
        // Find the maximum element in the left subtree, which is the next smaller element
        while (child->children[RIGHT]) {
            parent = child;
            child = child->children[RIGHT];
        }
        
        // Rotate the found node
        if (parent != _root) {
            // Perform the left rotation using _RR_Rotate() function
            parent = _RR_Rotate(parent);
            // Adjust the left child of the new parent node
            parent->children[LEFT] = _root->children[LEFT];
        }
        
        // Set the right subtree of the current root to be the right child of the successor
        parent->children[RIGHT] = _root->children[RIGHT];
        
        // The successor node becomes the new root
        _root = parent;
        return _root;
    }
};
#endif /* SplayTree_h */

