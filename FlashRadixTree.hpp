//
//  FlashRadixTree.hpp
//
//  Created by Matthew Varendorff on 26/2/2024.
//
//  GNU GENERAL PUBLIC LICENSE
//  Version 3, 29 June 2007
//

#ifndef FlashRadixTree_hpp
#define FlashRadixTree_hpp

#define USE_SPLAY_TREE
//#define USE_CHAR_MAP

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <stack>
#ifdef USE_SPLAY_TREE
#include "SplayTree.hpp"
#elif defined USE_CHAR_MAP
#include "CharMap.hpp"
#endif
#include <concepts>
#include <sstream>
#include <algorithm>
#include <optional>



template <typename T, typename Alloc, typename... Args>
std::unique_ptr<T, std::function<void(T*)>> make_unique_alloc(Alloc alloc, Args&&... args)
{
    using AllocTraits = std::allocator_traits<Alloc>;
    using pointer = typename AllocTraits::pointer;

    // Allocate space for one object of type T
    pointer ptr = AllocTraits::allocate(alloc, 1);

    // Construct an object of type T with the provided arguments
    AllocTraits::construct(alloc, ptr, std::forward<Args>(args)...);

    // Create a custom deleter as a lambda function that captures the allocator by value
    auto deleter = [alloc](T* ptr) mutable {
        AllocTraits::destroy(alloc, ptr); // Call destructor
        AllocTraits::deallocate(alloc, ptr, 1); // Deallocate memory
    };

    // Define the deleter type
    using DeleterType = std::function<void(T*)>;

    // Create and return a unique_ptr with the custom deleter
    return std::unique_ptr<T, DeleterType>(ptr, deleter);
}



template<typename T>
concept IsBasicString = requires {
    typename T::traits_type;
    typename T::allocator_type;
    // Check if T is an instantiation of std::basic_string
    requires std::is_same_v<T, std::basic_string<typename T::value_type,
                                                 typename T::traits_type,
                                                 typename T::allocator_type>>;
};

template<typename T>
concept IsBasicStringView = requires {
    typename T::traits_type;
    // Check if T is an instantiation of std::basic_string_view
    requires std::is_same_v<T, std::basic_string_view<typename T::value_type,
                                                      typename T::traits_type>>;
};

template<typename T>
concept BasicStringOrBasicStringView = IsBasicString<T> || IsBasicStringView<T>;

// Concept to check if a type supports streaming with '<<'
template<typename T>
concept Streaming = requires(std::ostream& os, const T& t) {
    { os << t } -> std::convertible_to<std::ostream&>;
};

enum class MatchMode {
    Prefix,
    Exact
};


template <BasicStringOrBasicStringView Key, Streaming Value, MatchMode MatchMode = MatchMode::Exact, typename Allocator = std::allocator<std::unique_ptr<Value>>>
class FlashRadixTree {
    enum class Sentinal { END, REND, NONE };

public:
    template<class Parent>
    struct FlashRadixTreeNodeBase
    {
        FlashRadixTreeNodeBase() = default;
        FlashRadixTreeNodeBase(bool isEndOfWord, Value&& value, const Key& prefix, Parent* parent)
        : isEndOfWord(isEndOfWord), value(std::move(value)), prefix(prefix), parent(parent)
        {};
        FlashRadixTreeNodeBase(bool isEndOfWord, const Value& value, const Key& prefix, Parent* parent)
        : isEndOfWord(isEndOfWord), value(value), prefix(prefix), parent(parent)
        {};
        
