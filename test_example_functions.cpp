#include "test_example_functions.h"
#include "search_server.h"
#include "remove_duplicates.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <sstream>

void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func, unsigned line,
    const std::string& hint) {
    if (!value) {
        std::cerr << file << "(" << line << "): " << func << ": "
            << "ASSERT(" << expr_str << ") failed.";
        if (!hint.empty()) {
            std::cerr << " Hint: " << hint;
        }
        std::cerr << std::endl;
        abort();
    }
}

std::vector<std::string> SplitToWords(const std::string& line) {
    std::vector<std::string> output;
    std::istringstream iss(line);
    std::string word;

    while (iss >> word) {
        output.push_back(word);
    }

    return output;
}

int AverageRating(const std::vector<int>& ratings) {
    int rating_sum = 0;
    for (const int rating : ratings) {
        rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer CreateTestServer() {
    SearchServer server;

    server.AddDocument(0, "����� ��� � ������ �������", DocumentStatus::ACTUAL, { 8, -3 });
    server.AddDocument(1, "�������� ��� �������� �����", DocumentStatus::ACTUAL, { 7, 2, 7 });
    server.AddDocument(2, "��������� �� ������������� �����", DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    server.AddDocument(3, "��������� ������� �������", DocumentStatus::ACTUAL, { 9 });
    server.AddDocument(4, "�� ����� ��������� �� �������", DocumentStatus::ACTUAL, { 8 });
    server.AddDocument(5, "������ ������� � ������ �������", DocumentStatus::BANNED, { 8,1,5 });
    server.AddDocument(6, "������� � �������", DocumentStatus::IRRELEVANT, { 9,5,2 });

    return server;
}

// ���� ���������, ��� ��������� ������� ��������� ����-����� ��� ���������� ����������
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const std::string content = "cat in the city";
    const std::vector<int> ratings = { 1, 2, 3 };
    // ������� ����������, ��� ����� �����, �� ��������� � ������ ����-����,
    // ������� ������ ��������
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in");
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    // ����� ����������, ��� ����� ����� �� �����, ��������� � ������ ����-����,
    // ���������� ������ ���������
    {
        SearchServer server;
        server.SetStopWords("in the");
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("in").empty());
    }
}

//���������, ���������� �����-����� ���������� �������, �� ������ ���������� � ���������� ������.
void TestExcludeDocumentsWithMinusWords() {
    const int doc_id = 42;
    const std::string content = "cat in the city";
    const std::vector<int> ratings = { 1, 2, 3 };

    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("in -the").empty());
    }
}

// ������� ����������. ��� �������� ��������� �� ���������� ������� ������ ���� ���������� ��� ����� ��
// ���������� �������, �������������� � ���������. ���� ���� ������������ ���� �� �� ������ �����-�����,
// ������ ������������ ������ ������ ����.
void TestMatchDocument() {
    const int doc_id = 42;
    const std::string content = "cat in the city";
    const std::vector<int> ratings = { 1, 2, 3 };

    {
        //��������� ������� ���� ����
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        std::string match_words = "cat in the city";
        const auto full_match = server.MatchDocument(match_words, doc_id);
        ASSERT(IsVectorsAreSimilar(std::get<0>(full_match), SplitToWords(content)));

        //��������� ������� ����� ����
        match_words = "in the";
        const auto part_match = server.MatchDocument(match_words, doc_id);
        ASSERT(IsVectorsAreSimilar(std::get<0>(part_match), SplitToWords(content)));


        //��������� ����� �����
        match_words = "in the -city";
        const auto match = server.MatchDocument(match_words, doc_id);
        ASSERT_EQUAL(std::get<0>(match).size(), 0);
    }
}

// ���������� ��������� ���������� �� �������������.
// ������������ ��� ������ ���������� ���������� ������ ���� ������������� � ������� �������� �������������.
void TestSortByRelevance() {
    SearchServer server = CreateTestServer();

    std::vector<int> right_answer{ 1,2,0 };
    const auto top_documents = server.FindTopDocuments("�������� ��� ������������� �����");
    for (size_t i = 0; i < top_documents.size(); ++i) {
        ASSERT_EQUAL(top_documents[i].id, right_answer[i]);
    }
}

