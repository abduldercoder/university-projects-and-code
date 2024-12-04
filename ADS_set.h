#ifndef ADS_SET_H
#define ADS_SET_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>

template <typename Key, size_t N = 7>
class ADS_set {
public:
    class Iterator;
    using value_type = Key;
    using key_type = Key;
    using reference = value_type &;
    using const_reference = const value_type &;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using const_iterator = Iterator;
    using iterator = const_iterator;
    using key_compare = std::less<key_type>;                         // B+-Tree
    using key_equal = std::equal_to<key_type>;                       // Hashing
    using hasher = std::hash<key_type>;                              // Hashing
private:
    struct Element{
        key_type key;
        Element *next {nullptr};
        Element(const key_type& key, Element* next): key(key), next(next){}
    };
    Element **table = new Element*[N] {nullptr};
    size_type elementSize {0};
    size_type tableSize {N};
    float lf {0.5};
    void add(const key_type& key);
    Element *locate(const key_type& key) const;
    size_type hash_func(const key_type& key)  const {return hasher{}(key) % tableSize;}
    void resize();
    
   

public:
    ADS_set() {}                                                                           // PH1
    ADS_set(std::initializer_list<key_type> ilist) { insert(ilist); }                       // PH1
    template<typename InputIt> ADS_set(InputIt first, InputIt last) { insert(first, last);}  // PH1
    ADS_set(const ADS_set &other)  {
        tableSize = other.tableSize;
        elementSize = 0;
        delete[] table;
        table = new Element*[tableSize]{nullptr};
        for(size_type i = 0; i < other.tableSize; ++i){
            for(Element* e = other.table[i]; e != nullptr; e = e->next){
                add(e->key);
            }
        }
    } //PH2
    ~ADS_set(){
        clear();
        delete[] table;
    } //PH2

   
    ADS_set &operator=(std::initializer_list<key_type> ilist){
        ADS_set temp(ilist);
        swap(temp);
        return *this;
    } //PH2


 ADS_set &operator=(const ADS_set &other){
        ADS_set temp(other);
        swap(temp);
        return *this;
    } //PH2


    size_type size() const  { return elementSize;}                                                   // PH1
    bool empty() const {  return elementSize == 0; }                                                      // PH1

    void insert(std::initializer_list<key_type> ilist) { insert(ilist.begin(), ilist.end());}        // PH1
    std::pair<iterator,bool> insert(const key_type &key){
        Element* e = locate(key);
        if(e != nullptr){ return {iterator{e, table, tableSize, hash_func(key)}, false};}
        add(key);
        return {iterator{locate(key), table, tableSize, hash_func(key)}, true};
    } //PH2
    template<typename InputIt> void insert(InputIt first, InputIt last){
        for(; first != last; ++first){
            add(*first);
        }
    }                                                                                               // PH1

    void clear(){
        for(size_type i = 0; i < tableSize; ++i){
            while (table[i] != nullptr){
                Element *next = table[i]->next;
                delete table[i];
                table[i] = next;
            }
        }
        elementSize = 0;
    } 
    size_type erase(const key_type &key){
         size_type index = hash_func(key);
         Element *prev = nullptr;
         Element *e = table[index];
         while (e != nullptr && !key_equal{}(e->key, key)) {
             prev = e;
             e = e->next;
         }
         if (e == nullptr) return 0;
         if (prev == nullptr) {
             table[index] = e->next;
         } else {
             prev->next = e->next;
         }
         delete e;
         --elementSize;
         return 1;
     }
    

    size_type count(const key_type &key) const {
        return locate(key) != nullptr;
    }                                                                                           // PH1
    iterator find(const key_type &key) const{
        const auto e = locate(key);
        if(e != nullptr) { return iterator{e, table, tableSize, hash_func(key)}; }
        return end();
    }

    void swap(ADS_set &other) {
        using std::swap;
        std::swap(table, other.table);
        std::swap(elementSize, other.elementSize);
        std::swap(tableSize, other.tableSize);
        std::swap(lf, other.lf);
    }

    const_iterator begin() const{
        for (size_type i = 0; i < tableSize; ++i) {
            if (table[i] != nullptr) {
                return const_iterator{table[i], table, tableSize, i};
            }
        }
        return end();
    } 
    const_iterator end() const{
        return const_iterator{nullptr, table, tableSize, tableSize};
    } 

   

    friend bool operator==(const ADS_set &lhs, const ADS_set &rhs){
        if(lhs.elementSize != rhs.elementSize) return false;
        for(const auto& it : lhs){
            if(!rhs.count(it)) return false;
        }
        return true;
    } 
    friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs){
        return !(lhs == rhs);
    } 

  





};
template <typename Key, size_t N>
void ADS_set<Key, N>::dump(std::ostream& o ) const {
    for (size_type i = 0; i < tableSize; ++i) {
        o << "[" << i << "] ";
        for (Element* e = table[i]; e != nullptr; e = e->next) {
            o << " -> " << e->key;
        }
        o << " -> nullptr\n";
    }
    o << "Size: " << elementSize << ", TableSize: " << tableSize << "\n";
}



//New implemented functions
template <typename Key, size_t N>
void ADS_set<Key,N>::add(const key_type &key){
    size_type index = hash_func(key);
    if (locate(key) == nullptr) {
        table[index] = new Element(key, table[index]);
        ++elementSize;
        if (static_cast<float>(elementSize) / tableSize > lf) {
            resize();
        }
    }
}

template <typename Key, size_t N>
void ADS_set<Key, N>::resize() {
    size_type newTableSize = (tableSize * 6)+1;
    Element** new_table = new Element*[newTableSize]{nullptr};

    for (size_type i = 0; i < tableSize; ++i) {
        Element* current = table[i];
        while (current != nullptr) {
            size_type newIndex = hasher{}(current->key) % newTableSize;
            Element* temp = current->next;
            current->next = new_table[newIndex];
            new_table[newIndex] = current;
            current = temp;
        }
    }
    delete[] table;
    table = new_table;
    tableSize = newTableSize;
}


template <typename Key, size_t N>
typename ADS_set<Key,N>::Element *ADS_set<Key,N>::locate(const key_type& key) const{
    size_type index = hash_func(key);
    for (Element *e = table[index]; e != nullptr; e = e->next) {
        if (key_equal{}(e->key, key))
            return e;
    }
    return nullptr;
}






template <typename Key, size_t N>
void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }



template <typename Key, size_t N>
class ADS_set<Key,N>::Iterator{
    Element* current;
    Element** table;
    size_type tableSize;
    size_type index;
    
public:
    using value_type = Key;
    using difference_type = std::ptrdiff_t;
    using reference = const value_type &;
    using pointer = const value_type *;
    using iterator_category = std::forward_iterator_tag;

    explicit Iterator(Element* current = nullptr, Element** table = nullptr, size_type tableSize = 0, size_type index = 0)
    :current(current), table(table), tableSize(tableSize), index(index){
        if (current == nullptr) ++(*this);
    }
    reference operator*() const{ return current->key; }
    pointer operator->() const { return &current->key; }
    Iterator &operator++() {
        
        if (current != nullptr) current = current->next;
        while (current == nullptr && ++index < tableSize) {
            current = table[index];
      
    }
    Iterator operator++(int){ //post
        Iterator tmp = *this;
        ++(*this);
        return tmp;
    }
    friend bool operator==(const Iterator &lhs, const Iterator &rhs){
        return lhs.current == rhs.current;
    }
    friend bool operator!=(const Iterator &lhs, const Iterator &rhs){
        return !(lhs == rhs);
    }
};



#endif // ADS_SET_H