        //move and move assignment
        FlashRadixTreeNodeBase(FlashRadixTreeNodeBase&& other) noexcept
        {
            isEndOfWord = other.isEndOfWord;
            value = std::move(other.value);
            prefix = std::move(other.prefix);
            deleted = other.deleted;
            next = other.next;
            prev = other.prev;
            other.next = nullptr;
            other.prev = nullptr;
        }
        FlashRadixTreeNodeBase& operator=(FlashRadixTreeNodeBase&& other) noexcept
        {
            if(this != &other)
            {
                isEndOfWord = other.isEndOfWord;
                value = std::move(other.value);
                prefix = std::move(other.prefix);
                deleted = other.deleted;
                next = other.next;
                prev = other.prev;
                other.next = nullptr;
                other.prev = nullptr;
            }
            return *this;
        }
        bool constexpr operator==(const FlashRadixTreeNodeBase& other) const
        {
            return (isEndOfWord == other.isEndOfWord)
            && (prefix == other.prefix)
            && (deleted == other.deleted);
        }
        bool constexpr operator!=(const FlashRadixTreeNodeBase& other) const
        {
            return !(*this == other);
        }
        Key getFullKey() const
        {
            if(fullKey.has_value())
                return fullKey.value();
            
            //traverse up levels and prefix the key
            fullKey = prefix;
            if(parent == nullptr) {
                return fullKey.value();
            }
            //else traverse up the tree
            for(auto p = parent; p != nullptr; p = p->parent) {
                fullKey.value().insert(0, p->prefix);
            }
            return fullKey.value();
        }
        
        void setDeleted()  noexcept
        {
            deleted = true;
        }
        
        void clear()
        {
            isEndOfWord = false;
            value = Value();
            prefix = Key();
            deleted = false;
        }
        
        bool isEndOfWord = false;
        Value value = Value();
        Key prefix = Key();
        bool deleted = false;
        Parent* next = nullptr;
        Parent* prev = nullptr;
        Parent* parent = nullptr;
    private:
        mutable std::optional<std::string> fullKey;
    };
    
    class FlashRadixTreeNodeNoOwner;
    
    class FlashRadixTreeNode : public FlashRadixTreeNodeBase<FlashRadixTreeNode> {
        using BaseType = FlashRadixTreeNodeBase<FlashRadixTreeNode>;
    public:
        using TreeNodePtr = std::unique_ptr<FlashRadixTreeNode, std::function<void(FlashRadixTreeNode*)>>;
#ifdef USE_SPLAY_TREE
        using Children = SplayTree<typename Key::value_type, TreeNodePtr, Allocator>;
#elif defined USE_CHAR_MAP
        using Children = CharMap<TreeNodePtr>;
#else
        using Children = std::map<typename Key::value_type, TreeNodePtr>;
#endif
        Children children = Children(Allocator());
        
        FlashRadixTreeNode(const Key& prefix, Value&& value, FlashRadixTreeNode* parent)
        : BaseType(false, std::forward<Value>(value), prefix, parent)
        {};
        
        FlashRadixTreeNode(const Key& prefix, Value&& value, bool isEndOfWord, FlashRadixTreeNode* parent)
        : BaseType(isEndOfWord, std::forward<Value>(value), prefix, parent)
        {};
        
        FlashRadixTreeNode() = default;
        ~FlashRadixTreeNode() = default;
        FlashRadixTreeNode(const FlashRadixTreeNodeNoOwner& ) = delete;
        FlashRadixTreeNode( FlashRadixTreeNode&& other) noexcept
        : BaseType(std::move(other))
        {
            children = std::move(other.children);
        }
        //operators
        FlashRadixTreeNode& operator=(const FlashRadixTreeNode& ) = delete;
        FlashRadixTreeNode& operator=(FlashRadixTreeNode&& other) noexcept
        {
            if(this != &other)
            {
                BaseType::operator=(std::move(other));
                children = std::move(other.children);
            }
            return *this;
        }

        void clear()
        {
            children.clear();
            FlashRadixTreeNodeBase<FlashRadixTreeNode>::clear();
        }
    };
    
    class FlashRadixTreeNodeNoOwner : public FlashRadixTreeNodeBase<FlashRadixTreeNode>
    {
        using BaseType = FlashRadixTreeNodeBase<FlashRadixTreeNode>;
    public:
        using TreeNodePtr = FlashRadixTreeNode*;
#ifdef USE_SPLAY_TREE
        using Children = SplayTree<typename Key::value_type, TreeNodePtr, Allocator>;
#elif defined USE_CHAR_MAP
        using Children = CharMap<TreeNodePtr>;
#else
        using Children = std::map<typename Key::value_type, TreeNodePtr>;
#endif
        Children children = Children(Allocator());
        
