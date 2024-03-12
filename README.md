# FlashRadixTree
Fast key value pair data structure with ordered traversal. For use with keys with common prefixes and asymetric key distribution.

## Performance Overview
### 1.7KB key Apple M2 (ARM 64), Somoma 14.3.1 (23D60) CPU Cycles Comparison
```
FlashRadixTree::find() (using partial key match)
355.00 cycles/find() min,753.93 avg, 38,146.00 max 
map::find()
548.00 cycles/find() min, 714.92 avg, 15,022.00 max 
unordered_map::find()
711.00 cycles/find() min, 824.49 avg, 13,226.00 max
```
### 6byte key 
```
FlashRadixTree::find() (using partial key match)
165.00 cycles/find() min, 257.14 avg, 1,1174.00 max
map::find()
221.00 cycles/find() min,   305.85 avg, 5,0012.00 max  
unordered_map::find()
156.00 cycles/find() min,   173.40 avg, 2,1561.00 max
```

## Overview
It's hard to beat STL's' unordered_map for raw speed. Especially lookup speed.
But if you need ordered traversal then you lose the abilty to hash and the penalty can be high. Typically the AVL tree implementation tries to balance, but its trying to optimised against the average case. In the case of calling find() over a key set with non uniform distribution your average AVL tree is not optimised for this. 

This project explores some ideas i've had regarding alternate data stuctures to the unorderd_map and map, and aiming the exploration to key sets with a non uniform distributin and key sets with large number of characters and common prefix's. Example of these can be found in many places. 

### What is a Radix tree? 
Its a compressed form of a Trie. A Trie is a tree of nodes, where each node is a character. The root node is a null character, and each node has a list of children, each child is a character. 

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

### How to search fast then using this advantage? 
The children of the tree will always have a unique first character, so we can assume that if we have a match of the first character and the node in question is End-of-word then we have a match, and we can save the comparison on the remaining characters. Likewise if we find a node which is end of word, and the lenghts of the key and prefix match we can also save a full comparison as this must be the key we're looking for. This may be an avantage, especially in situations with very long keys.
For this reason the FlashRadixTree has two match modes specified on construction, "Partial" and "Exact". Where partial assumes the case above, and Exact matches the entire key when required.

The non uniform nature of keys is an interesting case which this structure tries to focus on. In many cases you may find that some keys are more active than others. this is probably very true of a lot of other scenarios. Can we optimise for this. Well..? this is a ticky one...

Enter the curios case of the Splay tree. 

### What is a Splay tree? 
Its a self adjusting binary search tree with the additional property that recently accessed elements are quick to access again. It does this by rotating the trough through a series of "Zigs", "Zags", and "Zig Zag" rotations to bring the accessed node to the root of the tree during the access. This is a very interesting property. A good description of this can be found here: https://en.wikipedia.org/wiki/Splay_tree

It performs basic binary search tree operations such as insertion, look-up and removal in O(log n) amortized time. For many practical purposes, it is the same as the self-balancing binary search tree. It was invented by Daniel Sleator and Robert Tarjan in 1985.
A lot of research has been done into this data structure and to this day it remains a bit of an enigma as to its performance characteristics and temporal locatity benefits. But it does seem to work work well in some situatios for non uniform access. Particulaly because of the self adjusting property and the temporal locality of the data.

So in the FlashRadixTree i offer two options. To use the use of the Splay tree or the use of the map. The Splay tree or map is used to populate the child nodes of reach level of our radixt tree. As for which is better? It may depend on your key set, and I leave it to you to test which one works better. 


## Example Timings (Apple M2 (ARM 64), Somoma 14.3.1 (23D60)) 
... will aim to get some linux test soon. Sorry for now we timings can only be done on apple silicon. Will add intel soon hopefully.

