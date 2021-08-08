#pragma once

#include "document.h"
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

const int MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer {
public:
    explicit SearchServer() = default;

    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words)
            : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
        if(IsWordsHaveSpecialSymbols(stop_words_)){
            throw std::invalid_argument("Stop words contain invalid characters with codes from 0 to 31");
        }
    }

    explicit SearchServer(const std::string& stop_words_text);

    void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);

    [[nodiscard]] int GetDocumentId(const int position) const;

    void SetStopWords(const std::string& text);

    template <typename DocumentPredicate>
    [[nodiscard]] std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const {
        if(!IsQueryCorrect(raw_query)){
            throw std::invalid_argument("'raw_query' has one of the following errors:"
                                   "1.Search words contain invalid characters with codes from 0 to 31"
                                   "2.More than one minus sign in front of words"
                                   "3.No text after the 'minus' character");
        }

        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, document_predicate);

        sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
            if (std::abs(lhs.relevance - rhs.relevance) < 1e-6) {
                return lhs.rating > rhs.rating;
            } else {
                return lhs.relevance > rhs.relevance;
            }
        });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }

        return matched_documents;
    }

    [[nodiscard]] std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status) const;

    [[nodiscard]] std::vector<Document> FindTopDocuments(const std::string& raw_query) const;

    [[nodiscard]] unsigned int GetDocumentCount() const;

    [[nodiscard]] std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    struct QueryWord {
        std::string data;
        bool is_minus;
        bool is_stop;
    };

    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;


    [[nodiscard]] bool IsStopWord(const std::string& word) const;

    [[nodiscard]] std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

    static bool IsWordsHaveSpecialSymbols(const std::set<std::string>& words);

    static bool IsQueryCorrect(const std::string& query_words);

    static bool IsQueryWordCorrect(const std::string& word);

    static bool IsValidWord(const std::string& word);

    static int ComputeAverageRating(const std::vector<int>& ratings);


    QueryWord ParseQueryWord(std::string text) const;

    Query ParseQuery(const std::string& text) const;

    // Existence required
    double ComputeWordInverseDocumentFreq(const std::string& word) const;

    template <typename Func>
    std::vector<Document> FindAllDocuments(const Query& query, Func func) const {
        std::map<int, double> document_to_relevance;
        for (const std::string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                const auto [rating, status] = documents_.at(document_id);
                if (func(document_id,status, rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const std::string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        std::vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back({document_id,relevance,documents_.at(document_id).rating});
        }
        return matched_documents;
    }
};