        FlashRadixTreeNodeNoOwner(const Key& prefix, Value&& value, FlashRadixTreeNode* parent)
        : BaseType(false, std::forward<Value>(value), prefix, parent)
        {};
        
        FlashRadixTreeNodeNoOwner(const Key& prefix, const Value& value, bool isEndOfWord, FlashRadixTreeNode* parent)
        : BaseType(isEndOfWord, value, prefix, parent)
        {};
        
        FlashRadixTreeNodeNoOwner() = default;
        ~FlashRadixTreeNodeNoOwner() = default;
        FlashRadixTreeNodeNoOwner(const FlashRadixTreeNodeNoOwner& other)
        :BaseType(other)
        {
            children = other.children;
        }
        FlashRadixTreeNodeNoOwner( FlashRadixTreeNodeNoOwner&& other) noexcept
        : BaseType(std::move(other))
        {
            children = std::move(other.children);
        }
        //operators
        FlashRadixTreeNodeNoOwner& operator=(const FlashRadixTreeNodeNoOwner& other )
        {
            if(this != &other)
            {
                BaseType::operator=(other);
                children = other.children;
            }
            return *this;
        }
        
        FlashRadixTreeNodeNoOwner& operator=(FlashRadixTreeNodeNoOwner&& other) noexcept
        {
            if(this != &other)
            {
                BaseType::operator=(std::move(other));
                children = std::move(other.children);
            }
            return *this;
        }
    };
    
    using ValueType = Value;

private:

    
    enum class IteratorDirection { FORWARD, REVERSE};
    template<IteratorDirection Direction>
    class XFlashRadixTreeIterator
    {
    private:
        FlashRadixTreeNode* _node;
        const FlashRadixTree* _tree;
        IteratorDirection _direction = Direction;
        bool _end = true;
        XFlashRadixTreeIterator(FlashRadixTreeNode* node, const FlashRadixTree* tree)
        : _node(node), _tree(tree), _direction(Direction), _end(node == nullptr || tree == nullptr)
        {}
    public:
        XFlashRadixTreeIterator() noexcept = default;
        ~XFlashRadixTreeIterator() = default;
        
        // Default copy constructor - used for same type
        XFlashRadixTreeIterator(const XFlashRadixTreeIterator& other) noexcept = default;

        // Default copy assignment operator - used for same type
        XFlashRadixTreeIterator& operator=(const XFlashRadixTreeIterator& other) noexcept = default;

        // Prevent cross-direction copying and assignment using a deleted function template
        template<IteratorDirection OtherDirection>
        XFlashRadixTreeIterator(const XFlashRadixTreeIterator<OtherDirection>&) = delete;
       
        template<IteratorDirection OtherDirection>
        XFlashRadixTreeIterator& operator=(const XFlashRadixTreeIterator<OtherDirection>&) = delete;
        
        //converter functions to convert form forward to reverse and vice versa
        //template<IteratorDirection OtherDirection>
        auto get_other_direction() const  noexcept {
            if constexpr (Direction == IteratorDirection::FORWARD) {
                return XFlashRadixTreeIterator<IteratorDirection::REVERSE>(_node, _tree);
            } else {
                return XFlashRadixTreeIterator<IteratorDirection::FORWARD>(_node, _tree);
            }
        }

        
        
        XFlashRadixTreeIterator& operator++()  noexcept
        {
            if (_direction == IteratorDirection::FORWARD) {
                do{
                    _node = _node->next;
                }while(_node != nullptr && (!_node->isEndOfWord || _node->deleted));
            }
            else {
                do{
                    _node = _node->prev;
                }while(_node != nullptr && (!_node->isEndOfWord || _node->deleted));
            }
            _end = _node == nullptr;
            return *this;
        }
        XFlashRadixTreeIterator& operator--()  noexcept
        {
            if (_direction == IteratorDirection::FORWARD) {
                do{
                    _node = _node->prev;
                }while(_node != nullptr && (!_node->isEndOfWord || _node->deleted));
            }
            else {
                do{
                    _node = _node->next;
                }while(_node != nullptr && (!_node->isEndOfWord || _node->deleted));
            }
            _end = _node == nullptr;
            return *this;
        }
        
