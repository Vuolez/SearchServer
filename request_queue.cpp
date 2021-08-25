#include "request_queue.h"
#include "search_server.h"


RequestQueue::RequestQueue(const SearchServer& search_server) :
    server_(search_server)
{
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    const auto t = server_.FindTopDocuments(raw_query, status);
    AddResult(raw_query, t.size());
    return t;
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    const auto t = server_.FindTopDocuments(raw_query);
    AddResult(raw_query, t.size());
    return t;
}

int RequestQueue::GetNoResultRequests() const {
    size_t qty = 0;
    for (const auto r : requests_){
        if (r.results == 0){
            ++qty;
        }
    }
    return qty;
}

void RequestQueue::CheckResults(){
    if (requests_.size() > sec_in_day_){
        while (requests_.size() != sec_in_day_){
            requests_.pop_front();
        }
    }
}
void RequestQueue::AddResult(const std::string& raw_query, size_t query_res){
    requests_.push_back({ raw_query,query_res });
    CheckResults();
}