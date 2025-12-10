#include <iostream>
#include <fstream>
#include <string>
using namespace std;

// TrieNode and Trie DataSTrucutre forautocompletion..
class TrieNode
{
public:
    TrieNode *children[26];
    bool isWordEnd;

    TrieNode() : isWordEnd(false)
    {
        for(int i = 0; i < 26; ++i)
        {
            children[i] = NULL;
        }
    }
};

class Trie
{
private:
    TrieNode *root;

    // DFS[Depth First Search] forautocomplete....
    void collect(TrieNode *node, string word, string out[], int &cnt, int limit)
    {
        if(!node || cnt >= limit)
            return;

        if(node->isWordEnd)
        {
            out[cnt++] = word;
            if(cnt >= limit)
                return;
        }

        for(int i = 0; i < 26 && cnt < limit; ++i)
        {
            if(node->children[i])
            {
                char ch = 'a' + i;
                collect(node->children[i], word + ch, out, cnt, limit);
            }
        }
    }

    // Edit distance..
    void searchByEditDistance(TrieNode *node, char ch, string built, const string &target, int *prevRow, int len, int cost, string out[], int &cnt, int limit)
    {
        if(!node || cnt >= limit)
            return;

        int *row = new int[len + 1];
        row[0] = prevRow[0] + 1;
        int minVal = row[0];

        for(int j = 1; j <= len; ++j)
        {
            int ins = row[j - 1] + 1;
            int del = prevRow[j] + 1;
            int rep = prevRow[j - 1] + (target[j - 1] != ch);
            row[j] = min(ins, min(del, rep));
            minVal = min(minVal, row[j]);
        }

        if(node->isWordEnd && row[len] <= cost && cnt < limit)
        {
            out[cnt++] = built;
        }

        if(minVal <= cost && cnt < limit)
        {
            for(int i = 0; i < 26 && cnt < limit; ++i)
            {
                if(node->children[i])
                {
                    char next = 'a' + i;
                    searchByEditDistance(node->children[i], next, built + next, target, row, len, cost, out, cnt, limit);
                }
            }
        }

        delete[] row;
    }

public:
    Trie()
    {
        root = new TrieNode();
    }

    void insert(const string &word)
    {
        TrieNode *cur = root;
        for(char c : word)
        {
            if(c < 'a' || c > 'z')
                continue;
            int idx = c - 'a';
            if(!cur->children[idx])
                cur->children[idx] = new TrieNode();
            cur = cur->children[idx];
        }
        cur->isWordEnd = true;
    }

    bool contains(const string &word)
    {
        TrieNode *cur = root;
        for(char c : word)
        {
            int idx = c - 'a';
            if(idx < 0 || idx >= 26 || !cur->children[idx])
                return false;
            cur = cur->children[idx];
        }
        return cur->isWordEnd;
    }

    int suggest(const string &prefix, string out[], int max = 10)
    {
        TrieNode *cur = root;
        for(char c : prefix)
        {
            int idx = c - 'a';
            if(idx < 0 || idx >= 26 || !cur->children[idx])
                return 0;
            cur = cur->children[idx];
        }
        int cnt = 0;
        collect(cur, prefix, out, cnt, max);
        return cnt;
    }

    int approxSuggestion(const string &word, string out[], int max = 10, int cost = 2)
    {
        int len = word.length();
        if(len == 0)
            return 0;

        int *row = new int[len + 1];
        for(int j = 0; j <= len; ++j)
            row[j] = j;

        int cnt = 0;
        for(int i = 0; i < 26 && cnt < max; ++i)
        {
            TrieNode *child = root->children[i];
            if(child)
            {
                char ch = 'a' + i;
                searchByEditDistance(child, ch, string(1, ch), word, row, len, cost, out, cnt, max);
            }
        }

        delete[] row;
        return cnt;
    }
};

// Maping words forto spell checking...
class HashNode
{
public:
    string word;
    HashNode *next;

    HashNode(string w)
    {
        word = w;
        next = NULL;
    }
};

class HashTable
{
private:
    HashNode **table;
    int size;

    int hash(string word)
    {
        long long h = 0;
        for(int i = 0; i < word.length(); i++)
            h = (h * 31 + word[i]) % size;
        return (int)h;
    }

public:
    HashTable(int s = 50000)
    {
        size = s;
        table = new HashNode *[size];
        for(int i = 0; i < size; i++)
            table[i] = NULL;
    }

    void insert(string word)
    {
        int index = hash(word);
        HashNode *node = new HashNode(word);
        node->next = table[index];
        table[index] = node;
    }

    bool exists(string word)
    {
        int index = hash(word);
        HashNode *cur = table[index];
        while(cur != NULL)
        {
            if(cur->word == word)
                return true;
            cur = cur->next;
        }
        return false;
    }
};

// Loading the Dictionay and inserting in the words in trie and hash Table..
class DictionaryLoader
{
public:
    static void loadDictionary(string filename, Trie &trie, HashTable &dictHash)
    {
        ifstream fin(filename);
        if(!fin)
        {
            cout << "Dictionary file missing..." << endl;
            return;
        }

        string word;
        while(fin >> word)
        {
            for(int i = 0; i < word.length(); i++)
                if(word[i] >= 'A' && word[i] <= 'Z')
                    word[i] = word[i] - 'A' + 'a';

            trie.insert(word);
            dictHash.insert(word);
        }

        cout << "Dictionary Loaded..." << endl;
    }
};

