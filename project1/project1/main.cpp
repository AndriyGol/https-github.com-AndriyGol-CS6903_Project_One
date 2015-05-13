//
//  main.cpp
//  project1
//
//  Created by Andriy Goltsev and Yu-Jung Lee on 3/1/15.
//  Copyright (c) 2015 Andriy Goltsev. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <stdlib.h>     
#include <time.h>
#include <algorithm>
#include <thread> 
#include <ios>
#include <mutex>
#include <unordered_set>
#include "dictionaries.cpp"

using namespace std;

#define ALHPABET_SIZE 27
#define KEY_ALHPABET_SIZE 27
#define TEXT_ALPHA_START 64
#define CIPHER_ALPHA_START 65
#define KEY_SIZE 10
#define MAX_THREADS 40
#define CIPHER_TEXT_SIZE 100

typedef int (*functionJ) (int i, int t, int L);

vector<string> dictionary1;
vector<string> dictionary2;
vector<string> plainTextCandidates;
vector<functionJ> jfunctions;
vector<string> roots;

mutex mtx;


int j1(int i, int t, int L) { return i % t; }
int j2(int i, int t, int L) { return ((i % t) + i/t) % t; }
int j3(int i, int t, int L) { return rand() % t; }

// Alpha to number
int aton(int c) {
    if (c == ' '){
        return 0;
    }
    else {
        return c - 'a' + 1;
    }
}

// Number to alpha
char ntoa(int n) {
    if ( n == 0 ) {
        return ' ';
    }
    else {
        return n + 'a' - 1;
    }
}

int getShiftNumber(char cipher, char plain) {
    
    int c, p;
    c = aton(cipher);
    p = aton(plain);
    
    return c >= p ? c - p : c + (ALHPABET_SIZE - p);
}

char shiftBy(char ch, int rot) {
    return ntoa((aton(ch) + rot) % ALHPABET_SIZE);
}

vector<int> generateRandomKey(int keyLen) {
    // generate key
    vector<int> key;
    string stringKey;
    stringstream ss;
    int rot;
    for (int i = 0; i < keyLen; i++) {
        rot = rand() % KEY_ALHPABET_SIZE;
        key.push_back(rot);
    }

#ifdef VERBOSE_MODE
    sort(key.begin(), key.end());
    for (int& k : key) { ss << k << ' '; }
    cout << "Key : " << ss.str() << '\n';
#endif
    
    return key;
}

void addToKeyCandidates(string s) {
    mtx.lock();
    plainTextCandidates.push_back(s);
    mtx.unlock();
}

void readDictionary1() {
    dictionary1 = getDictionary1();
}

void readDictionary2() {
    
        dictionary2 = getDictionary2();
        size_t dictionarySize = dictionary2.size();
        roots.reserve(dictionarySize * dictionarySize * dictionarySize);
        
        for(size_t i = 0; i < dictionarySize; i++)
            for(size_t j = 0; j < dictionarySize; j++)
                for(size_t k = 0; k < dictionarySize; k++){
                    roots.push_back(dictionary2[i] + " " + dictionary2[j] + " " + dictionary2[k]);
                }
}

// General encrypting function.
string encrypt(string plainText, vector<int> key) {
    
    cout << "Plain text: " << plainText << '\n';
#ifdef VERBOSE_MODE
    cout << "******* Crypto Map ********\n";
#endif
    
    int c, p, k;
    string cipher;
    functionJ j;
    for (int i = 0; i < plainText.size(); i++) {
        p = aton(plainText[i]);
        j = jfunctions[rand() % jfunctions.size()]; // j3 for random
        k = j(i, (int)key.size(), CIPHER_TEXT_SIZE);
        c = ntoa((p + key[k]) % ALHPABET_SIZE );
#ifdef VERBOSE_MODE
        cout << "C(" << plainText[i] << " , " << key[k] << ")" << "\t=>\t" << c << " (" << (char)c  << ")\n";
#endif
        cipher.push_back(c);
        
    }
#ifdef VERBOSE_MODE
    cout << "******* Crypto Map END ********\n";
#endif
    return cipher;
}

void trimToSize(string& text, size_t size) {
    if (text.size() > size)
        text.resize(size);
}

// Generate test cipher text from Dictionary1 with a random key of length keyLen
string encryptD1(int keyLen) {
    
    // generate key
    vector<int> key =generateRandomKey(keyLen);
    
    string plainText = dictionary1[rand() % dictionary1.size()];
    string cipher = encrypt(plainText, key);

    return cipher;
}

