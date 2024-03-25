#ifndef TEST_HPP
#define TEST_HPP
#include <iostream>
#include <unordered_map>
#include "FlashRadixTree.hpp"

//custom allocator to limit construction to 1 element
static int throwOnAllocCount = 1;
template <typename T>
class ThrowingAllocationAllocator {
private:
    static int count;
public:
    using value_type = T;

    ThrowingAllocationAllocator() = default;

    template <class U>
    constexpr ThrowingAllocationAllocator(const ThrowingAllocationAllocator<U>&) noexcept {}

    T* allocate(std::size_t n) {
        ++count;
        if (count > throwOnAllocCount ){
            throw std::bad_alloc();
        }
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    void deallocate(T* p, std::size_t n) noexcept {
        ::operator delete(p);
    }
    
    static void resetCount() noexcept{ count = 0; }
};

template <typename T>
int ThrowingAllocationAllocator<T>::count = 0;

template <class T, class U>
bool operator==(const ThrowingAllocationAllocator<T>&, const ThrowingAllocationAllocator<U>&) { return true; }

template <class T, class U>
bool operator!=(const ThrowingAllocationAllocator<T>&, const ThrowingAllocationAllocator<U>&) { return false; }

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
    
    if(rTree.size() != 2)
    {
        std::cout << "Size action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
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
    if(rTree.size() != 0)
    {
        std::cout << "Size action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    value = 0;
    itInsert = rTree.insert("AB", value++);
    itInsert = rTree.insert("A", value++);
    
    
    if(rTree.size() != 2)
    {
        std::cout << "Size action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }

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
    
    if(rTree.size() != 1)
    {
        std::cout << "Size action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
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
    
    ss.str("");
    for(auto& it : rTree)
    {
        ss << it.prefix << "," << it.value << "," << it.getFullKey() << std::endl;
    }
    got = ss.str();
    expected = "A,0,AA\nB,1,AB\nBA,2,BA\n";
    
    if(got != expected)
    {
        std::cout << "Action failed insert @ " << __LINE__ << " in " << __FILE__ << std::endl;
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
    expected = "BA,2,BA\nB,1,AB\nA,0,AA\n";
    
    if(got != expected)
    {
        std::cout << "Action failed insert @ " << __LINE__ << " in " << __FILE__ << std::endl;
        std::cout << "Got: " << got << "\nExpected: " << expected << std::endl;
        rTree.print();
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
    
    if(rTree.size() != 5)
    {
        std::cout << "Size action failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }

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
    
    
   
    
    //test out of memory
    bool okOOM = false;
    FlashRadixTree<std::string, int, MatchMode::Exact, ThrowingAllocationAllocator<FlashRadixTree<std::string, int>::FlashRadixTreeNode>> rTreeExactOOM;
    throwOnAllocCount = 3;
    try
    {
        rTreeExactOOM.insert("AB", 1);
        rTreeExactOOM.insert("AC", 2);
    }
    catch(const std::bad_alloc& e)
    {
        okOOM = true;
        //test size ==1
        if(rTreeExactOOM.size() != 1)
        {
            std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
            return false;
        }
        //test finding AB
        auto valExpectOOM = rTreeExactOOM.find("AB");
        if(valExpectOOM == rTreeExactOOM.end())
        {
            std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
            return false;
        }
        
        //test not finding AC
        valExpectOOM = rTreeExactOOM.find("AC");
        if(valExpectOOM != rTreeExactOOM.end())
        {
            std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
            return false;
        }
        
        //test serialization
        FlashRadixTreeSerializer<std::string, int, MatchMode::Exact, ThrowingAllocationAllocator<FlashRadixTree<std::string, int>::FlashRadixTreeNode>> serializerOOM;
        std::string got = serializerOOM.serialize(rTreeExactOOM);
        std::string expected = "+[,0,*,<+[AB,1,√,<->]>]";
        if(got != expected)
        {
            std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
            std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
            return false;
        }
    }
        
    
    if(!okOOM)
    {
        std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    //test out of memory
    okOOM = false;
    rTreeExactOOM.clear(); //after clear we have to also reallocate a new root on next insert
    throwOnAllocCount += 3;
    try
    {
        rTreeExactOOM.insert("A", 1);
        rTreeExactOOM.insert("B", 2);
    }
    catch(const std::bad_alloc& e)
    {
        okOOM = true;
        //test size ==1
        if(rTreeExactOOM.size() != 1)
        {
            std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
            return false;
        }
        
        auto valExpectOOM = rTreeExactOOM.find("A");
        if(valExpectOOM == rTreeExactOOM.end())
        {
            std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
            return false;
        }
        
        valExpectOOM = rTreeExactOOM.find("B");
        if(valExpectOOM != rTreeExactOOM.end())
        {
            std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
            return false;
        }
        
        //test serialization
        FlashRadixTreeSerializer<std::string, int, MatchMode::Exact, ThrowingAllocationAllocator<FlashRadixTree<std::string, int>::FlashRadixTreeNode>> serializerOOM;
        std::string got = serializerOOM.serialize(rTreeExactOOM);
        std::string expected = "+[,0,*,<+[A,1,√,<->]>]";
        if(got != expected)
        {
            std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
            std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
            return false;
        }
    }
        
    
    if(!okOOM)
    {
        std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    //test out of memory
    okOOM = false;
    rTreeExactOOM.clear(); //after clear we have to also reallocate a new root on next insert
    throwOnAllocCount += 3;
    try
    {
        rTreeExactOOM.insert("A", 1);
        rTreeExactOOM.insert("AB", 2);
    }
    catch(const std::bad_alloc& e)
    {
        okOOM = true;
        //test size ==1
        if(rTreeExactOOM.size() != 1)
        {
            std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
            return false;
        }
        
        auto valExpectOOM = rTreeExactOOM.find("A");
        if(valExpectOOM == rTreeExactOOM.end())
        {
            std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
            return false;
        }
        
        valExpectOOM = rTreeExactOOM.find("AB");
        if(valExpectOOM != rTreeExactOOM.end())
        {
            std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
            return false;
        }
        
        //test serialization
        FlashRadixTreeSerializer<std::string, int, MatchMode::Exact, ThrowingAllocationAllocator<FlashRadixTree<std::string, int>::FlashRadixTreeNode>> serializerOOM;
        std::string got = serializerOOM.serialize(rTreeExactOOM);
        std::string expected = "+[,0,*,<+[A,1,√,<->]>]";
        if(got != expected)
        {
            std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
            std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
            return false;
        }
    }
        
    
    if(!okOOM)
    {
        std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    //test out of memory
    okOOM = false;
    rTreeExactOOM.clear(); //after clear we have to also reallocate a new root on next insert
    throwOnAllocCount += 4;
    try
    {
        rTreeExactOOM.insert("A", 1);
        rTreeExactOOM.insert("AB", 2);
        rTreeExactOOM.insert("ABC", 3);
    }
    catch(const std::bad_alloc& e)
    {
        okOOM = true;
        //test size ==1
        if(rTreeExactOOM.size() != 2)
        {
            std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
            return false;
        }
        
        auto valExpectOOM = rTreeExactOOM.find("A");
        if(valExpectOOM == rTreeExactOOM.end())
        {
            std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
            return false;
        }
        
        valExpectOOM = rTreeExactOOM.find("AB");
        if(valExpectOOM == rTreeExactOOM.end())
        {
            std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
            return false;
        }
        
        
        
        valExpectOOM = rTreeExactOOM.find("ABC");
        if(valExpectOOM != rTreeExactOOM.end())
        {
            std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
            return false;
        }
        
        //test serialization
        FlashRadixTreeSerializer<std::string, int, MatchMode::Exact, ThrowingAllocationAllocator<FlashRadixTree<std::string, int>::FlashRadixTreeNode>> serializerOOM;
        std::string got = serializerOOM.serialize(rTreeExactOOM);
        std::string expected = "+[,0,*,<+[A,1,√,<+[B,2,√,<->]>]>]";
        if(got != expected)
        {
            std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
            std::cout << "Got:      " << got << "\nExpected: " << expected << std::endl;
            return false;
        }
    }
        
    
    if(!okOOM)
    {
        std::cout << "Out of memory test failed @ " << __LINE__ << " in " << __FILE__ << std::endl;
        return false;
    }
    
    std::cout << "Unit tests passed" << std::endl;
    return true;
}

#endif
