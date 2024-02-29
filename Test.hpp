#ifndef TEST_HPP
#define TEST_HPP
#include <iostream>
#include <unordered_map>
#include "FlashRadixTree.hpp"

bool RunTests()
{
    std::cout << "Testing out redix tree \n";
    FlashRadixTree<std::string, int, MatchMode::Prefix> rTree;
    FlashRadixTreeSerializer<std::string, int, MatchMode::Prefix> serializer;
    
    int value = 0;
    auto* itInsert = rTree.insert("AB", value++);
    auto got = serializer.serialize(rTree);
#ifdef USE_SPLAY_TREE
    std::string expected = "+[,0,*,<+[AB,0,√,<->]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
#endif
    itInsert = rTree.insert("AC", value++);
#ifdef USE_SPLAY_TREE
    got = serializer.serialize(rTree);
    expected = "+[,0,*,<+[A,0,*,<+[B,0,√,<->],+[C,1,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
#endif
    auto valExpect = rTree.find("AB");
    if(valExpect == nullptr ||  valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("AC");
    if(valExpect == nullptr || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    bool okErase = rTree.erase("AB");
    if(!okErase)
    {
        std::cout << "Erase action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    got = serializer.serialize(rTree);
#ifdef USE_SPLAY_TREE
    expected = "+[,0,*,<+[A,0,*,<+[B,0,√X,<->],+[C,1,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
#endif
    valExpect = rTree.find("AB");
    if(valExpect != nullptr)
    {
        std::cout << "Find action should have failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    itInsert = rTree.insert("AB", value++);
#ifdef USE_SPLAY_TREE
    got = serializer.serialize(rTree);
    expected = "+[,0,*,<+[A,0,*,<+[B,2,√,<->],+[C,1,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
#endif
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("AB", value++);
    itInsert = rTree.insert("A", value++);
#ifdef USE_SPLAY_TREE
    got = serializer.serialize(rTree);
    expected = "+[,0,*,<+[A,1,√,<+[B,0,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
#endif
    valExpect = rTree.find("AB");
    if(valExpect == nullptr || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("A");
    if(valExpect == nullptr || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("A", value++);
    itInsert = rTree.insert("AB", value++);
    got = serializer.serialize(rTree);
#ifdef USE_SPLAY_TREE
    expected = "+[,0,*,<+[A,0,√,<+[B,1,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
#endif
    valExpect = rTree.find("A");
    if(valExpect == nullptr || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("AB");
    if(valExpect == nullptr || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    okErase = rTree.erase("A");
    if(!okErase)
    {
        std::cout << "Erase action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
#ifdef USE_SPLAY_TREE
    got = serializer.serialize(rTree);
    expected = "+[,0,*,<+[A,0,√X,<+[B,1,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
#endif
    valExpect = rTree.find("A");
    if(valExpect != nullptr)
    {
        std::cout << "Find action should have failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("AB");
    if(valExpect == nullptr || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    

    
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("AB", value++);
    itInsert = rTree.insert("AB", value++);
    got = serializer.serialize(rTree);
#ifdef USE_SPLAY_TREE
    expected = "+[,0,*,<+[AB,0,√,<->]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
#endif
    valExpect = rTree.find("AB");
    if(valExpect == nullptr || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("AB", value++);
    itInsert = rTree.insert("AC", value++);
    itInsert = rTree.insert("AC", value++);
    got = serializer.serialize(rTree);
#ifdef USE_SPLAY_TREE
    expected = "+[,0,*,<+[A,0,*,<+[B,0,√,<->],+[C,1,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
#endif
    valExpect = rTree.find("AB");
    if(valExpect == nullptr || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("AC");
    if(valExpect == nullptr || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("AB", value++);
    itInsert = rTree.insert("BC", value++);
    got = serializer.serialize(rTree);
#ifdef USE_SPLAY_TREE
    expected = "+[,0,*,<+[AB,0,√,<->],+[BC,1,√,<->]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
#endif
    valExpect = rTree.find("AB");
    if(valExpect == nullptr || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("BC");
    if(valExpect == nullptr || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("A", value++);
    itInsert = rTree.insert("AB", value++);
    itInsert = rTree.insert("ABC", value++);
#ifdef USE_SPLAY_TREE
    got = serializer.serialize(rTree);
    expected = "+[,0,*,<+[A,0,√,<+[B,1,√,<+[C,2,√,<->]>]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
#endif
    valExpect = rTree.find("A");
    if(valExpect == nullptr || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("AB");
    if(valExpect == nullptr || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("ABC");
    if(valExpect == nullptr || valExpect->value != 2)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("A", value++);
    itInsert = rTree.insert("ABC", value++);
    itInsert = rTree.insert("AB", value++);
#ifdef USE_SPLAY_TREE
    got = serializer.serialize(rTree);
    expected = "+[,0,*,<+[A,0,√,<+[B,2,√,<+[C,1,√,<->]>]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
#endif
    valExpect = rTree.find("A");
    if(valExpect == nullptr || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("ABC");
    if(valExpect == nullptr || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("AB");
    if(valExpect == nullptr || valExpect->value != 2)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("AA", value++);
    itInsert = rTree.insert("AB", value++);
    itInsert = rTree.insert("BA", value++);
#ifdef USE_SPLAY_TREE
    got = serializer.serialize(rTree);
    expected = "+[,0,*,<"
                    "+[A,0,*,<"
                            "+[A,0,√,<->],+[B,1,√,<->]"
                            ">],"
                    "+[BA,2,√,<->]"
                    ">]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
#endif
    valExpect = rTree.find("AA");
    if(valExpect == nullptr || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("AB");
    if(valExpect == nullptr || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("BA");
    if(valExpect == nullptr || valExpect->value != 2)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("AAA", value++);
    itInsert = rTree.insert("ABA", value++);
#ifdef USE_SPLAY_TREE
    got = serializer.serialize(rTree);
    expected = "+[,0,*,<"
                    "+[A,0,*,<"
                            "+[AA,0,√,<->],+[BA,1,√,<->]"
                            ">]"
                    ">]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
#endif
    itInsert = rTree.insert("AAB", value++);
#ifdef USE_SPLAY_TREE
    got = serializer.serialize(rTree);
    expected = "+[,0,*,<"
                        "+[A,0,*,<"
                                "+[A,0,*,<"
                                        "+[A,0,√,<->],+[B,2,√,<->]"
                                    ">]"
                                ",+[BA,1,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
#endif
    valExpect = rTree.find("AAA");
    if(valExpect == nullptr || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("ABA");
    if(valExpect == nullptr || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("AAB");
    if(valExpect == nullptr || valExpect->value != 2)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("AAA", value++);
    itInsert = rTree.insert("AAB", value++);
    itInsert = rTree.insert("ABA", value++);
#ifdef USE_SPLAY_TREE
    got = serializer.serialize(rTree);
    expected = "+[,0,*,<"
                        "+[A,0,*,<"
                                "+[A,0,*,<"
                                        "+[A,0,√,<->],+[B,1,√,<->]"
                                    ">]"
                                ",+[BA,2,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
#endif
    valExpect = rTree.find("AAA");
    if(valExpect == nullptr || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("AAB");
    if(valExpect == nullptr || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("ABA");
    if(valExpect == nullptr || valExpect->value != 2)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("AAA", value++);
    itInsert = rTree.insert("AAB", value++);
    itInsert = rTree.insert("ABA", value++);
    itInsert = rTree.insert("ACA", value++);
    itInsert = rTree.insert("ACB", value++);
#ifdef USE_SPLAY_TREE
    got = serializer.serialize(rTree);
    expected = "+[,0,*,<"
                        "+[A,0,*,<"
                                "+[A,0,*,<"
                                        "+[A,0,√,<->],+[B,1,√,<->]"
                                        ">],"
                                        "+[BA,2,√,<->],+[C,0,*,<"
                                                            "+[A,3,√,<->],+[B,4,√,<->]"
                                                            ">]"
                                ">]"
                        ">]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
#endif
    valExpect = rTree.find("AAA");
    if(valExpect == nullptr || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("AAB");
    if(valExpect == nullptr || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("ABA");
    if(valExpect == nullptr || valExpect->value != 2)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("ACA");
    if(valExpect == nullptr || valExpect->value != 3)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("ACB");
    if(valExpect == nullptr || valExpect->value != 4)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }

    
    std::cout << "Unit tests passed" << std::endl;
    return true;
}
#endif
