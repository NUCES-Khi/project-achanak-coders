#include <iostream>
#include <fstream>
#include <cstring>
using namespace std;

// Tries
class TrieNode
{
public:
    TrieNode *children[26];
    bool isEnd;

    TrieNode()
    {
        isEnd = false;
        for (int i = 0; i < 26; i++)
            children[i] = NULL;
    }
};

class Trie
{
private:
    TrieNode *root;

    // DFS for autocomplete
    void dfs(TrieNode *node, char *buffer, int depth)
    {
        if (node == NULL)
            return;

        if (node->isEnd)
        {
            buffer[depth] = '\0';
            cout << buffer << endl;
        }

        for (int i = 0; i < 26; i++)
        {
            if (node->children[i] != NULL)
            {
                buffer[depth] = 'a' + i;
                dfs(node->children[i], buffer, depth + 1);
            }
        }
    }

public:
    Trie()
    {
        root = new TrieNode();
    }

    void insertWord(const char *word)
    {
        TrieNode *current = root;

        for (int i = 0; word[i] != '\0'; i++)
        {
            char c = word[i];

            if (c < 'a' || c > 'z')
                continue; // skip  invalid chars
            int index = c - 'a';

            if (current->children[index] == NULL)
                current->children[index] = new TrieNode();

            current = current->children[index];
        }
        current->isEnd = true;
    }

    bool searchWord(const char *word)
    {
        TrieNode *current = root;

        for (int i = 0; word[i] != '\0'; i++)
        {
            int index = word[i] - 'a';
            if (index < 0 || index > 25)
                return false;

            if (current->children[index] == NULL)
                return false;

            current = current->children[index];
        }
        return current->isEnd;
    }

    void autocomplete(const char *prefix)
    {
        TrieNode *current = root;

        for (int i = 0; prefix[i] != '\0'; i++)
        {
            int index = prefix[i] - 'a';
            if (current->children[index] == NULL)
            {
                cout << "No suggestions found.\n";
                return;
            }
            current = current->children[index];
        }

        char buffer[100];
        int len = 0;

        // prefix to buffer
        for (len = 0; prefix[len] != '\0'; len++)
            buffer[len] = prefix[len];

        dfs(current, buffer, len);
    }
};

// Hashing
class HashNode
{
public:
    char word[50];
    HashNode *next;

    HashNode(const char *w)
    {
        strcpy(word, w);
        next = NULL;
    }
};

class HashTable
{
private:
    HashNode **table;
    int size;

    // Hash function..
    int hash(const char *word)
    {
        int h = 0;
        for (int i = 0; word[i] != '\0'; i++)
        {
            h = (h * 31 + word[i]) % size;
        }
        return h;
    }

public:
    HashTable(int s = 50000)
    {
        size = s;
        table = new HashNode *[size];
        for (int i = 0; i < size; i++)
            table[i] = NULL;
    }

    void insert(const char *word)
    {
        int index = hash(word);
        HashNode *node = new HashNode(word);

        node->next = table[index];
        table[index] = node;
    }

    bool exists(const char *word)
    {
        int index = hash(word);
        HashNode *current = table[index];

        while (current != NULL)
        {
            if (strcmp(current->word, word) == 0)
                return true;
            current = current->next;
        }
        return false;
    }
};

// Dictionary Class
class DictionaryLoader
{
public:
    static void loadDictionary(const char *filename, Trie &trie, HashTable &ht)
    {
        ifstream fin(filename);

        if (!fin)
        {
            cout << "Cannot open dictionary file...." << endl;
            return;
        }

        char word[50];

        while (fin >> word)
        {
            // lowercase vonverstion
            for (int i = 0; word[i] != '\0'; i++)
            {
                if (word[i] >= 'A' && word[i] <= 'Z')
                    word[i] = word[i] - 'A' + 'a';
            }

            trie.insertWord(word);
            ht.insert(word);
        }

        fin.close();
        cout << "Dictionary Loaded Successfully!..." << endl;
    }
};

int main()
{
    Trie trie;
    HashTable ht(50000);

    // Dictionary Loading...
    DictionaryLoader::loadDictionary("dictionary.txt", trie, ht);

    cout << "Testing Trie Search: " << endl;
    cout << "Search 'hello': " << (trie.searchWord("hello") ? "FOUND" : "NOT FOUND") << endl;

    cout << "Testing Autocomplete for 'he': " << endl;
    trie.autocomplete("he");

    cout << "Testing HashTable Exists: " << endl;
    cout << "Word exists? 'apple': " << (ht.exists("apple") ? "YES" : "NO") << endl;

    return 0;
}
