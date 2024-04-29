//
//  unique_ptr_alloc.hpp
//  FlashRadixTree
//
//  Created by Matthew Varendorff on 27/4/2024.
//

#ifndef unique_ptr_alloc_h
#define unique_ptr_alloc_h

namespace Flash {

template <typename T, typename Alloc, typename... Args>
static std::unique_ptr<T, std::function<void(T*)>> make_unique_alloc(Alloc alloc, Args&&... args)
{
    using AllocTraits = std::allocator_traits<Alloc>;
    using pointer = typename AllocTraits::pointer;

    // Allocate space for one object of type T
    pointer ptr = AllocTraits::allocate(alloc, 1);

    // Construct an object of type T with the provided arguments
    AllocTraits::construct(alloc, ptr, std::forward<Args>(args)...);

    // Create a custom deleter as a lambda function that captures the allocator by value
    auto deleter = [alloc](T* ptr) mutable {
        AllocTraits::destroy(alloc, ptr); // Call destructor
        AllocTraits::deallocate(alloc, ptr, 1); // Deallocate memory
    };

    // Define the deleter type
    using DeleterType = std::function<void(T*)>;

    // Create and return a unique_ptr with the custom deleter
    return std::unique_ptr<T, DeleterType>(ptr, deleter);
}

}

#endif /* unique_ptr_alloc_h */