        XFlashRadixTreeIterator operator++(int)  noexcept
        {
            XFlashRadixTreeIterator tmp = *this;
            ++(*this);
            return tmp;
        }
        
        XFlashRadixTreeIterator operator--(int)  noexcept
        {
            XFlashRadixTreeIterator tmp = *this;
            --(*this);
            return tmp;
        }
        
        bool operator==(const XFlashRadixTreeIterator& other) const
        {
            if(_tree != other._tree)
                return false;
            if(_end && other._end)
                return true;
            if(_node == nullptr && other._node == nullptr)
                return true;
            if(_node == nullptr || other._node == nullptr)
                return false;
            return *_node == *other._node;
        }
        bool operator!=(const XFlashRadixTreeIterator& other) const
        {
            return !(*this == other);
        }
        
        constexpr FlashRadixTreeNode* operator->()
        {
            return _node;
        }
        constexpr FlashRadixTreeNode& operator*()
        {
            return *_node;
        }
        friend class FlashRadixTree;
    };
public:
    
    using iterator = XFlashRadixTreeIterator<IteratorDirection::FORWARD>;
    using reverse_iterator = XFlashRadixTreeIterator<IteratorDirection::REVERSE>;
    using TreeNodeAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<FlashRadixTreeNode>;
    using UniquePtrAlloc = std::unique_ptr<FlashRadixTreeNode, std::function<void(FlashRadixTreeNode*)>>;
private:
    TreeNodeAllocator _nodeAllocator;
    UniquePtrAlloc _root;
    FlashRadixTreeNode* _firstWord = nullptr;
    iterator _endIt = iterator(nullptr, this);
    reverse_iterator _rendIt = reverse_iterator(nullptr, this);
    size_t _size = 0;
public:
    
    FlashRadixTree(const Allocator& alloc = Allocator())  noexcept
    : _nodeAllocator(alloc),
    _root(make_unique_alloc<FlashRadixTreeNode>(_nodeAllocator))
    {
    }
    ~FlashRadixTree() {
        clear();
    }
    
    FlashRadixTree(const FlashRadixTree& ) = delete;
    FlashRadixTree(FlashRadixTree&& other) noexcept
    : _nodeAllocator(std::move(other._nodeAllocator)), _root(std::move(other._root)), _firstWord(other._firstWord), _size(other._size) {
        other._firstWord = nullptr;
        other._size = 0;
    }
    
    FlashRadixTree& operator=(FlashRadixTree&& other) noexcept {
        if(this != &other) {
            clear();
            _nodeAllocator = std::move(other._nodeAllocator);
            _root = std::move(other._root);
            _firstWord = other._firstWord;
            _size = other._size;
            other._firstWord = nullptr;
            other._size = 0;
        }
        return *this;
    }
    
    constexpr FlashRadixTreeNode* getRoot() const  {
        return _root.get();
    }
    
    constexpr size_t size() const  noexcept {
        return _size;
    }
    
    iterator begin() const  noexcept {
        const auto node =  _getMinimum();
        if(node == nullptr) {
            return _endIt;
        }
        return iterator(node, this);
    }
    
    constexpr const iterator& end() const  noexcept {
        return _endIt;
    }
    
    reverse_iterator rbegin() const  noexcept {
        const auto node =  _getMaximum(_root.get());
        if(node == nullptr) {
            return _rendIt;
        }
        return reverse_iterator(node, this);
    }
    
    constexpr const reverse_iterator& rend() const  noexcept {
        return _rendIt;
    }
    
