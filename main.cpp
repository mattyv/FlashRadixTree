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
#include <optional>
#include <memory>
#ifdef __APPLE__
#include "M1-Cycles/m1cycles.h"
#endif
#include "LinuxPerformance.hpp"

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


#ifdef __APPLE__
std::string paddedString(const std::string& str, size_t minWidth) {
    if (str.length() >= minWidth) return str;
    return str + std::string(minWidth - str.length(), ' ');
}

void printResults(std::string op, performance_counters_holder& stats)
{
    performance_counters min = stats.min();
    performance_counters avg = stats.avg();
    performance_counters max = stats.max();
    performance_counters percentile90 = stats.percentile(0.9);
    performance_counters percentile50 = stats.percentile(0.5);
    performance_counters percentile10 = stats.percentile(0.1);

    // Determine the maximum width for each column
    std::vector<size_t> columnWidths = {
        24, // "Metric"
        10, // "Min"
        10, // "Average"
        10, // "Max"
        16, // "90th Percentile"
        16, // "50th Percentile"
        16  // "10th Percentile"
    };

    std::stringstream csvOutput;

    // Helper lambda function to format and pad numbers
    auto formatAndPadNumber = [&columnWidths](double number, size_t columnIndex) {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << number;
        return paddedString(stream.str(), columnWidths[columnIndex]);
    };

    // Header
    csvOutput << paddedString("\"Metric\"", columnWidths[0]) << ", ";
    csvOutput << paddedString("\"Min\"", columnWidths[1]) << ", ";
    csvOutput << paddedString("\"Average\"", columnWidths[2]) << ", ";
    csvOutput << paddedString("\"Max\"", columnWidths[3]) << ", ";
    csvOutput << paddedString("\"90th Percentile\"", columnWidths[4]) << ", ";
    csvOutput << paddedString("\"50th Percentile\"", columnWidths[5]) << ", ";
    csvOutput << paddedString("\"10th Percentile\"", columnWidths[6]) << "\n";

    // Instructions
    csvOutput << paddedString("\"Instructions/" + op + "\"", columnWidths[0]) << ", ";
    csvOutput << formatAndPadNumber(min.instructions, 1) << ", ";
    csvOutput << formatAndPadNumber(avg.instructions, 2) << ", ";
    csvOutput << formatAndPadNumber(max.instructions, 3) << ", ";
    csvOutput << formatAndPadNumber(percentile90.instructions, 4) << ", ";
    csvOutput << formatAndPadNumber(percentile50.instructions, 5) << ", ";
    csvOutput << formatAndPadNumber(percentile10.instructions, 6) << "\n";

    // Cycles
    csvOutput << paddedString("\"Cycles/" + op + "\"", columnWidths[0]) << ", ";
    csvOutput << formatAndPadNumber(min.cycles, 1) << ", ";
    csvOutput << formatAndPadNumber(avg.cycles, 2) << ", ";
    csvOutput << formatAndPadNumber(max.cycles, 3) << ", ";
    csvOutput << formatAndPadNumber(percentile90.cycles, 4) << ", ";
    csvOutput << formatAndPadNumber(percentile50.cycles, 5) << ", ";
    csvOutput << formatAndPadNumber(percentile10.cycles, 6) << "\n";

    // Instructions per Cycle
    csvOutput << paddedString("\"Instructions/Cycle\"", columnWidths[0]) << ", ";
    csvOutput << formatAndPadNumber(min.instructions / (min.cycles ? min.cycles : 1), 1) << "\n";

    // Branches
    csvOutput << paddedString("\"Branches/" + op + "\"", columnWidths[0]) << ", ";
    csvOutput << formatAndPadNumber(min.branches, 1) << ", ";
    csvOutput << formatAndPadNumber(avg.branches, 2) << ", ";
    csvOutput << formatAndPadNumber(max.branches, 3) << ", ";
    csvOutput << formatAndPadNumber(percentile90.branches, 4) << ", ";
    csvOutput << formatAndPadNumber(percentile50.branches, 5) << ", ";
    csvOutput << formatAndPadNumber(percentile10.branches, 6) << "\n";

    // Missed Branches
    csvOutput << paddedString("\"Missed Branches/" + op + "\"", columnWidths[0]) << ", ";
    csvOutput << formatAndPadNumber(min.missed_branches, 1) << ", ";
    csvOutput << formatAndPadNumber(avg.missed_branches, 2) << ", ";
    csvOutput << formatAndPadNumber(max.missed_branches, 3) << ", ";
    csvOutput << formatAndPadNumber(percentile90.missed_branches, 4) << ", ";
    csvOutput << formatAndPadNumber(percentile50.missed_branches, 5) << ", ";
    csvOutput << formatAndPadNumber(percentile10.missed_branches, 6) << "\n";

    // Output the formatted CSV data
    std::cout << csvOutput.str();
}