// Handles the typing is being done on the CLI...
class TextBuffer
{
private:
    string text;

public:
    void insertAt(int pos, char ch)
    {
        if(pos < 0)
            pos = 0;
        if(pos > text.length())
            pos = text.length();
        text.insert(pos, 1, ch);
    }

    void deleteAt(int pos)
    {
        if(pos < 0 || pos >= text.length())
            return;
        text.erase(pos, 1);
    }

    int length()
    {
        return text.length();
    }

    char getCharAt(int pos)
    {
        if(pos < 0 || pos >= text.length())
            return '\0';
        return text[pos];
    }

    string getText()
    {
        return text;
    }

    void setText(string t)
    {
        text = t;
    }

    void show()
    {
        cout << endl << "----- TEXT -----" << endl;
        cout << text;
        cout << endl << "----------------" << endl;
    }
};

/// Stack and action.. Action forundo and redo the task.. stack to store the character forundeo and redo...
class Action
{
public:
    char type; // 'i' insert, 'd' delete
    char ch;
    int pos;
    Action *next;

    Action(char t, char c, int p)
    {
        type = t;
        ch = c;
        pos = p;
        next = NULL;
    }
};

class Stack
{
public:
    Action *top;

    Stack()
    {
        top = NULL;
    }

    void push(char t, char c, int p)
    {
        Action *node = new Action(t, c, p);
        node->next = top;
        top = node;
    }

    bool pop(char &t, char &c, int &p)
    {
        if(top == NULL)
            return false;

        Action *temp = top;
        t = temp->type;
        c = temp->ch;
        p = temp->pos;
        top = top->next;
        delete temp;
        return true;
    }

    void clear()
    {
        char t, c;
        int p;
        while(pop(t, c, p))
        {
            // Loopp Until empty...
        }
    }
};

// Class that checks the spelling bt hashing...
class SpellChecker
{
private:
    Trie *trie;
    HashTable *dictHash;

public:
    SpellChecker()
    {
        trie = NULL;
        dictHash = NULL;
    }

    SpellChecker(Trie *t, HashTable *h)
    {
        trie = t;
        dictHash = h;
    }

    void setData(Trie *t, HashTable *h)
    {
        trie = t;
        dictHash = h;
    }

    void check(string word)
    {
        int n = word.length();
        if(n == 0)
            return;

        // lowercase
        for(int i = 0; i < n; i++)
            if(word[i] >= 'A' && word[i] <= 'Z')
                word[i] = word[i] - 'A' + 'a';

        if(dictHash->exists(word))
        {
            cout << "[OK:" << word << "] ";
        }
        else
        {
            cout << "[WRONG:" << word << "] ";
        }
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
            string updated = before + newWord + " " + after;

        buffer.setText(updated);

        undoStack.clear();
        redoStack.clear();
    }

public:
    Editor(Trie *t, HashTable *h)
    {
        trie = t;
        dictHash = h;
        checker.setData(t, h);
    }

    void typeChar(char ch)
    {
        int pos = buffer.length();
        buffer.insertAt(pos, ch);
        undoStack.push('i', ch, pos);
        redoStack.clear();
    }

    void spellCheckLine(string line)
    {
        string word = "";
        for(int i = 0; i < line.length(); i++)
        {
            char c = line[i];
            if(c == ' ' || c == '\t')
            {
                checker.check(word);
                word = "";
            }
            else
            {
                word += c;
            }
        }
        checker.check(word);
        cout << endl;
    }

    void autocompleteLastWord()
    {
        string prefix = getLastWord();
        if(prefix == "")
        {
            cout << "No word to autocomplete.." << endl;
            return;
        }

        for(int i = 0; i < prefix.length(); i++)
            if(prefix[i] >= 'A' && prefix[i] <= 'Z')
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

        if(name == "")
        {
            cout << "No filename..." << endl;
            return;
        }
        string content = FileManager::loadFile(name);
        if(content != "")
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

        bool removedAny = false;

        while (true)
        {
            if (!undoStack.pop(t, c, p))
            {
                if (!removedAny)
                    cout << "Nothing to undo..." << endl;
                break;
            }

            // reverse the action
            if (t == 'i')
                buffer.deleteAt(p);
            else if (t == 'd')
                buffer.insertAt(p, c);

            // push into redo stack
            redoStack.push(t, c, p);

            bool isSpace = (c == ' ' || c == '\n' || c == '\t');

            if (!isSpace)
            {
                removedAny = true; // at least part of a word removed
            }
            else
            {
                if (removedAny)
                    break;
            }
        }

        buffer.show();
    }

    void redo()
    {
        char t, c;
        int p;

        bool restoredAny = false;

        while (true)
        {
            if (!redoStack.pop(t, c, p))
            {
                if (!restoredAny)
                    cout << "Nothing to redo..." << endl;
                break;
            }

            // redo the original action
            if (t == 'i')
                buffer.insertAt(p, c);
            else if (t == 'd')
                buffer.deleteAt(p);

            // push back into undo stack
            undoStack.push(t, c, p);

            bool isSpace = (c == ' ' || c == '\n' || c == '\t');

            if (!isSpace)
            {
                restoredAny = true; // at least part of a word restored
            }
            else
            {
                if (restoredAny)
                    break;
            }
        }

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
    HashTable dictionary(50000);

    DictionaryLoader::loadDictionary("dictionary.txt", trie, dictionary);

    Editor editor(&trie, &dictionary);
    editor.run();

    return 0;
}
