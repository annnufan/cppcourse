//
// Created by Anna Kopeliovich(annnufan@gmail.com, nufan@list.ru) on 04.10.2016.
//

#include "persistent_set.h"

typedef int value_type;

#include <cassert>
#include <iostream>

// Создает пустой persistent_set.
persistent_set::persistent_set() {
    root = new node();
}

// Создает копию указанного persistent_set-а.
persistent_set::persistent_set(persistent_set const &rhs) {
    assert(rhs.root != nullptr);
    root = new node();
    root->left = rhs.root->left;
}

// Изменяет this так, чтобы он содержал те же элементы, что и rhs.
// Инвалидирует все итераторы, принадлежащие persistent_set'у this, включая end().
persistent_set& persistent_set::operator=(persistent_set const& rhs) {
    if (root != nullptr) {
        invalidate_all_iterators();
        root->dec_node();
    }
    root = new node();
    root->left = rhs.root->left;
    return *this;
}

// Деструктор. Вызывается при удалении объектов persistent_set.
// Инвалидирует все итераторы ссылающиеся на элементы этого persistent_set
// (включая итераторы ссылающиеся на элементы следующие за последними).
persistent_set::~persistent_set() {
    assert(root != nullptr);
    invalidate_all_iterators();
    root->dec_node();
    root = nullptr;
}


// Поиск элемента.
// Возвращает итератор на найденный элемент, либо end().
persistent_set::iterator persistent_set::find(value_type val) {
    assert(root != nullptr);
    node* now_node = root;
    while (now_node != nullptr && now_node->get_value() != val) {
        if (now_node->get_value() < val) {
            now_node = now_node->right;
        } else {
            now_node = now_node->left;
        }
    }
    iterator ans;
    if (now_node == nullptr)
        ans = iterator(root, root);
    else
        ans = iterator(now_node, root);
    list_of_iterator.push_back(&ans);
    return ans;
}

//функция, инвалидирующая все итераторы this
void persistent_set::invalidate_all_iterators() {
    while (!list_of_iterator.empty()) {
        iterator* x = list_of_iterator.back();
        list_of_iterator.pop_back();
        x->value = nullptr;
    }
}

// Вставка элемента.
// 1. Если такой ключ уже присутствует, вставка не производиться, возвращается итератор
//    на уже присутствующий элемент и false.
// 2. Если такого ключа ещё нет, производиться вставка, возвращается итератор на созданный
//    элемент и true.
// Инвалидирует все итераторы, принадлежащие persistent_set'у this, включая end().
std::pair<persistent_set::iterator, bool> persistent_set::insert(value_type val) {
    assert(root != nullptr);
    iterator f = find(val);
    if (f.value != root) {
        return std::make_pair(f, false);
    }
    node* add_node = new node(val);
    invalidate_all_iterators();
    root = insert_value(root, add_node);
    return std::make_pair(iterator(add_node, root), true);
};

//Функция, добавляющая новый элемент в наш сет
persistent_set::node* persistent_set::insert_value(node* v, node* add_node) {
    if (v->get_value() > add_node->get_value()) {
        if (v->left == nullptr) {
            return new node(v->get_value(), add_node, v->right);
        }
        return new node(v->get_value(), insert_value(v->left, add_node), v->right);
    }
//    std::cout << "WARNING " << v->get_value() << std::endl;
    if (v->right == nullptr) {
        return new node(v->get_value(), v->left, add_node);
    }
    return new node(v->get_value(), v->left, insert_value(v->right, add_node));
};

// Возващает итератор на элемент с минимальный ключом.
persistent_set::iterator persistent_set::begin() const {
    assert(root != nullptr);
    node* use = root;
    while (use -> left != nullptr) {
        use = use -> left;
    }
    return iterator(use, root);
}

// Возващает итератор на элемент следующий за элементом с максимальным ключом.
persistent_set::iterator persistent_set::end() const {
    assert(root != nullptr);
    return iterator(root, root);
}

 // Удаление элемента.
// Инвалидирует все итераторы, принадлежащие persistent_set'у this, включая end().
void persistent_set::erase(iterator elem) {
    assert(elem.value != nullptr);
    invalidate_all_iterators();
    root = erase_value(elem.version_root, elem.value);
}

persistent_set::node* persistent_set::erase_value(node* v, node* erase_node) {
    if (v->get_value() > erase_node->get_value()) {
        return new node(v->get_value(), erase_value(v->left, erase_node), v->right);
    }
    if (v->get_value() < erase_node->get_value()) {
        return new node(v->get_value(), v->left, erase_value(v->right, erase_node));
    }
    if (v->right == nullptr) {
        return (v->left == nullptr) ? nullptr : new node(v->left->get_value(), v->left->left, v->left->right);
    }
    return new node(v->right->get_min()->get_value(), v->left, simple_deleted(v->right));
}