template< class Data>
void runTest(int numOfRuns, performance_counters_holder& stats,
             std::function<void(const typename Data::value_type&)> testFunction,
             Data& data){
    for (int i = 0; i < numOfRuns; ++i) {
        for(auto& item : data)
        {
            performance_counters start = get_counters();
            testFunction(item);
            performance_counters end = get_counters();
            
            performance_counters diff = end - start;
            stats.push_back(diff);
        }
    }
}

template< class Data>
void runTest(performance_counters_holder& stats,
             std::function<void( typename Data::iterator&)> testFunction,
             Data& data){
    for(auto it = data.begin(); it != data.end();)
    {
        performance_counters start = get_counters();
        testFunction(it);
        performance_counters end = get_counters();
        
        performance_counters diff = end - start;
        stats.push_back(diff);
    }
}

template< class Data, class DataTop10>
bool runTest(int numOfRuns, performance_counters_holder& stats, performance_counters_holder& statsTop10,
             std::function<bool(const typename Data::value_type&, performance_counters& start, performance_counters& end)> testFunction,
             Data& data, DataTop10 top10){
    for(unsigned int i = 0; i < numOfRuns; ++i)
    {
        for (auto &item :data) {
            performance_counters start, end;
            if(!testFunction(item, start, end))
                return false;
            performance_counters diff = end - start;
            stats.push_back(diff);
            
            if(top10.find(item) != top10.end()) //look at top keys to get a closer look
                statsTop10.push_back(diff);
        }
    }
    return true;
}

#endif