    iterator insert(const Key& key, Value&& value)
    {
        std::optional<FlashRadixTreeNodeNoOwner> rollback;
        FlashRadixTreeNode* rollbackLocation = nullptr;
        
        if (_root == nullptr) {
            // If the root doesn't exist, create it.
            _root = make_unique_alloc<FlashRadixTreeNode>(_nodeAllocator);
        }
        
        try
        {
            FlashRadixTreeNode* currentNode = _root.get();
            FlashRadixTreeNode* inserted = nullptr;
            Key remaining = key;
            
            while (!remaining.empty()) {
                const typename FlashRadixTreeNode::Children::iterator& it = currentNode->children.find(remaining[0]);
                
                if( it != currentNode->children.end()) {
                    // Found a common prefix, split the edge if necessary
#if defined( USE_SPLAY_TREE) || defined USE_CHAR_MAP
                    const typename Key::value_type edgeKey = it->key;
                    FlashRadixTreeNode* childNode = it->value.get();
#else
                    const typename Key::value_type edgeKey = it->first;
                    FlashRadixTreeNode* childNode = it->second.get();
#endif
                    const Key& edge = childNode->prefix; // Assuming first is the key in the SplayTree
                    
                    // Determine the common prefix length
                    size_t commonPrefixLength = 0;
                    bool lineIsWholePrefix = false;
                    while (commonPrefixLength < remaining.size() && commonPrefixLength < edge.size()
                           && remaining[commonPrefixLength] == edge[commonPrefixLength]) {
                        ++commonPrefixLength;
                        lineIsWholePrefix = commonPrefixLength == remaining.size();
                    }
                    
                    if (commonPrefixLength < edge.length()) {
                        // Split the edge
                        const Key& commonPrefix = edge.substr(0, commonPrefixLength);
                        const Key& suffixEdge = edge.substr(commonPrefixLength);
                        
                        
                        // Create a new node for the common prefix
                        auto newChild = make_unique_alloc<FlashRadixTreeNode>(_nodeAllocator, commonPrefix, std::move((lineIsWholePrefix ? std::forward<Value>(value) : Value())), lineIsWholePrefix, currentNode);
                        inserted = newChild.get();
                        
                        //new node will take over location of old node in the list followed by existing split node
                        if(_firstWord == childNode)
                            _firstWord = inserted;
                        inserted->prev = childNode->prev;
                        childNode->prev = inserted;
                        inserted->next = childNode;
                        
                        // The new node should adopt the existing child node
                        rollback = _copyFromOwnerNode(childNode);
                        rollbackLocation = childNode;
                        childNode->prefix = suffixEdge;
#if defined( USE_SPLAY_TREE)  || defined USE_CHAR_MAP
                        newChild->children.insert(suffixEdge[0], std::move(it->value));
#else
                        newChild->children.emplace(suffixEdge[0], std::move(it->value));
#endif
                        childNode->parent = newChild.get();
                        
                        // Insert the new child with the common prefix in the current node's children
#if defined( USE_SPLAY_TREE) || defined USE_CHAR_MAP
                        auto itNewChild = currentNode->children.find(edgeKey);//, std::move(newChild));
                        itNewChild->value = std::move(newChild);
                        rollbackLocation = itNewChild->value.get();
                        currentNode = itNewChild->value.get();
#else
                        auto itNewChild = currentNode->children.emplace(commonPrefix[0], std::move(newChild));
                        currentNode = itNewChild.first->second;
#endif
                    } else {
                        // Entire edge is a common prefix, proceed with the child node
                        currentNode = childNode;
                        if(currentNode->isEndOfWord && currentNode->deleted)
                        {
                            //if the current node is deleted, then we can insert the value here
                            currentNode->value = std::move(value);
                            currentNode->deleted = false;
                            inserted = currentNode;
                        }
                    }
                    
                    // Update the remaining part of the key to insert
                    remaining = remaining.substr(commonPrefixLength);
                } else {
                    // No common prefix found, create a new edge for the remaining part of the key
                    auto newNode = make_unique_alloc<FlashRadixTreeNode>(_nodeAllocator, remaining, std::forward<Value>(value), currentNode);
                    const char newKey = remaining[0];
                    typename FlashRadixTreeNode::Children::iterator lowerBound;
#if defined( USE_SPLAY_TREE) || defined USE_CHAR_MAP
                    if(_firstWord)
                        lowerBound = currentNode->children.find_predecessor(newKey);
                    
                    auto itNewNode = currentNode->children.insert(newKey, std::move(newNode));
                    inserted = itNewNode->value.get();
#else
                    if(_firstWord)
                    {
                        lowerBound = currentNode->children.lower_bound(newKey);
                        if(lowerBound != currentNode->children.end())
                        {
                            if(lowerBound->first == newKey)
                                --lowerBound;
                        }
                    }
                    auto itNewNode = currentNode->children.emplace(newKey, std::move(newNode));
                    inserted = itNewNode.first->second.get();
#endif
                    itNewNode->value->isEndOfWord = true;
                    
                    //update the firstword
                    if(_firstWord == nullptr)
                        _firstWord = inserted;
                    else if(lowerBound != currentNode->children.end())
                    {
                        auto* lowerNode = lowerBound->value.get();
                        if(!lowerNode->children.empty())
                            lowerNode = _getMaximum(lowerNode);
                        inserted->prev = lowerNode;
                        inserted->next = lowerNode->next;
                        lowerNode->next = inserted;
                    }
                    else if(currentNode->children.size() == 1) //no other children to link to so we link with the last node of the children above
                    {
                        auto* parent = currentNode->parent;
                        if(parent)
                        {
                            auto* last = parent->children.rbegin()->value.get();
                            last->next = inserted;
                            inserted->prev = last;
                        }
                    }
                    
                    
                    // As we've inserted the rest of the key, we're done
                    remaining = Key();
                    currentNode = inserted;
                }
            }
            if(inserted == nullptr)
                return _endIt;
            else
            {
                ++_size;
                return iterator(inserted, this);
            }
        }
        catch (const std::bad_alloc&)
        {
            if(rollback.has_value())
            {
                //override the rollback location with the rollback node
                *rollbackLocation = _mergeFromNoOwnerNode(rollback.value(), rollbackLocation);
            }
            throw std::bad_alloc();
        }
    }
    
        
    //_erase() requires the function append() which will not work on a string_view.
    //in which case we use mark_erase()
    bool erase(const Key& key)
    {
        if constexpr(IsBasicString<Key>)
            return _erase(key);
        else
            return _mark_erase(key);
    }
    
