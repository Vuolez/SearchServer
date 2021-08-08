#include "document.h"

std::ostream& operator<<(std::ostream& output, Document& doc) {
    output << "{ "
        << "document_id = " << doc.id << ", "
        << "relevance = " << doc.relevance << ", "
        << "rating = " << doc.rating
        << " }";

    return output;
}