int main(int argc, const char * argv[]) {
    
    if(!RunTests())
        return 1;
    
#ifdef __APPLE__
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
    
    performance_counters_holder hash_map_counters;
    auto emplaceHashMap = [&hash_map](const decltype(uniqueSymbols)::value_type& item){
         hash_map.emplace(item, Data());
    };
    runTest<decltype(uniqueSymbols)> (1, hash_map_counters, emplaceHashMap, uniqueSymbols);
    
    performance_counters_holder map_counters;
    auto emplaceMap = [&map](const decltype(uniqueSymbols)::value_type& item){
        map.emplace(item, Data());
    };
    runTest<decltype(uniqueSymbols)> (1, map_counters, emplaceMap, uniqueSymbols);
    
    performance_counters_holder tree_counters;
    auto insertTree = [&tree](const decltype(uniqueSymbols)::value_type& item){
        tree.insert(item, Data());
    };
    runTest<decltype(uniqueSymbols)> (1, tree_counters, insertTree, uniqueSymbols);
    
    
    performance_counters_holder treeExactMatch_counters;
    auto insertTreeExact = [&treeExactMatch](const decltype(uniqueSymbols)::value_type& item){
        treeExactMatch.insert(item, Data());
    };
    runTest<decltype(uniqueSymbols)> (1, treeExactMatch_counters, insertTreeExact, uniqueSymbols);
    
    performance_counters_holder splay_counters;
    auto insertSplay = [&splay](const decltype(uniqueSymbols)::value_type& item){
        splay.insert(item, Data());
    };
    runTest<decltype(uniqueSymbols)> (1, splay_counters, insertSplay, uniqueSymbols);
    
    
    std::cout << "hash map insert time" << std::endl;
    printResults("insert()",hash_map_counters);
    
    std::cout << "map insert time" << std::endl;
    printResults("insert()", map_counters);
    
    std::cout << "tree insert time" << std::endl;
    printResults("insert()", tree_counters);
    
    std::cout << "tree exact match insert time" << std::endl;
    printResults("insert()", treeExactMatch_counters);
    
    std::cout << "splay insert time" << std::endl;
    printResults("insert()", splay_counters);
    
    std::cout << std::endl;
    
    //iterate over all the symbols
    hash_map_counters.clear();
    auto iterateHash = [&hash_map]( decltype(hash_map)::iterator& it){
        ++it;
    };
    runTest<decltype(hash_map)>(hash_map_counters, iterateHash, hash_map);
    
    
    map_counters.clear();
    auto iterateMap = [&map]( decltype(map)::iterator& it)
    {
        ++it;
    };
    runTest<decltype(map)>(map_counters, iterateMap, map);
    
    
    tree_counters.clear();
    auto iterateTree = [&tree]( decltype(tree)::iterator& it)
    {
        ++it;
    };
    runTest<decltype(tree)>(tree_counters, iterateTree, tree);
    
    treeExactMatch_counters.clear();
    auto iterateTreeExact = [&treeExactMatch]( decltype(treeExactMatch)::iterator& it)
    {
        ++it;
    };
    runTest<decltype(treeExactMatch)>(treeExactMatch_counters, iterateTreeExact, treeExactMatch);
    
    splay_counters.clear();
    auto iterateSplay = [&splay]( decltype(splay)::iterator& it)
    {
        ++it;
    };
    runTest<decltype(splay)>(splay_counters, iterateSplay, splay);
    
    
    std::cout << "hash map iterate time" << std::endl;
    printResults("++it", hash_map_counters);
    
    std::cout << "map iterate time" << std::endl;
    printResults("++it", map_counters);
    
    std::cout << "tree iterate time" << std::endl;
    printResults("++it", tree_counters);
    
    std::cout << "tree exact match iterate time" << std::endl;
    printResults("++it", treeExactMatch_counters);
    
    std::cout << "splay iterate time" << std::endl;
    printResults("++it", splay_counters);
    
    std::cout << std::endl;
    
    //iterate forwards and compare to unique keys
    std::stringstream ssUniqueSymbolsForwards;
    for (auto symbol : uniqueSymbols) {
        ssUniqueSymbolsForwards << symbol << " ";
    }
    
    std::stringstream ssTreeForwards;
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        ssTreeForwards << it->getFullKey() << " ";
    }
    
    std::string uniqueSymbolsForwards = ssUniqueSymbolsForwards.str();
    std::string treeForwards = ssTreeForwards.str();
    //compare
    if (uniqueSymbolsForwards != treeForwards) {
        std::cerr << "Tree forwards iterator does not match unique keys" << std::endl;
        
        //iterate again and stop at the difference
        auto itUnique = uniqueSymbols.begin();
        auto itTree = tree.begin();
        while (itUnique != uniqueSymbols.end() && itTree != tree.end()) {
            std::cout << *itUnique << " " << itTree->getFullKey() << std::endl;
            if (*itUnique != itTree->getFullKey()) {
                std::cerr << "Mismatch at " << *itUnique << " " << itTree->getFullKey() << std::endl;
                break;
            }
            ++itUnique;
            ++itTree;
        }
    }
    else
        std::cout << "Tree forwards iterator matches unique keys" << std::endl;
    
    //same test but in reverse
    std::stringstream ssUniqueSymbolsReverse;
    for (auto symbol = uniqueSymbols.rbegin(); symbol != uniqueSymbols.rend(); ++symbol) {
        ssUniqueSymbolsReverse << *symbol << " ";
    }
    
    std::stringstream ssTreeReverse;
    for (auto it = tree.rbegin(); it != tree.rend(); ++it) {
        ssTreeReverse << it->getFullKey() << " ";
    }
    
    std::string uniqueSymbolsReverse = ssUniqueSymbolsReverse.str();
    std::string treeReverse = ssTreeReverse.str();
    //compare
    if (uniqueSymbolsReverse != treeReverse) {
        std::cerr << "Tree reverse iterator does not match unique keys" << std::endl;
        
        //iterate again and stop at the difference
        auto itUnique = uniqueSymbols.rbegin();
        auto itTree = tree.rbegin();
        while (itUnique != uniqueSymbols.rend() && itTree != tree.rend()) {
            std::cout << *itUnique << " " << itTree->getFullKey() << std::endl;
            if (*itUnique != itTree->getFullKey()) {
                std::cerr << "Mismatch at " << *itUnique << " " << itTree->getFullKey() << std::endl;
                break;
            }
            ++itUnique;
            ++itTree;
        }
    }
    else
        std::cout << "Tree reverse iterator matches unique keys" << std::endl;
    
    //search for all symbols runNumber times
    hash_map_counters.clear();
    performance_counters_holder hash_map_countersTop10;
    auto findHashMap = [&hash_map](const std::string &symbol, performance_counters& start, performance_counters& end) ->bool {
        start = get_counters();
        auto it = hash_map.find(symbol);
        end = get_counters();
        return it != hash_map.end();
    };
    runTest(runNumbers, hash_map_counters, hash_map_countersTop10, findHashMap, symbols, topKeys);
    
    map_counters.clear();
    performance_counters_holder map_countersTop10;
    auto findMap = [&map](const std::string &symbol, performance_counters& start, performance_counters& end) ->bool {
        start = get_counters();
        auto it = map.find(symbol);
        end = get_counters();
        return it != map.end();
    };
    runTest(runNumbers, map_counters, map_countersTop10, findMap, symbols, topKeys);
    
    
    tree_counters.clear();
    performance_counters_holder tree_countersTop10;
    auto findTree = [&tree](const std::string &symbol, performance_counters& start, performance_counters& end) ->bool {
        start = get_counters();
        const auto found = tree.get(symbol);
        end = get_counters();
        return found != nullptr;
    };
    runTest(runNumbers, tree_counters, tree_countersTop10, findTree, symbols, topKeys);
    
    treeExactMatch_counters.clear();
    performance_counters_holder treeExactMatch_countersTop10;
    auto findTreeExactMatch = [&treeExactMatch](const std::string &symbol, performance_counters& start, performance_counters& end) ->bool {
        start = get_counters();
        auto found = treeExactMatch.find(symbol);
        end = get_counters();
        return found != treeExactMatch.end();
    };
    runTest(runNumbers, treeExactMatch_counters, treeExactMatch_countersTop10, findTreeExactMatch, symbols, topKeys);
    
    splay_counters.clear();
    performance_counters_holder splay_countersTop10;
    auto findSplay = [&splay](const std::string &symbol, performance_counters& start, performance_counters& end) ->bool {
        start = get_counters();
        auto found = splay.get(symbol);
        end = get_counters();
        return found != nullptr;
    };
    runTest(runNumbers, splay_counters, splay_countersTop10, findSplay, symbols, topKeys);
    
    std::cout << "hash map find time" << std::endl;
    printResults("find()", hash_map_counters);
    
    std::cout << "hash map top 10% find time" << std::endl;
    printResults("find()", hash_map_countersTop10);
    
    std::cout << "map find time" << std::endl;
    printResults("find()", map_counters);
    
    std::cout << "map top 10% find time" << std::endl;
    printResults("find()", map_countersTop10);
    
    std::cout << "tree prefix match find time" << std::endl;
    printResults("find()", tree_counters);
    
    std::cout << "tree top 10% prefix match find time" << std::endl;
    printResults("find()", tree_countersTop10);
    
    std::cout << "tree exact match find time" << std::endl;
    printResults("find()", treeExactMatch_counters);
    
    std::cout << "tree exact match top 10% find time" << std::endl;
    printResults("find()", treeExactMatch_countersTop10);
    
    std::cout << "splay find time" << std::endl;
    printResults("find()", splay_counters);
    
    std::cout << "splay top 10% find time" << std::endl;
    printResults("find()", splay_countersTop10);
    

    hash_map_counters.clear();
    auto hashErase = [&hash_map](const decltype(uniqueSymbols)::value_type& symbol) {
        hash_map.erase(symbol);
    };
    runTest<decltype(uniqueSymbols)> (1, hash_map_counters, hashErase, uniqueSymbols);
    
    map_counters.clear();
    auto mapErase = [&map](const decltype(uniqueSymbols)::value_type& symbol) {
        map.erase(symbol);
    };
    runTest<decltype(uniqueSymbols)> (1, map_counters, mapErase, uniqueSymbols);
    
    tree_counters.clear();
    auto treeErase = [&tree](const decltype(uniqueSymbols)::value_type& symbol) {
        tree.erase(symbol);
    };
    runTest<decltype(uniqueSymbols)> (1, tree_counters, treeErase, uniqueSymbols);
    
    treeExactMatch_counters.clear();
    auto treeExactMatchErase = [&treeExactMatch](const decltype(uniqueSymbols)::value_type& symbol) {
        treeExactMatch.erase(symbol);
    };
    runTest<decltype(uniqueSymbols)> (1, treeExactMatch_counters, treeExactMatchErase, uniqueSymbols);
    
    splay_counters.clear();
    auto splayErase = [&splay](const decltype(uniqueSymbols)::value_type& symbol) {
        splay.erase(symbol);
    };
    runTest<decltype(uniqueSymbols)> (1, splay_counters, splayErase, uniqueSymbols);
    
    
    std::cout << std::endl;
    
    std::cout << "hash map erase time" << std::endl;
    printResults("erase()", hash_map_counters);
    
    std::cout << "map erase time" << std::endl;
    printResults("erase()", map_counters);
    
    std::cout << "tree erase time" << std::endl;
    printResults("erase()", tree_counters);
    
    std::cout << "tree exact match erase time" << std::endl;
    printResults("erase()", treeExactMatch_counters);
    
    std::cout << "splay erase time" << std::endl;
    printResults("erase()", splay_counters);
    
    
    std::cout << "Number of runs for insert() " << uniqueSymbols.size() << std::endl;
    std::cout << std::endl << "Number of runs for find() " << (symbols.size() * runNumbers) << std::endl;
    
    
    std::cout << "Top 10% count of keys used (not printing keys as they can be long): " << std::endl;
    for (unsigned int i = 0; i < topKeys.size(); ++i) {
        std::cout << sorted[i].second << std::endl;
    }
#endif
    return 0;
}
