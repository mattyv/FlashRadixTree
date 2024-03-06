#ifndef TEST_HPP
#define TEST_HPP
#include <iostream>
#include <unordered_map>
#include "FlashRadixTree.hpp"

bool RunTests()
{
    std::cout << "Testing out redix tree \n";
    FlashRadixTree<std::string, int, MatchMode::Exact> rTree;
    FlashRadixTreeSerializer<std::string, int, MatchMode::Exact> serializer;
    
    
    int value = 0;
    auto* itInsert = rTree.insert("AB", value++);
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
    got = serializer.serialize(rTree);
    expected = "+[,0,*,<+[A,0,*,<+[B,0,√,<->],+[C,1,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
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
    if(valExpect != nullptr)
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

    expected = "+[,0,*,<+[A,0,*,<+[B,0,√,<->],+[C,1,√,<->]>]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }

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

    expected = "+[,0,*,<+[AB,0,√,<->],+[BC,1,√,<->]>]";
    if(got != expected)
    {
        std::cout << "Action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
        return false;
    }

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
    auto* itInsertVS = rTreeSV.insert(key1, value++);
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
    

    
    std::cout << "Unit tests passed" << std::endl;
    return true;
}

#endif
