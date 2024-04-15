
## Issue tracking
[![Open Issues](https://img.shields.io/github/issues-raw/mattyv/FlashRadixTree.svg)](https://github.com/mattyv/FlashRadixTree/issues)
# FlashRadixTree
Fast key value pair data structure with ordered traversal. On par with std::map with keys with common prefixes and asymmetric key distribution.

If you have keys such as "go" "google" "goggles", "hell", "hello" "hollow" and you need to keep them in an ordered container, it turns out a nice way to find the key is to break the keys up into prefixes and store them in a tree structure. If you know you keys will be there i.e your keys space doesn't change after you fill the container, you can search even faster by checking the first letter of parts of the tree... see below for details.

## Performance Overview
### Apple M2 (ARM 64), Somoma 14.3.1 (23D60) CPU Cycles Comparison
![Find() performance Flash Radix Tree](Performance/Performance/tree_prefix_match_cycles_metrics_find().png)
![Find() performance map](Performance/Performance/map_cycles_metrics_find().png)
![Find() performance unordered_map](Performance/Performance/hash_map_cycles_metrics_find().png)

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

[Benchmark Graphs](./Performance/Performance/) and
[Raw Benchmark Data](./Performance/benchmarks)

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
