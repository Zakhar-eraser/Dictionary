#include <iostream>
#include <fstream>
#include <vector>
#ifdef WINDOWS
    #define CLEAR "cls"
#else
    #define CLEAR "clear"
#endif

int CompareWords(std::string& word1, std::string& word2)
{
    return word1.compare(word2);;
}

int CompareSubstring(std::string& substring, std::string& word)
{
    std::string wordSubstring = word.substr(0, substring.length());
    return CompareWords(substring, wordSubstring);
}

class BinaryTree
{
    private:
        BinaryTree* left = nullptr;
        BinaryTree* right = nullptr;
        std::string word;
        std::string translation;
    public:
        BinaryTree(std::string word, std::string translation)
        {
            this->word = word;
            this->translation = translation;
        }
        std::string GetWord()
        {
            return word;
        }
        std::string GetTranslation()
        {
            return translation;
        }
        void SetWord(std::string& word)
        {
            this->word = word;
        }
        void SetTranslation(std::string& translation)
        {
            this->translation = translation;
        }
        BinaryTree** BranchPass(std::string& word, BinaryTree* current, BinaryTree** pCurrent,
        int (* Filter) (std::string&, std::string&),
        BinaryTree** (* MatchingActivity)(BinaryTree**),
        BinaryTree** (* NullTreeActivity)(BinaryTree**))
        {
            BinaryTree** next;
            int check = Filter(word, current->word);
            if(check)
            {
                if(check > 0)
                {
                    next = &(current->right);
                }
                else next = &(current->left);
            }
            else return (* MatchingActivity)(pCurrent);
            if(*next == nullptr)
            {
                return (* NullTreeActivity)(next);
            }
            else
            {
                return BranchPass(word, *next, next, Filter, MatchingActivity, NullTreeActivity);
            }
        }
        void TreePass(BinaryTree* tree, std::string& parameter,
        void (* ActionWithWords)(BinaryTree*, std::ostream&),
        bool (* Filter)(BinaryTree*, std::string&), std::ostream& stream)
        {
            if(tree != nullptr)
            {
                TreePass(tree->left, parameter, ActionWithWords, Filter, stream);
                BinaryTree* rightTree = tree->right;
                if((* Filter)(tree, parameter))(* ActionWithWords)(tree, stream);
                TreePass(rightTree, parameter, ActionWithWords, Filter, stream);
            }
        }
        BinaryTree** FindMinWord(BinaryTree* tree, BinaryTree** pointerTree)
        {
            if(tree->left == nullptr)
            {
                return pointerTree;
            }
            else return FindMinWord(tree->left, &(tree->left));
        }
        void RemoveWord(BinaryTree** removingWord)
        {
            BinaryTree* wordBranch = *removingWord;
            BinaryTree* wordBranchLeft = wordBranch->left;
            BinaryTree* wordBranchRight = wordBranch->right;
            if(wordBranchRight == nullptr)
            {
                if(wordBranchLeft == nullptr)
                {
                    *removingWord = nullptr;
                }
                else
                {
                    *removingWord = wordBranchLeft;
                }
            }
            else
            {
                BinaryTree** nextMinWord = FindMinWord(wordBranchRight, nullptr);
                if(nextMinWord == nullptr)
                {
                    *removingWord = wordBranchRight;
                    wordBranchRight->left = wordBranchLeft;
                }
                else
                {
                    *removingWord = *nextMinWord;
                    *nextMinWord = (*removingWord)->right;
                    (*removingWord)->right = wordBranchRight;
                    (*removingWord)->left = wordBranchLeft;
                    return;
                }
            }
        }
};

