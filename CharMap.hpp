//
//  CharMap.hpp
//  marketdataplay
//
//  Created by Matthew Varendorff on 28/2/2024.
//
//  GNU GENERAL PUBLIC LICENSE
//  Version 3, 29 June 2007
//

#ifndef CharMap_h
#define CharMap_h

#include <vector>
#include <cstddef>


template <typename Value>
class CharMap {
public:
    static constexpr size_t max_alignment_value_CharMapNode = max_alignment<char, Value, bool>();
    struct CharMapNode
    {
        CharMapNode(char key, const Value&& value) noexcept: key(key), value(value)  {}
        CharMapNode() noexcept = default;
        char key = 0;
        Value value = Value{};
        bool empty = true;
    };
    // Assuming standard ASCII characters (0-127), adjust if necessary.
    static const std::size_t MAX_CHAR = 128;

    CharMap() : char_map(MAX_CHAR, CharMapNode{}) {}

    // Inserts a character into the CharMap
    CharMapNode* insert(char c, const Value&& value) noexcept {
        char_map[static_cast<size_t>(c)] = CharMapNode(c, std::move(value));
        return &char_map[static_cast<size_t>(c)];
    }

    // Finds a character in the CharMap
    CharMapNode* find(char c) const noexcept {
        const auto& found = char_map[static_cast<size_t>(c)];
        if(found.empty )
            return nullptr;
        return const_cast<CharMapNode*>(&found);
    }

    // Erases a character from the CharMap
    bool erase(char c) noexcept {
        auto& found = char_map[static_cast<size_t>(c)];
        if(found.empty )
            return false;
        else
        {
            found.empty = true;
            return true;
        }
    }
    
    void inOrderAndOp(std::function<bool(const CharMapNode&)> op) const noexcept
    {
        for(const auto& node : char_map)
        {
            if(!node.empty)
                if(!op(node))
                    break;
        }
    }
    
    constexpr bool empty() const noexcept
    {
        for(const auto& node : char_map)
            if(!node.empty)
                return false;
        return true;
    }

private:
    std::vector<CharMapNode> char_map;
};

#endif /* CharMap_h */