Large messages FlashRadixTree perform well aginst unordered_map and map for find() in these tests. (again i encourage your own testing).
I also focus on the top 10% of keys used to find as typiclly you may wan them to be perfomant.
```
size of one line in kilobytes: 1.75781kb
hash map insert time
  2295.00 instructions/insert() min,  2450.13 avg,  8404.00 max 
   714.00 cycles/insert() min,  1023.88 avg,  6267.00 max 
     3.21 instructions/cycle 
   204.00 branches/insert() min,   227.83 avg,  1509.00 max 
   2.2500 mis. branches/insert() 
map insert time
   970.00 instructions/insert() min,  2136.51 avg,  3794.00 max 
   497.00 cycles/insert() min,   869.30 avg,  1445.00 max 
     1.95 instructions/cycle 
   170.00 branches/insert() min,   453.01 avg,   862.00 max 
   7.4826 mis. branches/insert() 
tree insert time
  2037.00 instructions/insert() min,  8122.93 avg, 20335.00 max 
   669.00 cycles/insert() min,  2061.91 avg,  5011.00 max 
     3.04 instructions/cycle 
   358.00 branches/insert() min,  1896.69 avg,  5020.00 max 
   9.0486 mis. branches/insert() 
tree exact match insert time
  2039.00 instructions/insert() min,  8130.41 avg, 20346.00 max 
   605.00 cycles/insert() min,  2054.74 avg,  4954.00 max 
     3.37 instructions/cycle 
   358.00 branches/insert() min,  1898.00 avg,  5020.00 max 
   8.8264 mis. branches/insert() 
splay insert time
  5520.00 instructions/insert() min,  9029.56 avg, 14926.00 max 
  1305.00 cycles/insert() min,  2173.40 avg,  4169.00 max 
     4.23 instructions/cycle 
   965.00 branches/insert() min,  1743.02 avg,  3165.00 max 
   8.7882 mis. branches/insert() 

hash map find time
  2460.00 instructions/find() min,  2464.83 avg,  2499.00 max 
   711.00 cycles/find() min,   824.49 avg, 13226.00 max 
     3.46 instructions/cycle 
   298.00 branches/find() min,   299.86 avg,   313.00 max 
   1.2616 mis. branches/find() 
hash map top 10% find time
  2460.00 instructions/find() min,  2465.62 avg,  2486.00 max 
   782.00 cycles/find() min,  1062.76 avg,  1970.00 max 
     3.15 instructions/cycle 
   298.00 branches/find() min,   300.16 avg,   308.00 max 
   1.4136 mis. branches/find() 
map find time
  2330.00 instructions/find() min,  2849.16 avg,  3583.00 max 
   548.00 cycles/find() min,   714.92 avg, 15022.00 max 
     4.25 instructions/cycle 
   570.00 branches/find() min,   697.47 avg,   879.00 max 
   4.1208 mis. branches/find() 
map top 10% find time
  2514.00 instructions/find() min,  2867.41 avg,  3440.00 max 
   630.00 cycles/find() min,  1039.65 avg,  3191.00 max 
     3.99 instructions/cycle 
   612.00 branches/find() min,   702.55 avg,   844.00 max 
   7.5932 mis. branches/find() 
tree prefix match find time
   979.00 instructions/find() min,  2881.42 avg,  4788.00 max 
   355.00 cycles/find() min,   753.93 avg, 38146.00 max 
     2.76 instructions/cycle 
   178.00 branches/find() min,   506.92 avg,   809.00 max 
   3.4475 mis. branches/find() 
tree top 10% prefix match find time
  2098.00 instructions/find() min,  2783.34 avg,  3507.00 max 
   475.00 cycles/find() min,   818.88 avg,  3122.00 max 
     4.42 instructions/cycle 
   373.00 branches/find() min,   486.71 avg,   610.00 max 
   5.4795 mis. branches/find() 
tree exact match find time
  1918.00 instructions/find() min,  3576.10 avg,  5154.00 max 
   540.00 cycles/find() min,   839.12 avg,  3871.00 max 
     3.55 instructions/cycle 
   411.00 branches/find() min,   678.85 avg,   930.00 max 
   3.8322 mis. branches/find() 
tree exact match top 10% find time
  2885.00 instructions/find() min,  3501.25 avg,  4142.00 max 
   607.00 cycles/find() min,   957.91 avg,  2153.00 max 
     4.75 instructions/cycle 
   568.00 branches/find() min,   664.44 avg,   767.00 max 
   5.9000 mis. branches/find() 
splay find time
  6437.00 instructions/find() min,  7904.54 avg, 15229.00 max 
  1201.00 cycles/find() min,  1637.45 avg, 33462.00 max 
     5.36 instructions/cycle 
  1325.00 branches/find() min,  1680.33 avg,  3461.00 max 
   6.9067 mis. branches/find() 
splay top 10% find time
  7097.00 instructions/find() min,  9712.68 avg, 12420.00 max 
  1354.00 cycles/find() min,  2240.98 avg,  3170.00 max 
     5.24 instructions/cycle 
  1476.00 branches/find() min,  2109.98 avg,  2778.00 max 
  12.5841 mis. branches/find() 

hash map erase time
  3086.00 instructions/erase() min,  3267.94 avg,  3393.00 max 
   838.00 cycles/erase() min,   946.31 avg,  5549.00 max 
     3.68 instructions/cycle 
   420.00 branches/erase() min,   451.76 avg,   470.00 max 
   1.9792 mis. branches/erase() 
map erase time
  2860.00 instructions/erase() min,  3540.08 avg,  4424.00 max 
   668.00 cycles/erase() min,  1360.00 avg,  7387.00 max 
     4.28 instructions/cycle 
   656.00 branches/erase() min,   822.37 avg,  1040.00 max 
  11.2396 mis. branches/erase() 
tree erase time
  2353.00 instructions/erase() min,  4653.68 avg,  7835.00 max 
   643.00 cycles/erase() min,  1610.23 avg,  9943.00 max 
     3.66 instructions/cycle 
   504.00 branches/erase() min,   902.82 avg,  1464.00 max 
   8.5903 mis. branches/erase() 
tree exact match erase time
  2353.00 instructions/erase() min,  4621.75 avg,  7840.00 max 
   612.00 cycles/erase() min,  1462.19 avg,  6306.00 max 
     3.84 instructions/cycle 
   504.00 branches/erase() min,   899.31 avg,  1464.00 max 
   8.6250 mis. branches/erase() 
splay erase time
  7237.00 instructions/erase() min, 11309.68 avg, 15303.00 max 
  1373.00 cycles/erase() min,  2397.78 avg,  7119.00 max 
     5.27 instructions/cycle 
  1472.00 branches/erase() min,  2401.01 avg,  3368.00 max 
  12.0347 mis. branches/erase() 
Number of runs for insert() 288

Number of runs for find() 103880
```
### Small messages FlashRadixTree performs well aginst map for find().
```
Unit tests passed
size of one line in bytes: 6b
hash map insert time
   387.00 instructions/insert() min,   476.30 avg,  6414.00 max 
   181.00 cycles/insert() min,   355.66 avg, 13375.00 max 
     2.14 instructions/cycle 
    65.00 branches/insert() min,    84.72 avg,  1363.00 max 
   2.0590 mis. branches/insert() 
map insert time
   320.00 instructions/insert() min,   752.78 avg,  1447.00 max 
   282.00 cycles/insert() min,   410.04 avg,   937.00 max 
     1.13 instructions/cycle 
    55.00 branches/insert() min,   176.87 avg,   356.00 max 
   5.5035 mis. branches/insert() 
tree insert time
   632.00 instructions/insert() min,  1246.94 avg,  2617.00 max 
   237.00 cycles/insert() min,   563.09 avg, 14052.00 max 
     2.67 instructions/cycle 
   104.00 branches/insert() min,   225.89 avg,   550.00 max 
   6.0000 mis. branches/insert() 
tree exact match insert time
   521.00 instructions/insert() min,  1254.32 avg,  2222.00 max 
   241.00 cycles/insert() min,   619.40 avg, 13870.00 max 
     2.16 instructions/cycle 
    91.00 branches/insert() min,   227.16 avg,   412.00 max 
   6.3264 mis. branches/insert() 
splay insert time
   436.00 instructions/insert() min,  1381.85 avg,  2806.00 max 
   231.00 cycles/insert() min,   561.34 avg,  4222.00 max 
     1.89 instructions/cycle 
    77.00 branches/insert() min,   303.84 avg,   656.00 max 
   7.1840 mis. branches/insert() 

hash map find time
   277.00 instructions/find() min,   280.26 avg,   329.00 max 
   156.00 cycles/find() min,   173.40 avg, 21561.00 max 
     1.78 instructions/cycle 
    53.00 branches/find() min,    54.25 avg,    73.00 max 
   0.2476 mis. branches/find() 
hash map top 10% find time
   277.00 instructions/find() min,   280.55 avg,   290.00 max 
   157.00 cycles/find() min,   189.05 avg,  1868.00 max 
     1.76 instructions/cycle 
    53.00 branches/find() min,    54.36 avg,    58.00 max 
   0.2955 mis. branches/find() 
map find time
   444.00 instructions/find() min,   543.00 avg,   608.00 max 
   221.00 cycles/find() min,   305.85 avg, 50012.00 max 
     2.01 instructions/cycle 
   117.00 branches/find() min,   145.41 avg,   165.00 max 
   1.4974 mis. branches/find() 
map top 10% find time
   483.00 instructions/find() min,   528.75 avg,   608.00 max 
   251.00 cycles/find() min,   337.23 avg,   532.00 max 
     1.92 instructions/cycle 
   127.00 branches/find() min,   141.77 avg,   165.00 max 
   3.6091 mis. branches/find() 
tree prefix match find time
   229.00 instructions/find() min,   489.16 avg,   844.00 max 
   165.00 cycles/find() min,   257.14 avg, 11174.00 max 
     1.39 instructions/cycle 
    39.00 branches/find() min,    97.86 avg,   165.00 max 
   2.2724 mis. branches/find() 
tree top 10% prefix match find time
   389.00 instructions/find() min,   554.02 avg,   736.00 max 
   185.00 cycles/find() min,   325.61 avg,  1012.00 max 
     2.10 instructions/cycle 
    75.00 branches/find() min,   105.68 avg,   136.00 max 
   4.2636 mis. branches/find() 
tree exact match find time
   299.00 instructions/find() min,   546.31 avg,   890.00 max 
   170.00 cycles/find() min,   266.10 avg, 75590.00 max 
     1.76 instructions/cycle 
    54.00 branches/find() min,   109.65 avg,   174.00 max 
   2.2679 mis. branches/find() 
tree exact match top 10% find time
   451.00 instructions/find() min,   611.65 avg,   790.00 max 
   198.00 cycles/find() min,   336.45 avg,   585.00 max 
     2.28 instructions/cycle 
    88.00 branches/find() min,   117.59 avg,   147.00 max 
   4.2568 mis. branches/find() 
splay find time
   434.00 instructions/find() min,   723.75 avg,  2672.00 max 
   173.00 cycles/find() min,   331.04 avg,  8834.00 max 
     2.51 instructions/cycle 
    91.00 branches/find() min,   166.34 avg,   651.00 max 
   2.4522 mis. branches/find() 
splay top 10% find time
   434.00 instructions/find() min,  1109.65 avg,  1899.00 max 
   241.00 cycles/find() min,   477.39 avg,   916.00 max 
     1.80 instructions/cycle 
    91.00 branches/find() min,   261.88 avg,   468.00 max 
   5.3182 mis. branches/find() 

hash map erase time
   501.00 instructions/erase() min,   526.14 avg,   680.00 max 
   201.00 cycles/erase() min,   282.58 avg,  3666.00 max 
     2.49 instructions/cycle 
   102.00 branches/erase() min,   109.88 avg,   135.00 max 
   0.6215 mis. branches/erase() 
map erase time
   500.00 instructions/erase() min,   773.65 avg,   936.00 max 
   317.00 cycles/erase() min,   500.17 avg,  6644.00 max 
     1.58 instructions/cycle 
   120.00 branches/erase() min,   196.26 avg,   242.00 max 
   6.8472 mis. branches/erase() 
tree erase time
   602.00 instructions/erase() min,  1075.25 avg,  1701.00 max 
   290.00 cycles/erase() min,   546.26 avg,  8181.00 max 
     2.08 instructions/cycle 
   135.00 branches/erase() min,   237.13 avg,   385.00 max 
   5.5000 mis. branches/erase() 
tree exact match erase time
   602.00 instructions/erase() min,  1072.15 avg,  1696.00 max 
   299.00 cycles/erase() min,   506.65 avg,  3293.00 max 
     2.01 instructions/cycle 
   135.00 branches/erase() min,   236.51 avg,   384.00 max 
   5.5000 mis. branches/erase() 
splay erase time
   622.00 instructions/erase() min,  1498.39 avg,  4071.00 max 
   264.00 cycles/erase() min,   615.36 avg,  5268.00 max 
     2.36 instructions/cycle 
   133.00 branches/erase() min,   346.36 avg,   972.00 max 
   6.4861 mis. branches/erase() 
Number of runs for insert() 288

Number of runs for find() 103880
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
