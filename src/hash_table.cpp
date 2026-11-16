#include "project/hash_table.h"

#include <cmath>
#include <stdexcept>
#include <utility>

HashTable::HashTable(std::size_t capacity)
    : table_(capacity), size_(0) {
    if (capacity == 0) {
        throw std::invalid_argument("Hash table capacity must be greater than zero");
    }
    for (std::size_t i = 0; i < table_.size(); ++i) {
        table_[i] = HashEntry();
    }
}

HashTable::~HashTable() = default;

std::size_t HashTable::hashFunction(const std::string& key) const {
    std::size_t hash = 2166136261u;
    for (unsigned char ch : key) {
        hash ^= ch;
        hash *= 16777619u;
    }
    return hash % table_.size();
}

std::size_t HashTable::probe(const std::string& key) const {
    // Quadratic probing: try home, home + 1^2, home + 2^2, ...
    // Stops at the slot holding this key (live or tombstone) or the first never-used slot.
    // Deleted slots are skipped so keys placed after them can still be found.
    // Returns capacity() if no such slot turns up within capacity() probes.
    std::size_t index = hashFunction(key);
    std::size_t original = index;
    std::size_t step = 1;

    while (table_[index].occupied && table_[index].key != key) {
        if (step >= table_.size()) {
            return table_.size();
        }
        index = (original + step * step) % table_.size();
        ++step;
    }
    return index;
}

bool HashTable::insert(const std::string& key, int value) {
    if (contains(key)) {
        return false;
    }

    // Grow before the load factor passes 0.5 so quadratic probing can always find a free slot.
    if (static_cast<double>(size_ + 1) / static_cast<double>(table_.size()) > 0.5) {
        rehash();
    }

    std::size_t index = probe(key);
    while (index == table_.size()) {
        rehash();
        index = probe(key);
    }

    // Either a never-used slot or this key's old tombstone.
    table_[index] = HashEntry(key, value, true, false);
    ++size_;
    return true;
}

bool HashTable::remove(const std::string& key) {
    if (!contains(key)) {
        return false;
    }

    // Leave a tombstone instead of clearing the slot, so later keys in the probe chain stay reachable.
    table_[probe(key)].deleted = true;
    --size_;
    return true;
}

bool HashTable::contains(const std::string& key) const {
    const std::size_t index = probe(key);
    return index < table_.size() && table_[index].occupied && !table_[index].deleted;
}

int HashTable::get(const std::string& key) const {
    if (!contains(key)) {
        throw std::out_of_range("Key not found: " + key);
    }
    return table_[probe(key)].value;
}

bool HashTable::update(const std::string& key, int newValue) {
    if (!contains(key)) {
        return false;
    }
    table_[probe(key)].value = newValue;
    return true;
}

std::size_t HashTable::size() const noexcept {
    return size_;
}

std::size_t HashTable::capacity() const noexcept {
    return table_.size();
}

bool HashTable::empty() const noexcept {
    return size_ == 0;
}

double HashTable::loadFactor() const noexcept {
    return static_cast<double>(size_) / static_cast<double>(table_.size());
}

std::vector<std::string> HashTable::keys() const {
    std::vector<std::string> result;
    for (const auto& entry : table_) {
        if (entry.occupied && !entry.deleted) {
            result.push_back(entry.key);
        }
    }
    return result;
}

std::vector<int> HashTable::values() const {
    std::vector<int> result;
    for (const auto& entry : table_) {
        if (entry.occupied && !entry.deleted) {
            result.push_back(entry.value);
        }
    }
    return result;
}

void HashTable::rehash() {
    // Move every live entry into a table about twice as big (next prime).
    // Tombstones are dropped, so the new table starts clean.
    std::vector<HashEntry> oldTable = std::move(table_);
    table_ = std::vector<HashEntry>(nextPrime(oldTable.size() * 2 + 1));
    size_ = 0;

    for (const auto& entry : oldTable) {
        if (entry.occupied && !entry.deleted) {
            table_[probe(entry.key)] = HashEntry(entry.key, entry.value, true, false);
            ++size_;
        }
    }
}

bool HashTable::isPrime(std::size_t value) const {
    if (value < 2) {
        return false;
    }
    if (value == 2) {
        return true;
    }
    if (value % 2 == 0) {
        return false;
    }
    for (std::size_t i = 3; i * i <= value; i += 2) {
        if (value % i == 0) {
            return false;
        }
    }
    return true;
}

std::size_t HashTable::nextPrime(std::size_t value) const {
    if (value <= 2) {
        return 2;
    }
    std::size_t candidate = value % 2 == 0 ? value + 1 : value;
    while (!isPrime(candidate)) {
        candidate += 2;
    }
    return candidate;
}
