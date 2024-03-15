#ifndef TEST_HPP
#define TEST_HPP
#include <iostream>
#include <unordered_map>
#include "FlashRadixTree.hpp"

bool RunTests()
{
    std::cout << "Testing out radix tree \n";
    FlashRadixTree<std::string, int, MatchMode::Exact> rTree;
    FlashRadixTreeSerializer<std::string, int, MatchMode::Exact> serializer;
    
    
    int value = 0;
    auto itInsert = rTree.insert("AB", value++);
    auto got = serializer.serialize(rTree);

    std::string expected = "+[,0,*,<+[AB,0,√,<->]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }

    itInsert = rTree.insert("AC", value++);

    got = serializer.serialize(rTree);
    expected = "+[,0,*,<+[A,0,*,<+[B,0,√,<->],+[C,1,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }

    auto valExpect = rTree.find("AB");
    if(valExpect == rTree.end() ||  valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("AC");
    if(valExpect == rTree.end() || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    got = serializer.serialize(rTree);
    expected = "+[,0,*,<+[A,0,*,<+[B,0,√,<->],+[C,1,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
    auto key = valExpect->getFullKey();
    if(key != "AC")
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << key << "\nExpected: " << "AC" << std::endl;
        return false;
    }
    std::stringstream ss;
    for( auto& it : rTree)
    {
        ss << it.prefix << "," << it.value << "," << it.getFullKey() << std::endl;
    }
    std::string gotStr = ss.str();
    std::string expectedStr = "B,0,AB\nC,1,AC\n";
    if(gotStr != expectedStr)
    {
        std::cout << "Action failed iterator @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << gotStr << "\nExpected: " << expectedStr << std::endl;
        return false;
    }
    
    //test reverse iterator
    ss.str("");
    for( auto it = rTree.rbegin(); it != rTree.rend(); ++it)
    {
        ss << it->prefix << "," << it->value << "," << it->getFullKey() << std::endl;
    }
    gotStr = ss.str();
    expectedStr = "C,1,AC\nB,0,AB\n";
    
    if(gotStr != expectedStr)
    {
        std::cout << "Action failed reverse iterator @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got: " << gotStr << "\nExpected: " << expectedStr << std::endl;
        return false;
    }
    
    auto okErase = rTree.erase("A");
    if(okErase)
    {
        std::cout << "Erase action should have failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    okErase = rTree.erase("AB");
    if(!okErase)
    {
        std::cout << "Erase action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    got = serializer.serialize(rTree);

    expected = "+[,0,*,<+[AC,1,√,<->]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }

    valExpect = rTree.find("AB");
    if(valExpect != rTree.end())
    {
        std::cout << "Find action should have failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    itInsert = rTree.insert("AB", value++);

    got = serializer.serialize(rTree);
    expected = "+[,0,*,<+[A,0,*,<+[B,2,√,<->],+[C,1,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }

    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("AB", value++);
    itInsert = rTree.insert("A", value++);

    got = serializer.serialize(rTree);
    expected = "+[,0,*,<+[A,1,√,<+[B,0,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
    
    valExpect = rTree.find("AB");
    if(valExpect == rTree.end() || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("A");
    if(valExpect == rTree.end() || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    ss.str("");
    for( auto& it : rTree)
    {
        ss << it.prefix << "," << it.value << "," << it.getFullKey() << std::endl;
    }
    gotStr = ss.str();
    expectedStr = "A,1,A\nB,0,AB\n";
    if(gotStr != expectedStr)
    {
        std::cout << "Action failed iterator @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << gotStr << "\nExpected: " << expectedStr << std::endl;
        return false;
    }
    
    //test reverse iterator
    ss.str("");
    for( auto it = rTree.rbegin(); it != rTree.rend(); ++it)
    {
        ss << it->prefix << "," << it->value << "," << it->getFullKey() << std::endl;
    }
    gotStr = ss.str();
    expectedStr = "B,0,AB\nA,1,A\n";
    if(gotStr != expectedStr)
    {
        std::cout << "Action failed reverse iterator @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got: " << gotStr << "\nExpected: " << expectedStr << std::endl;
        return false;
    }
    
    okErase = rTree.erase("AB");
    if(!okErase)
    {
        std::cout << "Erase action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    got = serializer.serialize(rTree);
    expected = "+[,0,*,<+[A,1,√,<->]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
    
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("A", value++);
    itInsert = rTree.insert("AB", value++);
    got = serializer.serialize(rTree);

    expected = "+[,0,*,<+[A,0,√,<+[B,1,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }

    valExpect = rTree.find("A");
    if(valExpect == rTree.end() || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("AB");
    if(valExpect == rTree.end() || valExpect->value != 1)
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

    got = serializer.serialize(rTree);
    expected = "+[,0,*,<+[AB,1,√,<->]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }

    valExpect = rTree.find("A");
    if(valExpect != rTree.end())
    {
        std::cout << "Find action should have failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("AB");
    if(valExpect == rTree.end() || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    okErase = rTree.erase("AB");
    if(!okErase)
    {
        std::cout << "Erase action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    got = serializer.serialize(rTree);
    expected = "+[,0,*,<->]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
    
    

    
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("AB", value++);
    itInsert = rTree.insert("AB", value++);
    got = serializer.serialize(rTree);

    expected = "+[,0,*,<+[AB,0,√,<->]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }

    valExpect = rTree.find("AB");
    if(valExpect == rTree.end() || valExpect->value != 0)
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

    expected = "+[,0,*,<+[A,0,*,<+[B,0,√,<->],+[C,1,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }

    valExpect = rTree.find("AB");
    if(valExpect == rTree.end() || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("AC");
    if(valExpect == rTree.end() || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("AB", value++);
    itInsert = rTree.insert("BC", value++);
    got = serializer.serialize(rTree);

    expected = "+[,0,*,<+[AB,0,√,<->],+[BC,1,√,<->]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }

    valExpect = rTree.find("AB");
    if(valExpect == rTree.end() || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("BC");
    if(valExpect == rTree.end() || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    ss.str("");
    for(auto& it: rTree)
    {
        ss << it.prefix << "," << it.value << "," << it.getFullKey() << std::endl;
    }
    
    got = ss.str();
    expected = "AB,0,AB\nBC,1,BC\n";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("A", value++);
    itInsert = rTree.insert("AB", value++);
    itInsert = rTree.insert("ABC", value++);

    got = serializer.serialize(rTree);
    expected = "+[,0,*,<+[A,0,√,<+[B,1,√,<+[C,2,√,<->]>]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }

    valExpect = rTree.find("A");
    if(valExpect == rTree.end() || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("AB");
    if(valExpect == rTree.end() || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("ABC");
    if(valExpect == rTree.end() || valExpect->value != 2)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    ss.str("");
    for(auto& it: rTree)
    {
        ss << it.prefix << "," << it.value << "," << it.getFullKey() << std::endl;
    }
    got = ss.str();
    expected = "A,0,A\nB,1,AB\nC,2,ABC\n";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
    
    okErase = rTree.erase("A");
    if(!okErase)
    {
        std::cout << "Erase action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    got = serializer.serialize(rTree);
    expected = "+[,0,*,<+[AB,1,√,<+[C,2,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
    
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("A", value++);
    itInsert = rTree.insert("AB", value++);
    itInsert = rTree.insert("ABC", value++);
    okErase = rTree.erase("AB");
    if(!okErase)
    {
        std::cout << "Erase action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    got = serializer.serialize(rTree);
    expected = "+[,0,*,<+[A,0,√,<+[BC,2,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("A", value++);
    itInsert = rTree.insert("ABC", value++);
    itInsert = rTree.insert("AB", value++);

    got = serializer.serialize(rTree);
    expected = "+[,0,*,<+[A,0,√,<+[B,2,√,<+[C,1,√,<->]>]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }

    valExpect = rTree.find("A");
    if(valExpect == rTree.end() || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("ABC");
    if(valExpect == rTree.end() || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("AB");
    if(valExpect == rTree.end() || valExpect->value != 2)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("AA", value++);
    itInsert = rTree.insert("AB", value++);
    itInsert = rTree.insert("BA", value++);

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

    valExpect = rTree.find("AA");
    if(valExpect == rTree.end() || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("AB");
    if(valExpect == rTree.end() || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("BA");
    if(valExpect == rTree.end() || valExpect->value != 2)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    okErase = rTree.erase("AA");
    if(!okErase)
    {
        std::cout << "Erase action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    got = serializer.serialize(rTree);
    expected = "+[,0,*,<"
                    "+[AB,1,√,<->],"
                    "+[BA,2,√,<->]"
                ">]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
    
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("AAA", value++);
    itInsert = rTree.insert("ABA", value++);

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

    itInsert = rTree.insert("AAB", value++);

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

    valExpect = rTree.find("AAA");
    if(valExpect == rTree.end() || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("ABA");
    if(valExpect == rTree.end() || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("AAB");
    if(valExpect == rTree.end() || valExpect->value != 2)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    okErase = rTree.erase("AAA");
    if(!okErase)
    {
        std::cout << "Erase action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    got = serializer.serialize(rTree);
    expected = "+[,0,*,<+[A,0,*,<+[AB,2,√,<->],+[BA,1,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
    
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("AAA", value++);
    itInsert = rTree.insert("AAB", value++);
    itInsert = rTree.insert("ABA", value++);

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

    valExpect = rTree.find("AAA");
    if(valExpect == rTree.end() || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("AAB");
    if(valExpect == rTree.end() || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("ABA");
    if(valExpect == rTree.end() || valExpect->value != 2)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    ss.str("");
    for(auto& it : rTree)
    {
        ss << it.prefix << "," << it.value << "," << it.getFullKey() << std::endl;
    }
    got = ss.str();
    expected = "A,0,AAA\nB,1,AAB\nBA,2,ABA\n";
    if(got != expected)
    {
        std::cout << "Action failed iterator @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
    
    
    rTree.clear();
    value = 0;
    itInsert = rTree.insert("AAA", value++);
    itInsert = rTree.insert("AAB", value++);
    itInsert = rTree.insert("ABA", value++);
    itInsert = rTree.insert("ACA", value++);
    itInsert = rTree.insert("ACB", value++);

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

    valExpect = rTree.find("AAA");
    if(valExpect == rTree.end() || valExpect->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("AAB");
    if(valExpect == rTree.end() || valExpect->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("ABA");
    if(valExpect == rTree.end() || valExpect->value != 2)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("ACA");
    if(valExpect == rTree.end() || valExpect->value != 3)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpect = rTree.find("ACB");
    if(valExpect == rTree.end() || valExpect->value != 4)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    ss.str("");
    for(auto& it : rTree)
    {
        ss << it.prefix << "," << it.value << "," << it.getFullKey() << std::endl;
    }
    got = ss.str();
    expected = "A,0,AAA\nB,1,AAB\nBA,2,ABA\nA,3,ACA\nB,4,ACB\n";
    if(got != expected)
    {
        std::cout << "Action failed iterator @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got: " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
    
    ss.str("");
    for(auto it = rTree.rbegin(); it != rTree.rend(); ++it)
    {
        ss << it->prefix << "," << it->value << "," << it->getFullKey() << std::endl;
    }
    got = ss.str();
    expected = "B,4,ACB\nA,3,ACA\nBA,2,ABA\nB,1,AAB\nA,0,AAA\n";
    if(got != expected)
    {
        std::cout << "Action failed reverse iterator @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got: " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }
    
    okErase = rTree.erase("AAA");
    if(!okErase)
    {
        std::cout << "Erase action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    got = serializer.serialize(rTree);
    expected = "+[,0,*,"
                        "<+[A,0,*,"
                                    "<+[AB,1,√,<->],"
                                     "+[BA,2,√,<->],"
                                     "+[C,0,*,"
                                        "<+[A,3,√,<->],"
                                         "+[B,4,√,<->]"
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
    
    
    FlashRadixTree<std::string_view, int, MatchMode::Exact> rTreeSV;
    FlashRadixTreeSerializer<std::string_view, int, MatchMode::Exact> serializerVS;
    
    value = 0;
    std::string key1 = "AAA";
    auto itInsertVS = rTreeSV.insert(key1, value++);
    std::string key2 = "AAB";
    itInsertVS = rTreeSV.insert(key2, value++);
    std::string key3 = "ABA";
    itInsertVS = rTreeSV.insert(key3, value++);
    std::string key4 = "ACA";
    itInsertVS = rTreeSV.insert(key4, value++);
    std::string key5 = "ACB";
    itInsertVS = rTreeSV.insert(key5, value++);
    
    
    okErase = rTreeSV.erase("AAA");
    if(!okErase)
    {
        std::cout << "Erase action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    got = serializerVS.serialize(rTreeSV);
    expected = "+[,0,*,<"
                        "+[A,0,*,<"
                                "+[A,0,*,<"
                                        "+[A,0,√X,<->],+[B,1,√,<->]"
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
   
    //some tests on prefix version
    FlashRadixTree<std::string, int, MatchMode::Prefix> rTreePrefix;
    FlashRadixTreeSerializer<std::string, int, MatchMode::Prefix> serializerPrefix;
    value = 0;
    auto itInsertPrefix = rTreePrefix.insert("AA", value++);
    itInsertPrefix = rTreePrefix.insert("AABB", value++);

    got = serializerPrefix.serialize(rTreePrefix);
    expected = "+[,0,*,<+[AA,0,√,<+[BB,1,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }

    auto valExpectPrefix = rTreePrefix.find("AA");
    if(valExpectPrefix == rTreePrefix.end() || valExpectPrefix->value != 0)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    valExpectPrefix = rTreePrefix.find("AABB");
    if(valExpectPrefix == rTreePrefix.end() || valExpectPrefix->value != 1)
    {
        std::cout << "Find action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    

    
    std::cout << "Unit tests passed" << std::endl;
    return true;
}

#endif
