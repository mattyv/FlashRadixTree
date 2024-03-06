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
    static constexpr size_t max_alignment_value = max_alignment<KeyType, ValueType, SplayTree<KeyType, ValueType>*>();

public:
    struct alignas(max_alignment_value) splay
    {
        splay() noexcept = default;
        splay(KeyType key, ValueType value) noexcept: key(key), value(value), lchild(nullptr), rchild(nullptr)  {}
        
        KeyType key;
        ValueType value;
        splay* lchild;
        splay* rchild;
    };

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
        /* This is BST that, all keys <= root->key is in root->lchild, all keys >
        root->key is in root->rchild. */
        if (key < root->key)
        {
            p_node->lchild = root->lchild;
            p_node->rchild = root;
            root->lchild = NULL;
            root = p_node;
        }
        else if (key > root->key)
        {
            p_node->rchild = root->rchild;
            p_node->lchild = root;
            root->rchild = NULL;
            root = p_node;
        }
        else
            return root;
        p_node = NULL;
        return root;
    }
    
    splay* _erase(KeyType key, splay* root) noexcept
    {
        splay* temp;
        if (!root)
            return NULL;
        root = _splay(key, root);
        if (key != root->key)
            return root;
        else
        {
            if (!root->lchild)
            {
                temp = root;
                root = root->rchild;
            }
            else
            {
                temp = root;
                /*Note: Since key == root->key,
                so after Splay(key, root->lchild),
                the tree we get will have no right child tree.*/
                root = _splay(key, root->lchild);
                root->rchild = temp->rchild;
            }
            --_size;
            delete temp;
            return root;
        }
    }
    splay* _find(KeyType key, splay* root) const noexcept
    {
      return _splay(key, root);
    }
    // RR(Y rotates to the right)
    splay* _RR_Rotate(splay* k2) const noexcept
    {
        splay* k1 = k2->lchild;
        k2->lchild = k1->rchild;
        k1->rchild = k2;
        return k1;
    }

    // LL(Y rotates to the left)
    splay* _LL_Rotate(splay* k2) const noexcept
    {
        splay* k1 = k2->rchild;
        k2->rchild = k1->lchild;
        k1->lchild = k2;
        return k1;
    }

    // An implementation of top-down splay tree
    splay* _splay(KeyType key, splay* root) const noexcept
    {
        if (!root)
            return NULL;
        splay header;
        /* header.rchild points to L tree;
        header.lchild points to R Tree */
        header.lchild = header.rchild = NULL;
        splay* LeftTreeMax = &header;
        splay* RightTreeMin = &header;
        while (1)
        {
            if (key < root->key)
            {
                if (!root->lchild)
                    break;
                if (key < root->lchild->key)
                {
                    root = _RR_Rotate(root);
                    // only zig-zig mode need to rotate once,
                    if (!root->lchild)
                        break;
                }
                /* Link to R Tree */
                RightTreeMin->lchild = root;
                RightTreeMin = RightTreeMin->lchild;
                root = root->lchild;
                RightTreeMin->lchild = NULL;
            }
            else if (key > root->key)
            {
                if (!root->rchild)
                    break;
                if (key > root->rchild->key)
                {
                    root = _LL_Rotate(root);
                    // only zag-zag mode need to rotate once,
                    if (!root->rchild)
                        break;
                }
                /* Link to L Tree */
                LeftTreeMax->rchild = root;
                LeftTreeMax = LeftTreeMax->rchild;
                root = root->rchild;
                LeftTreeMax->rchild = NULL;
            }
            else
                break;
        }
        /* assemble L Tree, Middle Tree and R tree */
        LeftTreeMax->rchild = root->lchild;
        RightTreeMin->lchild = root->rchild;
        root->lchild = header.rchild;
        root->rchild = header.lchild;
        return root;
    }
    
    

    splay* _New_Node(KeyType key, ValueType value) noexcept
    {
        splay* p_node = new splay(key, value);
        p_node->lchild = p_node->rchild = NULL;
        ++_size;
        return p_node;
    }
    

    void _printInOrder(splay* root) noexcept
    {
        if (root)
        {
            InOrder(root->lchild);
            cout<< "key: " <<root->key;
            if(root->lchild)
                cout<< " | left child: "<< root->lchild->key;
            if(root->rchild)
                cout << " | right child: " << root->rchild->key;
            cout<< "\n";
            InOrder(root->rchild);
        }
    }
    
    void _inOrderAndOp(splay* root, std::function<bool( splay*)> op) const noexcept{
        if (root)
        {
            _inOrderAndOp(root->lchild, op);
            if(!op(root))
                return;
            _inOrderAndOp(root->rchild, op);
        }
    }
    
    void _rinOrderAndOp(splay* root, std::function<bool( splay*)> op) const  noexcept{
        if (root)
        {
            _rinOrderAndOp(root->rchild, op);
            if(!op(root))
                return;
            _rinOrderAndOp(root->lchild, op);
        }
    }
    
    splay* _getMinimumAndSplay(splay* root) const  noexcept{
        //perfom zig-zig or zig to move the left most node to the root
        if(root == NULL) return NULL;
        
        splay* x = root;
        while (x->lchild != NULL) {
            if (x->lchild->lchild != NULL) {
                // "Zig-zig" step: make two right rotations
                x->lchild = _RR_Rotate(x->lchild);
                if (x->lchild != NULL)
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
        while (x->rchild != NULL) {
            if (x->rchild->rchild != NULL) {
                // "Zag-zag" step: make two left rotations
                x->rchild = _LL_Rotate(x->rchild);
                if (x->rchild != NULL)
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

