#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <deque>
#include <map>
#include <bits/stdc++.h>
#include <fstream>
#include <sstream>
#include "node.h"
#include "wlp4data.h"
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::getline;
using std::string;
using std::vector;
using std::deque;
using std::map;
using std::stringstream;

void print(deque<std::shared_ptr<Node>> &reduction, deque<std::shared_ptr<Node>> &input) {
    string middle_part = ".";
    size_t reduction_count = 0, input_count = 0;
    for (auto &it: reduction) {
        if (reduction_count != 0) cout << " ";
        it->print();
        ++reduction_count;
    }
    if (reduction_count != 0) cout << " ";
    cout << ".";
    for (auto &it: input) {
        it->print();
        ++input_count;
    }
    cout << endl;
}

void print(deque<std::shared_ptr<Node>> &reduction) {
    for (auto &it: reduction) {
        it->print();
    }
}

void shift(deque<std::shared_ptr<Node>> &reduction, deque<std::shared_ptr<Node>> &input) {
    if (input.size() == 0) return;
    std::shared_ptr<Node> &to_move = input[0];
    reduction.push_back(to_move);
    input.pop_front();
}

void reduce(deque<std::shared_ptr<Node>> &reduction, const string &LHS, deque<string> &RHS, 
            deque<string> &state_stack, map<string,map<string,string>> transition_table) {
    //cout << "reduction: " << reduction << endl;
    //cout << "LHS: " << LHS << endl;
    //cout << "RHS: " << RHS << endl;
    if (RHS.size() == 1 && RHS[0] == ".EMPTY") {
        std::shared_ptr<Node> parent = std::make_shared<Node> (LHS);
        std::shared_ptr<Node> child = std::make_shared<Node> (RHS[0]);
        parent->add(child);
        reduction.push_back(parent);
        string current_state = state_stack[state_stack.size() - 1];
        string next_state = transition_table[current_state][LHS];
        state_stack.push_back(next_state);
    } else {
        vector<std::shared_ptr<Node>> temp_vec;
        int i = reduction.size()-1, j = RHS.size() - 1;
        auto it = reduction.begin()+i;
        for ( ; !(i < 0) && !(j < 0) ; --i, --j ) {
            //if (reduction[i] != RHS[j]) break;
            std::shared_ptr<Node> child = *it;
            if (reduction.begin()+1 != it) --it;
            temp_vec.emplace_back(child);
            reduction.pop_back();
            state_stack.pop_back();
        }
        if (!(j < 0)) {
            cerr << "ERROR: can't reduce using this rule" << endl;
            cerr << "reduction sequence: ";
            for (auto &it: reduction) cout << it << " ";
            cout << endl;
            cerr << "RHS: ";
            for (auto &it: RHS) cout << it << " ";
            cout << endl;
        } else {
            std::shared_ptr<Node> parent = std::make_shared<Node> (LHS);
            for (int i = temp_vec.size()-1; !(i < 0); --i) {
                parent->add(temp_vec[i]);
            }
            reduction.push_back(parent);
            string current_state = state_stack[state_stack.size() - 1];
            string next_state = transition_table[current_state][LHS];
            state_stack.push_back(next_state);
        }
    }
}

void push(const string &RHS, deque<string> &temp) {
    size_t i = 0, j = 0;
    while (i < RHS.length()) {
        while (i < RHS.length() && RHS[i] == ' ') ++i;
        j = i;
        while (j < RHS.length() && RHS[j] != ' ') ++j;
        if (j != i) temp.push_back(RHS.substr(i,j-i));
        i = j;
    }
}

