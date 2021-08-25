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
    const string& hint);

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename Func>
void RunTestImpl(Func func, const string& func_name) {
    func();
    cout << func_name << " OK"s << endl;
}

#define RUN_TEST(func)  RunTestImpl(func, (#func))

template <typename Key, typename Value>
ostream& operator<<(ostream& out, const pair<Key, Value> element) {
    out << '(' << element.first << ", "s << element.second << ')';
    return out;
}

template <typename Elements>
void Print(ostream& out, const Elements& elements) {
    bool is_first = true;
    for (const auto& element : elements) {
        if (is_first) {
            out << element;
            is_first = false;
        }
        else {
            out << ", "s << element;
        }
    }
}

template <typename Elements>
ostream& operator<<(ostream& out, const vector<Elements>& elements) {
    out << '[';
    Print(out, elements);
    out << ']';
    return out;
}

template <typename Elements>
ostream& operator<<(ostream& out, const set<Elements>& elements) {
    out << '{';
    Print(out, elements);
    out << '}';
    return out;
}

template <typename Key, typename Value>
ostream& operator<<(ostream& os, const map<Key, Value>& map) {
    os << '{';
    size_t i = 0;
    for (const auto& [key, value] : map) {
        os << key << ": "s << value;
        if (i + 1 < map.size()) {
            os << ", "s;
        }
        ++i;
    }
    os << '}';
    return os;
}


template <typename A, typename B>
bool IsVectorsAreSimilar(const vector<A>& vec_a, vector<B> vec_b) {
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

vector<string> SplitToWords(const string& line);

int AverageRating(const vector<int>& ratings);

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