    void print() const  {
        _printRecursively(' ', _root.get(), 0);
        std::cout << std::endl;
    }

    iterator find(const Key& key) const {
        if (key.empty()) {
            return _endIt; // An empty key cannot be found.
        }
        
        auto* currentNode = _root.get();
        auto keyPrefix = key[0];
        Key remaining = key;
        size_t seen = 0;
        while( currentNode != nullptr)
        {
            const auto& it = currentNode->children.find(keyPrefix);

             if(it != currentNode->children.end())
            {
#if defined( USE_SPLAY_TREE) || defined USE_CHAR_MAP
                currentNode = it->value.get();
#else
                currentNode = it->second.get();
#endif
                if(MatchMode == MatchMode::Prefix && //if we are in prefix mode we can stop if we find the prefix
                   currentNode->isEndOfWord &&
                       (currentNode->children.empty() ||//if there are no children below this key we have our winner
                        currentNode->prefix.size() == remaining.size())) //if the prefix is the same size as the remaining key we can match on that also
                {
                    if(currentNode->deleted)
                        return _endIt;
                    else
                        return iterator(const_cast<FlashRadixTreeNode*>( currentNode), this);
                }
                else if(currentNode->isEndOfWord && remaining == currentNode->prefix) //else we no choice but to check the whole word
                {
                    if(currentNode->deleted)
                        return _endIt;
                    else
                        return iterator(const_cast<FlashRadixTreeNode*>( currentNode), this);
                }
                else if( key.size() > (seen += currentNode->prefix.size()) )
                {
                    //we look for children below looking at the next possible prefix
                    keyPrefix = key[seen];
                    remaining = key.substr(seen);
                }
            }
            else
                return _endIt; //assume doesn't exist
            
        }
        return _endIt;
    }
    
