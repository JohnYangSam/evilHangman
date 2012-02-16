/******************************************************
 * File: main.cpp
 * Author: John Yang-Sammataro
 *
 * This is the main.cpp file for a game of hangman implemented
 * using the evil hangman algorithm that makes it significantly
 * harder for the human player to win against the computer by
 * changing the word that is supposed to be guessed, based on
 * the guesses by the player.
 *
 * References: CS106L coursereader was referenced for Getline,
 * GetInteger, and file reading functions.
 *
 * Notes: This program requires the working directory to be set
 * to the same directory as the Xcode project. This can be done
 * by going in Xcode to Product->Edit Scheme->Options(tab) and
 * checking "Use custom directory"
 */

/* Include libraries and header files */
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <set>
#include <map>
using namespace std;

/* Constants */
const string HANGMAN_DICTIONARY = "dictionary.txt";
const string ALPHABET = "abcdefghijklmnopqrstuvwxyz";

/* Function Prototypes */
string GetLine();
char GetAlphabetCharacter();
int GetInteger();
int GetPositiveInteger();
void OpenFile(ifstream& input, string fileName);
void ReadDictionary(ifstream& dictionaryFile, set<string>& dictionarySet, set<int>& wordLengths);
void PromptForWordLength(set<int>& dictionaryWordLengths, int& wordLength);
void InitializeGuessedWord(int wordLength, string& guessedWord);
void InitializePossibleWords(int worldLength, set<string>& dictionarySet, set<string>& possibleWords);
void PromptForGuessesRemaining(int wordLength, int& guessesRemaining);
void PromptForDisplayOfNumberOfWordsRemaining(bool& displayNumberOfWordsRemaining);
bool PromptForYesOrNo();
void InitializeHangmanGame(ifstream& dictionaryFile, set<string>& dictionarySet, set<int>& dictionaryWordLengths, int& wordLength, string& guessedWord, set<string>& possibleWords, int& guessesRemaining, bool& displayNumberOfWordsRemaining);
void PrintWordSpaceDelinated(string word);
void PrintGuessesRemaining(int guessesRemaining);
void PrintWordsRemaining(set<string>& possibleWords, bool displayNumberOfWordsRemaining);
char PromptForCharacterGuess(string& charactersGuessed);
map<string, set<string> > MakeWordFamilyMap(set<string>& possibleWords, char guessChar);
set<string> CountLargestFamily(map<string, set<string> >& wordFamilyMap);
set<string> FindLargestWordFamily(char guessChar, set<string> possibleWords);
void UpdateGuessesRemaining(set<string>& newPossibleWords, int& guessesRemaining, char guessChar, string& guessedWord);
void UpdateGuessedWordAndCharactersGuessed(set<string>& newPossibleWords, string& guessedWord, string& charactersGuessed, char guessChar);
bool IsWordGuessed(string guessedWord);
void EndTurn (set<string>& possibleWords, int guessesRemaining, int wordLength, string guessedWord, bool& gameCompleted);
void PlayTurn(int wordLength, string& guessedWord, set<string>& possibleWords, int& guessesRemaining, string& charactersGuessed, bool displayNumberOfWordsRemaining);

/* Functions */

/* 
 * GetLine:
 * Takes in a line from the user and returns a string.
 * Safer than just using a getline(cin, str); function
 */
string GetLine() {
    string buffer;
    getline(cin, buffer);
    return buffer;
}

/*
 * GetAlphabetCharacter
 * Prompts until the user enters a character in the
 * English alphabet and returns that character in lowercase.
 */
char GetAlphabetCharacter() {
    while (true) {
        stringstream converter;
        converter << GetLine();
        char resultChar;
        converter >> resultChar;
        resultChar = tolower(resultChar);
        
        if(ALPHABET.find(resultChar) != -1) {
            char remaining;
            if (converter >> remaining) {
                cout << "Unexpected character: " << remaining << endl;
            } else {
                return resultChar;
            }
        } else {
            cout << "Please enter a character in the alphabet." << endl;
        }
        cout << "Retry: ";
    }
}

/* 
 * GetInteger:
 * Takes in an integer and prompts until a valid integer is inputed
 * by the user.
 */
