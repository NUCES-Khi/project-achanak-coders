#include <iostream>
#include <fstream>
#include <string>
using namespace std;

// TrieNode and Trie DataSTrucutre for autocompletion..
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

    // DFS[Depth First Search] for autocomplete....
    void dfsCollect(TrieNode *node, string prefix, string suggestions[], int &count, int maxSug)
    {
        if (node == NULL || count >= maxSug)
            return;

        if (node->isEnd)
        {
            suggestions[count] = prefix;
            count++;
            if (count >= maxSug)
                return;
        }

        for (int i = 0; i < 26 && count < maxSug; i++)
        {
            if (node->children[i] != NULL)
            {
                char nextChar = 'a' + i;
                dfsCollect(node->children[i], prefix + nextChar, suggestions, count, maxSug);
            }
        }
    }

    // Edit distance..
    void dfsEditDistance(TrieNode *node, char currentChar, string prefix, string target, int *prevRow, int targetLen, int maxCost, string suggestions[], int &count, int maxSug)
    {
        if (node == NULL || count >= maxSug)
            return;

        int *currentRow = new int[targetLen + 1];

        currentRow[0] = prevRow[0] + 1;
        int minInRow = currentRow[0];

        for (int j = 1; j <= targetLen; j++)
        {
            int insertCost = currentRow[j - 1] + 1;
            int deleteCost = prevRow[j] + 1;
            int replaceCost = prevRow[j - 1] + (target[j - 1] == currentChar ? 0 : 1);

            int val = insertCost;
            if (deleteCost < val)
                val = deleteCost;
            if (replaceCost < val)
                val = replaceCost;

            currentRow[j] = val;
            if (val < minInRow)
                minInRow = val;
        }

        // store suggestion if it is a word and within allowed cost
        if (node->isEnd && currentRow[targetLen] <= maxCost && count < maxSug)
        {
            suggestions[count] = prefix;
            count++;
        }

        if (minInRow <= maxCost && count < maxSug)
        {
            for (int i = 0; i < 26 && count < maxSug; i++)
            {
                if (node->children[i] != NULL)
                {
                    char nextChar = 'a' + i;
                    dfsEditDistance(node->children[i], nextChar, prefix + nextChar, target, currentRow, targetLen, maxCost, suggestions, count, maxSug);
                }
            }
        }

        delete[] currentRow;
    }

public:
    Trie()
    {
        root = new TrieNode();
    }

    void insertWord(string word)
    {
        TrieNode *current = root;

        for (int i = 0; i < word.length(); i++)
        {
            char c = word[i];
            if (c < 'a' || c > 'z')
                continue;

            int index = c - 'a';
            if (current->children[index] == NULL)
                current->children[index] = new TrieNode();

            current = current->children[index];
        }
        current->isEnd = true;
    }

    bool searchWord(string word)
    {
        TrieNode *current = root;

        for (int i = 0; i < word.length(); i++)
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

    // Autocompletion
    int getSuggestions(string prefix, string suggestions[], int maxSug)
    {
        TrieNode *current = root;

        for (int i = 0; i < prefix.length(); i++)
        {
            int index = prefix[i] - 'a';
            if (index < 0 || index > 25 || current->children[index] == NULL)
            {
                return 0;
            }
            current = current->children[index];
        }

        int count = 0;
        dfsCollect(current, prefix, suggestions, count, maxSug);
        return count;
    }

    // Suggestion using edit distance...
    int getApproximateSuggestions(string target, string suggestions[], int maxSug, int maxCost)
    {
        int targetLen = target.length();
        if (targetLen == 0)
            return 0;

        // row for distance from empty prefix
        int *currentRow = new int[targetLen + 1];
        for (int j = 0; j <= targetLen; j++)
            currentRow[j] = j;

        int count = 0;

        for (int i = 0; i < 26 && count < maxSug; i++)
        {
            TrieNode *child = root->children[i];
            if (child != NULL)
            {
                char c = 'a' + i;
                string prefix = "";
                prefix += c;

                dfsEditDistance(child, c, prefix, target, currentRow, targetLen, maxCost, suggestions, count, maxSug);
            }
        }

        delete[] currentRow;
        return count;
    }
};

