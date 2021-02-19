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
  using reference = key_type &;
  using const_reference = const key_type &;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using iterator = Iterator;
  using const_iterator = Iterator;
  using key_compare = std::less<key_type>;   // B+-Tree
  using key_equal = std::equal_to<key_type>; // Hashing
  using hasher = std::hash<key_type>;        // Hashing

private:
	struct element {
		key_type key;
		element *next;
		~element() { delete next;};
	};
	struct list {
		element *head{nullptr};
		bool end{false};
		~list() {delete head;};
	};
list *table{nullptr};
size_type table_size{0}, curr_size{0};
float max_lf{0.7};
size_type h(const key_type& key) const { return hasher{}(key) % table_size; }
void reserve(size_type n);
void rehash(size_type n);
element *insert_(const key_type& key);
element *find_(const key_type& key) const;

public:

  ADS_set() {rehash(N);}

  ADS_set(std::initializer_list<key_type> ilist) : ADS_set{} {insert(ilist);}

  template<typename InputIt> ADS_set(InputIt first, InputIt last) : ADS_set{} { insert(first,last); }

  ADS_set(const ADS_set &other): ADS_set{} {
    reserve(other.curr_size);
    for(const auto&k: other) {
      insert_(k);
    }
  }

  ~ADS_set(){delete[] table;}


  ADS_set &operator=(const ADS_set &other){
    if(this == &other) 
      return *this;
    ADS_set tmp{other};
    swap(tmp);
    return *this;
  }
  ADS_set &operator=(std::initializer_list<key_type> ilist) {
    ADS_set tmp{ilist};
    swap(tmp);
    return *this;
  }

  size_type size() const {return curr_size;}
  
  bool empty() const {return curr_size == 0;}

  size_type count(const key_type &key) const { return !!find_(key);}
  
  iterator find(const key_type &key) const {
    size_type idx {h(key)};
    list *pos_t {&table[idx]};
    if(auto p{find_(key)}) 
	return iterator{pos_t,p};
    return end();
  }

  void clear() {
    delete[] table;
    curr_size = 0;
    table_size = N;
    table = new list[table_size+1];
    table[table_size].end = true;
  }

  void swap(ADS_set &other) {
    using std::swap;
    swap(table, other.table);
    swap(curr_size, other.curr_size);
    swap(table_size, other.table_size);
    swap(max_lf, other.max_lf);
  }

  void insert(std::initializer_list<key_type> ilist) {
    for(const auto& elem_key : ilist){
      if(!count(elem_key))
	insert_(elem_key);
    }
  }

  std::pair<iterator,bool> insert(const key_type &key) {
    
    if(auto p {find_(key)}) {
    size_type idx {h(key)};
    list *pos_t {&table[idx]};
	return {iterator{pos_t,p}, false};
    }
    reserve(curr_size+1);
    size_type idx {h(key)};
    list *pos_t {&table[idx]};
    return {iterator{pos_t,insert_(key)}, true};
  }

  template<typename InputIt> void insert(InputIt first, InputIt last);

  size_type erase(const key_type &key) {
	if(count(key)) {
		size_type idx {h(key)};
		element *tmp{table[idx].head};
		element *before{nullptr};
		while(tmp!=nullptr){

		  if(key_equal{}(tmp->key, key) && tmp == table[idx].head) {
		    table[idx].head = tmp->next;
		    tmp->next = nullptr;
		    delete tmp;
		    --curr_size;
                    return 1;
		  }
		  if(key_equal{}(tmp->key, key) && before){
	 	    before->next = tmp->next;
		    tmp->next=nullptr;
		    delete tmp;
		    before = nullptr;
		    delete before;
		    --curr_size;
		    return 1;
		  }
		  before = tmp;
		  tmp = tmp->next;
		}
	}
	return 0;
}	

  const_iterator begin() const {
    for(size_type idx{0}; idx < table_size; ++idx){
      if(table[idx].head){
	 list *p_t{&table[idx]};
	 element *p_l{table[idx].head};
 	 return const_iterator{p_t,p_l};
      }
    }
    return end();
  }

  const_iterator end() const {
    list *p_t{&table[table_size]};
    return const_iterator{p_t}; 
  }

  void dump(std::ostream &o = std::cerr) const;

  friend bool operator==(const ADS_set &lhs, const ADS_set &rhs) {
    if(lhs.curr_size != rhs.curr_size) return false;
    for(const auto &k: rhs) {
      if(!lhs.count(k)) return false;
      }
    return true;
  }

  friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs) {return !(lhs==rhs);}
};

  template <typename Key, size_t N>
  typename ADS_set<Key,N>::element *ADS_set<Key,N>::insert_(const key_type& key) {

	size_type idx {h(key)};
	element *new_element = new element;
	new_element->key = key;
	new_element->next = table[idx].head;
	table[idx].head = new_element;
	++curr_size;
	return table[idx].head;
  }


  template <typename Key, size_t N>
  typename ADS_set<Key,N>::element *ADS_set<Key,N>::find_(const key_type& key) const {

	size_type idx {h(key)};
		element *tmp{table[idx].head};
		while(tmp != nullptr) {
			if(key_equal{}(tmp->key, key))
				return tmp;
			else 
				tmp = tmp->next;
		} 
	return nullptr;
  }

  template <typename Key, size_t N>
  template<typename InputIt> void ADS_set<Key,N>::insert(InputIt first, InputIt last) {

	for(auto it{first}; it!=last; ++it) {
		if(!count(*it)) {
			reserve(curr_size+1);
			insert_(*it);
		}
	}
  }

  template <typename Key, size_t N>
  void ADS_set<Key,N>::reserve(size_type n) {

	if(n > table_size * max_lf){
			size_type new_table_size{table_size};
		do {
			new_table_size = new_table_size * 2 + 1;
		} while(n > new_table_size * max_lf);
		rehash(new_table_size);
	}
  }

  template <typename Key, size_t N>
  void ADS_set<Key,N>::rehash(size_type n) {

	size_type new_table_size {std::max(N, std::max(n, size_type(curr_size / max_lf)))};
	list *new_table {new list[new_table_size+1]};
	new_table[new_table_size].end = true;
	size_type old_table_size {table_size};
	list *old_table {table};
	curr_size = 0;
	table = new_table;
	table_size = new_table_size;

	for(size_type idx{0}; idx < old_table_size; ++idx) {
		element *tmp{old_table[idx].head};
		while(tmp != nullptr){
			insert_(tmp->key);
			tmp = tmp->next; 
		}		
	}
	delete[] old_table;
  }

  template <typename Key, size_t N>
  void ADS_set<Key,N>::dump(std::ostream &o) const {

	o << "curr_size = " << curr_size << " table_size = " << table_size << '\n';
	for(size_type idx{0}; idx < table_size; ++idx) {
		o << idx << ": ";
		element *tmp{table[idx].head};
		while(tmp != nullptr){
			o << " --> " << tmp->key;
			tmp = tmp->next;
		}
		if(!table[idx].end)
		  o << " 0";
		o << '\n';
	}
	if(table[table_size].end)
		o << " 1" << '\n';	
  }