int GetInteger() {
    while(true) {
        stringstream converter;
        converter << GetLine();
        int resultInt;
        /* Check if an int is entered correctly */
        if(converter >> resultInt) {
            /*Check for extraneous input */
            char remaining;
            if(converter >> remaining) {
                cout << "Unexpected character: " << remaining << endl;
            } else {
                return resultInt;
            }
        } else {
            cout << "Please enter an integer." << endl;
        }
        cout << "Retry: ";
    }    
}

/*
 * GetPositiveInteger
 * Prompts the user until they enter
 * a positive integer and returns it.
 */
int GetPositiveInteger() {
    while(true) {
        int integer = GetInteger();
        if(integer > 0) {
            return integer;
        } else {
            cout << "Not a positive integer." << endl;
            cout << "Please enter a positive integer: ";
        }
    }
}

/*
 * OpenFile
 * Takes in a filestream by reference and a
 * file name.  Opens the file if is valid and
 * throws an exception otherwise.
 */
void OpenFile(ifstream& input, string fileName) {
    input.open(fileName.c_str());
    if(!input.is_open()) {
        cout << "Error reading " << fileName << endl;
        input.clear();
    }
}

/*
 * ReadDictionary
 * Takes in a dictionaryFile filestream, a dictionarySet string set,
 * and a wordlengths integer set all by reference.  The function then
 * reads in words from the dictionaryFile into the dictionarySet and 
 * keeps track of what word lengths have been read with thie the integer
 * wordLengths set.
 */
void ReadDictionary(ifstream& dictionaryFile, set<string>& dictionarySet, set<int>& wordLengths) {
    string word;
    while(!dictionaryFile.eof()) {
        getline(dictionaryFile, word);
        dictionarySet.insert(word);
        wordLengths.insert((int)word.size());
    }
}

/*
 * PromptForWordLength
 * Takes in an integer set (by reference for efficiency) and wordLength by reference.
 * Prompts the user to enter a word length to guess and checks to make sure it is
 * contained in the integer set.  It then sets the passed in variable to that integer and
 * returns in the integer exists in that set
 */
void PromptForWordLength(set<int>& dictionaryWordLengths, int& wordLength) {
    while(true) {
        cout << "Please enter the length of the word to guess: ";
        wordLength = GetPositiveInteger();
        if (dictionaryWordLengths.count(wordLength)) {
            return;
        } else {
            cout << "Sorry, no words exist with that length in this dictionary." << endl;
        }
    }
}

/*
 * MakeGuessedWord
 * Takes in a an integer word length by value and string by reference and
 * converts the string into a string of a number of "_" (underscore) characters
 * equal to the word length.
 */
void InitializeGuessedWord(int wordLength, string& guessedWord) {
    string result = "";
    for (size_t i = 0; i < wordLength; i++) {
        result += "_";
    }
    guessedWord = result;
    
}

/*
 * InitializePossibleWords
 * Takes in an integer wordLength by value, a dictionary string set by reference
 * (for efficiency), and string set of possible words by reference.  It then sets
 * the possibleWords set to all the words in the dictionary that have the word length.
 */
void InitializePossibleWords(int wordLength, set<string>& dictionarySet, set<string>& possibleWords) {
    for(set<string>::iterator wordItr = dictionarySet.begin(); wordItr != dictionarySet.end(); wordItr ++) {
        if(wordItr->size() == wordLength) {
            possibleWords.insert(*wordItr);
        }
    }
}

/*
 * PromptForGuessesRemaining
 * Takes in a word length integer by value and a guesses remaining integer 
 * by reference and sets it to the positive number of guesses at least equal 
 * to the number letters in the word length being guessed.
 */
void PromptForGuessesRemaining(int wordLength, int& guessesRemaining) {
    while(true) {
        cout << "Please enter the number of guesses you wish to have: ";
        guessesRemaining = GetPositiveInteger();
        return;
    /*
        if (guessesRemaining >= wordLength) {
            return;
        }
        cout << "Invalid number.  Number of guesses must be greater than ";
        cout << "or equal to the length of the word being guessed." << endl;
     */
    }
}

