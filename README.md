# FlashRadixTree
It's hard to beat STL's' unordered_map for raw speed. Especially lookup speed.
But if you need ordered traversal then you lose the abilty to hash and the penalty can be high. Typically the AVL tree implementation tries to balance, but its trying to optimise against the average case. In the case of calling find() over a key set with non uniform distribution your average AVL tree is not optimised for this. 

This project explores some ideas i've had regarding alternate data stuctures to the unorderd map, and aiming the exploration to key sets with a non uniform distributin and key sets with common prefix's. Example of these can be found in many places. The example's I draw upon are stock symbols, typically delivered in a fixed size set of characers on a locatoin delimetered blob of data. E.g "APPL  ", "GOOGL ", "MSFT  " etc.

What is a Radix tree? Its a compressed form of a Trie. A Trie is a tree of nodes, where each node is a character. The root node is a null character, and each node has a list of children, each child is a character. 

How does a Radix tree differ from a Trie? A Radix tree is a prefix flavor of a Trie. It compresses the trie by merging nodes with a single child to form strings of prefixes.

Below is an exampe of a Trie containing the words "go" "google" "goggles", "hell", "hellow" "hollow".

```
(ROOT)
|
+-- g
|   |
|   +-- o
|       |
|       +-- (end of "go")
|       |
|       +-- o
|       |   |
|       |   +-- g
|       |   |   |
|       |   |   +-- l
|       |   |   |   |
|       |   |   |   +-- e (end of "google")
|       |   |   |
|       |   |   +-- e
|       |   |       |
|       |   |       +-- s (end of "goggles")
|       |
|       +-- l
|           |
|           +-- l
|               |
|               +-- o
|                   |
|                   +-- w (end of "hollow")
|
+-- h
    |
    +-- e
    |   |
    |   +-- l
    |   |   |
    |   |   +-- l
    |   |       |
    |   |       +-- (end of "hell")
    |   |       |
    |   |       +-- o (end of "hello")
    |
    +-- o
        |
        +-- l
            |
            +-- l
                |
                +-- o
                    |
                    +-- w (end of "hollow")
```
                    
Below is an example of a Radix tree containing the same words.

```
(ROOT)
|
+-- go
|   |
|   +-- (end of "go")
|   |
|   +-- ogle (end of "google")
|   |
|   +-- oggles (end of "goggles")
|
+-- h
    |
    +-- ell
    |   |
    |   +-- (end of "hell")
    |   |
    |   +-- o (end of "hello")
    |
    +-- ollow (end of "hollow")
```
    
You can see its more compact and whats more it well structured for cases with common prefixes.
You'll also notice that each level of the tree the first letter is unique. We'll come back to this later.


With some data sets you want to optimise for find speed which as mentioned unordered_map does well, all be it the loss of ordered iteration. 
If you can pre-load the data structure ahead of time, and this may be an advantage you can work with. In the case of stock symbols you can defnitely pre-load the data structure ahead of time.
In the case of our Radix tree this can give us an advantage. 
If each child of the tree has a unique first character we can assume that if we have a match of the first character and the node in questoin is End-of-word then we have a match, and we can save the comparison on the remaining characters. This may be an avantage, especially in situaotins with very long keys.
For this reason the FlashRadixTree has two match modes specified on construction. Partial and Exact. Where partial assumes the case above, and Exact matches the entire key.
Either way knowing the first key is unique makes the stucture simpler in that each child node only needs to be keyed off a single first character. This is the case for the FlashRadixTree.

The non uniform nature of keys is an interesting case which this structure tries to focus on. In the case of stocks, in may cases you'll find that some stock are more active than others. this is probably very true of a lot of other scenarios. Can we optimise for this. Well..> this is a ticky one...

Enter the curios case of the Splay tree. 

What is a Splay tree? Its a self adjusting binary search tree with the additional property that recently accessed elements are quick to access again. Does this by rotating the trough through a series of "Zigs", "Zags", and "Zig Zag" rotaations to bring the accessed node to the root of the tree during the access. This is a very interesting property.

It performs basic binary search tree operations such as insertion, look-up and removal in O(log n) amortized time. For many practical purposes, it is the same as the self-balancing binary search tree. It was invented by Daniel Sleator and Robert Tarjan in 1985.
A lot of research has been done into this data structure and to this day it remains a bit of an enigma. But it does seem to work work well in some situatios for non uniform access. Particulaly because of the self adjusting property and the temporal locality of the data.

So in the FlashRadixTree i offer two options. To use the use of the Splay tree or the use of the unordered_map. I leave it to you to test which one works best with your unique key space. 
