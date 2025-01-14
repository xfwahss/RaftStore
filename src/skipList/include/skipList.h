//
// Created by xfwah on 2025/1/1.
//

#ifndef SKIPLIST_H
#define SKIPLIST_H

#include<cmath>
#include<cstdlib>
#include<cstring>
#include<fstream>
#include<iostream>
#include <mutex>
#include "Logger.h"

#define STORE_FILE "store/dumpFile"

static std::string delimiter = ":";

// Class template to implement node
template<typename K, typename V>
class Node {
public:
    Node() {}

    Node(K k, V v, int);

    ~Node();

    K get_key() const;

    V get_value() const;

    void set_value(V);

    // Linear array to hold pointers to next node of different level
    Node<K, V> **forward; // 理解一下为什么用二级指针
    int node_level;
private:
    K key;
    V value;
};

template<typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level) {
    this->key = k;
    this->value = v;
    this->node_level = level;

    // level + 1, because array index is from 0 - level
    this->forward = new Node<K, V> *[level + 1];

    // Fill forward array with 0 (NULL)
    memset(this->forward, 0, sizeof(Node<K, V> *) * (level + 1));
};

template<typename K, typename V>
Node<K, V>::~Node() {
    delete[] forward;
};

template<typename K, typename V>
K Node<K, V>::get_key() const {
    return key;
};

template<typename K, typename V>
V Node<K, V>::get_value() const {
    return value;
};

template<typename K, typename V>
void Node<K, V>::set_value(V value) {
    this->value = value;
};

// Class template to implement node
template<typename K, typename V>
class SkipListDump {
public:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & keyDumpVt_;
        ar & valDumpVt_;
    }

    std::vector<K> keyDumpVt_;
    std::vector<V> valDumpVt_;
public:
    void insert(const Node<K, V> &node);
};

// Class template for Skip list
template<typename K, typename V>
class SkipList {
public:
    SkipList(int);

    ~SkipList();

    int getRandomLevel();

    Node<K, V> *create_node(K, V, int);

    int insert_element(K, V);

    void displayList();

    bool searchElement(K, V &value);

    void deleteElement(K);

    void insertSetElement(K &, V &);

    std::string dump_file();

    void load_file(const std::string &dumpStr);

    // rec
    void clear(Node<K, V> *node);

    int size();

private:
    void get_key_value_from_string(const std::string &str, std::string *key, std::string *value);

    bool is_valid_string(const std::string &str);

private:
    // Maximum level for this skip list
    int maxLevel;
    // current level of skip list
    int skipListLevel;
    // pointer to header node
    Node<K, V> *header;

    // file operator
    std::ofstream fileWriter;
    std::ifstream fileReader;

    // skipList current size
    int elementCount;

    // mutex for critical section
    std::mutex mtx;
};

// create new node
template<typename K, typename V>
Node<K, V> *SkipList<K, V>::create_node(const K key, const V value, int level) {
    Node<K, V> *n = new Node<K, V>(key, value, level);
    return n;
}

template<typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value) {
    mtx.lock();
    Node<K, V> *current = this->header;
    // create update array and initialize it
    // update is array which put node that the node->forward[i] should be operated later
    Node<K, V> *update[maxLevel + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (maxLevel + 1));

    // start from highest level of skip list
    for (int i = skipListLevel; i >= 0; i--) {
        while (current->forward[i] != nullptr && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    // reached level 0 and forward pointer to right node, which is desired to insert key
    current = current->forward[0];

    // if current node have key equal to searched key, we get it
    if (current != nullptr && current->get_key() == key) {
        std::cout << "key: " << key << ", exists" << std::endl;
        mtx.unlock();
        return 1;
    }

    // if current is null that means we have reached to end of the level
    // if current's key is not equal to key that means we have to insert node between update[0] and current node
    if (current == nullptr || current->get_key() != key) {
        // Generate a random level for node
        int randomLevel = getRandomLevel();

        // If random level is greater than skip list's current level, initialize update value with pointer to header
        if (randomLevel > skipListLevel) {
            for (int i = skipListLevel + 1; i < randomLevel + 1; i++) {
                update[i] = header;
            }
            skipListLevel = randomLevel;
        }

        // create new node with random level generated
        Node<K, V> *inserted_node = create_node(key, value, randomLevel);

        // insert node
        for (int i = 0; i <= randomLevel; i++) {
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }

        Logger::getInstance()->info("Successfully inserted key: ", key, " value: ", value);
        elementCount++;
    }
    mtx.unlock();
    return 0;
}

// Display skip list
template<typename K, typename V>
void SkipList<K, V>::displayList() {
    Logger::getInstance()->setDispLogLevel(false);
    Logger::getInstance()->setDispTimeStamps(false);
    Logger::getInstance()->info("\n*****Skip List*****\n");
    for (int i = 0; i <= skipListLevel; i++) {
        Node<K, V> *node = this->header->forward[i];
        Logger::getInstance()->info("Level ", i, ": ");
        while (node != nullptr) {
            Logger::getInstance()->info(node->get_key(), ":", node->get_value(), ";");
            node = node->forward[i];
        }
        Logger::getInstance()->info("\n");
    }
    Logger::getInstance()->setDispLogLevel(true);
    Logger::getInstance()->setDispTimeStamps(true);
}

// TODO 考虑对于dump和load加锁
// Dump data in memory to file
template<typename K, typename V>
std::string SkipList<K, V>::dump_file() {
    Logger::getInstance()->debug("dump file ----------\n");
    Node<K, V> *node = this -> header -> forward[0];
    SkipListDump<K, V> dump;
    while(node != nullptr){
        dump.insert(*node);
        node = node -> forward[0];
    }

    std::stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << dump;
    return ss.str();
}

// Load data from disk
template <typename K, typename V>
void SkipList<K,V>::load_file(const std::string &dumpStr) {
    if(dumpStr.empty()){
        return;
    }
    SkipListDump<K,V> dump;
    std::stringstream iss(dumpStr);
    boost::archive::text_iarchive ia(iss);
    ia >> dump;
    for(int i=0; i < dump.keyDumpVt_.size(); i++){
        insert_element(dump.keyDumpVt_[i], dump.valDumpVt_[i]);
    }
}

// Get current skipList size
template <typename K, typename V>
int SkipList<K,V>::size() {
    return elementCount;
}

template <typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string &str, std::string *key, std::string *value) {
    if(!is_valid_string(str)){
        return;
    }
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter) + 1, str.length());
}