// Generate test cipher text from Dictionary2 with a random key of length keyLen
string encryptD2(int keyLen) {
    
    // generate key
    vector<int> key = generateRandomKey(keyLen);
    string plainText;
    string cipher;
    
    plainText = dictionary2[rand() % dictionary2.size()];
    while (plainText.size() < CIPHER_TEXT_SIZE) {
        plainText += " " + dictionary2[rand() % dictionary2.size()];
    }
    
    trimToSize(plainText, CIPHER_TEXT_SIZE);
    cipher = encrypt(plainText, key);
    
    return cipher;
}

// Decrypt using dictionary1
void decryptD1(int keySize, string cipherText) {
    
    
    vector<string>::const_iterator it = dictionary1.begin();
    string line;
    set<int> keySet;
    while (it != dictionary1.end()) {
        line = *it;
        keySet.clear();
        int shift;
        for (size_t i = 0; i < line.size(); i++) {
            shift = getShiftNumber(cipherText[i] ,line[i]);
            keySet.insert(shift);
        }
        
        if (keySet.size() <= keySize) {
            addToKeyCandidates(line);
        }
        
        it++;
    }

}

// Predicate function to check if the key fits constraint
// return   0  - line is a valid cadidate
//          1  - line can be a candidate
//          -1 - line does not satifies the predicate
int fitsKeyConstraint(const string& line, const string& cipherText, int myKeySize ) {

    unordered_set<int> keySet;
    int shift;
    for (size_t i = 0; i < line.size(); i++) {
        shift = getShiftNumber(cipherText[i] ,line[i]);
        keySet.insert(shift);
    }
    
    if (keySet.size() <= myKeySize) {
        return line.size() == cipherText.size() ? 0 : 1;
    }
    else {
        return -1;
    }
}

// Recursive Depth-First search function
void decryptRecursive(string line, const string& cipherText, int myKeySize) {
    
    // if we have too many candidates short-circuit
    if (plainTextCandidates.size() > 100) {
        return;
    }
    
    trimToSize(line, cipherText.length());
    int status = fitsKeyConstraint(line, cipherText, myKeySize);
    
    if (status == 0) {
        addToKeyCandidates(line);
    }
    else if ( status > 0 && line.size() < cipherText.size()) {
        for (int i = 0; i < dictionary2.size(); i++) {
            decryptRecursive(line + " " + dictionary2[i] , cipherText, myKeySize);
        }
    }
}

// Function to be used in thread
void decryptThread(size_t start, size_t end , const string& cipherText, int keySize) {
    
    for (size_t i = start; i < end; i++) {
            decryptRecursive(roots[i], cipherText, keySize);
    }
    
}

// Decrypt using deictionary2
void decryptD2(int keySize, string cipherText) {
    
    size_t chunkSize = roots.size() / MAX_THREADS;

    size_t start, end;
    vector<thread> threads;
    for (size_t i = 0;  i <= MAX_THREADS; i++) {
        
            start = i * chunkSize;
            end = (i + 1) * chunkSize;
        
            if (start < roots.size()) {
            
                if (end > roots.size()) {
                    end  = roots.size();
                }
                // Start thread
                threads.push_back(thread(decryptThread, start, end, cipherText, keySize));
            }
    }
        
    for (auto& th : threads) th.join();
    
    return;
}

int main (int argc, char** argv) {
    srand(time(NULL));
    string cipherText;
    int keySize;
    
    readDictionary1();
    readDictionary2();
    
    // Never finished :(((
    // I wanted to add different algorithms testing
    jfunctions.push_back(j1);
    jfunctions.push_back(j2);
    jfunctions.push_back(j3);
    
    
    do {
        plainTextCandidates.clear();
        
        cout << "Enter key size >> ";
        cin >> keySize;
        cin.ignore();
        // uncomment for testing
        cout << "Test key: " << (rand() % 2 == 1 ? encryptD2(keySize) : encryptD1(keySize)) << endl;
        cout << "Enter cipher text >> ";
        getline(cin, cipherText, '\n');
        decryptD1(keySize, cipherText);
        
        if(plainTextCandidates.empty()) {
            decryptD2(keySize, cipherText);
        }
        
        if(plainTextCandidates.empty()) {
            cout << "Could not decrypt " << cipherText << endl;
            cout << "Please try again." << endl;
        }
        else {
            cout << "Guess: " << plainTextCandidates[rand()%plainTextCandidates.size()] << endl;
        }
        
    } while (true);
    
    return 0;
}