class Dictionary
{
    private:
        std::vector<BinaryTree*> history;
        std::string pathToInputFile;
        std::string name;
        bool wasChanged = false;
        bool (* NoFilter)(BinaryTree*, std::string&) = [](BinaryTree* t, std::string& p){return true;};
        bool (* SubstringMatch)(BinaryTree*, std::string&) = [](BinaryTree* tree, std::string& parameter)
        {
            std::string param = tree->GetWord();
            return CompareSubstring(parameter, param) == 0;
        };
        BinaryTree** (* MatchingActivity)(BinaryTree**) = [](BinaryTree** tree)
        {
            std::cout.width(20);
            std::cout << std::left << (*tree)->GetWord() << " - "
            << (*tree)->GetTranslation() <<" (EXIST)" << std::endl;
            return (BinaryTree**)nullptr;
        };
        BinaryTree* core;
        void (* UserTolerantFormat)(BinaryTree*, std::ostream&) = [](BinaryTree* tree, std::ostream& stream)
        {
            stream.width(20);
            stream << std::left << tree->GetWord() << " - " << tree->GetTranslation() << std::endl;
        };
        void (* InputFormat)(BinaryTree*, std::ostream&) = [](BinaryTree* tree, std::ostream& stream)
        {
            stream << std::endl <<tree->GetWord() << " " << tree->GetTranslation();
        };
        void ChangeToUnsavedState()
        {
            wasChanged = true;
        }
    public:
        Dictionary(std::string dictionaryName)
        {
            name = dictionaryName;
            pathToInputFile = "../words/" + name + ".dict";
            core = new BinaryTree("Africa", "Африка");
        }
        ~Dictionary()
        {
            RemoveTree();
        }
        void AddPair(std::string& word, std::string& translation)
        {
            BinaryTree** addingWord = core->BranchPass(word, core, &core, CompareWords,
            MatchingActivity, [](BinaryTree** tree)
            {
                (*tree) = new BinaryTree("", "");
                return tree;
            });
            if(addingWord != nullptr)
            {
                (*addingWord)->SetWord(word);
                (*addingWord)->SetTranslation(translation);
                ChangeToUnsavedState();
            }
        }
        void RemovePair(std::string& word)
        {
            BinaryTree** removingWord = core->BranchPass(word, core, &core, CompareWords,
            [](BinaryTree** tree){return tree;}, [](BinaryTree**){return (BinaryTree**)nullptr;});
            if(removingWord != nullptr)
            {
                BinaryTree* temp = *removingWord;
                core->RemoveWord(removingWord);
                delete temp;
                ChangeToUnsavedState();
            }
            else
            {
                std::cout << std::endl << "No \"" << word << "\" in dictionary" << std::endl;
            }
        }
        void RemoveTree()
        {
            std::string param = core->GetWord();
            BinaryTree* temp = new BinaryTree("", "");
            temp->TreePass(core, param, [](BinaryTree* tree, std::ostream& stream)
            {
                delete tree;
            }, NoFilter, std::cout);
            delete temp;
        }
        void ChangeToSavedState()
        {
            wasChanged = false;
        }
        bool GetState()
        {
            return wasChanged;
        }
        std::string GetName()
        {
            return name;
        }
        void PrintDictionaryFormatted(std::ostream& stream = std::cout)
        {
            std::string param = core->GetWord();
            core->TreePass(core, param, UserTolerantFormat, NoFilter, stream);
        }
        void PrintDictionary(std::ostream& stream = std::cout)
        {
            std::string param = core->GetWord();
            core->TreePass(core, param, InputFormat, NoFilter, stream);
        }
        void PrintSubstringMatch(std::string& substring, std::ostream& stream = std::cout)
        {
            BinaryTree** firstEnter = core->BranchPass(substring, core, &core, CompareSubstring,
            [](BinaryTree** tree){return tree;}, [](BinaryTree** tree){return (BinaryTree**)nullptr;});
            if(firstEnter == nullptr)
            {
                std::cout << std::endl << "No words with \"" << substring << "\" substring" << std::endl;
            }
            else core->TreePass(*firstEnter, substring, UserTolerantFormat, SubstringMatch, stream);
        }
        void AddPairsFromFile()
        {
            std::ifstream file(pathToInputFile);
            char temp[255];
            file.getline(temp, 255);
            if(file.is_open())
            {
                while(!file.eof())
                {
                    std::string word, translation;
                    file >> word >> translation;
                    AddPair(word, translation);
                }
                file.close();
                ChangeToUnsavedState();
            }
        }
        void AddPairsToFile()
        {
            std::ofstream file(pathToInputFile);
            PrintDictionary(file);
            file.close();
        }
};