/*
 * PromptForDisplayOfNumberOfWordsRemaining
 * Takes in a bool by reference and prompts the user to indicate if they want to
 * see the number of words remaining while playing hangman.  It returns true
 * with yes and false with no.
 */
void PromptForDisplayOfNumberOfWordsRemaining(bool& displayNumberOfWordsRemaining) {
    cout << "Would you like to know the number of possible words left after each guess?" << endl;
    displayNumberOfWordsRemaining = PromptForYesOrNo();
}

/*
 * PromptForYesOrNo
 * Prompts the user for yes or no and they checks their input returning a bool
 * true if yes and false if no.  It can acccount for multiple yes, no input types.
 * by checking the input against two yes and no strings. If it encounters an 
 * unknown input, it will reprompt.
 */
bool PromptForYesOrNo() {
    string yes = " Y y YES Yes yes ";
    string no = " N n NO No no ";
   
    while(true) {
            cout << "(Y) or (N): ";
        string input = GetLine();
        string testString = " " + input + " ";
    /* std::basic_string::find returns npos = -1 if there are no matches 
        of the substring inputed */
        if(yes.find(testString) != -1) {
            
            //cout << "testString: |" << testString << "|" << endl;
            //cout << "Yes: " << yes << endl;
            
            return true;
        } else if (no.find(testString) != -1) {
            
            //cout << "testString: |" << testString << "|" << endl;
            //cout << "No: " << no << endl;
            
            return false;
        } else {
            cout << "Invalid input.  Please try again." << endl;
        }
    }
}

/*
 * InitializeHangmanGame
 * Takes in the state parameters for the hangman game by reference and calls upon
 * helper functions to initialize the hangman game.
 */
void InitializeHangmanGame(ifstream& dictionaryFile, set<string>& dictionarySet, set<int>& dictionaryWordLengths, int& wordLength, string& guessedWord, set<string>& possibleWords, int& guessesRemaining, bool& displayNumberOfWordsRemaining) {
    
    OpenFile(dictionaryFile, HANGMAN_DICTIONARY);
    ReadDictionary(dictionaryFile, dictionarySet, dictionaryWordLengths);
    PromptForWordLength(dictionaryWordLengths, wordLength);
    InitializeGuessedWord(wordLength, guessedWord);
    InitializePossibleWords(wordLength, dictionarySet, possibleWords);
    PromptForGuessesRemaining(wordLength, guessesRemaining);
    PromptForDisplayOfNumberOfWordsRemaining(displayNumberOfWordsRemaining);
    
}

/*
 * PrintWordSpaceDelinated
 * Takes in a string and prints it to the console with it's characters
 * delinated by spaces in between.
 */
void PrintWordSpaceDelinated(string word) {
    for (size_t i = 0; i < word.size(); i++) {
        cout << word[i] << " ";
    }
    cout << endl;
}

/*
 * PrintGuessesRemaining
 * Takes in an integer guesses remaining
 * and prints out the number with an appropriate message.
 */
void PrintGuessesRemaining(int guessesRemaining) {
    cout << "You have " << guessesRemaining << " guesses remaining." << endl;
}

/*
 * PrintWordsRemaining
 * Takes inthe possible words string set (by reference for efficiency)
 * and the displayNumberOfWordsRemaining boolean.  If the boolean is true
 * the function prints out the size of the set.  Otherwise, the function
 * does not have any effect.
 */
void PrintWordsRemaining(set<string>& possibleWords, bool displayNumberOfWordsRemaining) {
    if(displayNumberOfWordsRemaining) {
        cout << "There are " << possibleWords.size() << " possible words left." << endl;
    } else {
        //Do nothing
    }
}

/*
 * PromptForCharacterGuess
 * Takes in the charactersGuessed string by reference and prompts the user to
 * enter another character until they enter a character in the alphabet that 
 * has not already been guessed.  It then adds that character to the characters
 * guessed and returns the character.
 */
char PromptForCharacterGuess(string& charactersGuessed) {
    while (true) {
        cout << "Please guess a letter: ";
        char guessChar = GetAlphabetCharacter();
        if(charactersGuessed.find(guessChar) == -1) {
            charactersGuessed += guessChar;
            return guessChar;
        }
        cout << "The letter " << guessChar << " has already been guessed." << endl;
    }
}

