#include "string_processing.h"
#include <string>
#include <sstream>
#include <vector>

std::vector<std::string> SplitIntoWords(const std::string& text) {
    std::vector<std::string> output;
    std::istringstream iss(text);
    std::string word;

    while(iss >> word){
        output.push_back(word);
    }

    return output;
}