//#include "search_server.h"

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

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
                     const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cerr << boolalpha;
        cerr << file << "("s << line << "): "s << func << ": "s
             << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s
             << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
                const string& hint) {
    if (!value) {
        cerr << file << "("s << line << "): "s << func << ": "s
             << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename Func>
void RunTestImpl(Func func , const string& func_name) {
    func();
    cout << func_name << " OK"s << endl;
}

#define RUN_TEST(func)  RunTestImpl(func, (#func))

template <typename Key, typename Value>
ostream& operator<<(ostream& out, const pair<Key,Value> element){
    out << '(' << element.first << ", "s << element.second << ')';
    return out;
}

template <typename Elements>
void Print(ostream& out, const Elements& elements) {
    bool is_first = true;
    for(const auto& element : elements){
        if(is_first){
            out << element;
            is_first = false;
        }
        else{
            out << ", "s << element;
        }
    }
}

template <typename Elements>
ostream& operator<<(ostream& out, const vector<Elements>& elements) {
    out << '[' ;
    Print(out, elements);
    out << ']' ;
    return out;
}

template <typename Elements>
ostream& operator<<(ostream& out, const set<Elements>& elements) {
    out << '{' ;
    Print(out, elements);
    out << '}' ;
    return out;
}

template <typename Key, typename Value>
ostream& operator<<(ostream& os, const map<Key,Value>& map){
    os << '{';
    size_t i = 0;
    for(const auto&[key , value] : map){
        os << key  << ": "s << value;
        if(i + 1 < map.size()){
            os << ", "s;
        }
        ++i;
    }
    os << '}';
    return os;
}

vector<string> SplitToWords(const string& line) {
    vector<string> output;
    istringstream iss(line);
    string word;

    while(iss >> word){
        output.push_back(word);
    }

    return output;
}

template <typename A, typename B>
bool IsVectorsAreSimilar(const vector<A>&  vec_a, vector<B> vec_b){
    if(vec_a.size() != vec_a.size()){
        return false;
    }

    sort(vec_b.begin(), vec_b.end());

    for(const A& element: vec_a){
        if(!binary_search(vec_b.begin() , vec_b.end(), element)){
            return false;
        }
    }

    return true;
}


int AverageRating(const vector<int>& ratings) {
    int rating_sum = 0;
    for (const int rating : ratings) {
        rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
}


SearchServer CreateTestServer(){
    SearchServer server;

    server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::ACTUAL, { 9 });
    server.AddDocument(4, "не особо ухоженный пёс арсений"s, DocumentStatus::ACTUAL, { 8 });
    server.AddDocument(5, "черный скворец и модный ошейник"s, DocumentStatus::BANNED, { 8,1,5 });
    server.AddDocument(6, "серый лев красивые лапки"s, DocumentStatus::BANNED, { 9,5,2 });
    server.AddDocument(6, "скворец и ошейник"s, DocumentStatus::IRRELEVANT, { 9,5,2 });

    return server;
}

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    // Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
    // находит нужный документ
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    // Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
    // возвращает пустой результат
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("in"s).empty());
    }
}

//Документы, содержащие минус-слова поискового запроса, не должны включаться в результаты поиска.
void TestExcludeDocumentsWithMinusWords(){
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};

    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("in -the"s).empty());
    }
}

// Матчинг документов. При матчинге документа по поисковому запросу должны быть возвращены все слова из
// поискового запроса, присутствующие в документе. Если есть соответствие хотя бы по одному минус-слову,
// должен возвращаться пустой список слов.
void TestMatchDocument(){
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};

    {
        //Проверяем матчинг всех слов
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        string match_words = "cat in the city"s;
        const auto full_match = server.MatchDocument(match_words , doc_id);
        ASSERT(IsVectorsAreSimilar(get<0>(full_match), SplitToWords(content)));

        //Проверяем матчинг части слов
        match_words = "in the"s;
        const auto part_match = server.MatchDocument(match_words , doc_id);
        ASSERT(IsVectorsAreSimilar(get<0>(part_match), SplitToWords(content)));


        //Проверяем минус слова
        match_words = "in the -city"s;
        const auto match = server.MatchDocument(match_words , doc_id);
        ASSERT_EQUAL(get<0>(match).size(), 0);
    }
}

