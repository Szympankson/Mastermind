#include <iostream>
#include <regex>
#include <utility>
#include <vector>
#include <string>
#include <array>
#include <tuple>
#include <sstream>
#include <algorithm>

using std::cin, std::cout, std::cerr;
using std::regex, std::regex_match;
using std::pair;
using std::vector;
using std::string, std::stoi;
using std::tuple;
using std::array;
using std::stringstream;
using std::count;

#define MIN_K 2
#define MAX_K 256
#define MIN_N 2
#define MAX_N 10
#define MAX_POWER 16777216 // 2^24

// Things useful in both scenarios.
namespace {
    // In particular contains '\r' because we only allow it at the back of a
    // string, as a remnant of "\n\r". Whenever we use this array, that case
    // has already been handled and the '\r' has been removed.
    const array<char, 4> UNWANTED_WHITESPACE_CHARS = {'\t', '\r', '\v', '\f'}; 

    const regex SMALL_NUMBER_REGEX("\\b([0-9]|[1-9][0-9]{0,2})\\b"); // 0-999

    void error() {
        cerr << "ERROR\n";
        exit(1);
    }

    bool containsUnwantedWhitespaceChars(const string & s) {
        for (char c : UNWANTED_WHITESPACE_CHARS) {
            if (s.find(c) != string::npos) {
                return true;
            }
        }
        return false;
    }

    bool validDataSize(int k, int n) {
        if (k < MIN_K || MAX_K < k) {
            return false;
        }

        if (n < MIN_N || MAX_N < n) {
            return false;
        }

        int64_t power = 1;
        for (int i = 1; i <= n; i++) {
            power *= k;
            if (power > MAX_POWER) {
                return false;
            }
        }

        return true;
    }

    void popCarriageReturn(string & line) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }  
    }

    void initiallyVerify(const string & line) {
        if (line.empty()) {
            error();
        }
        if (containsUnwantedWhitespaceChars(line)) {
            error();
        }
        if (line.front() == ' ' || line.back() == ' ') {
            error();
        }
    }

    pair<int, int> responseToGuess(const vector<int> & code, 
    const vector<int> & guess, int k, int n) { 
        int b = 0;
        for (int i = 0; i < n; i++) {
            if (code[i] == guess[i]) {
                b++;
            }
        }

        int w = -b;
        for (int i = 0; i < k; i++) {
            w += std::min(
                count(code.begin(), code.end(), i), 
                count(guess.begin(), guess.end(), i)
            );
        } 

        return pair{b, w};
    }

}

// Things useful when computer plays as the codemaker.
namespace codemaker { namespace {

    tuple<int, int, vector<int>> parseArguments(int argc, char* argv[]) {
        if (!regex_match(argv[1], SMALL_NUMBER_REGEX)) {
            error();
        }

        int k = stoi(argv[1]);
        int n = argc - 2;

        if (!validDataSize(k, n)) {
            error();
        }

        vector<int> code;
        code.reserve(n);

        for (int i = 2; i < argc; i++) {
            if (regex_match(argv[i], SMALL_NUMBER_REGEX)) {
                code.push_back(stoi(argv[i]));
                if (code.back() >= k) {
                    error();
                }
            }
            else {
                error();
            }
        }   

        return tuple{k, n, code}; 
    }    

    // Initially verify and parse user's guess. 
    vector<int> parseLine(const string & line, int k, int n) {
        // If line survives following conditions, it is of form: 
        // word_1 + " " + ... + " " + word_n, where words don't contain
        // any whitespace characters.
        initiallyVerify(line);
        if (count(line.begin(), line.end(), ' ') != n - 1) {
            error();
        }
        if (line.find("  ") != string::npos) { // two consecutive spaces 
            error();
        }
        
        vector<int> guess;
        guess.reserve(n);
        
        stringstream lineStream(line);
        string word;

        while(lineStream >> word) {
            if (regex_match(word, SMALL_NUMBER_REGEX)) {
                guess.push_back(stoi(word));
                if (guess.back() >= k) {
                    error();
                }
            }
            else {
                error();
            }
        }

        return guess;
    }
    

