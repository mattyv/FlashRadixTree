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

#ifndef MAX_ALIGNMENT
#define MAX_ALIGNMENT
template <typename... Args>
constexpr size_t max_alignment() {
    return std::max({alignof(Args)...});
}
#endif

template<typename T>
concept StringLike = requires(T a, const T b, const char* s, std::size_t pos, std::size_t len, std::ostream& os, const T& t) {
    typename T::value_type; // Ensure that T has a value_type member
    { a = s };
    { a == b } -> std::convertible_to<bool>;
    { a != b } -> std::convertible_to<bool>;
    { a.starts_with(b) } -> std::convertible_to<bool>;
    { a.starts_with(s) } -> std::convertible_to<bool>;
    { a[0] } -> std::convertible_to<typename T::value_type>;
    { a.begin() } -> std::forward_iterator;
    { a.end() } -> std::forward_iterator;
    { a.size() } -> std::convertible_to<std::size_t>;
    { a.substr(pos, len) } -> std::convertible_to<T>;
    { a.length() } -> std::convertible_to<size_t>;
    T(s);
};
// Concept to check if a type supports streaming with '<<'
template<typename T>
concept Streaming = requires(std::ostream& os, const T& t) {
    { os << t } -> std::convertible_to<std::ostream&>;
};

// Concept to check if a type has an append() member function
template<typename T>
    concept HasAppend = requires(T a, const T& b) {
        { a.append(b) } -> std::same_as<T&>;
    };

enum class MatchMode {
    Prefix,
    Exact
};

template <StringLike Key, Streaming Value, MatchMode MatchMode = MatchMode::Exact>
class FlashRadixTree {
    
    static_assert(StringLike<Key>, "Key type must satisfy StringLike concept");
    static_assert(Streaming<Value>, "Value type must satisfy Streaming concept");

    
    enum class Sentinal { END, REND, NONE };
public:
    class FlashRadixTreeNode;
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
        XFlashRadixTreeIterator(FlashRadixTreeNode* node, const FlashRadixTree* tree) noexcept
        : _node(node), _tree(tree), _direction(Direction), _end(node == nullptr || tree == nullptr)
        {}
    public:
        //constructor to allow a reverse type from a forward
        XFlashRadixTreeIterator(const XFlashRadixTreeIterator<IteratorDirection::REVERSE>& other) noexcept
        : _node(other._node), _tree(other._tree), _direction(Direction), _end(other._end)
        {
        }
        
        XFlashRadixTreeIterator(const XFlashRadixTreeIterator<IteratorDirection::FORWARD>& other) noexcept
        : _node(other._node), _tree(other._tree), _direction(Direction), _end(other._end)
        {
        }
        
        //assign a reverse iterator to a forward iterator
        XFlashRadixTreeIterator& operator=(const XFlashRadixTreeIterator<IteratorDirection::REVERSE>& other) noexcept
        {
            _node = other._node;
            _tree = other._tree;
            _direction = Direction;
            _end = other._end;
            return *this;
        }
        
        //assign a forward iterator to a reverse iterator
        XFlashRadixTreeIterator& operator=(const XFlashRadixTreeIterator<IteratorDirection::FORWARD>& other) noexcept
        {
            _node = other._node;
            _tree = other._tree;
            _direction = Direction;
            _end = other._end;
            return *this;
        }
        
        XFlashRadixTreeIterator& operator++() noexcept
        {
            if (_direction == IteratorDirection::FORWARD) {
                _node = _tree->_next(_node);
            } else {
                _node = _tree->_prev(_node);
            }
            _end = _node == nullptr;
            return *this;
        }
        XFlashRadixTreeIterator& operator--() noexcept
        {
            if (_direction == IteratorDirection::FORWARD) {
                _node = _tree->_prev(_node);
            } else {
                _node = _tree->_next(_node);
            }
            _end = _node == nullptr;
            return *this;
        }
        
        XFlashRadixTreeIterator operator++(int) noexcept
        {
            XFlashRadixTreeIterator tmp = *this;
            ++(*this);
            return tmp;
        }
        
        XFlashRadixTreeIterator operator--(int) noexcept
        {
            XFlashRadixTreeIterator tmp = *this;
            --(*this);
            return tmp;
        }
        
        bool operator==(const XFlashRadixTreeIterator& other) const noexcept
        {
            if(_end && other._end)
                return true;
            if(_node == nullptr && other._node == nullptr)
                return true;
            if(_node == nullptr || other._node == nullptr)
                return false;
            return *_node == *other._node;
        }
        bool operator!=(const XFlashRadixTreeIterator& other) const noexcept
        {
            return !(*this == other);
        }
        
