#ifndef __TRIE_H
#define __TRIE_H

typedef struct TrieNode
{
	// number of times this string occurs in the corpus
	int count;

	// 26 TrieNode pointers, one for each letter of the alphabet
	struct TrieNode *children[26];

	// the co-occurrence subtrie for this string
	struct TrieNode *subtrie;
} TrieNode;


// Functional Prototypes

TrieNode *buildTrie(char *filename);


#endif
