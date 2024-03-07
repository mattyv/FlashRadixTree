# FlashRadixTree
It's hard to beat STL's' unordered_map for raw speed. Especially lookup speed.
But if you need ordered traversal then you lose the abilty to hash and the penalty can be high. Typically the AVL tree implementation tries to balance, but its trying to optimised against the average case. In the case of calling find() over a key set with non uniform distribution your average AVL tree is not optimised for this. 

This project explores some ideas i've had regarding alternate data stuctures to the unorderd_map and map, and aiming the exploration to key sets with a non uniform distributin and key sets with large number of characters and common prefix's. Example of these can be found in many places. 

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
A lot of research has been done into this data structure and to this day it remains a bit of an enigma as to its performance characteristics and temporal locatity benefits. But it does seem to work work well in some situatios for non uniform access. Particulaly because of the self adjusting property and the temporal locality of the data.

So in the FlashRadixTree i offer two options. To use the use of the Splay tree or the use of the map. The Splay tree or map is used to populate the child nodes of reach level of our radixt tree. As for which is better? It may depend on your key set, and I leave it to you to test which one works better. 


Example Timings (Apple M2 (ARM 64), Somoma 14.3.1 (23D60)) ... will aim to get some linux test soon. Sorry for now we timings can only be done on apple silicon. Will add intel soon hopefully.