        constexpr FlashRadixTreeNode* operator->() noexcept
        {
            return _node;
        }
        constexpr FlashRadixTreeNode& operator*() noexcept
        {
            return *_node;
        }
        friend class FlashRadixTree;
    };
public:
    using iterator = XFlashRadixTreeIterator<IteratorDirection::FORWARD>;
    using reverse_iterator = XFlashRadixTreeIterator<IteratorDirection::REVERSE>;
#ifdef USE_SPLAY_TREE
    static constexpr size_t max_alignment_value_FlashRadixTreeNode = max_alignment<Key, Value, bool, SplayTree<typename Key::value_type, void*>,std::optional<Key>>();
#elif defined USE_CHAR_MAP
    static constexpr size_t max_alignment_value_FlashRadixTreeNode = max_alignment<Key, Value, bool, CharMap<Value>,std::optional<Key>>();
#else
    static constexpr size_t max_alignment_value_FlashRadixTreeNode = max_alignment<Key, Value, bool, std::map<typename Key::value_type, void*>,std::optional<Key>>();
#endif
    class alignas(max_alignment_value_FlashRadixTreeNode) FlashRadixTreeNode {
    public:
#ifdef USE_SPLAY_TREE
        using Children = SplayTree<typename Key::value_type, FlashRadixTreeNode*>;
#elif defined USE_CHAR_MAP
        using Children = CharMap<FlashRadixTreeNode*>;
#else
        using Children = std::map<typename Key::value_type, FlashRadixTreeNode*>;
#endif
        Children children;
        bool isEndOfWord = false;
        Value value = Value();
        Key prefix = Key();
        bool deleted = false;
        Sentinal sentinal = Sentinal::NONE;
        Children::iterator my_iterator;
        FlashRadixTreeNode* parent = nullptr;
    private:
        mutable std::optional<Key> fullKey;
    public:
        
        FlashRadixTreeNode(const Key& prefix, const Value&& value, FlashRadixTreeNode* parent ) noexcept
        : isEndOfWord(false), value(std::move(value)), prefix(prefix), deleted(false), sentinal(Sentinal::NONE), parent(parent)
        {};
        
        FlashRadixTreeNode(const Key& prefix, const Value&& value, bool isEndOfWord, FlashRadixTreeNode* parent ) noexcept
        : isEndOfWord(isEndOfWord), value(std::move(value)), prefix(prefix), deleted(false), sentinal(Sentinal::NONE), parent(parent)
        {};
        
        FlashRadixTreeNode(Sentinal sentinal) noexcept
        : isEndOfWord(false), value(), prefix(), deleted(false), sentinal(sentinal), parent(nullptr)
        {};
        
        FlashRadixTreeNode() = default;
        ~FlashRadixTreeNode() = default;
        FlashRadixTreeNode(const FlashRadixTreeNode& ) = delete;
        FlashRadixTreeNode( FlashRadixTreeNode&& other) noexcept
        {
            children = std::move(other.children);
            isEndOfWord = other.isEndOfWord;
            value = std::move(other.value);
            prefix = std::move(other.prefix);
            deleted = other.deleted;
        }
        
        //operators
        FlashRadixTreeNode& operator=(const FlashRadixTreeNode& ) = delete;
        FlashRadixTreeNode& operator=(FlashRadixTreeNode&& other) noexcept
        {
            if(this != &other)
            {
                children = std::move(other.children);
                isEndOfWord = other.isEndOfWord;
                value = std::move(other.value);
                prefix = std::move(other.prefix);
                deleted = other.deleted;
            }
            return *this;
        }
        
        bool constexpr operator==(const FlashRadixTreeNode& other) const noexcept
        {
            return (isEndOfWord == other.isEndOfWord)
            && (prefix == other.prefix)
            && (deleted == other.deleted)
            && (sentinal == other.sentinal);
        }
        
        bool constexpr operator!=(const FlashRadixTreeNode& other) const noexcept
        {
            return !(*this == other);
        }
        
        Key getFullKey() const noexcept
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
        
        void setMyIterator(const Children::iterator& it) noexcept
        {
            my_iterator = it;
        }
        
        void setDeleted() noexcept
        {
            deleted = true;
        }
        
        void clear()
        {
            for(const auto& it : children) {
                auto child = it->value;
                if(child != nullptr) {
                    child->clear();
                    delete child;
                }
            }
            children.clear();
            isEndOfWord = false;
            value = Value();
            prefix = Key();
            deleted = false;
        }
    };
    using ValueType = Value;

