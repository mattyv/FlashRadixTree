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
#include <set>
#include <unordered_set>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include "M1-Cycles/m1cycles.h"

//struct to simulate a payload
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

void printResults(std::string op,performance_counters min, performance_counters avg, performance_counters max) {

    printf(" %8.2f instructions/%s min, %8.2f avg, %8.2f max ", min.instructions, op.c_str(),
           avg.instructions, max.instructions);
    printf("\n");
    printf(" %8.2f cycles/%s min, %8.2f avg, %8.2f max ", min.cycles, op.c_str(), avg.cycles, max.cycles);
    printf("\n");
    printf(" %8.2f instructions/cycle ",
           min.instructions / min.cycles);
    printf("\n");
    printf(" %8.2f branches/%s min, %8.2f avg, %8.2f max ", min.branches, op.c_str(), avg.branches, max.branches);
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
    unsigned int n = 1; //defines message size. Each character in the string is duplicated n times
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
    
    //find the count of top 10% of keys used to give an idea of the distribution
    std::unordered_map<std::string, unsigned int> counts;
    for (auto &symbol : symbols) {
        ++counts[symbol];
    }
    std::vector<std::pair<std::string, unsigned int>> sorted(counts.begin(), counts.end());
    std::sort(sorted.begin(), sorted.end(), [](const std::pair<std::string, unsigned int> &a, const std::pair<std::string, unsigned int> &b) {
        return a.second > b.second;
    });
    
    //erase lower 90% of the keys
    for (unsigned int i = 0; i < sorted.size() * (9.0 / 10.0); ++i) {
        counts.erase(sorted[i].first);
    }

    
    //create a set of top 10% of keys
    std::unordered_set<std::string> topKeys;
    for (auto &pair : counts) {
        topKeys.insert(pair.first);
    }
    
    std::set<std::string> uniqueSymbols(symbols.begin(), symbols.end());
    
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
    unsigned int runNumbers = 10;
    
    performance_counters agg_min_hash{1e300};
    performance_counters agg_avg_hash{0.0};
    performance_counters agg_max_hash{0.0};
    for (auto &symbol : uniqueSymbols) {
        performance_counters start = get_counters();
        hash_map.emplace(symbol, Data());
        performance_counters end = get_counters();
        
        performance_counters diff = end - start;
        agg_min_hash = agg_min_hash.min(diff);
        agg_max_hash = agg_max_hash.max(diff);
        agg_avg_hash += diff;
    }
    agg_avg_hash /= uniqueSymbols.size() ;
    
    performance_counters agg_min_map{1e300};
    performance_counters agg_avg_map{0.0};
    performance_counters agg_max_map{0.0};
    for (auto &symbol : uniqueSymbols) {
        performance_counters start = get_counters();
        map.emplace(symbol, Data());
        performance_counters end = get_counters();
        
        performance_counters diff = end - start;
        agg_min_map = agg_min_map.min(diff);
        agg_max_map = agg_max_map.max(diff);
        agg_avg_map += diff;
    }
    agg_avg_map /= uniqueSymbols.size() ;
    
    performance_counters agg_min_tree{1e300};
    performance_counters agg_avg_tree{0.0};
    performance_counters agg_max_tree{0.0};
    for (auto &symbol : uniqueSymbols) {
        performance_counters start = get_counters();
        tree.insert(symbol, Data());
        performance_counters end = get_counters();
        
        performance_counters diff = end - start;
        agg_min_tree = agg_min_tree.min(diff);
        agg_max_tree = agg_max_tree.max(diff);
        agg_avg_tree += diff;
    }
    agg_avg_tree /= uniqueSymbols.size() ;
    
    
    performance_counters agg_min_treeExact{1e300};
    performance_counters agg_avg_treeExact{0.0};
    performance_counters agg_max_treeExact{0.0};
    for (auto &symbol : uniqueSymbols) {
        performance_counters start = get_counters();
        treeExactMatch.insert(symbol, Data());
        performance_counters end = get_counters();
        
        performance_counters diff = end - start;
        agg_min_treeExact = agg_min_treeExact.min(diff);
        agg_max_treeExact = agg_max_treeExact.max(diff);
        agg_avg_treeExact += diff;
    }
    agg_avg_treeExact /= uniqueSymbols.size() ;
    
    performance_counters agg_min_splay{1e300};
    performance_counters agg_avg_splay{0.0};
    performance_counters agg_max_splay{0.0};
    for (auto &symbol : uniqueSymbols) {
        performance_counters start = get_counters();
        splay.insert(symbol, Data());
        performance_counters end = get_counters();
        
        performance_counters diff = end - start;
        agg_min_splay = agg_min_splay.min(diff);
        agg_max_splay = agg_max_splay.max(diff);
        agg_avg_splay += diff;
    }
    agg_avg_splay /= uniqueSymbols.size() ;
    
    
    std::cout << "hash map insert time" << std::endl;
    printResults("insert()",agg_min_hash, agg_avg_hash, agg_max_hash);
    
    std::cout << "map insert time" << std::endl;
    printResults("insert()", agg_min_map, agg_avg_map, agg_max_map);
    
    std::cout << "tree insert time" << std::endl;
    printResults("insert()",agg_min_tree, agg_avg_tree, agg_max_tree);
    
    std::cout << "tree exact match insert time" << std::endl;
    printResults("insert()",agg_min_treeExact, agg_avg_treeExact, agg_max_treeExact);
    
    std::cout << "splay insert time" << std::endl;
    printResults("insert()",agg_min_splay, agg_avg_splay, agg_max_splay);
    
    std::cout << std::endl;
    
    //iterate over all the symbols
    agg_min_hash = 1e300;
    agg_avg_hash = 0.0;
    agg_max_hash = 0.0;
    
    for( auto it  = map.begin(); it != map.end(); )
    {
        performance_counters start = get_counters();
        ++it;
        performance_counters end = get_counters();
     
        performance_counters diff = end - start;
        agg_min_hash = agg_min_hash.min(diff);
        agg_max_hash = agg_max_hash.max(diff);
        agg_avg_hash += diff;
        
    }
    agg_avg_hash /= map.size();
    
    
    //iterate over all the symbols
    agg_min_map = 1e300;
    agg_avg_map = 0.0;
    agg_max_map = 0.0;
    
    for( auto it  = map.begin(); it != map.end(); )
    {
        performance_counters start = get_counters();
        ++it;
        performance_counters end = get_counters();
     
        performance_counters diff = end - start;
        agg_min_map = agg_min_map.min(diff);
        agg_max_map = agg_max_map.max(diff);
        agg_avg_map += diff;
        
    }
    agg_avg_map /= map.size();
    
    
    //iterate over all the symbols
    agg_min_tree = 1e300;
    agg_avg_tree = 0.0;
    agg_max_tree = 0.0;
    
    std::stringstream ss;
    int count = 0;
    for( auto it  = tree.begin(); it != tree.end(); )
    {
        auto fullKey = it->getFullKey();
        ss << fullKey << "," << count << std::endl;
        performance_counters start = get_counters();
        ++it; ++count;
        performance_counters end = get_counters();
     
        performance_counters diff = end - start;
        agg_min_tree = agg_min_tree.min(diff);
        agg_max_tree = agg_max_tree.max(diff);
        agg_avg_tree += diff;
        
    }
    agg_avg_tree /= tree.size();
    
    std::cout << "Iterator symbols\n";
    std::cout << ss.str() << endl;
    
    
    std::cout << "Unique symbols: " << uniqueSymbols.size() << "\n";
    for(auto key : uniqueSymbols)
        std::cout << key << std::endl;
    
    //iterate over all the symbols
    agg_min_treeExact = 1e300;
    agg_avg_treeExact = 0.0;
    agg_max_treeExact = 0.0;
    
    for( auto it  = treeExactMatch.begin(); it != treeExactMatch.end(); )
    {
        performance_counters start = get_counters();
        ++it;
        performance_counters end = get_counters();
     
        performance_counters diff = end - start;
        agg_min_treeExact = agg_min_treeExact.min(diff);
        agg_max_treeExact = agg_max_treeExact.max(diff);
        agg_avg_treeExact += diff;
        
    }
    agg_avg_treeExact /= treeExactMatch.size();
    
    
    //iterate over all the symbols
    agg_min_splay = 1e300;
    agg_avg_splay = 0.0;
    agg_max_splay = 0.0;
    
    for( auto it  = splay.begin(); it != splay.end(); )
    {
        performance_counters start = get_counters();
        ++it;
        performance_counters end = get_counters();
     
        performance_counters diff = end - start;
        agg_min_splay = agg_min_splay.min(diff);
        agg_max_splay = agg_max_splay.max(diff);
        agg_avg_splay += diff;
        
    }
    agg_avg_splay /= splay.size();
    
    std::cout << "hash map iterate time" << std::endl;
    printResults("++it",agg_min_hash, agg_avg_hash, agg_max_hash);
    
    std::cout << "map iterate time" << std::endl;
    printResults("++it", agg_min_map, agg_avg_map, agg_max_map);
    
    std::cout << "tree iterate time" << std::endl;
    printResults("++it",agg_min_tree, agg_avg_tree, agg_max_tree);
    
    std::cout << "tree exact match iterate time" << std::endl;
    printResults("++it",agg_min_treeExact, agg_avg_treeExact, agg_max_treeExact);
    
    std::cout << "splay iterate time" << std::endl;
    printResults("++it",agg_min_splay, agg_avg_splay, agg_max_splay);
    
    std::cout << std::endl;
    
    //search for all symbols runNumber times
    agg_min_hash = 1e300;
    agg_avg_hash = 0.0;
    agg_max_hash = 0.0;
    
    performance_counters agg_min_hash_top10{1e300};
    performance_counters agg_avg_hash_top10{0.0};
    performance_counters agg_max_hash_top10{0.0};
    size_t topKeysCount = 0;
    
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
            agg_max_hash = agg_max_hash.max(diff);
            agg_avg_hash += diff;
            
            if(topKeys.find(symbol) != topKeys.end()) //look at top keys to get a closer look
            {
                agg_min_hash_top10 = agg_min_hash_top10.min(diff);
                agg_max_hash_top10 = agg_max_hash_top10.max(diff);
                agg_avg_hash_top10 += diff;
                ++topKeysCount;
            }
        }
    }
    agg_avg_hash /= symbols.size() * runNumbers;
    agg_avg_hash_top10 /= topKeysCount;
    
    
    agg_min_map = 1e300;
    agg_avg_map = 0.0;
    agg_max_map = 0.0;
    
    performance_counters agg_min_map_top10 = 1e300;
    performance_counters agg_avg_map_top10 = 0.0;
    performance_counters agg_max_map_top10 = 0.0;
    topKeysCount = 0;
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
            agg_max_map = agg_max_map.max(diff);
            agg_avg_map += diff;
            
            if(topKeys.find(symbol) != topKeys.end()) //look at top keys to get a closer look
            {
                agg_min_map_top10 = agg_min_map_top10.min(diff);
                agg_max_map_top10 = agg_max_map_top10.max(diff);
                agg_avg_map_top10 += diff;
                ++topKeysCount;
            }
        }
    }
    agg_avg_map /= symbols.size() * runNumbers;
    agg_avg_map_top10 /= topKeysCount;
    
    
    
    agg_min_tree = 1e300;
    agg_avg_tree = 0.0;
    agg_max_tree = 0.0;
    
    performance_counters agg_min_treeTop10 = 1e300;
    performance_counters agg_avg_treeTop10 = 0.0;
    performance_counters agg_max_treeTop10 = 0.0;
    topKeysCount = 0;
    
    for(unsigned int i = 0; i < runNumbers; ++i)
    {
        for (auto &symbol : symbols) {
            performance_counters start = get_counters();
            const auto found = tree.find(symbol);
            performance_counters end = get_counters();
            if (found == tree.end()) {
                return 1;
            }
            performance_counters diff = end - start;
            agg_min_tree = agg_min_tree.min(diff);
            agg_max_tree = agg_max_tree.max(diff);
            agg_avg_tree += diff;
            
            if(topKeys.find(symbol) != topKeys.end()) //look at top keys to get a closer look
            {
                agg_min_treeTop10 = agg_min_treeTop10.min(diff);
                agg_max_treeTop10 = agg_max_treeTop10.max(diff);
                agg_avg_treeTop10 += diff;
                ++topKeysCount;
            }
        }
    }
    agg_avg_tree /= symbols.size() * runNumbers;
    agg_avg_treeTop10 /= topKeysCount;
    
    agg_min_treeExact = 1e300;
    agg_avg_treeExact = 0.0;
    agg_max_treeExact = 0.0;
    
    performance_counters agg_min_treeExactTop10 = 1e300;
    performance_counters agg_avg_treeExactTop10 = 0.0;
    performance_counters agg_max_treeExactTop10 = 0.0;
    topKeysCount = 0;
    for(unsigned int i = 0; i < runNumbers; ++i)
    {
        for (auto &symbol : symbols) {
            performance_counters start = get_counters();
            auto found = treeExactMatch.find(symbol);
            performance_counters end = get_counters();
            if (found == treeExactMatch.end()) {
                return 1;
            }
            performance_counters diff = end - start;
            agg_min_treeExact = agg_min_treeExact.min(diff);
            agg_max_treeExact = agg_max_treeExact.max(diff);
            agg_avg_treeExact += diff;
            
            if(topKeys.find(symbol) != topKeys.end()) //look at top keys to get a closer look
            {
                agg_min_treeExactTop10 = agg_min_treeExactTop10.min(diff);
                agg_max_treeExactTop10 = agg_max_treeExactTop10.max(diff);
                agg_avg_treeExactTop10 += diff;
                ++topKeysCount;
            }
        }
    }
    agg_avg_treeExact /= symbols.size() * runNumbers;
    agg_avg_treeExactTop10 /= topKeysCount;
    
    agg_min_splay = 1e300;
    agg_avg_splay = 0.0;
    agg_max_splay = 0.0;
    
    performance_counters agg_min_splayTop10 = 1e300;
    performance_counters agg_avg_splayTop10 = 0.0;
    performance_counters agg_max_splayTop10 = 0.0;
    topKeysCount = 0;
    for(unsigned int i = 0; i < runNumbers; ++i)
    {
        
        for (auto &symbol : symbols) {
            performance_counters start = get_counters();
            auto found = splay.find(symbol);
            performance_counters end = get_counters();
            if (found == splay.end()) {
                return 1;
            }
            performance_counters diff = end - start;
            agg_min_splay = agg_min_splay.min(diff);
            agg_max_splay = agg_max_splay.max(diff);
            agg_avg_splay += diff;
            
            if(topKeys.find(symbol) != topKeys.end()) //look at top keys to get a closer look
            {
                agg_min_splayTop10 = agg_min_splayTop10.min(diff);
                agg_max_splayTop10 = agg_max_splayTop10.max(diff);
                agg_avg_splayTop10 += diff;
                ++topKeysCount;
            }
        }
    }
    agg_avg_splay /= symbols.size() * runNumbers;
    agg_avg_splayTop10 /= topKeysCount;
    
    std::cout << "hash map find time" << std::endl;
    printResults("find()", agg_min_hash, agg_avg_hash, agg_max_hash);
    
    std::cout << "hash map top 10% find time" << std::endl;
    printResults("find()", agg_min_hash_top10, agg_avg_hash_top10, agg_max_hash_top10);
    
    std::cout << "map find time" << std::endl;
    printResults("find()", agg_min_map, agg_avg_map, agg_max_map);
    
    std::cout << "map top 10% find time" << std::endl;
    printResults("find()", agg_min_map_top10, agg_avg_map_top10, agg_max_map_top10);
    
    std::cout << "tree prefix match find time" << std::endl;
    printResults("find()", agg_min_tree, agg_avg_tree, agg_max_tree);
    
    std::cout << "tree top 10% prefix match find time" << std::endl;
    printResults("find()", agg_min_treeTop10, agg_avg_treeTop10, agg_max_treeTop10);
    
    std::cout << "tree exact match find time" << std::endl;
    printResults("find()", agg_min_treeExact, agg_avg_treeExact, agg_max_treeExact);
    
    std::cout << "tree exact match top 10% find time" << std::endl;
    printResults("find()", agg_min_treeExactTop10, agg_avg_treeExactTop10, agg_max_treeExactTop10);
    
    std::cout << "splay find time" << std::endl;
    printResults("find()", agg_min_splay, agg_avg_splay, agg_max_splay);
    
    std::cout << "splay top 10% find time" << std::endl;
    printResults("find()", agg_min_splayTop10, agg_avg_splayTop10, agg_max_splayTop10);
    
    agg_min_hash = 1e300;
    agg_avg_hash = 0.0;
    agg_max_hash = 0.0;
    for (auto symbol : uniqueSymbols) {
        performance_counters start = get_counters();
        hash_map.erase(symbol);
        performance_counters end = get_counters();
        
        performance_counters diff = end - start;
        agg_min_hash = agg_min_hash.min(diff);
        agg_max_hash = agg_max_hash.max(diff);
        agg_avg_hash += diff;
    }
    agg_avg_hash /= uniqueSymbols.size() ;
    
    
    agg_min_map = 1e300;
    agg_avg_map = 0.0;
    agg_max_map = 0.0;
    for (auto symbol : uniqueSymbols) {
        performance_counters start = get_counters();
        map.erase(symbol);
        performance_counters end = get_counters();
        
        performance_counters diff = end - start;
        agg_min_map = agg_min_map.min(diff);
        agg_max_map = agg_max_map.max(diff);
        agg_avg_map += diff;
    }
    agg_avg_map /= uniqueSymbols.size() ;
    
    agg_min_tree = 1e300;
    agg_avg_tree = 0.0;
    agg_max_tree = 0.0;
    for (auto symbol : uniqueSymbols) {
        performance_counters start = get_counters();
        tree.erase(symbol);
        performance_counters end = get_counters();
        
        performance_counters diff = end - start;
        agg_min_tree = agg_min_tree.min(diff);
        agg_max_tree = agg_max_tree.max(diff);
        agg_avg_tree += diff;
    }
    agg_avg_tree /= uniqueSymbols.size() ;
    
    
    agg_min_treeExact = 1e300;
    agg_avg_treeExact = 0.0;
    agg_max_treeExact = 0.0;
    for (auto symbol : uniqueSymbols) {
        performance_counters start = get_counters();
        treeExactMatch.erase(symbol);
        performance_counters end = get_counters();
        
        performance_counters diff = end - start;
        agg_min_treeExact = agg_min_treeExact.min(diff);
        agg_max_treeExact = agg_max_treeExact.max(diff);
        agg_avg_treeExact += diff;
    }
    agg_avg_treeExact /= uniqueSymbols.size() ;
    
    
    agg_min_splay = 1e300;
    agg_avg_splay = 0.0;
    agg_max_splay = 0.0;
    for (auto symbol : uniqueSymbols) {
        performance_counters start = get_counters();
        splay.erase(symbol);
        performance_counters end = get_counters();
        
        performance_counters diff = end - start;
        agg_min_splay = agg_min_splay.min(diff);
        agg_max_splay = agg_max_splay.max(diff);
        agg_avg_splay += diff;
    }
    agg_avg_splay /= uniqueSymbols.size() ;
    
    
    std::cout << std::endl;
    
    std::cout << "hash map erase time" << std::endl;
    printResults("erase()", agg_min_hash, agg_avg_hash, agg_max_hash);
    
    std::cout << "map erase time" << std::endl;
    printResults("erase()", agg_min_map, agg_avg_map, agg_max_map);
    
    std::cout << "tree erase time" << std::endl;
    printResults("erase()", agg_min_tree, agg_avg_tree, agg_max_tree);
    
    
    std::cout << "tree exact match erase time" << std::endl;
    printResults("erase()", agg_min_treeExact, agg_avg_treeExact, agg_max_treeExact);
    
    std::cout << "splay erase time" << std::endl;
    printResults("erase()", agg_min_splay, agg_avg_splay, agg_max_splay);
    
    
    std::cout << "Number of runs for insert() " << uniqueSymbols.size() << std::endl;
    std::cout << std::endl << "Number of runs for find() " << (symbols.size() * runNumbers) << std::endl;
    
    
    std::cout << "Top 10% count of keys used (not printing keys as they can be long): " << std::endl;
    for (unsigned int i = 0; i < topKeys.size(); ++i) {
        std::cout << sorted[i].second << std::endl;
    }
    
    return 0;
}