    void play(int argc, char* argv[]) {
        auto [k, n, code] = parseArguments(argc, argv);

        string line; 
        bool gameFinished = false;

        // Game loop.
        while (!gameFinished && getline(cin, line)) {
            
            popCarriageReturn(line);
            vector<int> guess = parseLine(line, k, n);

            auto [b, w] = responseToGuess(code, guess, k, n); 

            cout << b << ' ' << w << '\n';

            if (b == n && w == 0) { 
                gameFinished = true; 
            }
        }
    }

}}

// Things useful when computer plays as the codebreaker.
namespace codebreaker { namespace {

    pair<int, int> parseArguments(char* argv[]) {
        if (!regex_match(argv[1], SMALL_NUMBER_REGEX) 
        || !regex_match(argv[2], SMALL_NUMBER_REGEX)) {
            error();
        }

        int k = stoi(argv[1]);
        int n = stoi(argv[2]);
        if (!validDataSize(k, n)) {
            error();
        }

        return pair{k, n};
    }

    // Parse and initially verify user's response to computer's guess.
    pair<int, int> parseLine(const string & line, int n) {
        // If line survives following conditions, then it is of form:
        // word1 + " " + word2 and words don't contain any whitespace character.
        initiallyVerify(line);
        if (count(line.begin(), line.end(), ' ') != 1) {
            error();
        }
        
        stringstream lineStream(line);
        string word1, word2;
        lineStream >> word1 >> word2;

        if (!regex_match(word1, SMALL_NUMBER_REGEX) 
        || !regex_match(word2, SMALL_NUMBER_REGEX)) {
            error();
        }
        
        int b = stoi(word1);
        int w = stoi(word2);

        if (b + w > n) {
            error();
        }

        return pair{b, w};
    }

    void lexicographicallyIncrement(vector<int> & v, int k, int n) {
        for (int i = n - 1; i >= 0; i--) {
            if (v[i] < k - 1) {
                v[i]++;
                return;
            }
            else {
                v[i] = 0;
            }
        }
        error(); // User's answers must have been internally contradictory.
    }

    using guess_t = tuple<vector<int>, int, int>;

    bool contradictsPreviousAnswers(const vector<int> & guess, 
    const vector<guess_t> & prevGuesses, int k, int n) {
        for (auto [prevGuess, b, w] : prevGuesses) {
            if (responseToGuess(guess, prevGuess, k, n) != pair{b, w}) {
                return true;
            }
        }
        return false;
    }

    vector<int> nextGuess(const vector<guess_t> & prevGuesses, int k, int n) {
        if (prevGuesses.empty()) {
            return vector<int>(n, 0); 
        }
    
        auto [guess, _1, _2] = prevGuesses.back(); // _1, _2 - not important
        (void)_1;
        (void)_2;

        do {
            lexicographicallyIncrement(guess, k, n); 
        } while (contradictsPreviousAnswers(guess, prevGuesses, k, n));
        
        return guess;
    }

    void play(char* argv[]) {
        auto [k, n] = parseArguments(argv);    

        bool gameFinished = false;
        string line;

        vector<guess_t> prev_Guesses;
        vector<int> guess; 
        
        while (!gameFinished) {
            guess = nextGuess(prev_Guesses, k, n);
            
            for (int i = 0; i < n; i++) {
                cout << guess[i] << ((i < n - 1) ? ' ' : '\n');
            }

            if (!getline(cin, line)) {
                break; // Closing input stream doesn't result in error.
            }
            popCarriageReturn(line);
            auto [b, w] = parseLine(line, n);

            if(b == n && w == 0) {
                gameFinished = true;
            }
            else {
                prev_Guesses.push_back(tuple{guess, b, w});                
            }
        } 
    }

}}

int main(int argc, char* argv[]) {

    if (argc == 3) { // Two 'real' command line arguments. 
        codebreaker::play(argv);
    }
    else if (argc > 3) {
        codemaker::play(argc, argv);
    }
    else { // Incorrect number of command line arguments.
        error();
    }

    return 0;
}
