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

//#define USE_SPLAY_TREE
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
    { a[0] } -> std::convertible_to<typename T::value_type>;
    { a.begin() } -> std::forward_iterator;
    { a.end() } -> std::forward_iterator;
    { a.size() } -> std::convertible_to<std::size_t>;
    { a.substr(pos, len) } -> std::convertible_to<T>;
    T(s);
};
// Concept to check if a type supports streaming with '<<'
template<typename T>
concept Streaming = requires(std::ostream& os, const T& t) {
    { os << t } -> std::convertible_to<std::ostream&>;
};

enum class MatchMode {
    Prefix,
    Exact
};

template <StringLike Key, Streaming Value, MatchMode MatchMode = MatchMode::Exact>
class FlashRadixTree {
public:
#ifdef USE_SPLAY_TREE
    static constexpr size_t max_alignment_value_FlashRadixTreeNode = max_alignment<Key, Value, bool, SplayTree<typename Key::value_type, void*>>();
#elif defined USE_CHAR_MAP
    static constexpr size_t max_alignment_value_FlashRadixTreeNode = max_alignment<Key, Value, bool, CharMap<Value>>();
#else
    static constexpr size_t max_alignment_value_FlashRadixTreeNode = max_alignment<Key, Value, bool, std::unordered_map<typename Key::value_type, void*>>();
#endif
    class alignas(max_alignment_value_FlashRadixTreeNode) FlashRadixTreeNode {
    public:
#ifdef USE_SPLAY_TREE
        using Children = SplayTree<typename Key::value_type, FlashRadixTreeNode*>;
#elif defined USE_CHAR_MAP
        using Children = CharMap<FlashRadixTreeNode*>;
#else
        using Children = std::unordered_map<typename Key::value_type, FlashRadixTreeNode*>;
#endif
        Children children;
        bool isEndOfWord = false;
        Value value;
        Key prefix;
        bool deleted = false;
        
        FlashRadixTreeNode(const Key& prefix, const Value&& value) noexcept
        : prefix(prefix), value(std::move(value)), isEndOfWord(false)
        {};
        
        FlashRadixTreeNode(const Key& prefix, const Value&& value, bool isEndOfWord) noexcept
        : prefix(prefix), value(std::move(value)), isEndOfWord(isEndOfWord)
        {};
        
        FlashRadixTreeNode() = default;
        FlashRadixTreeNode(const FlashRadixTreeNode& ) = delete;
        FlashRadixTreeNode(const FlashRadixTreeNode&& other) noexcept
        {
            children = std::move(other.children);
            isEndOfWord = other.isEndOfWord;
            value = other.value;
            prefix = other.prefix;
        }
        constexpr void setDeleted() noexcept
        {
            deleted = true;
        }
    };
    using ValueType = Value;
private:
    FlashRadixTreeNode* _root;
    
    
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
    
