#include "string_processing.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

vector<string> SplitIntoWords(const string& text) {
    vector<string> output;
    istringstream iss(text);
    string word;

    while(iss >> word){
        output.push_back(word);
    }

    return output;
}