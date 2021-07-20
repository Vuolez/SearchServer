#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <sstream>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> output;
    istringstream iss(text);
    string word;

    while(iss >> word){
        output.push_back(word);
    }

    return output;
}

template <typename StringContainer>
set<string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    set<string> non_empty_strings;
    for (const string& str : strings) {
        if (!str.empty()) {
            non_empty_strings.insert(str);
        }
    }
    return non_empty_strings;
}

struct Document {
    Document() = default;

    Document(int id, double relevance, int rating)
            : id(id)
            , relevance(relevance)
            , rating(rating) {
    }

    int id = 0;
    double relevance = 0;
    int rating = 0;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words)
            : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
        if(IsWordsHaveSpecialSymbols(stop_words_)){
            throw invalid_argument("Stop words contain invalid characters with codes from 0 to 31");
        }
    }

    explicit SearchServer(const string& stop_words_text)
            : SearchServer(SplitIntoWords(stop_words_text))  // Invoke delegating constructor from string container
    {
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
        const vector<string> words = SplitIntoWordsNoStop(document);

        //Check response for correctness
        for(const string& word : words){
            if(!IsValidWord(word)){
                throw invalid_argument("Words in the 'document' must not contain invalid characters with codes from 0 to 31");
            }
            else if(document_id < 0){
                throw invalid_argument("'document_id' must be a positive number");
            }
            else if(documents_.count(document_id)){
                throw invalid_argument("The document with the given 'document_id' already exists");
            }
        }

        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }

        documents_.emplace(document_id,DocumentData{ComputeAverageRating(ratings),status});
    }

    [[nodiscard]] int GetDocumentId(const int position) const{
        if(documents_.size() < position || position < 0){
            throw out_of_range("Must satisfy the condition:  0 < 'position' < SearchServer::GetDocumentCount()");
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

    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }


    template <typename DocumentPredicate>
    [[nodiscard]] vector<Document> FindTopDocuments(const string& raw_query, DocumentPredicate document_predicate) const {
        if(!IsQueryCorrect(raw_query)){
            throw invalid_argument("'raw_query' has one of the following errors:"
                                   "1.Search words contain invalid characters with codes from 0 to 31"
                                   "2.More than one minus sign in front of words"
                                   "3.No text after the 'minus' character");
        }

        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, document_predicate);

        sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
            if (abs(lhs.relevance - rhs.relevance) < 1e-6) {
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

    [[nodiscard]] vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status) const {
        return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
    }

    [[nodiscard]] vector<Document> FindTopDocuments(const string& raw_query) const {
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }

    [[nodiscard]] unsigned int GetDocumentCount() const {
        return documents_.size();
    }

    [[nodiscard]] tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const {
        if(!IsQueryCorrect(raw_query)){
            throw invalid_argument("'raw_query' has one of the following errors:"
                                   "1.Search words contain invalid characters with codes from 0 to 31"
                                   "2.More than one minus sign in front of words"
                                   "3.No text after the 'minus' character");
        }

        const Query query = ParseQuery(raw_query);
        vector<string> matched_words;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }

        tuple<vector<string>, DocumentStatus> result = {matched_words, documents_.at(document_id).status};
        return result;
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;

    [[nodiscard]] bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    [[nodiscard]] vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    static bool IsWordsHaveSpecialSymbols(const set<string>& words){
        for(const string& word : words){
            if(!IsValidWord(word)){
                return true;
            }
        }

        return false;
    }

    static bool IsQueryCorrect(const string& query_words){
        for(const string& word : SplitIntoWords(query_words)){
            if(!IsQueryWordCorrect(word)){
                return false;
            }
        }
        return true;
    }

    static bool IsQueryWordCorrect(const string& word){
        if((word.size() == 1 && word[0] == '-')
           || (word[0] == '-' && word[1] == '-')
           || !IsValidWord(word)){
            return false;
        }

        return true;
    }

    static bool IsValidWord(const string& word) {
        // A valid word must not contain special characters
        return none_of(word.begin(), word.end(), [](char c) {
            return c >= '\0' && c < ' ';
        });
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        int rating_sum = 0;
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const {
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

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
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
    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    template <typename Func>
    vector<Document> FindAllDocuments(const Query& query, Func func) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
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

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back({
                                                document_id,
                                                relevance,
                                                documents_.at(document_id).rating
                                        });
        }
        return matched_documents;
    }
};