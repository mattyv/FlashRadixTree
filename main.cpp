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
#include <unordered_set>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>

//struct to simulate and market order
struct Data
{
    std::string str;
    unsigned int data1 = 0;
    unsigned int data2 = 0;
    
    //streaming operator
    friend std::ostream& operator<<(std::ostream& os, const Data& order)
    {
        os << "str: " << order.str << " data1: " << order.data1 << " data2: " << order.data2;
        return os;
    }
};

int main(int argc, const char * argv[]) {
    
    if(!RunTests())
        return 1;
    
    
    std::vector<std::string> symbols;
    std::ifstream file;
    file.open("/Users/matthew/Documents/Code/CPP/FlashRadixTree/FlashRadixTree/sample_data.txt");
    std::string line;
    unsigned int n = 300;
    while (std::getline(file, line)) {
        //dupliacate each character in the string n times
        std::string newLine;
        for (auto &c : line) {
            for (unsigned int i = 0; i < n; ++i) {
                newLine.push_back(c);
            }
        }
        symbols.push_back(newLine);
    }
    file.close();
    
    std::unordered_set<string> uniqueSymbols(symbols.begin(), symbols.end());
    
    //print the size of one line in kilobytes
    if(symbols[0].size() > 1024)
        std::cout << "size of one line in kilobytes: " << symbols[0].size() / 1024.0 << "kb" << std::endl;
    else
        std::cout << "size of one line in bytes: " << symbols[0].size() << "b" << std::endl;
    
    FlashRadixTree<std::string, Data, MatchMode::Prefix> tree;
    FlashRadixTree<std::string, Data, MatchMode::Exact> treeExactMatch;
    std::unordered_map<std::string, Data> hash_map;
    std::map<std::string, Data> map;
    SplayTree<std::string, Data> splay;
    unsigned int runNumbers = 100;
    
    auto startHashMap = std::chrono::high_resolution_clock::now();
    for (auto &symbol : uniqueSymbols) {
            hash_map.emplace(symbol, Data());
    }
    auto endHashMap = std::chrono::high_resolution_clock::now();
    
    auto mapStart = std::chrono::high_resolution_clock::now();
    for (auto &symbol : uniqueSymbols) {
        map.emplace(symbol, Data()); //override
    }
    auto mapEnd = std::chrono::high_resolution_clock::now();
    
    auto startTree = std::chrono::high_resolution_clock::now();
    for (auto &symbol : uniqueSymbols) {
        tree.insert(symbol, Data());
    }
    auto endTree = std::chrono::high_resolution_clock::now();
    
    auto startTreeExactMatch = std::chrono::high_resolution_clock::now();
   for (auto &symbol : uniqueSymbols) {
        treeExactMatch.insert(symbol, Data());
    }
    auto endTreeExactMatch = std::chrono::high_resolution_clock::now();
    
    auto startSplay = std::chrono::high_resolution_clock::now();
    for (auto &symbol : uniqueSymbols) {
        splay.insert(symbol, Data());
    }
    auto endSplay = std::chrono::high_resolution_clock::now();
    
    
    std::cout << "hash map insert time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endHashMap - startHashMap)/symbols.size()).count() << "ns" << std::endl;
    
    std::cout << "map insert time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((mapEnd - mapStart)/symbols.size()).count() << "ns" << std::endl;
    
    std::cout << "tree prefix match insert time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endTree - startTree)/symbols.size()).count() << "ns" << std::endl;
    
    std::cout << "tree exact match insert time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endTreeExactMatch - startTreeExactMatch)/symbols.size()).count() << "ns" << std::endl;
    
    std::cout << "splay insert time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endSplay - startSplay)/symbols.size()).count() << "ns" << std::endl;
    
    std::cout << std::endl;
    
    //search for all symbols runNumber times
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
    
    std::cout << "tree prefix match find time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endTree - startTree)/(symbols.size() * runNumbers)).count() << "ns" << std::endl;
    
    std::cout << "tree exact match find time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endTreeExactMatch - startTreeExactMatch)/(symbols.size() * runNumbers)).count() << "ns" << std::endl;
    
    std::cout << "splay find time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endSplay - startSplay)/(symbols.size() * runNumbers)).count() << "ns" << std::endl;
    
    startHashMap = std::chrono::high_resolution_clock::now();
    for (auto symbol : uniqueSymbols) {
        hash_map.erase(symbol);
    }
    endHashMap = std::chrono::high_resolution_clock::now();
    
    mapStart = std::chrono::high_resolution_clock::now();
    for (auto symbol : uniqueSymbols) {
        map.erase(symbol);
    }
    mapEnd = std::chrono::high_resolution_clock::now();
    
    startTree = std::chrono::high_resolution_clock::now();
    for (auto symbol : uniqueSymbols) {
        tree.erase(symbol);
    }
    endTree = std::chrono::high_resolution_clock::now();
    
    startTreeExactMatch = std::chrono::high_resolution_clock::now();
    for (auto symbol : uniqueSymbols) {
        treeExactMatch.erase(symbol);
    }
    endTreeExactMatch = std::chrono::high_resolution_clock::now();
    
    startSplay = std::chrono::high_resolution_clock::now();
    for (auto symbol : uniqueSymbols) {
        splay.erase(symbol);
    }
    endSplay = std::chrono::high_resolution_clock::now();
    
    std::cout << std::endl;
    
    std::cout << "hash map erase time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endHashMap - startHashMap)/symbols.size()).count() << "ns" << std::endl;
    
    std::cout << "map erase time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((mapEnd - mapStart)/symbols.size()).count() << "ns" << std::endl;
    
    std::cout << "tree erase time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endTree - startTree)/symbols.size()).count() << "ns" << std::endl;
    
    std::cout << "tree exact match erase time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endTreeExactMatch - startTreeExactMatch)/symbols.size()).count() << "ns" << std::endl;
    
    std::cout << "splay erase time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endSplay - startSplay)/symbols.size()).count() << "ns" << std::endl;
    
    
    std::cout << "Number of runs for insert() " << uniqueSymbols.size() << std::endl;
    std::cout << std::endl << "Number of runs for find() " << (symbols.size() * runNumbers) << std::endl;
    
    //output the count of top 20% of keys used to give an idea of the distribution
    std::unordered_map<std::string, unsigned int> counts;
    for (auto &symbol : symbols) {
        ++counts[symbol];
    }
    std::vector<std::pair<std::string, unsigned int>> sorted(counts.begin(), counts.end());
    std::sort(sorted.begin(), sorted.end(), [](const std::pair<std::string, unsigned int> &a, const std::pair<std::string, unsigned int> &b) {
        return a.second > b.second;
    });
    
    std::cout << "Top 20% count of keys used (not printing keys as they can be long): " << std::endl;
    for (unsigned int i = 0; i < sorted.size() / 5; ++i) {
        std::cout << sorted[i].second << std::endl;
    }
    
    return 0;
}