Large messages FlashRadixTree perform well aginst unordered_map and map for find() in these tests. (again i encourage your own testing)
```
size of one line in kilobytes: 1.75781kb
hash map insert time
  2295.00 instructions/insert() (+/- 6.7 %) 
   674.00 cycles/insert() (+/- 60.1 %) 
     3.41 instructions/cycle 
   204.00 branches/insert() (+/- 11.6 %) 
   2.1597 mis. branches/insert() 
map insert time
   968.00 instructions/insert() (+/- 120.6 %) 
   472.00 cycles/insert() (+/- 86.3 %) 
     2.05 instructions/cycle 
   170.00 branches/insert() (+/- 166.4 %) 
   7.2326 mis. branches/insert() 
tree insert time
  1961.00 instructions/insert() (+/- 302.6 %) 
   592.00 cycles/insert() (+/- 236.1 %) 
     3.31 instructions/cycle 
   350.00 branches/insert() (+/- 435.6 %) 
  10.0764 mis. branches/insert() 
tree exact match insert time
  1962.00 instructions/insert() (+/- 303.0 %) 
   588.00 cycles/insert() (+/- 240.9 %) 
     3.34 instructions/cycle 
   350.00 branches/insert() (+/- 436.2 %) 
  10.4097 mis. branches/insert() 
splay insert time
  5484.00 instructions/insert() (+/- 47.7 %) 
  1200.00 cycles/insert() (+/- 69.8 %) 
     4.57 instructions/cycle 
   953.00 branches/insert() (+/- 60.1 %) 
   8.3299 mis. branches/insert() 

hash map find time
  2459.00 instructions/find() (+/- 0.2 %) 
   714.00 cycles/find() (+/- 14.6 %) 
     3.44 instructions/cycle 
   298.00 branches/find() (+/- 0.6 %) 
   1.2591 mis. branches/find() 
map find time
  2329.00 instructions/find() (+/- 22.3 %) 
   554.00 cycles/find() (+/- 28.9 %) 
     4.20 instructions/cycle 
   570.00 branches/find() (+/- 22.4 %) 
   4.1265 mis. branches/find() 
tree prefix match find time
   951.00 instructions/find() (+/- 186.7 %) 
   362.00 cycles/find() (+/- 96.0 %) 
     2.63 instructions/cycle 
   174.00 branches/find() (+/- 181.8 %) 
   3.2353 mis. branches/find() 
tree exact match find time
  1890.00 instructions/find() (+/- 81.0 %) 
   546.00 cycles/find() (+/- 46.9 %) 
     3.46 instructions/cycle 
   407.00 branches/find() (+/- 62.7 %) 
   3.2459 mis. branches/find() 
splay find time
  6418.00 instructions/find() (+/- 13.1 %) 
  1188.00 cycles/find() (+/- 29.4 %) 
     5.40 instructions/cycle 
  1320.00 branches/find() (+/- 15.5 %) 
   6.4271 mis. branches/find() 

hash map erase time
  2899.00 instructions/erase() (+/- 0.8 %) 
   800.00 cycles/erase() (+/- 11.0 %) 
     3.62 instructions/cycle 
   388.00 branches/erase() (+/- 2.0 %) 
   1.7812 mis. branches/erase() 
map erase time
  2488.00 instructions/erase() (+/- 27.3 %) 
   648.00 cycles/erase() (+/- 57.5 %) 
     3.84 instructions/cycle 
   594.00 branches/erase() (+/- 28.0 %) 
  11.3750 mis. branches/erase() 
tree erase time
  2529.00 instructions/erase() (+/- 80.7 %) 
   623.00 cycles/erase() (+/- 119.9 %) 
     4.06 instructions/cycle 
   538.00 branches/erase() (+/- 69.3 %) 
   9.4722 mis. branches/erase() 
tree exact match erase time
  2489.00 instructions/erase() (+/- 81.0 %) 
   583.00 cycles/erase() (+/- 106.2 %) 
     4.27 instructions/cycle 
   532.00 branches/erase() (+/- 69.4 %) 
  10.3021 mis. branches/erase() 
splay erase time
  7811.00 instructions/erase() (+/- 35.9 %) 
  1478.00 cycles/erase() (+/- 50.7 %) 
     5.28 instructions/cycle 
  1578.00 branches/erase() (+/- 39.9 %) 
  11.7118 mis. branches/erase() 
Number of runs for insert() 288
```
Small messages FlashRadixTree performs well aginst map for find().
```
Testing out redix tree 
Unit tests passed
size of one line in kilobytes: 1.75781kb
hash map insert time
  2295.00 instructions/insert() (+/- 6.7 %) 
   674.00 cycles/insert() (+/- 60.1 %) 
     3.41 instructions/cycle 
   204.00 branches/insert() (+/- 11.6 %) 
   2.1597 mis. branches/insert() 
map insert time
   968.00 instructions/insert() (+/- 120.6 %) 
   472.00 cycles/insert() (+/- 86.3 %) 
     2.05 instructions/cycle 
   170.00 branches/insert() (+/- 166.4 %) 
   7.2326 mis. branches/insert() 
tree insert time
  1961.00 instructions/insert() (+/- 302.6 %) 
   592.00 cycles/insert() (+/- 236.1 %) 
     3.31 instructions/cycle 
   350.00 branches/insert() (+/- 435.6 %) 
  10.0764 mis. branches/insert() 
tree exact match insert time
  1962.00 instructions/insert() (+/- 303.0 %) 
   588.00 cycles/insert() (+/- 240.9 %) 
     3.34 instructions/cycle 
   350.00 branches/insert() (+/- 436.2 %) 
  10.4097 mis. branches/insert() 
splay insert time
  5484.00 instructions/insert() (+/- 47.7 %) 
  1200.00 cycles/insert() (+/- 69.8 %) 
     4.57 instructions/cycle 
   953.00 branches/insert() (+/- 60.1 %) 
   8.3299 mis. branches/insert() 

hash map find time
  2459.00 instructions/find() (+/- 0.2 %) 
   714.00 cycles/find() (+/- 14.6 %) 
     3.44 instructions/cycle 
   298.00 branches/find() (+/- 0.6 %) 
   1.2591 mis. branches/find() 
map find time
  2329.00 instructions/find() (+/- 22.3 %) 
   554.00 cycles/find() (+/- 28.9 %) 
     4.20 instructions/cycle 
   570.00 branches/find() (+/- 22.4 %) 
   4.1265 mis. branches/find() 
tree prefix match find time
   951.00 instructions/find() (+/- 186.7 %) 
   362.00 cycles/find() (+/- 96.0 %) 
     2.63 instructions/cycle 
   174.00 branches/find() (+/- 181.8 %) 
   3.2353 mis. branches/find() 
tree exact match find time
  1890.00 instructions/find() (+/- 81.0 %) 
   546.00 cycles/find() (+/- 46.9 %) 
     3.46 instructions/cycle 
   407.00 branches/find() (+/- 62.7 %) 
   3.2459 mis. branches/find() 
splay find time
  6418.00 instructions/find() (+/- 13.1 %) 
  1188.00 cycles/find() (+/- 29.4 %) 
     5.40 instructions/cycle 
  1320.00 branches/find() (+/- 15.5 %) 
   6.4271 mis. branches/find() 

hash map erase time
  2899.00 instructions/erase() (+/- 0.8 %) 
   800.00 cycles/erase() (+/- 11.0 %) 
     3.62 instructions/cycle 
   388.00 branches/erase() (+/- 2.0 %) 
   1.7812 mis. branches/erase() 
map erase time
  2488.00 instructions/erase() (+/- 27.3 %) 
   648.00 cycles/erase() (+/- 57.5 %) 
     3.84 instructions/cycle 
   594.00 branches/erase() (+/- 28.0 %) 
  11.3750 mis. branches/erase() 
tree erase time
  2529.00 instructions/erase() (+/- 80.7 %) 
   623.00 cycles/erase() (+/- 119.9 %) 
     4.06 instructions/cycle 
   538.00 branches/erase() (+/- 69.3 %) 
   9.4722 mis. branches/erase() 
tree exact match erase time
  2489.00 instructions/erase() (+/- 81.0 %) 
   583.00 cycles/erase() (+/- 106.2 %) 
     4.27 instructions/cycle 
   532.00 branches/erase() (+/- 69.4 %) 
  10.3021 mis. branches/erase() 
splay erase time
  7811.00 instructions/erase() (+/- 35.9 %) 
  1478.00 cycles/erase() (+/- 50.7 %) 
     5.28 instructions/cycle 
  1578.00 branches/erase() (+/- 39.9 %) 
  11.7118 mis. branches/erase() 
Number of runs for insert() 288
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
