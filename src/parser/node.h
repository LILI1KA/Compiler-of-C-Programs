#ifndef NODE_H
#define NODE_H

#include <vector>
#include <memory>
#include <string>
#include <iostream>

using std::vector;
using std::string;
using std::cout;
using std::endl;

class Node {
public:
    string root;
    vector<std::shared_ptr<Node>> children;

    // ctor
    Node(string str): root(str) {}

    // copy ctor
    Node(const Node& other): root{other.root} {
        for (auto &it: other.children) children.emplace_back(std::make_shared<Node>(*it));
    }

    // copy assignment
    Node& operator=(const Node& other) {
        if (this != &other) {
            root = other.root;
            children.clear();
            for (auto &it: other.children) children.emplace_back(std::make_shared<Node>(*it));
        }
        return *this;
    }

    // move ctor
    Node(Node&& other): root{other.root}, children{std::move(other.children)} {}

    // Move assignment operator
    Node& operator=(Node&& other) {
        if (this != &other) {
            root = other.root;
            children = std::move(other.children);
        }
        return *this;
    }

    ~Node() { children.clear(); }

    void add(std::shared_ptr<Node> &new_child) {
        children.emplace_back(new_child);
    }

    string get_root() { return root; }

    void print() {
        if (root == "BOF") {
            cout << "BOF BOF" << endl;
        } else if (root == "EOF") {
            cout << "EOF EOF" << endl;
        } else if (children.size() > 0) {
            cout << root;
            for (auto &it: children) cout << " " << it->get_root();
            cout << endl;
            for (auto &it: children) it->print();
        }
    }

};

#endif