template <typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string &str) {
    if(str.empty()){
        return false;
    }
    if(str.find(delimiter) == std::string::npos){
        return false;
    }
    return true;
}

// Delete element from skip list
template <typename K, typename V>
void SkipList<K, V>::deleteElement(K key) {
    mtx.lock();
    Node<K, V> *current = this -> header;
    Node<K, V> *update[maxLevel + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (maxLevel + 1));

    // start from highest level of skip list
    for(int i = skipListLevel; i >= 0; i--){
        while(current -> forward[i] != nullptr && current -> forward[i] -> get_key() < key){
            current = current -> forward[i];
        }
        update[i] = current;
    }

    current = current -> forward[0];

    if(current != nullptr && current -> get_key() == key){
        // start for lowest level and delete the current node of each level
        for(int i = 0; i <= skipListLevel; i++){
            // If at level i, next node is not target node, break the loop
            if(update[i] -> forward[i] != current){
                break;
            }
            update[i] -> forward[i] = current -> forward[i];
        }

        // Remove levels having no elements
        while(skipListLevel > 0 && header -> forward[skipListLevel] == nullptr){
            skipListLevel--;
        }
        Logger::getInstance() -> info("Successfully deleted key: ", key, "\n");
        delete current;
        elementCount--;
    }
    mtx.unlock();
}

/**
 * \brief insert element into skip list, when the key is already in the list, update the value
 */
template <typename K, typename V>
void SkipList<K, V>::insertSetElement(K &key, V &value){
    V oldValue;
    if(searchElement(key, oldValue)){
        deleteElement(key);
    }
    insertSetElement(key, value);
}

template<typename K, typename V>
bool SkipList<K, V>::searchElement(K key, V&value){
    Node<K, V> *current = this -> header;
    for(int i = skipListLevel; i >= 0; i--){
        while(current -> forward[i] && current -> forward[i] -> get_key() < key){
            current = current -> forward[i];
        }
    }
    current = current -> forward[0];
    if(current && current -> get_key() == key){
        value = current -> get_value();
        return true;
    }
    Logger::getInstance()->info("Not Found Key: ", key, "\n");
    return false;
}

template <typename K, typename V>
void SkipListDump<K, V>::insert(const Node<K, V> &node) {
    keyDumpVt_.emplace_back(node.get_key());
    valDumpVt_.emplace_back(node.get_value());
}

// construct skip list
template <typename K, typename V>
SkipList<K, V>::SkipList(int maxLevel) {
    this->maxLevel = maxLevel;
    this->skipListLevel = 0;
    this->elementCount = 0;
    K k;
    V v;
    this->header = new Node<K, V>(k, v, maxLevel);
}

template <typename K, typename V>
SkipList<K, V>::~SkipList() {
    if(fileWriter.is_open()){
        fileWriter.close();
    }
    if(fileReader.is_open()){
        fileWriter.close();
    }
    if(header->forward[0]!= nullptr){
        clear(header->forward[0]);
    }
    delete (header);
}

template <typename K, typename V>
void SkipList<K, V>::clear(Node<K, V> *cur){
    if(cur->forward[0] != nullptr){
        clear(cur->forward[0]);
    }
    delete (cur);
}

template <typename K, typename V>
int SkipList<K, V>::getRandomLevel(){
    int k = 1;
    while(rand() % 2){
        k++;
    }
    k = (k < maxLevel) ? k : maxLevel;
    return k;
}
#endif //SKIPLIST_H