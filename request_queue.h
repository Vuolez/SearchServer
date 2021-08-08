#pragma once

#include "search_server.h"
#include <deque>

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        const auto t = server_.FindTopDocuments(raw_query, document_predicate);
        AddResult(raw_query, t.size());
        return t;
    }

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    
    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;

private:
    const SearchServer& server_;

    struct QueryResult {
        std::string query;
        size_t results;
    };

    std::deque<QueryResult> requests_;
    const static int sec_in_day_ = 1440;
    
    void CheckResults();
    void AddResult(const std::string& raw_query, size_t query_res);
};