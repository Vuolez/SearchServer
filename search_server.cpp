#include "search_server.h"
#include "string_processing.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <sstream>
#include <cerrno>

SearchServer::SearchServer(const std::string& stop_words_text)
        : SearchServer(SplitIntoWords(stop_words_text))  // Invoke delegating constructor from std::string container
{
}

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status,
                               const std::vector<int>& ratings)
{
    const std::vector<std::string> words = SplitIntoWordsNoStop(document);

    //Check response for correctness
    for(const std::string& word : words){
        if(!IsValidWord(word)){
            throw std::invalid_argument("Words in the 'document' must not contain invalid characters with codes from 0 to 31");
        }
        else if(document_id < 0){
            throw std::invalid_argument("'document_id' must be a positive number");
        }
        else if(documents_.count(document_id)){
            throw std::invalid_argument("The document with the given 'document_id' already exists");
        }
    }

    const double inv_word_count = 1.0 / words.size();
    for (const std::string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }

    documents_.emplace(document_id,DocumentData{ComputeAverageRating(ratings),status});
}

int SearchServer::GetDocumentId(const int position) const{
    if(documents_.size() < position || position < 0){
        throw std::out_of_range("Must satisfy the condition:  0 < 'position' < SearchServer::GetDocumentCount()");
    }

    size_t i = 0;
    int result = 0;
    for(const auto& [id, document] : documents_){
        if(i == id){
            result = id;
            break;
        }
        ++i;
    }

    return result;
}

void SearchServer::SetStopWords(const std::string& text) {
    for (const std::string& word : SplitIntoWords(text)) {
        stop_words_.insert(word);
    }
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
    });
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

unsigned int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query, int document_id) const {
    if(!IsQueryCorrect(raw_query)){
        throw std::invalid_argument("'raw_query' has one of the following errors:"
                               "1.Search words contain invalid characters with codes from 0 to 31"
                               "2.More than one minus sign in front of words"
                               "3.No text after the 'minus' character");
    }

    const Query query = ParseQuery(raw_query);
    std::vector<std::string> matched_words;
    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }

    std::tuple<std::vector<std::string>, DocumentStatus> result = {matched_words, documents_.at(document_id).status};
    return result;
}

bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) > 0;
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) {
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

bool SearchServer::IsWordsHaveSpecialSymbols(const std::set<std::string>& words){
    for(const std::string& word : words){
        if(!IsValidWord(word)){
            return true;
        }
    }

    return false;
}

bool SearchServer::IsQueryCorrect(const std::string& query_words){
    for(const std::string& word : SplitIntoWords(query_words)){
        if(!IsQueryWordCorrect(word)){
            return false;
        }
    }
    return true;
}

bool SearchServer::IsQueryWordCorrect(const std::string& word){
    if((word.size() == 1 && word[0] == '-')
       || (word[0] == '-' && word[1] == '-')
       || !IsValidWord(word)){
        return false;
    }

    return true;
}

bool SearchServer::IsValidWord(const std::string& word) {
    // A valid word must not contain special characters
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    int rating_sum = 0;
    for (const int rating : ratings) {
        rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string text) const {
    bool is_minus = false;
    // Word shouldn't be empty
    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
    }
    return {
            text,
            is_minus,
            IsStopWord(text)
    };
}

SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {
    Query query;
    for (const std::string& word : SplitIntoWords(text)) {
        const QueryWord query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                query.minus_words.insert(query_word.data);
            } else {
                query.plus_words.insert(query_word.data);
            }
        }
    }
    return query;
}

// Existence required
double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}