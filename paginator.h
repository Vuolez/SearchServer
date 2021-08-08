#pragma once

#include <vector>
#include <algorithm>
#include <iostream>

template<typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end)
            : begin_(begin), end_(end) {
        size_ = end_ - begin_;

    }

    Iterator begin() {
        return begin_;
    }

    Iterator end() {
        return end_;
    }

    size_t size() {
        return size_;
    }

private:
    Iterator begin_;
    Iterator end_;
    size_t size_;
};

template <typename Iterator>
std::ostream& operator<<(std::ostream& output, IteratorRange<Iterator> it) {
    for (auto data = it.begin(); data != it.end(); ++data) {
        std::vector doc(data, data + 1);
        for (auto& i : doc) {
            std::cout << i;
        }

    }

    return output;
}


template<typename Iterator>
class Paginator {
public:
    Paginator(Iterator begin, Iterator end, size_t page_size) {
        for (size_t left = distance(begin, end); left > 0;) {
            const size_t current_page_size = std::min(page_size, left);
            const Iterator current_page_end = next(begin, current_page_size);
            pages_.push_back({begin, current_page_end});

            left -= current_page_size;
            begin = current_page_end;
        }
    }

    auto begin() const {
        return pages_.begin();
    }

    auto end() const {
        return pages_.end();
    }
private:
    std::vector<IteratorRange<Iterator>> pages_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}