class CommandHandler
{
    private:
        Dictionary* dictionary;
        std::string command;
        std::string translation;
        void CreateDictionary(std::string name)
        {
            dictionary = new Dictionary(name);
            dictionary->AddPairsFromFile();
            dictionary->ChangeToSavedState();
        }
        void OpenDictionary()
        {
            std::cout << "Enter dictionary name" << std::endl;
            char name[20];
            std::cin.getline(name, 21);
            CreateDictionary(name);
        }
        void PrintPairCommandNotation(std::string commandName, std::string notation)
        {
            std::cout << std::endl;
            std::cout.width(15);
            std::cout << std::left << commandName << notation;
        }
        void Greeting()
        {
            system(CLEAR);
            std::cout << dictionary->GetName() << " dictionary opened" << std::endl;
        }
        void AskForSave()
        {
            if(dictionary->GetState())
            {
                std::string response;
                do
                {
                    std::cout << std::endl << "Save dictionary before closing? [y/n]: ";
                    std::cin >> response;
                    std::string yesWord = "y";
                    std::string noWord = "n";
                    if(CompareWords(response, yesWord) == 0)
                    {
                        dictionary->AddPairsToFile();
                        break;
                    }
                    else if(CompareWords(response, noWord) == 0) break;
                } while (true);
            }
        }
    public:
        CommandHandler()
        {
            OpenDictionary();
        }
        CommandHandler(char* name)
        {
            CreateDictionary(name);
        }
        ~CommandHandler()
        {
            delete dictionary;
        }
        void CommandLoop()
        {
            Greeting();
            while(true)
            {
                std::cin.clear();
                std::cin >> command;
                int firstParenthesis = command.find_first_of('{');
                int secondParenthesis = command.find_last_of('}');
                if(firstParenthesis < 0 || secondParenthesis < 0 || firstParenthesis > secondParenthesis)
                {
                    std::cin >> translation;
                    dictionary->AddPair(command, translation);
                }
                else
                {
                    command = command.substr(firstParenthesis + 1, secondParenthesis - firstParenthesis - 1);
                    if(command == "show")
                    {
                        dictionary->PrintDictionaryFormatted();
                    }
                    else if(command == "sshow")
                    {
                        std::string substring;
                        std::cin >> substring;
                        dictionary->PrintSubstringMatch(substring);
                    }
                    else if(command == "save")
                    {
                        if(dictionary->GetState())
                        {
                            dictionary->AddPairsToFile();
                            std::cout << std::endl << dictionary->GetName() << " has saved" << std::endl;
                            dictionary->ChangeToSavedState();
                        }
                    }
                    else if(command == "remove")
                    {
                        std::string word;
                        std::cin >> word;
                        dictionary->RemovePair(word);
                    }
                    else if(command == "exit")
                    {
                        AskForSave();
                        break;
                    }
                    else if(command == "new")
                    {
                        std::string name;
                        std::cin >> name;
                        AskForSave();
                        CreateDictionary(name);
                        Greeting();
                    }
                    else if(command == "input")
                    {
                        dictionary->AddPairsFromFile();
                    }
                    else if(command == "help")
                    {
                        std::cout << std::endl <<
                        "To add pair word - translation just enter word and its translation through space." <<
                        std::endl <<"Commands should be placed into figure parenthesis like this: {new}." <<
                        std::endl <<"The list of commands:";
                        PrintPairCommandNotation("show",
                        "Show all words of dictionary from A to Z");
                        PrintPairCommandNotation("sshow", "Show all words that starts with entered* substring");
                        PrintPairCommandNotation("save",
                        "Save all dictionary into file with name such \"name_of_dictionary.dict\"");
                        PrintPairCommandNotation("remove", "Remove given* word from the dictionary");
                        PrintPairCommandNotation("new", "Create new dictionary with given* name or open exiting");
                        PrintPairCommandNotation("exit", "End the program");
                        PrintPairCommandNotation("input", "Add words to the dictionary from file named as dictionary + \".dict\". Note that dictionary passes first line in file");
                        std::cout << std::endl << "*Input data should be given after command through space like this: {command} arg" << std::endl;
                    }
                    else
                    {
                        std::cout << std::endl << "No such command" << std::endl;
                    }
                }
            }
        }
};

int main(int argc, char* argv[])
{
    CommandHandler* handler;
    if(argc > 1)
    {
        handler = new CommandHandler(argv[1]);
    }
    else handler = new CommandHandler();
    handler->CommandLoop();
    delete handler;
    return 0;
}