/*
 * MakeWordFamilyMap
 * Takes in a set of string possible words by reference for efficiency and a guess 
 * character. Then makes a map of string sets of word families based on occurances
 * of the guess char within the words. For instance, with guess char 'e'
 * "else"-> e__e. The function then returns the map of string set<string> pairs.
 */
map<string, set<string> > MakeWordFamilyMap(set<string>& possibleWords, char guessChar) {
    map<string, set<string> > wordFamilyMap;
    for(set<string>::iterator wordPtr = possibleWords.begin(); wordPtr != possibleWords.end(); wordPtr++) {
        string familyType = "";
        for (int i = 0; i < wordPtr->size(); i++) {
            if (wordPtr->at(i) == guessChar) {
                familyType += guessChar;
            } else {
                familyType += '_';
            }
        }
        if (wordFamilyMap.count(familyType)) {
            // operator[] returns a reference to the map element. Important for efficiency
            wordFamilyMap[familyType].insert(*wordPtr);
        } else {
            set<string> familySet;
            familySet.insert(*wordPtr);
            wordFamilyMap[familyType] = familySet; 
        }
    }
    return wordFamilyMap;
}

/*
 * CountLargestWordFamily
 * Takes in a wordFamily map of strings to string sets by reference for efficiency
 * and counts the families returning the largest string set word family.
 */
set<string> CountLargestFamily(map<string, set<string> >& wordFamilyMap) {
    set<string> largestWordFamily;
    for (map<string, set<string> >::iterator pairItr = wordFamilyMap.begin(); pairItr != wordFamilyMap.end(); pairItr++) {
        set<string> wordFamily = pairItr->second;
        if (wordFamily.size() > largestWordFamily.size()) largestWordFamily = wordFamily;
    }
    return largestWordFamily;
}

/*
 * FindLargestWordFamily
 * A recursive function that takes in a character guesssed, a set of poossible
 * word strings, an integer wordLength, and an integer wordIndex whose default is
 * 0. The function then finds the largest family of words which have a permutation
 * of the character guessed (i.e. one family is words that don't contain the character
 * guessed.  The second family might have the letter in the first and third spots, etc.).
 */

set<string> FindLargestWordFamily(char guessChar, set<string> possibleWords) {
    map<string, set<string> > wordFamilyMap = MakeWordFamilyMap(possibleWords, guessChar);
    return CountLargestFamily(wordFamilyMap);
}
    
    /*
     ***** Implementation notes *****
     make a map map<string, set<string> > wordFamilyMap
     iterate through the possible words
        on a word
            make a string
                add to the string for each for loop
            check if the string exists in a map
            if it does add it to the set
                else
            add a new set with the string in it.
     
     Iterate over and find the largest set
     */

/*
 * UpdateGuessesRemaining
 * Processes the player guess by taking in the new and old possible string sets
 * (by reference for efficiency) and determining if the player guessed a correct
 * letter. It then updates the guesses remaining accordingly, printing appropriate
 * messages to the player.
 */
void UpdateGuessesRemaining(set<string>& newPossibleWords, int& guessesRemaining, char guessChar, string& guessedWord) {
    set<string>::iterator wordPtr = newPossibleWords.begin();
    if (wordPtr->find(guessChar) == -1) {
        cout << "Sorry, incorrect guess. ";
        PrintGuessesRemaining(--guessesRemaining);
    } else {
        cout << "Correct! The word contains " << guessChar << "." << endl;
    }
}

/*
 * UpdateGuessedWordAndCharactersGuessed
 * Updates the guessedWord and charactersGuessed strings according to the newPossibleWords
 * string set.  All parameters are taken in by reference (the strings to be updated
 * and the set for efficiency) except for the characterGuessed.
 */
void UpdateGuessedWordAndCharactersGuessed(set<string>& newPossibleWords, string& guessedWord, string& charactersGuessed, char guessChar) {
    set<string>::iterator pointer = newPossibleWords.begin();
    
    for(int i = 0; i < pointer->size(); i++) {
        if(pointer->at(i) == guessChar) {
            guessedWord[i] = guessChar;
        }
    }
    charactersGuessed += guessChar;
}