// Сортировка найденных документов по релевантности.
// Возвращаемые при поиске документов результаты должны быть отсортированы в порядке убывания релевантности.
void TestSortByRelevance() {
    SearchServer server = CreateTestServer();

    vector<int> right_answer{1,2,0};
    const auto top_documents = server.FindTopDocuments("пушистый кот выразительные глаза"s);
    for (size_t i = 0 ; i < top_documents.size() ; ++i) {
        ASSERT_EQUAL(top_documents[i].id, right_answer[i]);
    }
}

// Вычисление рейтинга документов. Рейтинг добавленного документа равен среднему арифметическому оценок документа.
void TestComputeAverageRating(){
    SearchServer server;

    vector<vector<int>>  ratings {{ 7, 2, 7 },{ 5, -12, 2, 1 },{ 8, -3 }};

    server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, ratings[2]);
    server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, ratings[0]);
    server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, ratings[1]);
    server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::ACTUAL, { 9 });
    server.AddDocument(4, "не особо ухоженный пёс арсений"s, DocumentStatus::ACTUAL, { 8 });
    server.AddDocument(5, "черный скворец и модный ошейник"s, DocumentStatus::ACTUAL, { 8,1,5 });
    server.AddDocument(6, "черный скворец и модный ошейник"s, DocumentStatus::BANNED, { 9,5,2 });


    const auto top_documents = server.FindTopDocuments("пушистый кот выразительные глаза"s);
    for(size_t i = 0 ; i < top_documents.size() ; ++i){
        ASSERT_EQUAL(top_documents[i].rating, AverageRating(ratings[i]));
    }
}

// Фильтрация результатов поиска с использованием предиката, задаваемого пользователем.
void TestPredicateFilter(){
    SearchServer server = CreateTestServer();

    vector<int> right_result{1,2,0};
    vector<Document> result = server.FindTopDocuments("пушистый кот выразительные глаза"s,
                                                      [](int document_id, DocumentStatus in_status, int rating) {
                                                          return DocumentStatus::ACTUAL == in_status;
                                                      });
    for(size_t i = 0; i < result.size(); ++i){
        ASSERT_EQUAL(result[i].id, right_result[i]);
    }
}

// Поиск документов, имеющих заданный статус.
void TestFindCorrectStatus(){
    SearchServer server = CreateTestServer();

    vector<int> right_result{1,2,0};
    vector<Document> result = server.FindTopDocuments("пушистый кот выразительные глаза"s, DocumentStatus::ACTUAL);
    for(size_t i = 0; i < result.size(); ++i){
        ASSERT_EQUAL(result[i].id, right_result[i]);
    }

    right_result = {6};
    result = server.FindTopDocuments("пушистый кот красивые лапки"s, DocumentStatus::BANNED);
    for(size_t i = 0; i < result.size(); ++i){
        ASSERT_EQUAL(result[i].id, right_result[i]);
    }
}

// Корректное вычисление релевантности найденных документов.
void TestComputeRelevance(){
    SearchServer server = CreateTestServer();

    vector<double> right_result{1.2861458166514987,0.97295507452765662,0.25055259369907362};
    vector<Document> result = server.FindTopDocuments("пушистый кот выразительные глаза"s, DocumentStatus::ACTUAL);
    for(size_t i = 0; i < result.size(); ++i){
        ASSERT(fabs(result[i].relevance - right_result[i]) < 1e-15);
    }
}

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestExcludeDocumentsWithMinusWords);
    RUN_TEST(TestMatchDocument);
    RUN_TEST(TestSortByRelevance);
    RUN_TEST(TestComputeAverageRating);
    RUN_TEST(TestPredicateFilter);
    RUN_TEST(TestFindCorrectStatus);
    RUN_TEST(TestComputeRelevance);
}