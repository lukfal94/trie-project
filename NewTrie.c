// Luke Fallon
// 05 May 2014
// =============
// NewTrie.c
// =============
/*
	A program that takes in a corpus (with little formatting) and collects statistics
	and information about the text.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Trie.h"

TrieNode *createTrieNode(void);
TrieNode *buildSubTrie(TrieNode *root);
TrieNode *buildSubTrieHelper(TrieNode *root, TrieNode *temp, char *buffer, int k);
TrieNode *mergeTriesHelper(TrieNode *root, TrieNode *new, char *buffer, int k);
TrieNode *mergeTries(TrieNode *root, TrieNode *new);
TrieNode *getNode(TrieNode *root, char *str);
TrieNode *insertString(TrieNode *root, char *str);
TrieNode *duplicateTrie(TrieNode *root, TrieNode *new);
TrieNode *deleteString(TrieNode *root, char *str);
TrieNode *deleteStringHelper(TrieNode *root, char *str);
char *removeLast(char *str);
int hasChildren(TrieNode *root);
int hasPunct(char *str);
void printTrieHelper(TrieNode *root, char *buffer, int k);
void printTrie(TrieNode *root, int useSubtrieFormatting);
char *toLower(char *str);

typedef struct Corpus
{
	TrieNode *root;
	int num_word;
	int num_sent;
}

int main(int argc, char **argv)
{
	// Build a trie from the input corpus
	TrieNode *trie = buildTrie(argv[1]), *subTrie;
	FILE *ifp = fopen(argv[2], "r");
	char buffer[1024], *str;

	// Scan in the next string in the input file
	while(fscanf(ifp, "%s", buffer) != EOF)
	{
		// If it is an exclamation point, print the trie
		if(!strcmp(buffer, "!"))
		{
			printTrie(trie, 0);
		}
		// Otherwise, it's a word and we need to print its subtrie.
		else
		{
			// Print the string as input. (Note: might be capitalized strangely)
			printf("%s\n", buffer);
			
			// Convert from the buffer to a more useful, lowercase string
			str = malloc(sizeof(char) * (strlen(buffer) + 1));
			strcpy(str, buffer);
			str = toLower(str);
			
			// Locate the node of the desired word.
			subTrie = getNode(trie, str);
			
			// Print accordingly
			if(subTrie == NULL || subTrie->count == 0)
				printf("(INVALID STRING)\n");
			else if(subTrie->subtrie == NULL)
				printf("(EMPTY)\n");
			else
				printTrie(subTrie->subtrie, 1);
		}
	}

	fclose(ifp);
	return 0;
}

// Creates a trie from an input corpus with words less than 1024 characters.
TrieNode *buildTrie(char *filename)
{
	TrieNode *root = NULL, *new = NULL, *temp;
	FILE *ifp = fopen(filename,"r");
	char buffer[1024], *str;
	int numWords = 0;
	
	// Read in a sentence word by word.
	while((fscanf(ifp, "%s", buffer) != EOF))
	{
		
		if(hasPunct(buffer))
		{
			// Convert from the buffer to a more useful, lowercase string
			str = malloc(sizeof(char) * (strlen(buffer) + 1));
			strcpy(str, buffer);
			
			str = removeLast(str);
			str = toLower(str);
			
			// Insert string into the trie.
			new = insertString(new, str);
			
			// If string is unique in the trie, its count will be one.
			temp = getNode(new, str);
			if(temp->count == 1)
				numWords++;

		}
		else
		{
			// Only build co-occurrence trie if there's more than one unique word in the sentence.
			if(numWords > 1)
				new = buildSubTrie(new);
			
			// Reset the number of unique words in the setence.
			numWords = 0;
			
			// Merge the trie for the newest sentence with the existing trie.
			root = mergeTries(root, new);
			
			// Create a new "sentence" trie.
			new = createTrieNode();
		}
	}
	
	fclose(ifp);
	return root;
}

// Returns a pointer to an empty TrieNode.
TrieNode *createTrieNode(void)
{
	return calloc(1, sizeof(TrieNode));
}

// Input:	The root of a trie, and a string to insert.
//
// Output:	Insert the string into the trie, returns the root of the trie.
TrieNode *insertString(TrieNode *root, char *str)
{
	TrieNode *temp;
	int i, len;
	
	if(root == NULL)
	{
		root = createTrieNode();
	}	

	// Convert from the buffer to a manageable, lowercase string
	// and find its length.
	str = toLower(str);
	len = strlen(str);

	temp = root;
	for(i = 0; i < len; i++)
	{
		// Create a new node if one isn't already made.
		if(temp->children[str[i] - 'a'] == NULL)
			temp->children[str[i] - 'a'] = createTrieNode();
			
		temp = temp->children[str[i] - 'a'];
	}

	temp->count++;
	return root;
}

// Returns NULL if str is not in the trie, or a pointer to the node 
// of the string if it exists.
TrieNode *getNode(TrieNode *root, char *str)
{
	TrieNode *temp = root;
	int i;
		
	for(i = 0; i < strlen(str); i++)
	{
		temp = temp->children[str[i] - 'a'];
		if(temp == NULL) 
			return NULL;
	}
	
	return temp;
}

// Recursive helper function for buildSubTrie. Based off of printTrieHelper.
TrieNode *buildSubTrieHelper(TrieNode *root, TrieNode *temp, char *buffer, int k)
{
	int i;
	
	if (temp == NULL)
		return NULL;

	// Found a word represented in the trie.
	if (temp->count > 0)
	{
		// Duplicate the trie and root it at the words subtrie pointer.
		// Delete any occurrences of the word itself.
		temp->subtrie = duplicateTrie(root, temp->subtrie);
		temp->subtrie = deleteString(temp->subtrie, buffer);
	}
	
	buffer[k + 1] = '\0';

	for (i = 0; i < 26; i++)
	{
		buffer[k] = 'a' + i;
		buildSubTrieHelper(root, temp->children[i], buffer, k + 1);
	}

	buffer[k] = '\0';
		
	return temp;
} 

// Takes in a trie representative of a sentence and creates co-occurrence subtries
// for all word in the sentence.
//
// Returns a pointer to the root of the trie.
TrieNode *buildSubTrie(TrieNode *root)
{	
	TrieNode *withSubs = NULL;
	char buffer[1026];
	
	withSubs = duplicateTrie(root, withSubs);
	
	return buildSubTrieHelper(root, withSubs, buffer, 0);
}

// Creates and returns a pointer to a duplicate of a trie.
TrieNode *duplicateTrie(TrieNode *root, TrieNode *new)
{
	int i;
	
	if(root == NULL)
		return NULL;
		
	if(new == NULL)
		new = createTrieNode();
		
	for(i = 0; i < 26; i++)
	{
		if(root->children[i] != NULL)
		{
			
			new->children[i] = duplicateTrie(root->children[i], new->children[i]);
			new->children[i]->count = root->children[i]->count;
			new->children[i]->subtrie = root->children[i]->subtrie;
		}
	}
	
	return new;
}

// mergeTries() helper function.
TrieNode *mergeTriesHelper(TrieNode *root, TrieNode *new, char *buffer, int k)
{
	int i, j;
	
	if (new == NULL)
		return root;

	// A word represented in new, insert it into root.
	if (new->count > 0)
	{
		for(j = 0; j < new->count; j++)
			root = insertString(root, buffer);
	
		getNode(root, buffer)->subtrie = mergeTries(getNode(root, buffer)->subtrie, new->subtrie);
	}
	
	buffer[k + 1] = '\0';

	for (i = 0; i < 26; i++)
	{
		buffer[k] = 'a' + i;
		mergeTriesHelper(root, new->children[i], buffer, k + 1);
	}

	buffer[k] = '\0';
	
	return root;
}

// Combines two tries, root and new.
TrieNode *mergeTries(TrieNode *root, TrieNode *new)
{	
	char buffer[1026];
	
	if(root == NULL && new == NULL)
		return NULL;
	else if(root == NULL)
		return new;
	else if(new == NULL)
		return root;
	return mergeTriesHelper(root, new, buffer, 0);
}

// Removes all copies of a string from a trie.
TrieNode *deleteString(TrieNode *root, char *str)
{
	TrieNode *node = getNode(root, str);
	
    if(node->count > 0 || hasChildren(node))
    	node->count = 0;
    else
    {
    	free(node);
    	node = NULL;
    	return deleteStringHelper(root, removeLast(str));
	}
	return root;
}

// Helper function for deleteString()
TrieNode *deleteStringHelper(TrieNode *root, char *str)
{
	TrieNode *node = getNode(root, str);
	
	if(node->count > 1 || hasChildren(node))
		return root;
	else
		return deleteStringHelper(root, removeLast(str));
}

// Replaces the last character of a string with a null terminator
char *removeLast(char *str)
{
	int len = strlen(str), i;
	char *new;
	
	if(len == 1)
	{
		str[0] = '\0';
		return str;
	}
	
	new = malloc(sizeof(char) * len);
	for(i = 0; i < len - 1; i++)
		new[i] = str[i];
	new[i] = '\0';
		
	return new;
}

// Returns 1 if a node has children, 0 otherwise.
int hasChildren(TrieNode *root)
{
	int i;
	for(i = 0; i < 26; i++)
		if(root->children[i] != NULL)
			return 1;
	return 0;
}

// Returns 1 if there is any punctuation in the word.
int hasPunct(char *str)
{
	int i;
	
	for(i = 0; i < strlen(str) -1; i++)
		if(!isalpha(str[i]))
			return 1;
	
	return 0;
}

// Converts a string to lowercase letters
//
// Returns pointer to the new string.
char *toLower(char *str)
{
	int i = 0;
	char *lower = malloc(sizeof(char) * (strlen(str) + 1));
	
	while(str[i] != '\0')
	{
		lower[i] = tolower(str[i]);
		i++;
	}
	
	return lower;
}

// Helper function called by printTrie()
void printTrieHelper(TrieNode *root, char *buffer, int k)
{
	int i;

	if (root == NULL)
		return;
	if (root->count > 0)
	{
		
		printf("%s (%d)\n", buffer, root->count);
	}
	buffer[k + 1] = '\0';

	for (i = 0; i < 26; i++)
	{
		buffer[k] = 'a' + i;

		printTrieHelper(root->children[i], buffer, k + 1);
	}

	buffer[k] = '\0';
}

// If printing a subtrie, the second parameter should be 1; otherwise, 0.
void printTrie(TrieNode *root, int useSubtrieFormatting)
{
	char buffer[1026];

	if (useSubtrieFormatting)
	{
		strcpy(buffer, "- ");
		printTrieHelper(root, buffer, 2);
	}
	else
	{
		strcpy(buffer, "");
		printTrieHelper(root, buffer, 0);
	}
}