    //delete all items non recursively
    void clear()  {
        if(_root != nullptr)
            _root->clear();
        _root = nullptr;
        _size = 0;
        _firstWord = nullptr;
    }
        
private:
    void _printRecursively(const typename Key::value_type& key, FlashRadixTreeNode* node, int level) const {
        if (node == nullptr) {
            return;
        }
        
        for (int i = 0; i < level; ++i) {
            std::cout << "  ";
        }
        
        std::cout << "K: " << key << " P: " << node->prefix << " (" << node->value << ")" << " is EOW " << (node->isEndOfWord ? "Yes" : "No") << std::endl;
        
#if defined( USE_SPLAY_TREE)
        for(const auto& it : node->children)
        {
            _printRecursively(it->key, it->value.get(), level + 1);
        }
#elif defined( USE_CHAR_MAP)
        node->children.inOrderAndOp([&](const auto& node)->bool {
            _printRecursively(node->key, node->value, level + 1);
            return true;
        });
#else
        for (const auto& it : node->children) {
            _printRecursively(it.first, it.second, level + 1);
        }
#endif
    }
  
        
        
    bool _mark_erase(const Key& key)
    {
        auto found = find(key);
        if (found == end()) {
            return false;
        }
        else
        {
            found->deleted = true;
            return true;
        }
    }
    
    bool _erase(const Key& key)  {
        if (key.empty()) {
            return false; // Cannot erase an empty key
        }

        FlashRadixTreeNode* currentNode = _root.get();
        FlashRadixTreeNode* parentNode = nullptr;
        Key remainingKey = key;

        // Step 1: Find the node
        while (currentNode != nullptr && !remainingKey.empty()) {
            parentNode = currentNode;
            auto it = currentNode->children.find(remainingKey[0]);
            
            if (it == currentNode->children.end()) {
                return false; // Key not found
            }

            const Key& nodePrefix = it->value->prefix;
            FlashRadixTreeNode* childNode = it->value.get();

            if (remainingKey.starts_with(nodePrefix)) {
                // Prefix matches, move to the next node
                const auto nodePrefixSize = nodePrefix.size();
                remainingKey = remainingKey.substr(nodePrefixSize);
                currentNode = childNode;
            } else {
                // Prefix does not match the remaining key
                return false; // Key not found
            }
        }

        // If the end of the key has been reached and it's not marked as an end of a word, the key does not exist
        if (!currentNode->isEndOfWord) {
            return false;
        }

        // Step 2: Delete the node or unmark the end of the word
        currentNode->isEndOfWord = false; // Unmark as the end of a word

        // If the current node has has more than one child then we're done
        if (currentNode->children.size() > 1) {
            --_size;
            return true;
        }

        // Step 3: Clean up the tree
        if ((parentNode != nullptr) && (currentNode->children.size() <= 1) && !currentNode->isEndOfWord) {

            // Remove the leaf node if it does not have any children
            if(currentNode->children.empty())
            {
                //tidy up linked list of end of words
                if(currentNode->prev)
                    currentNode->prev->next = currentNode->next;
                if(currentNode->next)
                    currentNode->next->prev = currentNode->prev;
                    
                parentNode->children.erase(currentNode->prefix[0]);
            }

            // If the parent node is now a leaf and is not an end-of-word node, set it as the current node for the next iteration
            if (parentNode->children.empty() && !parentNode->isEndOfWord) {
                currentNode = parentNode;
                if (parentNode != nullptr) {
                    remainingKey = parentNode->prefix;
                }
            }
            else if(currentNode->children.size() == 1 && !currentNode->isEndOfWord)
            {
                auto remainingChild = std::move(currentNode->children.root()->value);
                //tidy up linked list of end of words
                if(remainingChild->prev)
                    remainingChild->prev->next = remainingChild->next;
                if(remainingChild->next)
                    remainingChild->next->prev = remainingChild->prev;
                
                currentNode->prefix.append(remainingChild->prefix);
                currentNode->value = remainingChild->value;
                currentNode->isEndOfWord = remainingChild->isEndOfWord;
                currentNode->children = std::move(remainingChild->children); //overrride children.
            }
            //if the parent node has only one child we can compress (unless we're root. that makes no sense)
            else if(parentNode != _root.get() && parentNode->children.size() == 1 && !parentNode->isEndOfWord)
            {
                auto remainingChild = std::move(parentNode->children.root()->value);
                //tidy up linked list of end of words
                if(remainingChild->prev)
                    remainingChild->prev->next = remainingChild->next;
                if(remainingChild->next)
                    remainingChild->next->prev = remainingChild->prev;
                
                parentNode->prefix.append(remainingChild->prefix);
                parentNode->value = remainingChild->value;
                parentNode->isEndOfWord = remainingChild->isEndOfWord;
                parentNode->children = std::move(remainingChild->children);
            }
        }
        --_size;
        return true;
    }
    