private:
    
    FlashRadixTreeNode* _root;
    iterator _endIt = iterator(nullptr, nullptr);
    reverse_iterator _rendIt = reverse_iterator(nullptr, nullptr);
public:
    
    FlashRadixTree() : _root( new FlashRadixTreeNode()) {}
    ~FlashRadixTree() {
        clear();
    }
    
    FlashRadixTree(const FlashRadixTree& ) = delete;
    FlashRadixTree(FlashRadixTree&& other) noexcept
    : _root(other._root) {
        other._root = nullptr;
    }
    
    constexpr FlashRadixTreeNode* getRoot() const noexcept {
        return _root;
    }
    
    iterator begin() const noexcept {
        const auto node =  _getMinimum();
        if(node == nullptr) {
            return _endIt;
        }
        return iterator(node, this);
    }
    
    constexpr const iterator& end() const noexcept {
        return _endIt;
    }
    
    reverse_iterator rbegin() const noexcept {
        const auto node =  _getMaximum();
        if(node == nullptr) {
            return _rendIt;
        }
        return reverse_iterator(node, this);
    }
    
    constexpr const reverse_iterator& rend() const noexcept {
        return _rendIt;
    }
    
    iterator insert(const Key& key, Value&& value) noexcept
    {
        if (_root == nullptr) {
            // If the root doesn't exist, create it.
            _root = new FlashRadixTreeNode();
        }
        
        FlashRadixTreeNode* currentNode = _root;
        FlashRadixTreeNode* inserted = nullptr;
        Key remaining = key;
        
        while (!remaining.empty()) {
            const auto it = currentNode->children.find(remaining[0]);
            
            if( it != currentNode->children.end()) {
                // Found a common prefix, split the edge if necessary
#if defined( USE_SPLAY_TREE) || defined USE_CHAR_MAP
                const typename Key::value_type edgeKey = it->key;
                FlashRadixTreeNode* childNode = it->value;
#else
                const typename Key::value_type edgeKey = it->first;
                FlashRadixTreeNode* childNode = it->second;
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
                    auto* newChild = new FlashRadixTreeNode(commonPrefix, std::move((lineIsWholePrefix ? std::move(value) : Value())), lineIsWholePrefix, currentNode);
                    
                    // The new node should adopt the existing child node
                    childNode->prefix = suffixEdge;
#if defined( USE_SPLAY_TREE)  || defined USE_CHAR_MAP
                    auto it = newChild->children.insert(suffixEdge[0], std::move(childNode));
#else
                    auto it = newChild->children.emplace(suffixEdge[0], std::move(childNode));
#endif
                    childNode->parent = newChild;
                    childNode->setMyIterator(it);
                    currentNode->children.erase(edgeKey);
                    
                    // Insert the new child with the common prefix in the current node's children
#if defined( USE_SPLAY_TREE) || defined USE_CHAR_MAP
                    auto itChild = currentNode->children.insert(commonPrefix[0], std::move(newChild));
                    currentNode = itChild->value;
#else
                    auto itChild = currentNode->children.emplace(commonPrefix[0], std::move(newChild));
                    currentNode = itChild.first->second;
#endif
                    newChild->setMyIterator(itChild);
                } else {
                    // Entire edge is a common prefix, proceed with the child node
                    currentNode = childNode;
                    if(currentNode->isEndOfWord && currentNode->deleted)
                    {
                        currentNode->value = std::move(value);
                        currentNode->deleted = false;
                        inserted = currentNode;
                    }
                }
                
                // Update the remaining part of the key to insert
                remaining = remaining.substr(commonPrefixLength);
            } else {
                // No common prefix found, create a new edge for the remaining part of the key
                auto newNode = new FlashRadixTreeNode(remaining, std::move(value), currentNode);
#if defined( USE_SPLAY_TREE) || defined USE_CHAR_MAP
                auto it = currentNode->children.insert(remaining[0], newNode);
                currentNode = it->value;
#else
                auto it = currentNode->children.emplace(remaining[0], newNode);
                currentNode = it.first->second;
#endif
                newNode->setMyIterator(it);
                currentNode->isEndOfWord = true;
                
                // As we've inserted the rest of the key, we're done
                remaining = Key();
                inserted = currentNode;
            }
        }
        if(inserted == nullptr)
            return _endIt;
        else
            return iterator(inserted, this);
    }
    
        
    //_erase() requires the function append() which will not work on a string_view.
    //in which case we use mark_erase()
    bool erase(const Key& key) noexcept
    {
        if constexpr(HasAppend<Key>)
            return _erase(key);
        else
            return _mark_erase(key);
    }
    
    void print() const noexcept {
        _printRecursively(' ', _root, 0);
        std::cout << std::endl;
    }

    iterator find(const Key& key) const noexcept {
        if (key.empty()) {
            return _endIt; // An empty key cannot be found.
        }
        
        const FlashRadixTreeNode* currentNode = _root;
        char keyPrefix = key[0];
        Key remaining = key;
        size_t seen = 0;
        while( currentNode != nullptr)
        {
            auto it = currentNode->children.find(keyPrefix);

            if(it != currentNode->children.end())
            {
#if defined( USE_SPLAY_TREE) || defined USE_CHAR_MAP
                currentNode = it->value;
#else
                currentNode = it->second;
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
    void clear() noexcept {
        if(_root != nullptr)
            _root->clear();
        _root = nullptr;
    }
        
private:
    void _printRecursively(const typename Key::value_type& key, FlashRadixTreeNode* node, int level) const noexcept{
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
            _printRecursively(it->key, it->value, level + 1);
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
    
    void inOrderAndOpRecursively( FlashRadixTreeNode* node, const Key& key, const std::function<bool(const Key&, const Value&)>& op) const noexcept {
        if (node == nullptr) {
            return;
        }
        
        node->children.inOrderAndOp([this, &op]( auto splay)->bool
                                    {
                                        this->inOrderAndOpRecursively(splay->value, splay->value->prefix, op);
                                        return true;
                                    });
        if(!op(key, node->value))
            return;
    }
        
        
    bool _mark_erase(const Key& key) noexcept
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
    
    bool _erase(const Key& key) noexcept {
        if (key.empty()) {
            return false; // Cannot erase an empty key
        }

        FlashRadixTreeNode* currentNode = _root;
        FlashRadixTreeNode* parentNode = nullptr;
        Key remainingKey = key;

        // Step 1: Find the node
        while (currentNode != nullptr && !remainingKey.empty()) {
            parentNode = currentNode;
            auto it = currentNode->children.find(remainingKey.at(0));
            
            if (it == currentNode->children.end()) {
                return false; // Key not found
            }

            const Key& nodePrefix = it->value->prefix;
            FlashRadixTreeNode* childNode = it->value;

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
            return true;
        }

        // Step 3: Clean up the tree
        if ((parentNode != nullptr) && (currentNode->children.size() <= 1) && !currentNode->isEndOfWord) {

            // Remove the leaf node if it does not have any children
            if(currentNode->children.empty())
                parentNode->children.erase(currentNode->prefix.at(0));

            // If the parent node is now a leaf and is not an end-of-word node, set it as the current node for the next iteration
            if (parentNode->children.empty() && !parentNode->isEndOfWord) {
                currentNode = parentNode;
                if (parentNode != nullptr) {
                    remainingKey = parentNode->prefix;
                }
            }
            else if(currentNode->children.size() == 1 && !currentNode->isEndOfWord)
            {
                auto remainingChild = currentNode->children.root()->value;
                currentNode->prefix.append(remainingChild->prefix);
                currentNode->value = remainingChild->value;
                currentNode->isEndOfWord = remainingChild->isEndOfWord;
                currentNode->children = std::move(remainingChild->children);
                delete remainingChild;
            }
            //if the parent node has only one child we can compress (unless we're root. that makes no sense)
            else if(parentNode != _root && parentNode->children.size() == 1 && !parentNode->isEndOfWord)
            {
                auto remainingChild = parentNode->children.root()->value;
                parentNode->prefix.append(remainingChild->prefix);
                parentNode->value = remainingChild->value;
                parentNode->isEndOfWord = remainingChild->isEndOfWord;
                parentNode->children = std::move(remainingChild->children);
                delete currentNode;
            }
        }

        return true;
    }
    
    FlashRadixTreeNode* _getMinimum() const noexcept
    {
        if(_root == nullptr)
            return nullptr;
        
        auto* children = &_root->children;
        auto it = children->begin();
        while( children != nullptr)
        {
            if(it->value->isEndOfWord)
                break;
            const auto check = children->begin();
            if(check == children->end())
                break;
            it = check;
            children = &it->value->children;
        }
        return it->value;
    }
    
    FlashRadixTreeNode* _getMaximum() const noexcept
    {
        if(_root == nullptr)
            return nullptr;
        
        auto* children = &_root->children;
        auto it = children->rbegin();
        while( children != nullptr)
        {
            const auto check = children->rbegin();
            if(check == children->rend())
                break;
            it = check;
            children = &it->value->children;
        }
        return it->value;
    }
        
    
    
    FlashRadixTreeNode* _next(FlashRadixTreeNode* node) const noexcept
    {
        if(node == nullptr)
            return nullptr;
        
        //next will be first node on lower level if there is one, and its ann end of word and not deleted
        //otherwise it will be the next node on the same level that is an end of word and not deleted
        //or if we reach the end of the current level we go up one level and repeat
        auto* currentNode = node;
        auto it = node->my_iterator;
        while(!currentNode->children.empty())
        {
            it = currentNode->children.begin();
            if(it != currentNode->children.end() ) {
                if( it->value->isEndOfWord && !it->value->deleted)
                    return it->value;
                currentNode = it->value; //else cycle down
            }
        }
        // we iterate once at the current level
        ++it;
        
        //if we reach the end of the current level we go up one level and repeat
        if(it == currentNode->children.end() && currentNode->parent != nullptr)
        {
            currentNode = currentNode->parent;
            it = currentNode->my_iterator;
            ++it;
        }
        //if this node is not end of word or is deleted we go down
        if(it != currentNode->children.end() && (!it->value->isEndOfWord || it->value->deleted))
        {
            currentNode = it->value;
            while(!currentNode->children.empty())
            {
                it = currentNode->children.begin();
                if(it != currentNode->children.end() ) {
                    if( it->value->isEndOfWord && !it->value->deleted)
                        return it->value;
                    currentNode = it->value; //else cycle down
                }
            }
        }
        if(currentNode->parent == nullptr || it == currentNode->parent->children.end())
            return nullptr;
        return it->value;
    }
    
    FlashRadixTreeNode* _prev(FlashRadixTreeNode* node) const noexcept
    {
        if(node == nullptr)
            return nullptr;
        
        //next will be next the next node on the same level that is an end of word and not deleted
        //otherwise it will be node on the max child of the next node on the level above.
        auto* currentNode = node;
        typename FlashRadixTreeNode::Children::reverse_iterator  it = node->my_iterator; //explicitly use reverse here as my_iterator is a forward iterator
    
        ++it;
        
        //if we reach the end of the current level we go up one level and repeat
        if(it == currentNode->children.rend() && currentNode->parent != nullptr)
        {
            currentNode = currentNode->parent;
            it = currentNode->my_iterator;
            if(*it != nullptr && it != currentNode->children.rend() && (!it->value->isEndOfWord || it->value->deleted))
            {
                ++it;
            }
            while(it == currentNode->children.rend() && currentNode->parent != nullptr) //if we reach the end of the current level we go up one level and repeat
            {
                currentNode = currentNode->parent;
                it = currentNode->my_iterator;
                ++it;
                if(it != currentNode->children.rend())
                    currentNode = it->value;
            }
        }
        //if this node is not end of word or is deleted we go all the way down
        if(it != currentNode->children.rend() && !it->value->children.empty() && (!it->value->isEndOfWord || it->value->deleted))
        {
            currentNode = it->value;
            while(!currentNode->children.empty())
            {
                it = currentNode->children.rbegin();
                if(it != currentNode->children.rend() ) {
                    currentNode = it->value; //else cycle down
                }
            }

        }
        if(currentNode->parent == nullptr || it == currentNode->children.rend())
            return nullptr;
        return it->value;
    }
    
};


template <Streaming Key, Streaming Value, MatchMode FindMode = MatchMode::Exact>
class FlashRadixTreeSerializer {
    static_assert(StringLike<Key>, "Key type must satisfy StringLike concept");
    static_assert(Streaming<Value>, "Value type must satisfy Streaming concept");
public:
    std::string serialize(const FlashRadixTree<Key, Value, FindMode>& tree) {
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
    std::string serializeNode(const typename FlashRadixTree<Key, Value, FindMode>::FlashRadixTreeNode* node) const  {
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
            /*node->children.inOrderAndOp([&ss, &first](const SplayTree<typename Key::value_type, typename FlashRadixTree<Key, Value, FindMode>::FlashRadixTreeNode*>::splay* childPair) -> bool {
                if (!first) {
                    ss << ",";
                }
                ss << serializeNode(childPair->value);
                first = false;
                return true;
            });*/
            for(const auto& it : node->children)
            {
                if (!first) {
                    ss << ",";
                }
                ss << serializeNode(it->value);
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
