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
 
//key, value and compare function
template <typename KeyType, typename ValueType>
class SplayTree
{
    static constexpr size_t max_alignment_value = max_alignment<KeyType, ValueType, void*[2]>();
    
public:
    struct alignas(max_alignment_value) splay
    {
        splay() noexcept = default;
        splay(KeyType key, ValueType value) noexcept: key(key), value(value), children{nullptr, nullptr}  {}
        
        KeyType key;
        ValueType value;
        splay* children[2] = {nullptr, nullptr};
    };
    enum Child { LEFT = 0, RIGHT };
    
    SplayTree() noexcept = default;
    ~SplayTree() noexcept
    {
        //iterate and delete
        inOrderAndOp([this]( splay* node)noexcept->bool{delete node;return true;});
    }
    
    SplayTree(const SplayTree& ) = delete;
    SplayTree& operator=(const SplayTree& ) = delete;
    
    //move constructor
    SplayTree(const SplayTree&& other) noexcept
    {
        _root = other._root;
        _size = other._size;
        other._root = nullptr;
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
    splay* insert(KeyType key, ValueType value) noexcept
    {
        _root = _insert(key, value, _root);
        return _root;;
    }
    
    splay* find(KeyType key) const noexcept
    {
        _root = _find(key, _root);
        if(_root && _root->key != key)
            return nullptr;
        return _root;
    }
    
    splay* erase(KeyType key) noexcept
    {
        _root = _erase(key, _root);
        return _root;
    }
    
    
    
    void printInOrder() noexcept
    {
        _printInOrder(_root);
    }
    
    splay* getMinimum() const  noexcept{
        _root = _getMinimumAndSplay(_root);
        return _root;
    }
    
    splay* getMaximum() const  noexcept{
        _root = _getMaximumAndSplay(_root);
        return _root;
    }
    void inOrderAndOp(std::function<bool( splay*)> op)const  noexcept{
        _inOrderAndOp(_root, op);
    }
    
    void rinOrderAndOp(std::function<bool( splay*)> op) const  noexcept{
        _rinOrderAndOp(_root, op);
    }
    //move assignment
    SplayTree& operator=(const SplayTree&& other) noexcept
    {
        if (this != &other)
        {
            _root = other._root;
            _size = other._size;
            other._root = nullptr;
        }
        return *this;
    }
private:
    mutable splay* _root = nullptr;
    size_t _size = 0;
    
    splay* _insert(KeyType key, ValueType value, splay* root) noexcept
    {
        static splay* p_node = NULL; //TODO: maybe get rid of this static variable
        if (!p_node)
            p_node = _New_Node(key, value);
        else
            p_node->key = key;
        if (!root)
        {
            root = p_node;
            p_node = NULL;
            return root;
        }
        root = _splay(key, root);
        /* This is BST that, all keys <= root->key is in root->children[LEFT], all keys >
         root->key is in root->children[RIGHT]. */
        
        int cmp = key > root->key;// dir is 0 if key is less than root->key, 1 otherwise
        
        if (key != root->key)
        {
            p_node->children[cmp] = root->children[cmp];
            p_node->children[!cmp] = root;
            root->children[cmp] = NULL;
            root = p_node;
        }
        p_node = NULL;
        return root;
    }
    
    splay* _erase(KeyType key, splay* root) noexcept
    {
        if (!root)
            return NULL;
        root = _splay(key, root);
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
        p_node->children[LEFT] = p_node->children[RIGHT] = NULL;
        ++_size;
        return p_node;
    }
    

    void _printInOrder(splay* root) noexcept
    {
        if (root)
        {
            InOrder(root->children[LEFT]);
            cout<< "key: " <<root->key;
            if(root->children[LEFT])
                cout<< " | left child: "<< root->children[LEFT]->key;
            if(root->children[RIGHT])
                cout << " | right child: " << root->children[RIGHT]->key;
            cout<< "\n";
            InOrder(root->children[RIGHT]);
        }
    }
    
    void _inOrderAndOp(splay* root, std::function<bool( splay*)> op) const noexcept{
        if (root)
        {
            _inOrderAndOp(root->children[LEFT], op);
            if(!op(root))
                return;
            _inOrderAndOp(root->children[RIGHT], op);
        }
    }
    
    void _rinOrderAndOp(splay* root, std::function<bool( splay*)> op) const  noexcept{
        if (root)
        {
            _rinOrderAndOp(root->children[RIGHT], op);
            if(!op(root))
                return;
            _rinOrderAndOp(root->children[LEFT], op);
        }
    }
    
    splay* _getMinimumAndSplay(splay* root) const  noexcept{
        //perfom zig-zig or zig to move the left most node to the root
        if(root == NULL) return NULL;
        
        splay* x = root;
        while (x->children[LEFT] != NULL) {
            if (x->children[LEFT]->children[LEFT] != NULL) {
                // "Zig-zig" step: make two right rotations
                x->children[LEFT] = _RR_Rotate(x->children[LEFT]);
                if (x->children[LEFT] != NULL)
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
        if(root == NULL) return NULL;
        
        splay* x = root;
        while (x->children[RIGHT] != NULL) {
            if (x->children[RIGHT]->children[RIGHT] != NULL) {
                // "Zag-zag" step: make two left rotations
                x->children[RIGHT] = _LL_Rotate(x->children[RIGHT]);
                if (x->children[RIGHT] != NULL)
                    x = _LL_Rotate(x);
            } else {
                // "Zag" step: a single rotation is enough when there is no right child for the right child of x
                x = _LL_Rotate(x);
            }
        }
        root = x;
        return root;
    }
};
#endif /* SplayTree_h */