// ���������� �������� ����������. ������� ������������ ��������� ����� �������� ��������������� ������ ���������.
void TestComputeAverageRating() {
    SearchServer server;

    std::vector<std::vector<int>>  ratings{ { 7, 2, 7 },{ 5, -12, 2, 1 },{ 8, -3 } };

    server.AddDocument(0, "����� ��� � ������ �������", DocumentStatus::ACTUAL, ratings[2]);
    server.AddDocument(1, "�������� ��� �������� �����", DocumentStatus::ACTUAL, ratings[0]);
    server.AddDocument(2, "��������� �� ������������� �����", DocumentStatus::ACTUAL, ratings[1]);
    server.AddDocument(3, "��������� ������� �������", DocumentStatus::ACTUAL, { 9 });
    server.AddDocument(4, "�� ����� ��������� �� �������", DocumentStatus::ACTUAL, { 8 });
    server.AddDocument(5, "������ ������� � ������ �������", DocumentStatus::ACTUAL, { 8,1,5 });
    server.AddDocument(6, "������ ������� � ������ �������", DocumentStatus::BANNED, { 9,5,2 });


    const auto top_documents = server.FindTopDocuments("�������� ��� ������������� �����");
    for (size_t i = 0; i < top_documents.size(); ++i) {
        ASSERT_EQUAL(top_documents[i].rating, AverageRating(ratings[i]));
    }
}

// ���������� ����������� ������ � �������������� ���������, ����������� �������������.
void TestPredicateFilter() {
    SearchServer server = CreateTestServer();

    std::vector<int> right_result{ 1,2,0 };
    std::vector<Document> result = server.FindTopDocuments("�������� ��� ������������� �����",
        [](int document_id, DocumentStatus in_status, int rating) {
            return DocumentStatus::ACTUAL == in_status;
        });
    for (size_t i = 0; i < result.size(); ++i) {
        ASSERT_EQUAL(result[i].id, right_result[i]);
    }
}

// ����� ����������, ������� �������� ������.
void TestFindCorrectStatus() {
    SearchServer server = CreateTestServer();

    std::vector<int> right_result{ 1,2,0 };
    std::vector<Document> result = server.FindTopDocuments("�������� ��� ������������� �����", DocumentStatus::ACTUAL);
    for (size_t i = 0; i < result.size(); ++i) {
        ASSERT_EQUAL(result[i].id, right_result[i]);
    }

    right_result = { 6 };
    result = server.FindTopDocuments("�������� ��� �������� �����", DocumentStatus::BANNED);
    for (size_t i = 0; i < result.size(); ++i) {
        ASSERT_EQUAL(result[i].id, right_result[i]);
    }
}

// ���������� ���������� ������������� ��������� ����������.
void TestComputeRelevance() {
    SearchServer server = CreateTestServer();

    std::vector<double> right_result{ 1.2861458166514987,0.97295507452765662,0.25055259369907362 };
    std::vector<Document> result = server.FindTopDocuments("�������� ��� ������������� �����", DocumentStatus::ACTUAL);
    for (size_t i = 0; i < result.size(); ++i) {
        ASSERT(std::fabs(result[i].relevance - right_result[i]) < 1e-15);
    }
}

void TestRemoveDuplicates() {
    std::string stop_words = "and with";
    SearchServer search_server(stop_words);

    search_server.AddDocument(1, "funny pet and nasty rat", DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "funny pet with curly hair", DocumentStatus::ACTUAL, { 1, 2 });

    // �������� ��������� 2, ����� �����
    search_server.AddDocument(3, "funny pet with curly hair", DocumentStatus::ACTUAL, { 1, 2 });

    // ������� ������ � ����-������, ������� ����������
    search_server.AddDocument(4, "funny pet and curly hair", DocumentStatus::ACTUAL, { 1, 2 });

    // ��������� ���� ����� ��, ������� ���������� ��������� 1
    search_server.AddDocument(5, "funny funny pet and nasty nasty rat", DocumentStatus::ACTUAL, { 1, 2 });

    // ���������� ����� �����, ���������� �� ��������
    search_server.AddDocument(6, "funny pet and not very nasty rat", DocumentStatus::ACTUAL, { 1, 2 });

    // ��������� ���� ����� ��, ��� � id 6, �������� �� ������ �������, ������� ����������
    search_server.AddDocument(7, "very nasty rat and not very funny pet", DocumentStatus::ACTUAL, { 1, 2 });

    // ���� �� ��� �����, �� �������� ����������
    search_server.AddDocument(8, "pet with rat and rat and rat", DocumentStatus::ACTUAL, { 1, 2 });

    // ����� �� ������ ����������, �� �������� ����������
    search_server.AddDocument(9, "nasty rat with curly hair", DocumentStatus::ACTUAL, { 1, 2 });

    std::vector<int> right_result{ 1,2,6,8,9 };
    RemoveDuplicates(search_server);
    size_t index = 0;
    for (const int id : search_server) {
        ASSERT_EQUAL(id, right_result[index]);
        ++index;
    }
}

// ������� TestSearchServer �������� ������ ����� ��� ������� ������
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestExcludeDocumentsWithMinusWords);
    RUN_TEST(TestMatchDocument);
    RUN_TEST(TestSortByRelevance);
    RUN_TEST(TestComputeAverageRating);
    RUN_TEST(TestPredicateFilter);
    RUN_TEST(TestFindCorrectStatus);
    RUN_TEST(TestComputeRelevance);
    RUN_TEST(TestRemoveDuplicates);
}