    FlashRadixTreeNode* _getMinimum() const  noexcept
    {
        auto* node = _firstWord;
        while(node != nullptr && (!node->isEndOfWord || node->deleted))
            node = node->next;
        return node;
    }
    
    FlashRadixTreeNode* _getMaximum(FlashRadixTreeNode* root) const  noexcept
    {
        if(root == nullptr)
            return nullptr;
        
        auto* children = &root->children;
        auto it = children->rbegin();
        while( children != nullptr)
        {
            const auto check = children->rbegin();
            if(check == children->rend())
                break;
            it = check;
            children = &it->value->children;
        }
        auto* node = it->value.get();
        //hunt backwards for a node which is not deleted
        while(node != nullptr && (!node->isEndOfWord || node->deleted))
            node = node->prev;
        return node;
    }
    
    
    
    FlashRadixTreeNodeNoOwner _copyFromOwnerNode( FlashRadixTreeNode* node)
    {
        FlashRadixTreeNodeNoOwner newNode(node->prefix, node->value, node->isEndOfWord, node->parent);
        for(auto it : node->children)
            newNode.children.insert(it->key, it->value.get());
        
        return newNode;
    }
    
    //move from current node to rollback node if the child existed prior to the insert op
    FlashRadixTreeNode _mergeFromNoOwnerNode( FlashRadixTreeNodeNoOwner& node, FlashRadixTreeNode* current)
    {
        FlashRadixTreeNode newNode( node.prefix, std::move(node.value), node.isEndOfWord, node.parent);
        for(auto it :  node.children)
        {
            auto found = current->children.find(it->key);
            if(found != current->children.end())
                newNode.children.insert(it->key, std::move(found->value));
            
        }
        return newNode;
    }
    
};



template <Streaming Key, Streaming Value, MatchMode FindMode = MatchMode::Exact, typename Allocator = std::allocator<std::unique_ptr<Value>>>
class FlashRadixTreeSerializer {
    
public:
    std::string serialize(const FlashRadixTree<Key, Value, FindMode, Allocator>& tree) {
        return serializeNode(tree.getRoot());
    }
    
    std::string format(const std::string& serialized) {
        //iterate though string and insert new lines and tabs to format according to grammar
        std::string formatted;
        int indent = 0;
        for (auto c : serialized) {
            if (c == '[') {
                formatted += c;
                formatted += '\n';
                indent++;
                for (int i = 0; i < indent; i++) {
                    formatted += '\t';
                }
            } else if (c == ']') {
                formatted += '\n';
                indent--;
                for (int i = 0; i < indent; i++) {
                    formatted += '\t';
                }
                formatted += c;
            } else {
                formatted += c;
            }
        }
        return formatted;
    }

private:
    std::string serializeNode(const typename FlashRadixTree<Key, Value, FindMode, Allocator>::FlashRadixTreeNode* node) const  {
        if (node == nullptr) {
            return "";
        }

        std::stringstream ss;
        ss << "+[" << node->prefix << "," << node->value << "," << (node->isEndOfWord ? "√": "*")  << (node->deleted ? "X" : "") << ",<";

        if(node->children.empty())
            ss << "-";
        else
        {
            bool first = true;
#ifdef USE_SPLAY_TREE
            for(const auto& it : node->children)
            {
                if (!first) {
                    ss << ",";
                }
                ss << serializeNode(it->value.get());
                first = false;
            }
#elif defined( USE_CHAR_MAP)
            node->children.inOrderAndOp([&ss, &first](const auto& childPair) -> bool {
                if (!first) {
                    ss << ",";
                }
                ss << serializeNode(childPair.value);
                first = false;
                return true;
            });
#else
            for (const auto& childPair : node->children) {
                if (!first) {
                    ss << ",";
                }
                ss << serializeNode(childPair.second);
                first = false;
            }
#endif
        }
        
        ss << ">]";
        return ss.str();
    }
   
};

#endif /* FlashRadixTree */
