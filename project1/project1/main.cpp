//
//  main.cpp
//  project1
//
//  Created by Yu-Jung Lee and Andriy Goltsev on 3/1/15.
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
using namespace std;

#define ALHPABET_SIZE 27
#define KEY_ALHPABET_SIZE 26
//#define CIPHER_TEXT_ALHPABET_SIZE KEY_ALHPABET_SIZE
#define TEXT_ALPHA_START 64
#define CIPHER_ALPHA_START 65

vector<string> dictionary1;
vector<string> dictionary2;

vector<string> roots;
int keySize;

//unsigned int getShiftNumber(char cipher, char plain);

int aton(char c) {
    if (c == ' '){
        return 0;
    }
    else {
        return c - 'A' + 1;
    }
}

char ntoa(int n) {
    if ( n == 0 ) {
        return ' ';
    }
    else {
        return n + 'A' - 1;
    }
}

int getShiftNumber(char cipher, char plain) {
    
    int c, p;
    c = aton(cipher);
    p = aton(plain);
    
    return c >= p ? c - p : c + (ALHPABET_SIZE - p);
}

char shiftBy(char ch, int rot) {
    return ntoa((aton(ch)+rot)%ALHPABET_SIZE);
}

bool readDictionary1() {
    string fileName = "/Users/AndriyGoltsev/git/CryptProject1/project1/project1/Dictionary1.txt";
    string line;
    ifstream myfile (fileName);
    if (myfile.is_open()){
        
        while ( getline (myfile,line) ) {
            std::transform(line.begin(), line.end(), line.begin(), ::toupper);
            dictionary1.push_back(line);
        }
        myfile.close();
        cout << "Read Dictionary 1 of size " << dictionary1.size() << " lines.\n";
        
        return true;
    }
    else {
        cerr << "Unable to open file " + fileName;
        return false;
    }
    
    return false;
}

bool readDictionary2() {
    string fileName = "/Users/AndriyGoltsev/git/CryptProject1/project1/project1/Dictionary2.txt";
    string line;
    ifstream myfile (fileName);
    if (myfile.is_open()){
        
        while ( getline (myfile,line) ) {
            std::transform(line.begin(), line.end(), line.begin(), ::toupper);
            dictionary2.push_back(line);
        }
        myfile.close();
        
        size_t dictionarySize = dictionary2.size();
        roots.reserve(dictionarySize * dictionarySize * dictionarySize);
        
        for(size_t i = 0; i < dictionarySize; i++)
            for(size_t j = 0; j < dictionarySize; j++)
                for(size_t k = 0; k < dictionarySize; k++){
                    roots.push_back(dictionary2[i] + " " + dictionary2[j] + " " + dictionary2[k] + " ");
                }
        
        cout << "Read Dictionary 2 of size " << dictionary2.size() << " lines.\n";
        
        return true;
    }
    else {
        cerr << "Unable to open file " + fileName;
        return false;
    }
    
    return false;
}

string encryptD1(int keyLen) {
    
    // generate key
    vector<int> key;
    string stringKey;
    stringstream ss;
    int rot;
    for (int i = 0; i < keyLen; i++) {
        rot = 1 + rand() % KEY_ALHPABET_SIZE;
        key.push_back(rot);
        ss << rot << ' ';
    }
    
    string plainText = dictionary1[rand() % dictionary1.size()];
    string cipher;
    
    int c, p;
    for (int i = 0; i < plainText.size(); i++) {
        p = aton(plainText[i]);
        
        c = ntoa((p + key[rand() % key.size()]) % ALHPABET_SIZE );
        
        cipher.push_back(c);
        
    }
    
    cout << "Plain text: " << plainText << '\n';
    cout << "Key : " << ss.str() << '\n';
    cout << "Cipher text : " << cipher << '\n';
    
    return cipher;
}

int main (int argc, char** argv) {
    int keySize;
    srand(time(NULL));
    
    string cipherText;
    
    
    readDictionary1();
    readDictionary2();
    
    if(argc == 3) {
        keySize = atoi(argv[1]);
        cipherText = argv[2];
    }
    else {
    
        keySize = 23;
        cipherText = encryptD1(keySize);//argv[2];
    }

    vector<string>::const_iterator it = dictionary1.begin();
    string line;
    set<int> keySet;
    while (it != dictionary1.end()) {
        line = *it;
        keySet.clear();
        int shift;
        for (size_t i = 0; i < line.size(); i++) {
            shift = getShiftNumber(cipherText[i] ,line[i]);
            //cout << shift << ' ';
            keySet.insert(shift);
        }
        
        
        
        if (keySet.size() <= keySize) {
            cout << '\n';
            cout << line;
            cout << '\n';
        }
        
        it++;
        //cout << '\n';

    }
    
//    cout << roots.size() << '\n';
//    
//    char c = 'b';
//    for (int i = 0; i < 27; i++) {
//        cout << (char)(c + (char)(i%ALHPABET_SIZE))  << ':' << getShiftNumber(c, c + (char)i) << '\n';
//    }
    
    return 0;
}