int main() {
    //std::ifstream in{"test_1.in"};
    stringstream ss(WLP4_COMBINED);
    string line;
    getline(ss,line);
    vector<std::pair<string,deque<string>>> rule_vector;
    
    if (line != ".CFG") {
        cerr << "ERROR: the first part should be CFG component" << endl;
        return 1;
    }
    
    size_t num_of_rules = 0;
    while (getline(ss,line)) {
        if (line == ".TRANSITIONS") break;
        string LHS = line.substr(0, line.find(" "));
        string RHS = line.substr(line.find(" "));
        deque<string> temp;
        push(RHS, temp);
        //cout << "LHS:" << LHS << endl;
        //cout << "RHS:" << RHS << endl;
        //cout << "index: " << num_of_rules << endl;
        rule_vector.emplace_back(std::make_pair(LHS,temp));
        ++num_of_rules;
    }
    //cout << num_of_rules << endl;
    
    /*
    for (size_t i = 0; i < num_of_rules; ++i) {
        cout << "rule " << i << ": " << rule_vector[i].first << " becomes " ;
        for (auto &it: rule_vector[i].second) cout << it << " ";
        cout << endl;
    }
    */
    
    
    

    if (line != ".TRANSITIONS") {
        cerr << "ERROR: the second part should be TRANSITIONS component" << endl;
        return 1;
    }

    
    string input = "";

    map<string,map<string,string>> transition_table; // map<from,map<by,to>>

    while (ss >> input) {
        if (input == ".REDUCTIONS") break;
        string from_state = input, by = "", to_state = "";
        ss >> by;
        ss >> to_state;
        transition_table[from_state][by] = to_state;
    }

    if (input != ".REDUCTIONS") {
        cerr << "ERROR: the third part should be REDUCTIONS component" << endl;
        return 1;
    }

    map<string,map<string,int>> reduction_table; // map<current_state<follow_symbol,rule_number>>

    while (ss >> input) {
        if (input == ".END") break;
        string current_state = input, rule = "", follow_symbol = "";
        ss >> rule;
        int rule_number = stoi(rule);
        ss >> follow_symbol;
        reduction_table[current_state][follow_symbol] = rule_number;
    }
    
    if (input != ".END") {
        cerr << "ERROR: the final part should be END component" << endl;
        return 1;
    }

    //for (auto &it:input_sequence) cout << it << " ";
    //cout << endl;


    
    deque<std::shared_ptr<Node>> reduction_sequence;
    deque<std::shared_ptr<Node>> input_sequence;
    std::shared_ptr<Node> type_start = std::make_shared<Node>("BOF");
    std::shared_ptr<Node> lexeme_start = std::make_shared<Node>("BOF");
    type_start->add(lexeme_start);
    input_sequence.push_back(type_start);
    
    //std::ifstream in{"test.in"};
    while (cin >> input) {
        std::shared_ptr<Node> type = std::make_shared<Node>(input);
        cin >> input;
        std::shared_ptr<Node> lexeme = std::make_shared<Node>(input);
        type->add(lexeme);
        input_sequence.push_back(type);
    }
    std::shared_ptr<Node> type_end = std::make_shared<Node>("EOF");
    std::shared_ptr<Node> lexeme_end = std::make_shared<Node>("EOF");
    type_start->add(lexeme_end);
    input_sequence.push_back(type_end);
    

    int k = -1;
    deque<string> state_stack;
    // reduction_sequence is the symbol stack
    string current_shift = input_sequence[0]->get_root();
    string current_state = "0";
    shift(reduction_sequence, input_sequence);
    ++k;
    state_stack.push_back(transition_table[current_state][current_shift]);
    while (input_sequence.size() > 0) {
        current_shift = input_sequence[0]->get_root();
        string stack_top = state_stack[state_stack.size()-1];
        while (reduction_table.find(stack_top) != reduction_table.end()
            && reduction_table[stack_top].find(current_shift) != reduction_table[stack_top].end()) {
            
            int rule_to_follow = reduction_table[stack_top][current_shift];
    
            string LHS = rule_vector[rule_to_follow].first;
            deque<string> &RHS = rule_vector[rule_to_follow].second;

            reduce(reduction_sequence, LHS, RHS, state_stack, transition_table);

            stack_top = state_stack[state_stack.size()-1];
        }
        
        current_shift = input_sequence[0]->get_root();
        current_state = stack_top;
        shift(reduction_sequence, input_sequence);
        ++k;
        if (transition_table.find(current_state) == transition_table.end() 
        ||  transition_table[current_state].find(current_shift) == transition_table[current_state].end()) {
            cerr << "ERROR at " << k << endl;
            return 1;
        } else {
            state_stack.push_back(transition_table[current_state][current_shift]);
        }
    }

    string stack_top = state_stack[state_stack.size()-1];
    if (!(reduction_sequence[0]->get_root() == "BOF" && reduction_sequence[reduction_sequence.size()-1]->get_root() == "EOF")
        || reduction_table.find(stack_top) == reduction_table.end() 
        || reduction_table[stack_top].find(".ACCEPT") == reduction_table[stack_top].end()) {
        cerr << "ERROR at " << k << endl;
        return 1;
    }
    int rule = reduction_table[stack_top][".ACCEPT"];
    string LHS = rule_vector[rule].first;
    deque<string> RHS = rule_vector[rule].second;
    if (RHS.size() == reduction_sequence.size()) {
        std::shared_ptr<Node> root = std::make_shared<Node> (LHS);
        vector<std::shared_ptr<Node>> temp_vec;
        while (RHS.size() > 0) {
            if (reduction_sequence[RHS.size()-1]->get_root() != RHS[RHS.size()-1]) {
                cerr << "ERROR at " << k << endl;
                return 1;
            }
            temp_vec.push_back(reduction_sequence[reduction_sequence.size()-1]);
            reduction_sequence.pop_back();
            RHS.pop_back();
        }
        for (int i = temp_vec.size()-1; !(i < 0); --i) {
            root->add(temp_vec[i]);
        }
        reduction_sequence.push_back(root);

    }
    
    print(reduction_sequence);

    return 0;
}