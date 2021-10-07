#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

template <typename Type>
class SingleLinkedList {
    // Узел списка
    struct Node {
        Node() = default;
        Node(const Type& val, Node* next)
            : value(val)
            , next_node(next) {
        }
        Type value{};
        Node* next_node = nullptr;
    };
    template <typename ValueType>
    class BasicIterator {
        friend class SingleLinkedList;
        explicit BasicIterator(Node* node) {
            node_ = node;
        }
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Type;
        using difference_type = std::ptrdiff_t;
        using pointer = ValueType*;
        using reference = ValueType&;

        BasicIterator() = default;
        BasicIterator(const BasicIterator<Type>& other) noexcept {
            node_ = other.node_;
        }
        BasicIterator& operator=(const BasicIterator& rhs) = default;
        [[nodiscard]] bool operator==(const BasicIterator<const Type>& rhs) const noexcept {
            return (node_ == rhs.node_);
        }
        [[nodiscard]] bool operator!=(const BasicIterator<const Type>& rhs) const noexcept {
            return (node_ != rhs.node_);
        }
        [[nodiscard]] bool operator==(const BasicIterator<Type>& rhs) const noexcept {
            return (node_ == rhs.node_);
        }
        [[nodiscard]] bool operator!=(const BasicIterator<Type>& rhs) const noexcept {
            return (node_ != rhs.node_);
        }
        BasicIterator& operator++() noexcept {
            assert(node_ != nullptr);
            node_ = node_->next_node;
            return *this;
        }
        BasicIterator operator++(int) noexcept {
            auto prev_node = node_;
            node_ = node_->next_node;
            return BasicIterator(prev_node);
        }
        [[nodiscard]] reference operator*() const noexcept {
            assert(node_ != nullptr);
            return node_->value;
        }
        [[nodiscard]] pointer operator->() const noexcept {
            assert(node_ != nullptr);
            return &node_->value;
        }

    private:
        Node* node_ = nullptr;
    };
public:
    using value_type = Type;
    using reference = value_type&;
    using const_reference = const value_type&;

    using Iterator = BasicIterator<Type>;
    using ConstIterator = BasicIterator<const Type>;

    //Конструктор по умолчанию, который создаёт пустой список
    SingleLinkedList() :head_(Node()), size_(0) {}
    ~SingleLinkedList() {
        Clear();
    }

    SingleLinkedList(std::initializer_list<Type> values) : head_(Node()), size_(0) {
        SingleLinkedList temp;
        for (auto it = std::rbegin(values); it != std::rend(values); ++it) {
            temp.PushFront(*it);
        }

        swap(temp);
    }

    SingleLinkedList(const SingleLinkedList& other) : head_(Node()), size_(0) {
        SingleLinkedList temp;

        Node* current_node = &temp.head_;
        for (const Type& value : other) {
            current_node->next_node = new Node(value, nullptr);
            current_node = current_node->next_node;
            ++temp.size_;
        }

        swap(temp);
    }

    SingleLinkedList& operator=(const SingleLinkedList& rhs) {
        SingleLinkedList temp(rhs);

        if (!IsEmpty()) {
            Clear();
        }
        swap(temp);

        return *this;
    }

    // Обменивает содержимое списков за время O(1)
    void swap(SingleLinkedList& other) noexcept {
        Node* node = head_.next_node;
        head_.next_node = other.head_.next_node;
        other.head_.next_node = node;

        std::swap(size_, other.size_);
    }




    [[nodiscard]] Iterator begin() noexcept {
        return Iterator(head_.next_node);
    }
    [[nodiscard]] Iterator end() noexcept {
        return {};
    }
    [[nodiscard]] ConstIterator begin() const noexcept {
        return ConstIterator{ head_.next_node };
    }
    [[nodiscard]] ConstIterator end() const noexcept {
        return ConstIterator{};
    }
    [[nodiscard]] ConstIterator cbegin() const noexcept {
        return begin();
    }
    [[nodiscard]] ConstIterator cend() const noexcept {
        return ConstIterator{};
    }
    [[nodiscard]] Iterator before_begin() noexcept {
        return Iterator(&head_);
    }
    [[nodiscard]] ConstIterator cbefore_begin() const noexcept {
        return ConstIterator(const_cast<Node*>(&head_));
    }
    [[nodiscard]] size_t GetSize() const noexcept {
        return size_;
    }
    [[nodiscard]] bool IsEmpty() const noexcept {
        return size_ == 0;
    }


    /*
     * Вставляет элемент value после элемента, на который указывает pos.
     * Возвращает итератор на вставленный элемент
     * Если при создании элемента будет выброшено исключение, список останется в прежнем состоянии
     */
    Iterator InsertAfter(ConstIterator pos, const Type& value) {
        pos.node_->next_node = new Node(value, pos.node_->next_node);
        ++size_;
        return Iterator{ pos.node_->next_node };
    }

    void PopFront() noexcept {
        if (!IsEmpty()) {
            auto temp = head_.next_node;
            head_.next_node = head_.next_node->next_node;
            delete temp;
            --size_;
        }
    }

    /*
     * Удаляет элемент, следующий за pos.
     * Возвращает итератор на элемент, следующий за удалённым
     */
    Iterator EraseAfter(ConstIterator pos) noexcept {
        if (!pos.node_ || !pos.node_->next_node) {
            return end();
        }

        auto temp = pos.node_->next_node;
        pos.node_->next_node = pos.node_->next_node->next_node;
        delete temp;

        --size_;
        return Iterator{ pos.node_->next_node };
    }

    void PushFront(const Type& value) {
        head_.next_node = new Node(value, head_.next_node);
        ++size_;
    }

    Type GetHead() {
        return head_.value;
    }
    Type GetHeadNext() {
        return head_.next_node->value;
    }

    void Clear() noexcept {
        while (!IsEmpty()) {
            PopFront();
        }
    }
private:
    // Фиктивный узел, используется для вставки "перед первым элементом"
    Node head_;
    size_t size_;
};




template <typename Type>
void swap(SingleLinkedList<Type>& lhs, SingleLinkedList<Type>& rhs) noexcept {
    lhs.swap(rhs);
}

template <typename Type>
bool operator==(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
bool operator!=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
bool operator<(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
bool operator<=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return (lhs < rhs || lhs == rhs);
}

template <typename Type>
bool operator>(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return !(lhs <= rhs);
}

template <typename Type>
bool operator>=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return (lhs == rhs || lhs > rhs);
}

template <typename Type>
void PrintList(const SingleLinkedList<Type>& _list) {
    for (const Type value : _list) {
        std::cout << value << ' ';
    }
    std::cout << std::endl;
}