// Maping words for to spell checking...
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
        for (int i = 0; i < word.length(); i++)
            h = (h * 31 + word[i]) % size;
        return (int)h;
    }

public:
    HashTable(int s = 50000)
    {
        size = s;
        table = new HashNode *[size];
        for (int i = 0; i < size; i++)
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
        while (cur != NULL)
        {
            if (cur->word == word)
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
    static void loadDictionary(string filename, Trie &trie, HashTable &ht)
    {
        ifstream fin(filename);
        if (!fin)
        {
            cout << "Dictionary file missing..." << endl;
            return;
        }

        string word;
        while (fin >> word)
        {
            for (int i = 0; i < word.length(); i++)
                if (word[i] >= 'A' && word[i] <= 'Z')
                    word[i] = word[i] - 'A' + 'a';

            trie.insertWord(word);
            ht.insert(word);
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
        if (pos < 0)
            pos = 0;
        if (pos > text.length())
            pos = text.length();
        text.insert(pos, 1, ch);
    }

    void deleteAt(int pos)
    {
        if (pos < 0 || pos >= text.length())
            return;
        text.erase(pos, 1);
    }

    int length()
    {
        return text.length();
    }

    char getCharAt(int pos)
    {
        if (pos < 0 || pos >= text.length())
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
        cout << endl
             << "----- TEXT -----" << endl;
        cout << text;
        cout << endl
             << "----------------" << endl;
    }
};

/// Stack and action.. Action for undo and redo the task.. stack to store the character for undeo and redo...
class Action
{
public:
    char type; // 'i' for insert, 'd' for delete
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
        if (top == NULL)
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
        while (pop(t, c, p))
        {
        }
    }
};

// Class that checks the spelling bt hashing...
class SpellChecker
{
private:
    Trie *trie;
    HashTable *ht;

public:
    SpellChecker()
    {
        trie = NULL;
        ht = NULL;
    }

    SpellChecker(Trie *t, HashTable *h)
    {
        trie = t;
        ht = h;
    }

    void setData(Trie *t, HashTable *h)
    {
        trie = t;
        ht = h;
    }

    void checkWord(string word)
    {
        int n = word.length();
        if (n == 0)
            return;

        // lowercase
        for (int i = 0; i < n; i++)
            if (word[i] >= 'A' && word[i] <= 'Z')
                word[i] = word[i] - 'A' + 'a';

        if (ht->exists(word))
        {
            cout << "[OK:" << word << "] ";
        }
        else
        {
            cout << "[WRONG:" << word << "] ";
        }
    }
};
class FileManager{
    public:
        static string ensureTxt(string name)
        {
            int n = name.length();
            if(n >= 4)
            {
                char a = name[n - 4];
                char b = name[n - 3];
                char c = name[n - 2];
                char d = name[n - 1];
                if(a == '.' &&
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
            if(!fout)
            {
                cout << "Error saving file...." << endl;
                return;
            }
            fout << text;
            fout.close();
            cout << "Saved as: " << filename << endl;
        }

        static string loadFile(string filename){
            filename = ensureTxt(filename);
            ifstream fin(filename);
            if(!fin)
            {
                cout << "File not found..." << endl;
                return "";
            }
            string line, full = "";
            while(getline(fin, line))
                full += line + "\n";
            fin.close();
            cout << "Loaded: " << filename << endl;
            return full;
        }
    };
    class Editor{
    private:
        TextBuffer buffer;
        Stack undoStack, redoStack;
        Trie *trie;
        HashTable *dictHash;
        SpellChecker checker;

        string trim(string s){
            int end = s.length();
            int start = 0;
            while(start < end && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '\n'))
                start++;
            int j = end - 1;
            while(j >= 0 && (s[j] == ' ' || s[j] == '\t' || s[j] == '\r' || s[j] == '\n'))
                j--;
            if(start > j)
                return "";
            return s.substr(start, j - start + 1);
        }
        string getLastWord(){
            string t = buffer.getText();
            int i = t.length() - 1;

            while(i >= 0 && (t[i] == ' ' || t[i] == '\n' || t[i] == '\t'))
                i--;

            if(i < 0)
                return "";

            int end = i;
            while(i >= 0 && t[i] != ' ' && t[i] != '\n' && t[i] != '\t')
                i--;

            int start = i + 1;
            if(start > end)
                return "";
            return t.substr(start, end - start + 1);
        }

        void replaceLastWord(string newWord){
            string t = buffer.getText();
            int i = t.length() - 1;
            if(i >= 0 && t[i] == '\n')
            {
                t.erase(i, 1);
                i--;
            }
            while(i >= 0 && (t[i] == ' ' || t[i] == '\t'))
                i--;

            if(i < 0)
                return;

            int end = i;
            while(i >= 0 && t[i] != ' ' && t[i] != '\t')
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
            int count = trie->suggest(prefix, suggestions, 10);
            if(count == 0){
                cout << "No suggestions...." << endl;
                return;
            }
            cout << "Suggestions for\"" << prefix << "\": "<< endl ;
            for(int i = 0; i < count; i++)
                cout << (i + 1) << ". " << suggestions[i] << endl;

            cout << "Choice (0 cancel): ";
            int choice;
            cin >> choice;
            cin.ignore(1000, '\n');

            if(choice > 0 && choice <= count){
                replaceLastWord(suggestions[choice - 1]);
                cout << "Replaced last word with: " << suggestions[choice - 1] << endl;
            }
            else{
                cout << "Autocomplete cancelled..." << endl;
            }
        }
        void saveFile(){
            cout << "Enter filename: ";
            string name;
            getline(cin, name);
            name = trim(name);
            if(name == ""){
                cout << "No filename..." << endl;
                return;
            }
            FileManager::saveToFile(name, buffer.getText());
        }
        void openFile(){
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

            while(true){
                cout << "> ";
                string line;
                if(!getline(cin, line))
                    break;

                string cmd = trim(line);
                // Exit
                if(cmd == "/exit"){
                    cout << "Save before exit? (y/n): ";
                    char ch;
                    cin >> ch;
                    cin.ignore(1000, '\n');
                    if(ch == 'y' || ch == 'Y')
                    {
                        saveFile();
                    }
                    break;
                }
                if(cmd == "/save"){
                    saveFile();
                    continue;
                }
                if(cmd == "/open"){
                    openFile();
                    continue;
                }
                if(cmd == "/undo"){
                    undo();
                    continue;
                }
                if(cmd == "/redo"){
                    redo();
                    continue;
                }
                if(cmd == "/ac"){
                    autocompleteLastWord();
                    buffer.show();
                    continue;
                }
                int pos = line.rfind("/ac");
                bool inlineAc = false;
                if(pos != -1){
                    int j = pos + 3;
                    while(j < line.length() && (line[j] == ' ' || line[j] == '\t'))
                        j++;
                    if(j == line.length())
                    {
                        inlineAc = true;
                    }
                }
                if(inlineAc){
                    string part = line.substr(0, pos);
                    int end = part.length() - 1;
                    while(end >= 0 && (part[end] == ' ' || part[end] == '\t'))
                        end--;
                    if(end >= 0)
                        part = part.substr(0, end + 1);
                    else
                        part = "";

                    for(int i = 0; i < part.length(); i++)
                    {
                        char ch2 = part[i];
                        typeChar(ch2);
                    }
                    typeChar('\n');
                    cout << "Spell check: ";
                    spellCheckLine(part);

                    buffer.show();
                    autocompleteLastWord();
                    buffer.show();
                    continue;
                }

                for(int i = 0; i < line.length(); i++){
                    char ch2 = line[i];
                    typeChar(ch2);
                }
                typeChar('\n');

                cout << "Spell check: ";
                spellCheckLine(line);
                buffer.show();
            }
            cout << endl;
            cout << "Final document: "<< endl;
            buffer.show();
        }
};

int main(){
    Trie trie;
    HashTable ht(50000);

    DictionaryLoader::loadDictionary("dictionary.txt", trie, ht);

    Editor editor(&trie, &ht);
    editor.run();

    return 0;
}
