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

// Class that manages all the file handling tasks...
class FileManager
{
public:
    static string ensureTxt(string name)
    {
        int n = name.length();
        if (n >= 4)
        {
            char a = name[n - 4];
            char b = name[n - 3];
            char c = name[n - 2];
            char d = name[n - 1];
            if (a == '.' &&
                (b == 't' || b == 'T') &&
                (c == 'x' || c == 'X') &&
                (d == 't' || d == 'T'))
            {
                return name;
            }
        }
        return name + ".txt";
    }

    static void saveToFile(string filename, string text)
    {
        filename = ensureTxt(filename);
        ofstream fout(filename);
        if (!fout)
        {
            cout << "Error saving file...." << endl;
            return;
        }
        fout << text;
        fout.close();
        cout << "Saved as: " << filename << endl;
    }

    static string loadFile(string filename)
    {
        filename = ensureTxt(filename);
        ifstream fin(filename);
        if (!fin)
        {
            cout << "File not found..." << endl;
            return "";
        }
        string line, full = "";
        while (getline(fin, line))
            full += line + "\n";
        fin.close();
        cout << "Loaded: " << filename << endl;
        return full;
    }
};

// Editor class.. where everything happens..
class Editor
{
private:
    TextBuffer buffer;
    Stack undoStack, redoStack;
    Trie *trie;
    HashTable *ht;
    SpellChecker checker;