template <typename Key, size_t N>
class ADS_set<Key,N>::Iterator {
public:
  using value_type = Key;
  using difference_type = std::ptrdiff_t;
  using reference = const value_type &;
  using pointer = const value_type *;
  using iterator_category = std::forward_iterator_tag;
private:
  list *pos_table;
  element *pos_list;
  void skip() {while(pos_table->head == nullptr && pos_table->end != true) ++pos_table; pos_list = pos_table->head; }
public:
  explicit Iterator(list *pos_table = nullptr, element *pos_list = nullptr):
	            pos_table{pos_table}, pos_list{pos_list}
	            {}
  reference operator*() const {return pos_list->key;}
  pointer operator->() const {return &pos_list->key;}
  Iterator &operator++() {
   if(pos_list->next!=nullptr) 
     pos_list = pos_list->next; 
   else{
     ++pos_table;
     pos_list = pos_table->head;
     skip();
       }
   return *this; 
  }
  Iterator operator++(int){ auto rc{*this}; ++*this; return rc; }
  friend bool operator==(const Iterator &lhs, const Iterator &rhs)
	 {
	  return lhs.pos_table == rhs.pos_table && lhs.pos_list == rhs.pos_list;
	 }
  friend bool operator!=(const Iterator &lhs, const Iterator &rhs){return !(lhs == rhs);}
};

template <typename Key, size_t N> void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }

#endif // ADS_SET_H