# Smart Text Editor (Trie + Spell Check + Autocomplete)

|id|Name|
|-|-|
|K24-2513|Ansh Kumar|
|K24-2546|Esfan Merchant|

## Introduction
The **Smart Text Editor** is an intelligent text editing application that provides real-time spell checking, autocomplete suggestions, and basic text editing features such as undo/redo and file operations. The project aims to demonstrate the application of key data structures—such as Tries, Hash Tables, and Stacks—within a real-world scenario, showcasing how DSA principles can power efficient text processing systems.

## Description
- Implement a **text buffer** to handle dynamic text insertion, deletion, and modification.  
- Use a **Trie** data structure to store a large dictionary for efficient word lookup and prefix-based autocomplete.  
- Implement a **spell checker** using Trie traversal and approximate string matching (Levenshtein distance).  
- Design **undo/redo** functionality using Stack data structures to record user actions.  
- Integrate **hashing** for quick word existence checks during spell correction.  
- Add **file I/O** for loading dictionaries and saving edited text files.  
- Optimize performance through recursion and memory-safe dynamic allocations.

## Data Structures that will be used

| Data Structure | Purpose | Usage Details |
|----------------|----------|----------------|
| **Trie (Prefix Tree)** | Autocomplete and Spell Check | Each node represents a character. Words are inserted into the Trie for O(L) prefix search. Used to generate suggestions as the user types. |
| **Hash Table** | Word Existence and Spell Checking | Stores words from dictionary for O(1) lookup to validate spellings quickly. |
| **Stack** | Undo/Redo Functionality | Two stacks maintain user actions. One for undo history, the other for redo operations. |
| **Dynamic Array / Linked List** | Text Buffer Management | Stores and modifies text lines dynamically during typing and editing. |
| **Recursion** | Trie Traversal & Spell Suggestions | Recursive DFS through Trie for generating autocomplete and correction options. |
| **File I/O** | Data Persistence | To load dictionary files and save edited text to files on disk. |