    string trim(string s)
    {
        int n = s.length();
        int i = 0;
        while (i < n && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n'))
            i++;
        int j = n - 1;
        while (j >= 0 && (s[j] == ' ' || s[j] == '\t' || s[j] == '\r' || s[j] == '\n'))
            j--;
        if (i > j)
            return "";
        return s.substr(i, j - i + 1);
    }

    string getLastWord()
    {
        string t = buffer.getText();
        int i = t.length() - 1;

        while (i >= 0 && (t[i] == ' ' || t[i] == '\n' || t[i] == '\t'))
            i--;

        if (i < 0)
            return "";

        int end = i;

        while (i >= 0 && t[i] != ' ' && t[i] != '\n' && t[i] != '\t')
            i--;

        int start = i + 1;
        if (start > end)
            return "";

        return t.substr(start, end - start + 1);
    }

    void replaceLastWord(string newWord)
    {
        string t = buffer.getText();
        int i = t.length() - 1;

        // Remove newline caused by enter
        if (i >= 0 && t[i] == '\n')
        {
            t.erase(i, 1);
            i--;
        }

        // Skip spaces
        while (i >= 0 && (t[i] == ' ' || t[i] == '\t'))
            i--;

        if (i < 0)
            return;

        int end = i;
        while (i >= 0 && t[i] != ' ' && t[i] != '\t')
            i--;

        int start = i + 1;

        string before = t.substr(0, start);
        string after = t.substr(end + 1);

        // Replace word and  add space to continue from the same line
        string updated = before + newWord + " " + after;

        buffer.setText(updated);

        undoStack.clear();
        redoStack.clear();
    }

public:
    Editor(Trie *t, HashTable *h)
    {
        trie = t;
        ht = h;
        checker.setData(t, h);
    }

    void typeChar(char ch)
    {
        int pos = buffer.length();
        buffer.insertAt(pos, ch);
        undoStack.push('i', ch, pos);
        redoStack.clear();
    }

    void spellCheckLineWords(string line)
    {
        string word = "";
        for (int i = 0; i < line.length(); i++)
        {
            char c = line[i];
            if (c == ' ' || c == '\t')
            {
                checker.checkWord(word);
                word = "";
            }
            else
            {
                word += c;
            }
        }
        checker.checkWord(word);
        cout << endl;
    }

    void autocompleteOnLastWord()
    {
        string prefix = getLastWord();
        if (prefix == "")
        {
            cout << "No word to autocomplete.." << endl;
            return;
        }

        for (int i = 0; i < prefix.length(); i++)
            if (prefix[i] >= 'A' && prefix[i] <= 'Z')
                prefix[i] = prefix[i] - 'A' + 'a';

        string suggestions[10];
        int count = trie->getSuggestions(prefix, suggestions, 10);

        if (count == 0)
        {
            cout << "No suggestions.\n";
            return;
        }

        cout << "Suggestions for \"" << prefix << "\":\n";
        for (int i = 0; i < count; i++)
            cout << (i + 1) << ". " << suggestions[i] << endl;

        cout << "Choice (0 cancel): ";

        int choice;
        cin >> choice;
        cin.ignore(1000, '\n');

        if (choice > 0 && choice <= count)
        {
            replaceLastWord(suggestions[choice - 1]);
            cout << "Replaced last word with: " << suggestions[choice - 1] << endl;
        }
        else
        {
            cout << "Autocomplete cancelled..." << endl;
        }
    }

    void saveFile()
    {
        cout << "Enter filename: ";
        string name;
        getline(cin, name);
        name = trim(name);
        if (name == "")
        {
            cout << "No filename..." << endl;
            return;
        }
        FileManager::saveToFile(name, buffer.getText());
    }

    void openFile()
    {
        cout << "Enter filename to open: ";
        string name;
        getline(cin, name);
        name = trim(name);

        if (name == "")
        {
            cout << "No filename..." << endl;
            return;
        }
        string content = FileManager::loadFile(name);
        if (content != "")
        {
            buffer.setText(content);
            undoStack.clear();
            redoStack.clear();
            buffer.show();
        }
    }

    void undo()
    {
        char t, c;
        int p;
        if (!undoStack.pop(t, c, p))
        {
            cout << "Nothing to undo..." << endl;
            return;
        }

        if (t == 'i')
            buffer.deleteAt(p);
        else if (t == 'd')
            buffer.insertAt(p, c);

        redoStack.push(t, c, p);
        buffer.show();
    }

    void redo()
    {
        char t, c;
        int p;
        if (!redoStack.pop(t, c, p))
        {
            cout << "Nothing to redo..." << endl;
            return;
        }

        if (t == 'i')
            buffer.insertAt(p, c);
        else if (t == 'd')
            buffer.deleteAt(p);

        undoStack.push(t, c, p);
        buffer.show();
    }

    void run()
    {
        cout << "SMART TEXT EDITOR" << endl;
        cout << "Commands: " << endl;
        cout << "  /save  -> save file" << endl;
        cout << "  /open  -> open file" << endl;
        cout << "  /undo  -> undo last edit" << endl;
        cout << "  /redo  -> redo last edit" << endl;
        cout << "  /ac    -> autocomplete last word" << endl;
        cout << "  /exit  -> exit" << endl << endl;

        while (true)
        {
            cout << "> ";
            string line;
            if (!getline(cin, line))
                break;

            string cmd = trim(line);

            // Exit
            if (cmd == "/exit")
            {
                cout << "Save before exit? (y/n): ";
                char ch;
                cin >> ch;
                cin.ignore(1000, '\n');
                if (ch == 'y' || ch == 'Y')
                {
                    saveFile();
                }
                break;
            }

            // All Commands..
            if (cmd == "/save")
            {
                saveFile();
                continue;
            }

            if (cmd == "/open")
            {
                openFile();
                continue;
            }

            if (cmd == "/undo")
            {
                undo();
                continue;
            }

            if (cmd == "/redo")
            {
                redo();
                continue;
            }

            // /ac
            if (cmd == "/ac")
            {
                autocompleteOnLastWord();
                buffer.show();
                continue;
            }

            // inline '/ac"
            int pos = line.rfind("/ac");
            bool inlineAc = false;

            if (pos != -1)
            {
                int j = pos + 3;
                while (j < line.length() && (line[j] == ' ' || line[j] == '\t'))
                    j++;
                // "/ac"  at end
                if (j == line.length())
                {
                    inlineAc = true;
                }
            }

            if (inlineAc)
            {
                // Text before "/ac"
                string part = line.substr(0, pos);
                // right-trim spaces
                int end = part.length() - 1;
                while (end >= 0 && (part[end] == ' ' || part[end] == '\t'))
                    end--;
                if (end >= 0)
                    part = part.substr(0, end + 1);
                else
                    part = "";

                // Add that text to buffer
                for (int i = 0; i < part.length(); i++)
                {
                    char ch2 = part[i];
                    typeChar(ch2);
                }
                typeChar('\n');

                // Spell checking..
                cout << "Spell check: ";
                spellCheckLineWords(part);

                buffer.show();

                // autocomplete on last word in buffer
                autocompleteOnLastWord();
                buffer.show();
                continue;
            }

            // Simple textline
            for (int i = 0; i < line.length(); i++)
            {
                char ch2 = line[i];
                typeChar(ch2);
            }
            typeChar('\n');

            cout << "Spell check: ";
            spellCheckLineWords(line);

            buffer.show();
        }

        cout << "\nFinal document:\n";
        buffer.show();
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
