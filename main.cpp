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
#include "M1-Cycles/m1cycles.h"

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

void printResults(std::string op,performance_counters min, performance_counters avg) {
  printf(" %8.2f instructions/%s (+/- %3.1f %%) ", min.instructions, op.c_str(),
         (avg.instructions - min.instructions) * 100.0 /
             min.instructions);
  printf("\n");
  printf(" %8.2f cycles/%s (+/- %3.1f %%) ", min.cycles, op.c_str(),
         (avg.cycles - min.cycles) * 100.0 /
             min.cycles);
  printf("\n");
  printf(" %8.2f instructions/cycle ",
         min.instructions / min.cycles);
  printf("\n");
  printf(" %8.2f branches/%s (+/- %3.1f %%) ", min.branches, op.c_str(),
         (avg.branches - min.branches) * 100.0 /
             min.branches);
  printf("\n");
  printf(" %8.4f mis. branches/%s ", avg.missed_branches, op.c_str());
  printf("\n");
}

int main(int argc, const char * argv[]) {
    
    if(!RunTests())
        return 1;
    
    
    std::vector<std::string> symbols;
    std::ifstream file;
    file.open("/Users/matthew/Documents/Code/CPP/FlashRadixTree/FlashRadixTree/sample_data.txt");
    std::string line;
    unsigned int n = 1;
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
    setup_performance_counters();
    unsigned int runNumbers = 100;
    
    performance_counters agg_min_hash{1e300};
    performance_counters agg_avg_hash{0.0};
    for (auto &symbol : uniqueSymbols) {
        performance_counters start = get_counters();
        hash_map.emplace(symbol, Data());
        performance_counters end = get_counters();
        
        performance_counters diff = end - start;
        agg_min_hash = agg_min_hash.min(diff);
        agg_avg_hash += diff;
    }
    agg_avg_hash /= uniqueSymbols.size() ;
    
    performance_counters agg_min_map{1e300};
    performance_counters agg_avg_map{0.0};
    for (auto &symbol : uniqueSymbols) {
        performance_counters start = get_counters();
        map.emplace(symbol, Data());
        performance_counters end = get_counters();
        
        performance_counters diff = end - start;
        agg_min_map = agg_min_map.min(diff);
        agg_avg_map += diff;
    }
    agg_avg_map /= uniqueSymbols.size() ;
    
    performance_counters agg_min_tree{1e300};
    performance_counters agg_avg_tree{0.0};
    for (auto &symbol : uniqueSymbols) {
        performance_counters start = get_counters();
        tree.insert(symbol, Data());
        performance_counters end = get_counters();
        
        performance_counters diff = end - start;
        agg_min_tree = agg_min_tree.min(diff);
        agg_avg_tree += diff;
    }
    agg_avg_tree /= uniqueSymbols.size() ;
    
    
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
    
    
    std::cout << "hash map insert time" << std::endl;
    printResults("insert()",agg_min_hash, agg_avg_hash);
    
    std::cout << "map insert time" << std::endl;
    printResults("insert()", agg_min_map, agg_avg_map);
    
    std::cout << "tree insert time" << std::endl;
    printResults("insert()",agg_min_tree, agg_avg_tree);
    
    std::cout << "tree exact match insert time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endTreeExactMatch - startTreeExactMatch)/symbols.size()).count() << "ns" << std::endl;
    
    std::cout << "splay insert time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endSplay - startSplay)/symbols.size()).count() << "ns" << std::endl;
    
    std::cout << std::endl;
    
    //search for all symbols runNumber times
    agg_min_hash = 1e300;
    agg_avg_hash = 0.0;
    for(unsigned int i = 0; i < runNumbers; ++i)
    {
     
        for (auto &symbol : symbols) {
            performance_counters start = get_counters();
            auto it = hash_map.find(symbol);
            performance_counters end = get_counters();
            
            if (it == hash_map.end()) {
                return 1;
            }
            performance_counters diff = end - start;
            agg_min_hash = agg_min_hash.min(diff);
            agg_avg_hash += diff;
        }
    }
    agg_avg_hash /= symbols.size() * runNumbers;
    
    
    agg_min_map = 1e300;
    agg_avg_map = 0.0;
    for(unsigned int i = 0; i < runNumbers; ++i)
    {
        
        for (auto &symbol : symbols) {
            performance_counters start = get_counters();
            auto it = map.find(symbol);
            performance_counters end = get_counters();
            if (it == map.end()) {
                return 1;
            }
            
            performance_counters diff = end - start;
            agg_min_map = agg_min_map.min(diff);
            agg_avg_map += diff;
        }
        agg_avg_map /= symbols.size() * runNumbers;
    }
    
    
    
    agg_min_tree = 1e300;
    agg_avg_tree = 0.0;
    for(unsigned int i = 0; i < runNumbers; ++i)
    {
        for (auto &symbol : symbols) {
            performance_counters start = get_counters();
            const auto found = tree.find(symbol);
            performance_counters end = get_counters();
            if (found == nullptr) {
                return 1;
            }
            performance_counters diff = end - start;
            agg_min_tree = agg_min_tree.min(diff);
            agg_avg_tree += diff;
        }
    }
    agg_avg_tree /= symbols.size() * runNumbers;
    
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
    
    std::cout << "hash map find time" << std::endl;
    printResults("find()", agg_min_hash, agg_avg_hash);
    
    std::cout << "map find time" << std::endl;
    printResults("find()", agg_min_map, agg_avg_map);
    
    std::cout << "tree prefix match find time" << std::endl;
    printResults("find()", agg_min_tree, agg_avg_tree);
    
    std::cout << "tree exact match find time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endTreeExactMatch - startTreeExactMatch)/(symbols.size() * runNumbers)).count() << "ns" << std::endl;
    
    std::cout << "splay find time: " << std::chrono::duration_cast<std::chrono::nanoseconds>((endSplay - startSplay)/(symbols.size() * runNumbers)).count() << "ns" << std::endl;
    
    agg_min_hash = 1e300;
    agg_avg_hash = 0.0;
    for (auto symbol : uniqueSymbols) {
        performance_counters start = get_counters();
        hash_map.erase(symbol);
        performance_counters end = get_counters();
        
        performance_counters diff = end - start;
        agg_min_hash = agg_min_hash.min(diff);
        agg_avg_hash += diff;
    }
    agg_avg_hash /= uniqueSymbols.size() ;
    
    
    agg_min_map = 1e300;
    agg_avg_map = 0.0;
    for (auto symbol : uniqueSymbols) {
        performance_counters start = get_counters();
        map.erase(symbol);
        performance_counters end = get_counters();
        
        performance_counters diff = end - start;
        agg_min_map = agg_min_map.min(diff);
        agg_avg_map += diff;
    }
    agg_avg_map /= uniqueSymbols.size() ;
    
    agg_min_tree = 1e300;
    agg_avg_tree = 0.0;
    for (auto symbol : uniqueSymbols) {
        performance_counters start = get_counters();
        tree.erase(symbol);
        performance_counters end = get_counters();
        
        performance_counters diff = end - start;
        agg_min_tree = agg_min_tree.min(diff);
        agg_avg_tree += diff;
    }
    agg_avg_tree /= uniqueSymbols.size() ;
    
    
    startTreeExactMatch = std::chrono::high_resolution_clock::now();
    for (auto symbol : uniqueSymbols) {
        treeExactMatch.erase(symbol);
    }
    endTreeExactMatch = std::chrono::high_resolution_clock::now();
    
    
    auto minSplayDuration = std::numeric_limits<long long>::max();
    for (auto symbol : uniqueSymbols) {
        const auto startSplay = std::chrono::steady_clock::now();
        splay.erase(symbol);
        const auto endSplay = std::chrono::steady_clock::now();
        
        const auto durationSplay = std::chrono::duration_cast<std::chrono::nanoseconds>(endSplay - startSplay).count();
        minSplayDuration = std::min(minSplayDuration, durationSplay);
    }
    
    
    std::cout << std::endl;
    
    std::cout << "hash map erase time" << std::endl;
    printResults("erase()", agg_min_hash, agg_avg_hash);
    
    std::cout << "map erase time" << std::endl;
    printResults("erase()", agg_min_map, agg_avg_map);
    
    std::cout << "tree erase time" << std::endl;
    printResults("erase()", agg_min_tree, agg_avg_tree);
    
    
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
    
    std::cout << "Top 10% count of keys used (not printing keys as they can be long): " << std::endl;
    for (unsigned int i = 0; i < sorted.size() / 10; ++i) {
        std::cout << sorted[i].second << std::endl;
    }
    
    return 0;
}
