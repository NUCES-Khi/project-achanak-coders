#include <iostream>
#include <fstream>
#include <string>
using namespace std;

// Trie
class TrieNode{
public:
    TrieNode *children[26];
    bool isEnd;

    TrieNode(){
        isEnd = false;
        for (int i = 0; i < 26; i++)
            children[i] = NULL;
    }
};

class Trie{
private:
    TrieNode *root;

    // For autocompletion
    void dfs(TrieNode *node, string prefix){
        if (node == NULL)
            return;

        if (node->isEnd)
            cout << prefix << endl;

        for (int i = 0; i < 26; i++){
            if (node->children[i] != NULL){
                char nextChar = 'a' + i;
                dfs(node->children[i], prefix + nextChar);
            }
        }
    }

public:
    Trie(){
        root = new TrieNode();
    }

    void insertWord(string word){
        TrieNode *current = root;

        for (int i = 0; i < word.length(); i++){
            char c = word[i];

            if (c < 'a' || c > 'z') // Ignore Invalid
                continue;

            int index = c - 'a';

            if (current->children[index] == NULL)
                current->children[index] = new TrieNode();

            current = current->children[index];
        }

        current->isEnd = true;
    }

    bool searchWord(string word){
        TrieNode *current = root;

        for (int i = 0; i < word.length(); i++){
            int index = word[i] - 'a';

            if (index < 0 || index > 25)
                return false;

            if (current->children[index] == NULL)
                return false;

            current = current->children[index];
        }

        return current->isEnd;
    }

    void autocomplete(string prefix){
        TrieNode *current = root;

        for (int i = 0; i < prefix.length(); i++){
            int index = prefix[i] - 'a';

            if (current->children[index] == NULL)
            {
                cout << "No suggestions found.\n";
                return;
            }

            current = current->children[index];
        }

        dfs(current, prefix);
    }
};

// Hashing
class HashNode{
public:
    string word;
    HashNode *next;

    HashNode(string w)
    {
        word = w;
        next = NULL;
    }
};

class HashTable{
private:
    HashNode **table;
    int size;

    int hash(string word){
        long long h = 0;

        for (int i = 0; i < word.length(); i++)
            h = (h * 31 + word[i]) % size;

        return (int)h;
    }

public:
    HashTable(int s = 50000){
        size = s;
        table = new HashNode *[size];

        for (int i = 0; i < size; i++)
            table[i] = NULL;
    }

    void insert(string word){
        int index = hash(word);
        HashNode *node = new HashNode(word);

        node->next = table[index];
        table[index] = node;
    }

    bool exists(string word){
        int index = hash(word);
        HashNode *current = table[index];

        while (current != NULL){
            if (current->word == word)
                return true;

            current = current->next;
        }

        return false;
    }
};


// Loading dictionay using filing
class DictionaryLoader{
public:
    static void loadDictionary(string filename, Trie &trie, HashTable &ht){
        ifstream fin(filename);

        if (!fin){
            cout << "Cannot open dictionary file..." << endl;
            return;
        }

        string word;

        while (fin >> word){
            for (int i = 0; i < word.length(); i++)
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

int main(){
    Trie trie;
    HashTable ht(50000);

    DictionaryLoader::loadDictionary("dictionary.txt", trie, ht);

    cout << "Testing Trie Search:" << endl;
    if (trie.searchWord("hello"))
        cout << "FOUND\n";
    else
        cout << "NOT FOUND\n";

    cout << "Autocomplete for 'he':" << endl;
    trie.autocomplete("he");

    cout << "Check HashTable for 'apple': ";
    if (ht.exists("apple"))
        cout << "YES" << endl;
    else
        cout << "NO" << endl;

    return 0;
}
