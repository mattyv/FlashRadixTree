
## Issue tracking
[![Open Issues](https://img.shields.io/github/issues-raw/mattyv/FlashRadixTree.svg)](https://github.com/mattyv/FlashRadixTree/issues)
# FlashRadixTree
Fast key value pair data structure with ordered traversal. On par with std::map with keys with common prefixes and asymmetric key distribution.


If you have keys such as "go" "google" "goggles", "hell", "hello" "hollow" and you need to keep them in an ordered container, a nice way to find the key is to break the keys up into prefixes and store them in a tree structure. If you know you keys will be there i.e your keys space doesn't change after you fill the container, you can search even faster by checking the first letter of parts of the tree... see below for details.

## Performance Overview
### 1.7KB key Apple M2 (ARM 64), Somoma 14.3.1 (23D60) CPU Cycles Comparison
```
FlashRadixTree::find() (using partial key match)
181.00 cycles/find() min,   314.75 avg, 22,068.00 max   
map::find()
546.00 cycles/find() min,   731.58 avg,  7,287.00 max 
unordered_map::find()
701.00 cycles/find() min,   808.13 avg,  3,624.00 max 
```
### 6byte key 
```
FlashRadixTree::find() (using partial key match)
166.00 cycles/find() min,   289.63 avg, 16,319.00 max  
map::find()
206.00 cycles/find() min,   275.71 avg,  4,841.00 max  
unordered_map::find()
154.00 cycles/find() min,   170.39 avg, 33,652.00 max
```

## Overview
It's hard to beat STL's' unordered_map for raw speed. Especially lookup speed.
But if you need ordered traversal then you lose the ability to hash and the penalty can be high. Typically the AVL tree implementation tries to balance, but it's trying to optimised against the average case. In the case of calling find() over a key set with non uniform distribution of find() calls your average AVL tree is not optimised for this. 

This project explores some ideas i've had regarding alternate data structures to the unorderd_map and map, and aiming the exploration to key sets with a non uniform distribution and key sets with common prefix's. Example of these can be found in many places. 

### What is a Radix tree? 
Its a compressed form of a Trie. A Trie is a tree of nodes, where each node is a character. The root node is a null character, and each node has a list of children, each child is a character. 

How does a Radix tree differ from a Trie? A Radix tree is a prefix flavour of a Trie. It compresses the trie by merging nodes with a single child to form strings of prefixes.

Below is an example of a Trie containing the words "go" "google" "goggles", "hell", "hello" "hollow".

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
    |       +-- o  (end of word "hello")
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
    |   +-- o (end of word "hello")
    |
    +-- ollow (end of word "hollow")
```
    
You can see its more compact and whats more it well structured for cases with common prefixes.
You'll also notice that each level of the tree the first letter is unique. We'll come back to this later.


With some data sets you want to optimise for find speed which as mentioned unordered_map does well, all be it the loss of ordered iteration. 
If you can pre-load the data structure ahead of time, and this may be an advantage you can work with. 
In the case of our Radix tree this can give us an advantage. 

### How to search fast then using this advantage? 
The children of the tree will always have a unique first character, so we can assume that if we have a match of the first character and the node in question is 'End-of-word' with no children, then we have a match, and we can save the comparison on the remaining characters. Likewise if we find a node which is end of word with children, and the lengths of the key and prefix match we can also save a full comparison as this must be the key we're looking for. This is an advantage for most keys sizes, but especially in situations with very long keys.
For this reason the FlashRadixTree has two match modes specified on construction, "Partial" and "Exact". Where partial assumes the case above, and Exact matches the entire key when required.

Let’s run through 3 examples. 
1. We're looking for the key "google" using prefix matching. First character is 'g', and we find a node with some children and the key is an end-of-word. We check the size of "google" to "go", they don't match so we continue... We deduce our next key will be 'o' after stripping off "go" and find a node with no children and the key is an end-of-word. We have our match with 3 actions: find 'g', size check, find 'o'.
2. We're looking for the key "go" using prefix matching. First character is 'g', and we find a node with some children and the key is an end-of-word and the lengths match. We have our key with 2 actions: find 'g', and size check. 
3. We're looking for the key "hello" using prefix matching. First character is 'h', and we find a node with some children and the key is not an end-of-word, so we continue... We deduce the next key is 'e' and we find "ell" which is a node with some children and is an end-of-word, but the key lengths of "ello" and "ell" don't match so we continue on to the last node "o". We have our key with 4 actions: find 'h', find 'e', size check, find 'o'.
You can see that in many cases we make less comparisons than the key length (if you ignore slightly the cost of the find). And the longer the prefixes are the better the ratio of key length to comparison gets. Given the find on each lower child only searches for a single character, we also save the cost of repeated comparisons on the remaining portion of the key.  

The non uniform nature of keys is an interesting case which this structure tries to focus on. In many cases you may find that some keys are more active than others. this is probably very true of a lot of other scenarios. Can we optimise for this. Well..? this is a ticky one...

Enter the curios case of the Splay tree. 

### What is a Splay tree? 
Its a self adjusting binary search tree with the additional property that recently accessed elements are quick to access again. It does this by rotating the trough through a series of "Zigs", "Zags", and "Zig Zag" rotations to bring the accessed node to the root of the tree during the access. This is a very interesting property. A good description of this can be found here: https://en.wikipedia.org/wiki/Splay_tree

It performs basic binary search tree operations such as insertion, look-up and removal in O(log n) amortised time. For many practical purposes, it is the same as the self-balancing binary search tree. It was invented by Daniel Sleator and Robert Tarjan in 1985.
A lot of research has been done into this data structure and to this day it remains a bit of an enigma as to its performance characteristics and temporal locality benefits. But it does seem to work well in some situations for non uniform access. Particularly because of the self adjusting property and the temporal locality of the data.

So in the FlashRadixTree i offer two options. To use the use of the Splay tree or the use of the map. The Splay tree or map is used to populate the child nodes of reach level of our radix tree. As for which is better? It may depend on your key set, and I leave it to you to test which one works better. 


## Example Timings (Apple M2 (ARM 64), Somoma 14.3.1 (23D60)) 
... will aim to get some linux test soon. Sorry for now we timings can only be done on apple silicon. Will add intel soon hopefully.

Large messages FlashRadixTree performs well against unordered_map and map for find() in these tests. (again i encourage your own testing).
I also focus on the top 10% of keys used to find as typically you may want them to be performant.
```
size of one line in kilobytes: 1.75781kb
hash map insert time
  2262.00 instructions/insert() min,  2426.19 avg,  8397.00 max 
   692.00 cycles/insert() min,  1002.21 avg,  5443.00 max 
     3.27 instructions/cycle 
   200.00 branches/insert() min,   224.50 avg,  1539.00 max 
   2.1424 mis. branches/insert() 
map insert time
   980.00 instructions/insert() min,  3122.35 avg,  5012.00 max 
   553.00 cycles/insert() min,   825.36 avg,  1748.00 max 
     1.77 instructions/cycle 
   173.00 branches/insert() min,   685.28 avg,  1161.00 max 
   3.2708 mis. branches/insert() 
tree insert time
  2271.00 instructions/insert() min, 10575.11 avg, 27559.00 max 
   705.00 cycles/insert() min,  2207.84 avg,  5866.00 max 
     3.22 instructions/cycle 
   400.00 branches/insert() min,  2368.66 avg,  6416.00 max 
   5.8160 mis. branches/insert() 
tree exact match insert time
  2157.00 instructions/insert() min, 10581.44 avg, 27554.00 max 
   710.00 cycles/insert() min,  2261.52 avg,  5567.00 max 
     3.04 instructions/cycle 
   384.00 branches/insert() min,  2370.49 avg,  6414.00 max 
   5.6771 mis. branches/insert() 
splay insert time
   886.00 instructions/insert() min,  2183.43 avg,  4266.00 max 
   431.00 cycles/insert() min,   655.52 avg,  1208.00 max 
     2.06 instructions/cycle 
   156.00 branches/insert() min,   452.43 avg,   973.00 max 
   0.7882 mis. branches/insert() 

hash map iterate time
   137.00 instructions/++it min,   141.43 avg,   203.00 max 
   121.00 cycles/++it min,   163.69 avg,   497.00 max 
     1.13 instructions/cycle 
    24.00 branches/++it min,    25.48 avg,    37.00 max 
   1.5278 mis. branches/++it 
map iterate time
   137.00 instructions/++it min,   141.43 avg,   203.00 max 
   120.00 cycles/++it min,   153.06 avg,   407.00 max 
     1.14 instructions/cycle 
    24.00 branches/++it min,    25.48 avg,    37.00 max 
   1.4861 mis. branches/++it 
tree iterate time
   136.00 instructions/++it min,   140.98 avg,   141.00 max 
   127.00 cycles/++it min,   142.40 avg,   743.00 max 
     1.07 instructions/cycle 
    23.00 branches/++it min,    25.99 avg,    26.00 max 
   0.0174 mis. branches/++it 
tree exact match iterate time
   139.00 instructions/++it min,   140.99 avg,   141.00 max 
   122.00 cycles/++it min,   142.22 avg,   668.00 max 
     1.14 instructions/cycle 
    25.00 branches/++it min,    25.00 avg,    25.00 max 
   0.0208 mis. branches/++it 
splay iterate time
   159.00 instructions/++it min,  3021.51 avg,  3035.00 max 
   151.00 cycles/++it min,   656.76 avg,  1535.00 max 
     1.05 instructions/cycle 
    29.00 branches/++it min,   736.53 avg,   739.00 max 
   3.5938 mis. branches/++it 

Tree forwards iterator matches unique keys
Tree reverse iterator matches unique keys
hash map find time
  2458.00 instructions/find() min,  2462.52 avg,  2497.00 max 
   699.00 cycles/find() min,   810.09 avg, 11889.00 max 
     3.52 instructions/cycle 
   297.00 branches/find() min,   298.74 avg,   312.00 max 
   1.3190 mis. branches/find() 
hash map top 10% find time
  2458.00 instructions/find() min,  2464.50 avg,  2497.00 max 
   769.00 cycles/find() min,   993.64 avg,  3642.00 max 
     3.20 instructions/cycle 
   297.00 branches/find() min,   299.50 avg,   312.00 max 
   1.5071 mis. branches/find() 
map find time
  2364.00 instructions/find() min,  2933.80 avg,  3493.00 max 
   545.00 cycles/find() min,   737.00 avg,  7274.00 max 
     4.34 instructions/cycle 
   569.00 branches/find() min,   710.81 avg,   850.00 max 
   4.1207 mis. branches/find() 
map top 10% find time
  2364.00 instructions/find() min,  2878.93 avg,  3244.00 max 
   616.00 cycles/find() min,   945.95 avg,  1727.00 max 
     3.84 instructions/cycle 
   569.00 branches/find() min,   697.02 avg,   789.00 max 
   6.8024 mis. branches/find() 
tree prefix match find time
   238.00 instructions/find() min,   443.23 avg,   895.00 max 
   179.00 cycles/find() min,   314.57 avg,  6368.00 max 
     1.33 instructions/cycle 
    41.00 branches/find() min,    74.79 avg,   137.00 max 
   2.2801 mis. branches/find() 
tree top 10% prefix match find time
   363.00 instructions/find() min,   505.02 avg,   693.00 max 
   200.00 cycles/find() min,   389.50 avg,  1095.00 max 
     1.81 instructions/cycle 
    61.00 branches/find() min,    81.27 avg,   109.00 max 
   4.3548 mis. branches/find() 
tree exact match find time
  2865.00 instructions/find() min,  4599.09 avg,  6380.00 max 
   713.00 cycles/find() min,  1082.47 avg, 30288.00 max 
     4.02 instructions/cycle 
   646.00 branches/find() min,   926.71 avg,  1191.00 max 
   4.2474 mis. branches/find() 
tree exact match top 10% find time
  3880.00 instructions/find() min,  4441.59 avg,  5199.00 max 
   801.00 cycles/find() min,  1125.55 avg,  2482.00 max 
     4.84 instructions/cycle 
   811.00 branches/find() min,   897.84 avg,  1018.00 max 
   6.1952 mis. branches/find() 
splay find time
  2991.00 instructions/find() min,  3927.80 avg, 17704.00 max 
   585.00 cycles/find() min,   936.30 avg, 27581.00 max 
     5.11 instructions/cycle 
   728.00 branches/find() min,   949.70 avg,  3876.00 max 
   6.4087 mis. branches/find() 
splay top 10% find time
  2991.00 instructions/find() min,  5174.70 avg,  7034.00 max 
   680.00 cycles/find() min,  1516.75 avg,  3978.00 max 
     4.40 instructions/cycle 
   728.00 branches/find() min,  1241.32 avg,  1695.00 max 
  11.7571 mis. branches/find() 

hash map erase time
  3078.00 instructions/erase() min,  3277.92 avg,  3505.00 max 
   848.00 cycles/erase() min,  1122.11 avg, 10035.00 max 
     3.63 instructions/cycle 
   418.00 branches/erase() min,   452.85 avg,   491.00 max 
   1.9201 mis. branches/erase() 
map erase time
  2847.00 instructions/erase() min,  3415.26 avg,  3936.00 max 
   625.00 cycles/erase() min,  1013.75 avg,  7298.00 max 
     4.56 instructions/cycle 
   650.00 branches/erase() min,   783.53 avg,   914.00 max 
   4.8646 mis. branches/erase() 
tree erase time
  3314.00 instructions/erase() min,  6132.85 avg,  9834.00 max 
   778.00 cycles/erase() min,  1785.12 avg, 15184.00 max 
     4.26 instructions/cycle 
   673.00 branches/erase() min,  1160.93 avg,  1801.00 max 
   7.0417 mis. branches/erase() 
tree exact match erase time
  3306.00 instructions/erase() min,  5548.51 avg,  8528.00 max 
   788.00 cycles/erase() min,  1454.56 avg,  4540.00 max 
     4.20 instructions/cycle 
   671.00 branches/erase() min,  1060.91 avg,  1580.00 max 
   6.0833 mis. branches/erase() 
splay erase time
  4809.00 instructions/erase() min,  5544.08 avg,  7589.00 max 
   930.00 cycles/erase() min,  1175.55 avg,  2173.00 max 
     5.17 instructions/cycle 
  1051.00 branches/erase() min,  1229.50 avg,  1720.00 max 
   4.8958 mis. branches/erase() 
Number of runs for insert() 288

Number of runs for find() 103880
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
### Small messages FlashRadixTree performs well against map for find().
```
Unit tests passed
size of one line in bytes: 6b
hash map insert time
   400.00 instructions/insert() min,   469.73 avg,  6559.00 max 
   217.00 cycles/insert() min,   406.56 avg, 10388.00 max 
     1.84 instructions/cycle 
    68.00 branches/insert() min,    85.95 avg,  1417.00 max 
   1.7465 mis. branches/insert() 
map insert time
   333.00 instructions/insert() min,  1134.60 avg,  1904.00 max 
   315.00 cycles/insert() min,   512.12 avg,  3965.00 max 
     1.06 instructions/cycle 
    60.00 branches/insert() min,   281.19 avg,   480.00 max 
   2.5139 mis. branches/insert() 
tree insert time
   762.00 instructions/insert() min,  1533.51 avg,  4753.00 max 
   333.00 cycles/insert() min,   762.87 avg,  7537.00 max 
     2.29 instructions/cycle 
   136.00 branches/insert() min,   282.01 avg,   858.00 max 
   2.9167 mis. branches/insert() 
tree exact match insert time
   763.00 instructions/insert() min,  1540.53 avg,  4725.00 max 
   329.00 cycles/insert() min,   636.47 avg,  7549.00 max 
     2.32 instructions/cycle 
   136.00 branches/insert() min,   283.86 avg,   912.00 max 
   1.8750 mis. branches/insert() 
splay insert time
   538.00 instructions/insert() min,   569.95 avg,  1336.00 max 
   182.00 cycles/insert() min,   234.03 avg,  4508.00 max 
     2.96 instructions/cycle 
    97.00 branches/insert() min,   105.30 avg,   300.00 max 
   0.7014 mis. branches/insert() 

hash map iterate time
   137.00 instructions/++it min,   141.43 avg,   203.00 max 
   122.00 cycles/++it min,   199.68 avg,  4165.00 max 
     1.12 instructions/cycle 
    24.00 branches/++it min,    25.48 avg,    37.00 max 
   1.7674 mis. branches/++it 
map iterate time
   137.00 instructions/++it min,   141.43 avg,   203.00 max 
   123.00 cycles/++it min,   159.72 avg,   381.00 max 
     1.11 instructions/cycle 
    24.00 branches/++it min,    25.48 avg,    37.00 max 
   1.7847 mis. branches/++it 
tree iterate time
   136.00 instructions/++it min,   140.98 avg,   141.00 max 
   126.00 cycles/++it min,   134.47 avg,   231.00 max 
     1.08 instructions/cycle 
    23.00 branches/++it min,    25.99 avg,    26.00 max 
   0.0174 mis. branches/++it 
tree exact match iterate time
   137.00 instructions/++it min,   141.98 avg,   142.00 max 
   123.00 cycles/++it min,   133.10 avg,   208.00 max 
     1.11 instructions/cycle 
    23.00 branches/++it min,    25.99 avg,    26.00 max 
   0.0174 mis. branches/++it 
splay iterate time
   160.00 instructions/++it min,   403.64 avg,   408.00 max 
   153.00 cycles/++it min,   199.64 avg,   580.00 max 
     1.05 instructions/cycle 
    29.00 branches/++it min,    87.80 avg,    88.00 max 
   0.5938 mis. branches/++it 

Tree forwards iterator matches unique keys
Tree reverse iterator matches unique keys
hash map find time
   275.00 instructions/find() min,   276.69 avg,   327.00 max 
   155.00 cycles/find() min,   167.00 avg, 13692.00 max 
     1.77 instructions/cycle 
    53.00 branches/find() min,    53.65 avg,    73.00 max 
   0.1323 mis. branches/find() 
hash map top 10% find time
   275.00 instructions/find() min,   278.71 avg,   301.00 max 
   155.00 cycles/find() min,   189.32 avg,   819.00 max 
     1.77 instructions/cycle 
    53.00 branches/find() min,    54.43 avg,    63.00 max 
   0.2952 mis. branches/find() 
map find time
   474.00 instructions/find() min,   568.67 avg,   760.00 max 
   211.00 cycles/find() min,   278.69 avg,  3691.00 max 
     2.25 instructions/cycle 
   117.00 branches/find() min,   145.59 avg,   200.00 max 
   1.4273 mis. branches/find() 
map top 10% find time
   515.00 instructions/find() min,   550.55 avg,   719.00 max 
   246.00 cycles/find() min,   340.54 avg,   462.00 max 
     2.09 instructions/cycle 
   129.00 branches/find() min,   140.38 avg,   188.00 max 
   3.8238 mis. branches/find() 
tree prefix match find time
   237.00 instructions/find() min,   439.02 avg,   889.00 max 
   170.00 cycles/find() min,   284.10 avg, 11275.00 max 
     1.39 instructions/cycle 
    41.00 branches/find() min,    73.18 avg,   135.00 max 
   2.2892 mis. branches/find() 
tree top 10% prefix match find time
   360.00 instructions/find() min,   495.81 avg,   688.00 max 
   174.00 cycles/find() min,   336.83 avg,  1973.00 max 
     2.07 instructions/cycle 
    60.00 branches/find() min,    79.44 avg,   107.00 max 
   4.2238 mis. branches/find() 
tree exact match find time
   360.00 instructions/find() min,   692.88 avg,  1174.00 max 
   200.00 cycles/find() min,   335.18 avg, 10988.00 max 
     1.80 instructions/cycle 
    74.00 branches/find() min,   150.54 avg,   240.00 max 
   2.2924 mis. branches/find() 
tree exact match top 10% find time
   566.00 instructions/find() min,   735.66 avg,   973.00 max 
   210.00 cycles/find() min,   394.70 avg,  6641.00 max 
     2.70 instructions/cycle 
   121.00 branches/find() min,   152.01 avg,   195.00 max 
   4.2143 mis. branches/find() 
splay find time
   364.00 instructions/find() min,   590.35 avg,  9600.00 max 
   155.00 cycles/find() min,   287.37 avg, 13605.00 max 
     2.35 instructions/cycle 
    77.00 branches/find() min,   132.86 avg,  2273.00 max 
   2.4201 mis. branches/find() 
splay top 10% find time
   364.00 instructions/find() min,   920.34 avg,  1456.00 max 
   197.00 cycles/find() min,   431.57 avg,   821.00 max 
     1.85 instructions/cycle 
    77.00 branches/find() min,   212.41 avg,   345.00 max 
   5.4333 mis. branches/find() 

hash map erase time
   497.00 instructions/erase() min,   529.74 avg,   686.00 max 
   205.00 cycles/erase() min,   320.38 avg,  4139.00 max 
     2.42 instructions/cycle 
   103.00 branches/erase() min,   111.26 avg,   136.00 max 
   0.7049 mis. branches/erase() 
map erase time
   496.00 instructions/erase() min,   722.40 avg,   892.00 max 
   234.00 cycles/erase() min,   314.42 avg,  3217.00 max 
     2.12 instructions/cycle 
   117.00 branches/erase() min,   174.86 avg,   225.00 max 
   1.4201 mis. branches/erase() 
tree erase time
   896.00 instructions/erase() min,  1257.93 avg,  1748.00 max 
   334.00 cycles/erase() min,   523.02 avg,  5917.00 max 
     2.68 instructions/cycle 
   195.00 branches/erase() min,   283.75 avg,   396.00 max 
   2.5000 mis. branches/erase() 
tree exact match erase time
   889.00 instructions/erase() min,  1257.41 avg,  1744.00 max 
   330.00 cycles/erase() min,   530.84 avg,  9227.00 max 
     2.69 instructions/cycle 
   193.00 branches/erase() min,   283.01 avg,   394.00 max 
   2.4097 mis. branches/erase() 
splay erase time
   583.00 instructions/erase() min,   687.20 avg,  1076.00 max 
   226.00 cycles/erase() min,   281.38 avg,   748.00 max 
     2.58 instructions/cycle 
   125.00 branches/erase() min,   151.22 avg,   246.00 max 
   1.4097 mis. branches/erase() 
Number of runs for insert() 288

Number of runs for find() 103880
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

## Usage

The container comes with many standard map like functions. 
For speed I recomend using value_type_pointer get(const Key& key) instead of iterator find(const Key& key) as find() returns an iterator and get() returns a pointer which is much faster. 

The 'Key' type needs to be either std::basic_string or std::basic_string_view. You'll get an error if its not. You can replace they 'value_type' of the type as needed, but it needs to support less than, greater than, and equals operators.  

I recomend using string_view unless you need to erase a lot. As erasing colapses tree nodes so its not possible to combine strig_views to form new string_views. So for the string_view variation it will 'mark as deleted', while string variation will actually erase. string_view will be a lot faster and avoids copying keys so use it if you can get away with it. 

Example of creating a new object
```
FlashRadixTree<std::string, int, MatchMode::Exact> rTreeExact;
FlashRadixTree<std::string_view, int, MatchMode::Prefix> rTreePrefix;

auto itInsert = rTree.insert("AB", 1);
auto valExpect = rTree.find("AB");
std::stringstream ss;
for( auto& it : rTree)
{
    ss << it.prefix << "," << it.value << "," << it.getFullKey() << std::endl;
}
rTree.erase("AB");
```
