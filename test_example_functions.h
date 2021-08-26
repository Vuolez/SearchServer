#pragma once

#include "search_server.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <sstream>

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str, const std::string& u_str, const std::string& file,
    const std::string& func, unsigned line, const std::string& hint) {
    if (t != u) {
        std::cerr << std::boolalpha;
        std::cerr << file << "(" << line << "): " << func << ": "
            << "ASSERT_EQUAL(" << t_str << ", " << u_str << ") failed: "
            << t << " != " << u << ".";
        if (!hint.empty()) {
            std::cerr << " Hint: " << hint;
        }
        std::cerr << std::endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, "")

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func, unsigned line,
    const std::string& hint);

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, "")

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename Func>
void RunTestImpl(Func func, const std::string& func_name) {
    func();
    std::cout << func_name << " OK" << std::endl;
}

#define RUN_TEST(func)  RunTestImpl(func, (#func))

template <typename Key, typename Value>
std::ostream& operator<<(std::ostream& out, const std::pair<Key, Value> element) {
    out << '(' << element.first << ", " << element.second << ')';
    return out;
}

template <typename Elements>
void Print(std::ostream& out, const Elements& elements) {
    bool is_first = true;
    for (const auto& element : elements) {
        if (is_first) {
            out << element;
            is_first = false;
        }
        else {
            out << ", " << element;
        }
    }
}

template <typename Elements>
std::ostream& operator<<(std::ostream& out, const std::vector<Elements>& elements) {
    out << '[';
    Print(out, elements);
    out << ']';
    return out;
}

template <typename Elements>
std::ostream& operator<<(std::ostream& out, const std::set<Elements>& elements) {
    out << '{';
    Print(out, elements);
    out << '}';
    return out;
}

template <typename Key, typename Value>
std::ostream& operator<<(std::ostream& os, const std::map<Key, Value>& map) {
    os << '{';
    size_t i = 0;
    for (const auto& [key, value] : map) {
        os << key << ": " << value;
        if (i + 1 < map.size()) {
            os << ", ";
        }
        ++i;
    }
    os << '}';
    return os;
}


template <typename A, typename B>
bool IsVectorsAreSimilar(const std::vector<A>& vec_a, std::vector<B> vec_b) {
    if (vec_a.size() != vec_a.size()) {
        return false;
    }

    sort(vec_b.begin(), vec_b.end());

    for (const A& element : vec_a) {
        if (!binary_search(vec_b.begin(), vec_b.end(), element)) {
            return false;
        }
    }

    return true;
}

std::vector<std::string> SplitToWords(const std::string& line);

int AverageRating(const std::vector<int>& ratings);

SearchServer CreateTestServer();

// ���� ���������, ��� ��������� ������� ��������� ����-����� ��� ���������� ����������
void TestExcludeStopWordsFromAddedDocumentContent();

//���������, ���������� �����-����� ���������� �������, �� ������ ���������� � ���������� ������.
void TestExcludeDocumentsWithMinusWords();

// ������� ����������. ��� �������� ��������� �� ���������� ������� ������ ���� ���������� ��� ����� ��
// ���������� �������, �������������� � ���������. ���� ���� ������������ ���� �� �� ������ �����-�����,
// ������ ������������ ������ ������ ����.
void TestMatchDocument();

// ���������� ��������� ���������� �� �������������.
// ������������ ��� ������ ���������� ���������� ������ ���� ������������� � ������� �������� �������������.
void TestSortByRelevance();

// ���������� �������� ����������. ������� ������������ ��������� ����� �������� ��������������� ������ ���������.
void TestComputeAverageRating();

// ���������� ����������� ������ � �������������� ���������, ����������� �������������.
void TestPredicateFilter();

// ����� ����������, ������� �������� ������.
void TestFindCorrectStatus();

// ���������� ���������� ������������� ��������� ����������.
void TestComputeRelevance();

void TestRemoveDuplicates();

// ������� TestSearchServer �������� ������ ����� ��� ������� ������
void TestSearchServer();