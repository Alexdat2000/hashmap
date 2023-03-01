#include <algorithm>
#include <functional>
#include <memory>
#include <stdexcept>
#include <list>


template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {

public:
    typedef typename std::list<std::pair<const KeyType, ValueType>> lst;
    typedef typename lst::iterator iterator;
    typedef typename lst::const_iterator const_iterator;

    HashMap(Hash hasher = Hash(), size_t cap = 11): hasher_(hasher) {
        init(cap);
    }

    HashMap(const HashMap& other): hasher_(other.hasher_) {
        cap_ = std::max(11ul, other.size_ * 3);
        make_new_arrays();
        for (auto [key, val] : other) {
            insert(key, val);
        }
    }

    template<class Iterator>
    HashMap(const Iterator &begin, const Iterator &end, Hash hasher=Hash()): hasher_(hasher) {
        init();
        Iterator it = begin;
        while (it != end) {
            insert(it->first, it->second);
            it++;
        }
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> list, Hash hasher = Hash()): hasher_(hasher) {
        init();
        auto it = list.begin();
        while (it != list.end()) {
            insert(it->first, it->second);
            it++;
        }
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    Hash hash_function() const {
        return hasher_;
    }

    size_t insert(KeyType key, ValueType val) {
        size_t hash = hasher_(key);
        size_t pos = hash % cap_;

        elem_list_.emplace_back(key, val);
        iterator now = std::prev(elem_list_.end());
        size_t step_now = 1;

        while (true) {
            if (move_cnt_[pos] < step_now) {
                std::swap(list_iters_[pos], now);
                std::swap(move_cnt_[pos], step_now);
                if (step_now == 0) {
                    break;
                }
            }
            step_now++;
            pos++;
            if (pos == cap_) {
                pos = 0;
            }
        }
        size_++;

        if (size_ * 2 > cap_) {
            rebuild();
        }
        return find_pos(key);
    }

    size_t insert(const std::pair<KeyType, ValueType> pr) {
        return insert(pr.first, pr.second);
    }

    void erase(KeyType key) {
        size_t pos = find_pos(key);
        if (move_cnt_[pos] == 0) {
            return;
        }
        elem_list_.erase(list_iters_[pos]);
        move_cnt_[pos] = 0;
        size_t next = pos + 1;
        if (next == cap_) {
            next = 0;
        }
        while (move_cnt_[next] > 1) {
            move_cnt_[pos] = move_cnt_[next] - 1;
            move_cnt_[next] = 0;
            list_iters_[pos] = list_iters_[next];
            pos++, next++;
            if (pos == cap_) {
                pos = 0;
            }
            if (next == cap_) {
                next = 0;
            }
        }
        size_--;
    }

    void clear() {
        auto elems = elem_list_;
        for (auto [key, val]: elems) {
            erase(key);
        }
    }

    ValueType &operator[](const KeyType &key) {
        size_t pos = find_pos(key);
        if (move_cnt_[pos] == 0) {
            pos = insert(key, ValueType());
        }
        return list_iters_[pos]->second;
    }

    const ValueType &at(const KeyType key) const {
        size_t pos = find_pos(key);
        if (move_cnt_[pos] == 0) {
            throw std::out_of_range("Element not found");
        }
        return list_iters_[pos]->second;
    }

    iterator begin() {
        return elem_list_.begin();
    }

    const_iterator begin() const {
        return elem_list_.begin();
    }

    iterator end() {
        return elem_list_.end();
    }

    const_iterator end() const {
        return elem_list_.end();
    }

    iterator find(const KeyType &key) {
        size_t pos = find_pos(key);
        if (move_cnt_[pos] == 0) {
            return elem_list_.end();
        }
        return list_iters_[pos];
    }

    const_iterator find(const KeyType &key) const {
        size_t pos = find_pos(key);
        if (move_cnt_[pos] == 0) {
            return elem_list_.end();
        }
        return list_iters_[pos];
    }

    HashMap operator=(const HashMap& other) {
        clear();
        hasher_ = other.hash_function();
        cap_ = std::max(11ul, other.size_ * 3);
        make_new_arrays();
        auto it = other.begin();
        while (it != other.end()) {
            insert(it->first, it->second);
            it++;
        }
        return *this;
    }

    ~HashMap() {
        delete[] move_cnt_;
        delete[] list_iters_;
    }
private:
    Hash hasher_;
    size_t *move_cnt_ = nullptr;
    lst elem_list_;
    typename lst::iterator *list_iters_ = nullptr;
    size_t cap_ = 0, size_ = 0;

    bool is_prime(size_t num) {
        for (size_t i = 2; i * i <= num; i++) {
            if (num % i == 0) {
                return false;
            }
        }
        return true;
    }

    void init(size_t cap = 11) {
        cap_ = cap;
        make_new_arrays();
    }

    void make_new_arrays() {
        delete[] move_cnt_;
        delete[] list_iters_;
        size_ = 0;
        move_cnt_ = new size_t[cap_];
        list_iters_ = new iterator[cap_];
        std::fill(move_cnt_, move_cnt_ + cap_, 0);
    }

    void rebuild() {
        cap_ = cap_ * 2 + 1;
        while (!is_prime(cap_)) {
            cap_ += 2;
        }

        make_new_arrays();
        auto elems = elem_list_;
        elem_list_.clear();
        auto it = elems.begin();
        while (it != elems.end()) {
            insert(it->first, it->second);
            it++;
        }
    }

    size_t find_pos(const KeyType &key) const {
        size_t pos = hasher_(key) % cap_;
        while (move_cnt_[pos] != 0 && !(list_iters_[pos]->first == key)) {
            pos++;
            if (pos == cap_) {
                pos = 0;
            }
        }
        return pos;
    }
};