/*
 * IsWordGuessed
 * Takes in a string word and returns false
 * if it contains blank "_" [underscore] characters
 * and returns true if it is a complete word.
 */
bool IsWordGuessed(string guessedWord) {
    char blank = '_';
    for(int i; i < guessedWord.size(); i++) {
        if (guessedWord[i] == blank) return false;
    }
    return true;
}

/*
 * EndTurn
 * Ends the player turn by taking in set of possible words, the guessesRemaining, wordLength,
 * string of the guessedWord and the boolean for the game completion. It then determines whether
 * the game has ended and prints out appropriate messages to the player before signaling by
 * chaing the boolean, to end the while loop.
 */
void EndTurn (set<string>& possibleWords, int guessesRemaining, int wordLength, string guessedWord, bool& gameCompleted) {
    if (IsWordGuessed(guessedWord)) {
        cout << "Congratulations! You WIN!" << endl;
        cout << "The word is ";
        PrintWordSpaceDelinated(guessedWord);
        gameCompleted = true;
    } else if (guessesRemaining == 0) {
        cout << "Sorry, you lose. The word is: " << endl;
        set<string>::iterator pointer = possibleWords.begin();
        PrintWordSpaceDelinated(*pointer);
        gameCompleted = true;
    } else {
        //Do nothing
    }
}

void PlayTurn(int wordLength, string& guessedWord, set<string>& possibleWords, int& guessesRemaining, string& charactersGuessed, bool displayNumberOfWordsRemaining) {
    
    /* Update player */
    PrintGuessesRemaining(guessesRemaining);
    PrintWordSpaceDelinated(guessedWord);
    PrintWordsRemaining(possibleWords, displayNumberOfWordsRemaining);
    
    /* Guessing character */
    char guessChar = PromptForCharacterGuess(charactersGuessed);
    set<string> newPossibleWords = FindLargestWordFamily(guessChar, possibleWords);
    
    UpdateGuessesRemaining(newPossibleWords, guessesRemaining, guessChar, guessedWord);
    UpdateGuessedWordAndCharactersGuessed(newPossibleWords, guessedWord, charactersGuessed, guessChar);
    possibleWords = newPossibleWords;
}


/* Main function */

int main () {
    /* Dictionary */
    ifstream dictionaryFile;
    set<string> dictionarySet;
    set<int> dictionaryWordLengths;
    
    /* States */ 
    int wordLength, guessesRemaining;
    string guessedWord;
    string charactersGuessed = "";
    set<string> possibleWords;
    bool displayNumberOfWordsRemaining;
    bool gameCompleted = false;
    
    /* Initialize the hangman game */
    InitializeHangmanGame(dictionaryFile, dictionarySet, dictionaryWordLengths, wordLength, guessedWord, possibleWords, guessesRemaining, displayNumberOfWordsRemaining);
    
    /* Play hangman turns */
    while (!gameCompleted) {
        PlayTurn(wordLength, guessedWord, possibleWords, guessesRemaining, charactersGuessed,displayNumberOfWordsRemaining);
        EndTurn(possibleWords, guessesRemaining, wordLength, guessedWord, gameCompleted);
    }
    
    return 0;
}

/*
 ***** Implementation Notes *****
 
 Prompting
 ReadDictionaryFile
 Prompt user for positive word length that corresponds to a real word length
 Prompt for a positive number of guesses
 Prompt the user to ask if they want a running total of the remaining words in the list
 
 List of all words with matching length
 Print out
 Guesses remaining
 Letters guessed
 Number of words remaining if prompted
 
 Prompt the user for single letter guess
 Single letter
 Letter in the alphabet
 Not a letter already guessed
 
 Mark different word families
 Letter position and number matter
 Find the largest word family.  If it doesn't include the letter, subtract a guess from the user
 
 Remove words that are not in the current family.  Update letters if there is a correct guess
 
 If the player has run out of guesses, then pick a word a display it.
 
 If the player picks the correct word then congratulate him
 
 Reprompt accordingly
 vector, map, stack, queque
 
 
 */