    FlashRadixTreeNode* insert(const Key& key, const Value&& value) noexcept {
        if (_root == nullptr) {
            // If the root doesn't exist, create it.
            _root = new FlashRadixTreeNode();
        }
        
        FlashRadixTreeNode* currentNode = _root;
        FlashRadixTreeNode* inserted = nullptr;
        Key remaining = key;
        
        while (!remaining.empty()) {
            const auto it = currentNode->children.find(remaining[0]);
            
#if defined( USE_SPLAY_TREE) || defined USE_CHAR_MAP
            if (it != nullptr) {
#else
            if( it != currentNode->children.end()) {
#endif
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
                    auto* newChild = new FlashRadixTreeNode(commonPrefix, std::move((lineIsWholePrefix ? std::move(value) : Value())), lineIsWholePrefix);
                    
                    // The new node should adopt the existing child node
                    childNode->prefix = suffixEdge;
#if defined( USE_SPLAY_TREE)  || defined USE_CHAR_MAP
                    newChild->children.insert(suffixEdge[0], std::move(childNode));
#else
                    newChild->children.emplace(suffixEdge[0], std::move(childNode));
#endif
                    currentNode->children.erase(edgeKey);
                    
                    // Insert the new child with the common prefix in the current node's children
#if defined( USE_SPLAY_TREE) || defined USE_CHAR_MAP
                    currentNode = currentNode->children.insert(commonPrefix[0], std::move(newChild))->value;
#else
                    currentNode = currentNode->children.emplace(commonPrefix[0], std::move(newChild)).first->second;
#endif
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
#if defined( USE_SPLAY_TREE) || defined USE_CHAR_MAP
                currentNode = currentNode->children.insert(remaining[0], new FlashRadixTreeNode(remaining, std::move(value)))->value;
#else
                currentNode = currentNode->children.emplace(remaining[0], new FlashRadixTreeNode(remaining, std::move(value))).first->second;
#endif
                currentNode->isEndOfWord = true;
                
                // As we've inserted the rest of the key, we're done
                remaining = Key();
                inserted = currentNode;
            }
        }
        if(inserted == nullptr)
            return nullptr;
        else
            return inserted;
    }
        
    bool erase(const Key& key) noexcept
    {
        const auto found = find(key);
        if (found == nullptr) {
            return false;
        }
        else
        {
            found->deleted = true;
            return true;
        }
    }
    
    void print() const noexcept {
        _printRecursively(' ', _root, 0);
        std::cout << std::endl;
    }

    FlashRadixTreeNode* find(const Key& key) const noexcept {
        if (key.empty()) {
            return nullptr; // An empty key cannot be found.
        }
        
        const FlashRadixTreeNode* currentNode = _root;
        char keyPrefix = key[0];
        Key remaining = key;
        size_t seen = 0;
        while( currentNode != nullptr)
        {
            auto it = currentNode->children.find(keyPrefix);
#if defined( USE_SPLAY_TREE) || defined USE_CHAR_MAP
            if(it != nullptr)
#else
            if(it != currentNode->children.end())
#endif
            {
#if defined( USE_SPLAY_TREE) || defined USE_CHAR_MAP
                currentNode = it->value;
#else
                currentNode = it->second;
#endif
                if(MatchMode == MatchMode::Prefix && //if we are in prefix mode we can stop if we find the prefix
                   currentNode->isEndOfWord && currentNode->children.empty() )//if there are no children below this key we have our winner
                {
                    if(currentNode->deleted)
                        return nullptr;
                    else
                        return const_cast<FlashRadixTreeNode*>( currentNode);
                }
                else if(currentNode->isEndOfWord && remaining == currentNode->prefix) //else we no choice but to check the whole word
                {
                    if(currentNode->deleted)
                        return nullptr;
                    else
                        return const_cast<FlashRadixTreeNode*>( currentNode);
                }
                else if( key.size() > (seen += currentNode->prefix.size()) )
                {
                    //we look for children below looking at the next possible prefix
                    keyPrefix = key[seen];
                    remaining = key.substr(seen);
                }
            }
            else
                return nullptr; //assume doesn't exist
            
        }
        return nullptr;
    }
        
    void clear() noexcept {
        _clearRecursively(_root);
        _root = nullptr;
    }
        
    void inOrderAndOp(const std::function<bool(const Key&, const Value&)>& op) const noexcept {
        inOrderAndOpRecursively(_root, Key(), op);
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
        node->children.inOrderAndOp([&](const SplayTree<typename Key::value_type, FlashRadixTreeNode*>::splay* node)->bool {
            _printRecursively(node->key, node->value, level + 1);
            return true;
        });
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
        
    void _clearRecursively(FlashRadixTreeNode* node) noexcept {
        if (node == nullptr) {
            return;
        }
#if defined( USE_SPLAY_TREE)
        node->children.inOrderAndOp([&](const SplayTree<typename Key::value_type, FlashRadixTreeNode*>::splay* node)->bool {
            _clearRecursively(node->value);
            return true;
        });
#elif defined( USE_CHAR_MAP)
        node->children.inOrderAndOp([&](const auto node)->bool {
            _clearRecursively(node.value);
            return true;
        });
#else
        for (auto& it : node->children) {
            _clearRecursively(it.second);
        }
#endif
        
        delete node;
    }
public:
        
    //todo: work in progress
    /*bool erase(const Key& key) noexcept {
        if (key.empty()) {
            return false; // Cannot erase an empty key
        }

        FlashRadixTreeNode* currentNode = _root;
        Key remainingKey = key;
        std::stack<FlashRadixTreeNode*> path; // Stack to keep track of the path taken to find the key

        // Step 1: Find the node
        while (currentNode != nullptr && !remainingKey.empty()) {
            path.push(currentNode); // Push the current node onto the path
            auto it = currentNode->children.find(remainingKey.at(0));
            
            if (it == nullptr) {
                return false; // Key not found
            }

            const Key& nodePrefix = it->value->prefix;
            FlashRadixTreeNode* childNode = it->value;

            if (remainingKey.starts_with(nodePrefix)) {
                // Prefix matches, move to the next node
                remainingKey.substr(nodePrefix.size());
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

        // If the current node has children, we are done
        if (!currentNode->children.empty()) {
            return true;
        }

        // Step 3: Clean up the tree
        while (!path.empty() && currentNode->children.empty() && !currentNode->isEndOfWord) {
            FlashRadixTreeNode* parentNode = path.top();
            path.pop();

            // Remove the leaf node if it does not have any children
            parentNode->children.erase(remainingKey.substr(0, parentNode->prefix.size()).at(0));

            // If the parent node is now a leaf and is not an end-of-word node, set it as the current node for the next iteration
            if (parentNode->children.empty() && !parentNode->isEndOfWord) {
                currentNode = parentNode;
                if (!path.empty()) {
                    remainingKey = path.top()->prefix;
                }
            } else {
                // If the parent node still has children or it is an end-of-word node, we're done cleaning up
                break;
            }
        }

        return true;
    }*/
        

    

    
    
};


template <Streaming Key, Streaming Value, MatchMode FindMode = MatchMode::Exact>
class FlashRadixTreeSerializer {
public:
    static std::string serialize(const FlashRadixTree<Key, Value, FindMode>& tree) {
        return serializeNode(tree.getRoot());
    }

private:
    static std::string serializeNode(const typename FlashRadixTree<Key, Value, FindMode>::FlashRadixTreeNode* node) {
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
            node->children.inOrderAndOp([&ss, &first](const SplayTree<typename Key::value_type, typename FlashRadixTree<Key, Value, FindMode>::FlashRadixTreeNode*>::splay* childPair) -> bool {
                if (!first) {
                    ss << ",";
                }
                ss << serializeNode(childPair->value);
                first = false;
                return true;
            });
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
