# FlashRadixTree
It's hard to beat STL's' unordered_map for raw speed. Especially lookup speed.
But if you need ordered traversal then you lose the abilty to hash and the penalty can be high. Typically the AVL tree implementation tries to balance, but its trying to optimised against the average case. In the case of calling find() over a key set with non uniform distribution your average AVL tree is not optimised for this. 

This project explores some ideas i've had regarding alternate data stuctures to the unorderd map, and aiming the exploration to key sets with a non uniform distributin and key sets with large number of characters and common prefix's. Example of these can be found in many places. 

What is a Radix tree? Its a compressed form of a Trie. A Trie is a tree of nodes, where each node is a character. The root node is a null character, and each node has a list of children, each child is a character. 

How does a Radix tree differ from a Trie? A Radix tree is a prefix flavor of a Trie. It compresses the trie by merging nodes with a single child to form strings of prefixes.

Below is an exampe of a Trie containing the words "go" "google" "goggles", "hell", "hellow" "hollow".

```
Root
|
+-- g
|   |
|   +-- o
|       |
|       +-- (end of word "go")
|       |
|       +-- o
|       |   |
|       |   +-- g
|       |   |   |
|       |   |   +-- l
|       |   |   |   |
|       |   |   |   +-- e (end of word "google")
|       |   |   |
|       |   |   +-- e (end of word "goggles")
|       |   |
|       |   +-- g
|       |       |
|       |       +-- l
|       |       |   |
|       |       |   +-- e
|       |       |   |   |
|       |       |   |   +-- s (end of word "goggles")
|       |
|       +-- l
|           |
|           +-- l
|               |
|               +-- o
|                   |
|                   +-- w (end of word "hollow")
|
+-- h
    |
    +-- e
    |   |
    |   +-- l
    |   |   |
    |   |   +-- l (end of word "hell")
    |   |
    |   +-- l
    |       |
    |       +-- o
    |           |
    |           +-- w (end of word "hellow")
    |
    +-- o
        |
        +-- l
            |
            +-- l
                |
                +-- o
                    |
                    +-- w (end of word "hollow")
```
                    
Below is an example of a Radix tree containing the same words.
```
Root
|
+-- go
|   |
|   +-- (end of word "go")
|   |
|   +-- ogle (end of word "google")
|   |
|   +-- oggles (end of word "goggles")
|
+-- h
    |
    +-- ell
    |   |
    |   +-- (end of word "hell")
    |   |
    |   +-- ow (end of word "hellow")
    |
    +-- ollow (end of word "hollow")
```
    
You can see its more compact and whats more it well structured for cases with common prefixes.
You'll also notice that each level of the tree the first letter is unique. We'll come back to this later.


With some data sets you want to optimise for find speed which as mentioned unordered_map does well, all be it the loss of ordered iteration. 
If you can pre-load the data structure ahead of time, and this may be an advantage you can work with. 
In the case of our Radix tree this can give us an advantage. 
If each child of the tree has a unique first character we can assume that if we have a match of the first character and the node in questoin is End-of-word then we have a match, and we can save the comparison on the remaining characters. This may be an avantage, especially in situaotins with very long keys.
For this reason the FlashRadixTree has two match modes specified on construction. "Partial" and "Exact". Where partial assumes the case above, and Exact matches the entire key.
Either way knowing the first key is unique makes the stucture simpler in that each child node only needs to be keyed off a single first character. This is the case for the FlashRadixTree.

The non uniform nature of keys is an interesting case which this structure tries to focus on. In many cases you may find that some keys are more active than others. this is probably very true of a lot of other scenarios. Can we optimise for this. Well..? this is a ticky one...

Enter the curios case of the Splay tree. 

What is a Splay tree? Its a self adjusting binary search tree with the additional property that recently accessed elements are quick to access again. Does this by rotating the trough through a series of "Zigs", "Zags", and "Zig Zag" rotaations to bring the accessed node to the root of the tree during the access. This is a very interesting property. A good description of this can be found here: https://en.wikipedia.org/wiki/Splay_tree

It performs basic binary search tree operations such as insertion, look-up and removal in O(log n) amortized time. For many practical purposes, it is the same as the self-balancing binary search tree. It was invented by Daniel Sleator and Robert Tarjan in 1985.
A lot of research has been done into this data structure and to this day it remains a bit of an enigma. But it does seem to work work well in some situatios for non uniform access. Particulaly because of the self adjusting property and the temporal locality of the data.

So in the FlashRadixTree i offer two options. To use the use of the Splay tree or the use of the map. The Splay tree or map is used to populate the child nodes of reach level of our radixt tree. As for which is better? It may depend on your key set, and I leave it to you to test which one works better. 

Example Timings (Apple M2 (ARM 64), Somoma 14.3.1 (23D60)) ... will aim to get some linux test soom.

Large messages FlashRadixTree perform well aginst unordered_map and map for find() in these tests. (again i encourage your own testing)
```
ssize of one line in kilobytes: 1.75781kb
hash map insert time: 13ns
map insert time: 11ns
tree prefix match insert time: 20ns
tree exact match insert time: 20ns
splay insert time: 21ns

hash map find time:  215ns
map find time: 268ns
tree prefix match find time: 160ns
tree exact match find time: 193ns
splay find time: 418ns

hash map erase time: 14ns
map erase time: 16ns
tree erase time: 15ns
tree exact match erase time: 12ns
splay erase time: 23ns
Number of runs for insert() 288

Number of runs for find() 1038800
```
Small messages FlashRadixTree performs well aginst map for find().
```
size of one line in bytes: 6b
hash map insert time: 1ns
map insert time: 3ns
tree prefix match insert time: 3ns
tree exact match insert time: 3ns
splay insert time: 3ns

hash map find time:  13ns
map find time: 94ns
tree prefix match find time: 31ns
tree exact match find time: 34ns
splay find time: 60ns

hash map erase time: 1ns
map erase time: 4ns
tree erase time: 3ns
tree exact match erase time: 3ns
splay erase time: 5ns
Number of runs for insert() 288

Number of runs for find() 1038800
```
To give and idea of key distribution
```
Top 10% count of keys used (not printing keys as they can be long): 
1965
1718
725
689
433
290
282
202
195
170
148
146
144
142
137
129
124
116
113
107
88
83
71
66
62
60
55
53
```
