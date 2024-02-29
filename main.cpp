//
//  main.cpp
//  FlashRadixTree
//
//  Created by Matthew Varendorff on 29/2/2024.
//
//  GNU GENERAL PUBLIC LICENSE
//  Version 3, 29 June 2007
//

#include "FlashRadixTree.hpp"
#include "Test.hpp"
#include "SplayTree.hpp"
#include <unordered_map>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

int main(int argc, const char * argv[]) {
    
    std::vector<std::string> symbols;
    std::ifstream file;
    file.open("/Users/matthew/Documents/Code/CPP/FlashRadixTree/FlashRadixTree/sample_data.txt");
    std::string line;
    while (std::getline(file, line)) {
        symbols.push_back(line);
    }
    file.close();
    
    FlashRadixTree<std::string_view, unsigned int, MatchMode::Prefix> tree;
    FlashRadixTree<std::string_view, unsigned int, MatchMode::Exact> treeExactMatch;
    std::unordered_map<std::string_view, unsigned int> hash_map;
    std::map<std::string_view, unsigned int> map;
    SplayTree<std::string_view, unsigned int> splay;
    unsigned int runNumbers = 100;
    
    auto startHashMap = std::chrono::high_resolution_clock::now();
    unsigned int count = 0;
    for(unsigned int i = 0; i < runNumbers; ++i)
    {
        
        for (auto &symbol : symbols) {
            auto it = hash_map.find(symbol); //typical usage
            hash_map.emplace_hint(it, symbol, count++);
        }
    }
    auto endHashMap = std::chrono::high_resolution_clock::now();
    
    auto mapStart = std::chrono::high_resolution_clock::now();
    count = 0;
    for(unsigned int i = 0; i < runNumbers; ++i)
    {
        for (auto &symbol : symbols) {
            auto it = map.find(symbol); //typical usage
            map.emplace_hint(it, symbol, count++);
        }
    }
    auto mapEnd = std::chrono::high_resolution_clock::now();
    
    auto startTree = std::chrono::high_resolution_clock::now();
    count = 0;
    for(unsigned int i = 0; i < runNumbers; ++i)
    {
        for (auto &symbol : symbols) {
            tree.insert(symbol, count++);
        }
    }
    auto endTree = std::chrono::high_resolution_clock::now();
    
    auto startTreeExactMatch = std::chrono::high_resolution_clock::now();
    count = 0;
    for(unsigned int i = 0; i < runNumbers; ++i)
    {
        for (auto &symbol : symbols) {
            treeExactMatch.insert(symbol, count++);
        }
    }
    auto endTreeExactMatch = std::chrono::high_resolution_clock::now();
    
    auto startSplay = std::chrono::high_resolution_clock::now();
    count = 0;
    for(unsigned int i = 0; i < runNumbers; ++i)
    {
        
        for (auto &symbol : symbols) {
            splay.insert(symbol, count++);
        }
    }
    auto endSplay = std::chrono::high_resolution_clock::now();
    
    std::cout << "hash map insert time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endHashMap - startHashMap)/(symbols.size() * runNumbers)).count() << "ns" << std::endl;
    
    std::cout << "map insert time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((mapEnd - mapStart)/(symbols.size() * runNumbers)).count() << "ns" << std::endl;
    
    std::cout << "tree prefix match insert time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endTree - startTree)/(symbols.size() * runNumbers)).count() << "ns" << std::endl;
    
    std::cout << "tree exact match insert time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endTreeExactMatch - startTreeExactMatch)/(symbols.size() * runNumbers)).count() << "ns" << std::endl;
    
    std::cout << "splay insert time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endSplay - startSplay)/(symbols.size() * runNumbers)).count() << "ns" << std::endl;
    
    std::cout << std::endl;
    
    //search for all symbols
    startHashMap = std::chrono::high_resolution_clock::now();
    for(unsigned int i = 0; i < runNumbers; ++i)
    {
        
        for (auto &symbol : symbols) {
            auto it = hash_map.find(symbol);
            if (it == hash_map.end()) {
                return 1;
            }
        }
    }
    endHashMap = std::chrono::high_resolution_clock::now();
    
    mapStart = std::chrono::high_resolution_clock::now();
    for(unsigned int i = 0; i < runNumbers; ++i)
    {
        
        for (auto &symbol : symbols) {
            auto it = map.find(symbol);
            if (it == map.end()) {
                return 1;
            }
        }
    }
    mapEnd = std::chrono::high_resolution_clock::now();
    
    startTree = std::chrono::high_resolution_clock::now();
    
    for(unsigned int i = 0; i < runNumbers; ++i)
    {
        for (auto &symbol : symbols) {
            if (tree.find(symbol) == nullptr) {
                return 1;
            }
        }
    }
    endTree = std::chrono::high_resolution_clock::now();
    
    startTreeExactMatch = std::chrono::high_resolution_clock::now();
    for(unsigned int i = 0; i < runNumbers; ++i)
    {
        for (auto &symbol : symbols) {
            if (treeExactMatch.find(symbol) == nullptr) {
                return 1;
            }
        }
    }
    endTreeExactMatch = std::chrono::high_resolution_clock::now();
    
    startSplay = std::chrono::high_resolution_clock::now();
    for(unsigned int i = 0; i < runNumbers; ++i)
    {
        
        for (auto &symbol : symbols) {
            if (splay.find(symbol) == nullptr) {
                return 1;
            }
        }
    }
    endSplay = std::chrono::high_resolution_clock::now();
    
    std::cout << "hash map find time:  " << std::chrono::duration_cast<std::chrono::nanoseconds>((endHashMap - startHashMap)/ (symbols.size() * runNumbers)).count() << "ns" << std::endl;
    
    std::cout << "map find time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((mapEnd - mapStart)/(symbols.size() * runNumbers)).count() << "ns" << std::endl;
    
    std::cout << "tree find time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endTree - startTree)/(symbols.size() * runNumbers)).count() << "ns" << std::endl;
    
    std::cout << "tree exact match find time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endTreeExactMatch - startTreeExactMatch)/(symbols.size() * runNumbers)).count() << "ns" << std::endl;
    
    std::cout << "splay find time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endSplay - startSplay)/(symbols.size() * runNumbers)).count() << "ns" << std::endl;
    
    std::cout << "Number of runs " << (symbols.size() * runNumbers) << std::endl;
    
    RunTests();
    
    return 0;
}