persistent_set::node* persistent_set::simple_deleted(node* v) {
    if (v->left == nullptr)
        return (v->right == nullptr) ? nullptr : new node(v->right->get_value(), v->right->left, v->right->right);
    return new node(v->get_value(), simple_deleted(v->left), v->right);
}


persistent_set::node::node() {
    value = INT_MAX;
    count = 1;
    left = right = nullptr;
}

persistent_set::node::node(value_type v, node* l, node* r) {
    value = v;
    count = 1;
    left = l;
    right = r;
}

//уменьшает число ссылок на нее
//если ссылок 0, вершина удаляется
void persistent_set::node::dec_node() {
    count--;
    if (count == 0) {
        delete this;
    }
}

value_type& persistent_set::node::get_value(){
    return value;
}

//конструктор по умолчанию и конструктор от ссылки на определенную node
persistent_set::iterator::iterator() {
    value = version_root = nullptr;
}

persistent_set::iterator::iterator(node* x, node* v) {
    value = x;
    version_root = v;
}

// Элемент на который сейчас ссылается итератор.
// Разыменование итератора end() неопределено.
// Разыменование невалидного итератора неопределено.
value_type const& persistent_set::iterator::operator*() const {
    assert(value != nullptr);
    return value->get_value();
}

// Сравнение. Итераторы считаются эквивалентными если одни ссылаются на один и тот же элемент.
// Сравнение с невалидным итератором не определено.
// Сравнение итераторов двух разных контейнеров не определено.
bool operator==(persistent_set::iterator u, persistent_set::iterator v) {
    return u.value == v.value && u.version_root == v.version_root;
}
bool operator!=(persistent_set::iterator u, persistent_set::iterator v) {
    return u.value != v.value || u.version_root != v.version_root;
}

persistent_set::node* persistent_set::node::get_min() {
    assert(this != nullptr);
    node* use = this;
    while (use -> left != nullptr) {
        use = use->left;
    }
    return use;
}

persistent_set::node* persistent_set::node::get_max() {
    assert(this != nullptr);
    node* use = this;
    while (use -> right != nullptr) {
        use = use -> right;
    }
    return use;
}


// Переход к элементу со следующим по величине ключом.
// Инкремент итератора end() неопределен.
// Инкремент невалидного итератора неопределен.
persistent_set::iterator& persistent_set::iterator::operator++() {
    if (value->right != nullptr) {
        value = value->right->get_min();
        return *this;
    }
    node* r = version_root;
    std::vector<node*> parent;
    while (r->right != value && r->left != value) {
        parent.push_back(r);
        if (r->get_value() < value->get_value()) {
            r = r->right;
        } else {
            r = r->left;
        }
    }
    parent.push_back(r);
    while (parent.back()->right == value) {
        value = parent.back();
        parent.pop_back();
    }
    value = parent.back();
    return *this;
}
persistent_set::iterator persistent_set::iterator::operator++(int) {
    assert(value != nullptr);
    iterator ans(value, version_root);
    iterator me = *this;
    *this = ++me;
    return ans;
}

// Переход к элементу со следующим по величине ключом.
// Декремент итератора begin() неопределен.
// Декремент невалидного итератора неопределен.
persistent_set::iterator& persistent_set::iterator::operator--() {
    if (value -> left != nullptr) {
        value = value->left->get_max();
        return *this;
    }
    node* r = version_root;
    std::vector<node*> parent;
    while (r->right != value && r->left != value) {
        parent.push_back(r);
        if (r->get_value() < value->get_value()) {
            r = r->right;
        } else {
            r = r->left;
        }
    }
    parent.push_back(r);
    while (parent.back()->left == value) {
        value = parent.back();
        parent.pop_back();
    }
    value = parent.back();
    return *this;
}
persistent_set::iterator persistent_set::iterator::operator--(int) {
    assert(value != nullptr);
    iterator ans(value, version_root);
    iterator me = *this;
    *this = --me;
    return ans;
}

void persistent_set::print() {
    node::print_node(root);
    std::cout << std::endl;
}

void persistent_set::node::print_node(node* v) {
    if (v == nullptr)
        return;
    std::cout << '(';
    print_node(v->left);
    std::cout << v->get_value();
    print_node(v->right);
    std::cout << ')';
}

// Testing part
int main() {
    int a[6] = {2, 5, 3, 7, 1, 9};
    persistent_set me;
    for (int i : a) {
        me.insert(i);
    }
    me.print();
    persistent_set::iterator ex = me.end();
    std::cout << *(ex) << std::endl;
    while (ex != me.begin()) {
        std::cout << *(ex--) << std::endl;
    }
    for (int i : a) {
        me.erase(me.find(i));
        me.print();
    }
/*    for (int i : a) {
        me.erase(me.find(i));
        ex = me.begin();
        std::cout << *ex << std::endl;
    }
  */  return 0;
}