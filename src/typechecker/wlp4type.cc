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
#include "non_terminal.h"
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::getline;
using std::string;
using std::vector;
using std::deque;
using std::map;
using std::stringstream;
using std::shared_ptr;
using std::make_shared;
using std::pair;



map<string, int> non_terminal_map;

map<string,pair<vector<string>,map<string,string>>> symbol_map; 
// map<pair<name,vector of parameter>,map<name,type>>
string current_procedure = "";

shared_ptr<Node> build_tree(shared_ptr<Node> root, std::istream &in) {
    string word, line, label;
    in >> word;
    if (!root) root = make_shared<Node>(word);
    getline(in,line);
    std::istringstream words(line);

    while (words >> word) {
        //cout << "word: " << word << " " << endl;
        shared_ptr<Node> child = make_shared<Node> (word);
        root->add(child);
    }

    //cout << "current root: " << root->get_root() << endl;
    string root_label = root->get_root();
    for (int i = 0; i < root->size(); ++i) {
        label = (root->at(i))->get_root();
        //cout << "label out: " << label << endl;
        if ((label != ".EMPTY") && (non_terminal_map.find(label) != non_terminal_map.end()) && (root_label != label || root->size() > 1)) {
            //cout << "map size: " << non_terminal_map.size() << endl;
            //cout << "label in: " << label << endl;
            shared_ptr<Node> child = build_tree(root->at(i), in);
            root->at(i) = child;
        }
    }
    return root;
}


string id_name(shared_ptr<Node> root) { 
    return (root->get_root() == "ID")? (root->at(0))->get_root() : ""; 
}

string type_name(shared_ptr<Node> root) { 
    return (root->get_root() == "ID")? (root->at(0))->get_type() : ""; 
}

string dcl_id(shared_ptr<Node> root) { 
    return (root->get_root() == "dcl" && root->size() == 2)? id_name((root->at(1))) : "";
}

string dcl_type(shared_ptr<Node> root) { 
    return (root->get_root() == "dcl" && root->size() == 2)? type_name((root->at(1))) : "";
}


void attach_type(shared_ptr<Node> root, string type) {
    root->set_type(type);
    for (int i = 0; i < root->size(); ++i) {
        attach_type(root->at(i), type);
    }
}

string get_type(shared_ptr<Node> root) {
    string root_label = root->get_root();
    if (root_label == "int" || root_label == "*") {
        return root_label;
    } else {
        string result = "";
        for (int i = 0; i < root->size(); ++i) {
            result += get_type(root->at(i));
        }
        return result;
    }
}

void assign_type_dcl(shared_ptr<Node> root) {
    if (root->get_root() == "dcl") {
        string type = get_type(root->at(0));
        if (root->size() == 2) {
            attach_type(root->at(1), type);
        }
    } else {
        for (int i = 0; i < root->size(); ++i) {
            assign_type_dcl(root->at(i));
        }
    }
}


bool check_signature(shared_ptr<Node> root); // signature for checking signature

string set_expr_type(shared_ptr<Node> root) {
    string root_label = root->get_root();
    string type = "";
    if (root_label == "NUM") {
        root->set_type("int");
        (root->at(0))->set_type("int");
        return "int";
    } else if (root_label == "NULL") {
        root->set_type("int*");
        (root->at(0))->set_type("int*");
        return "int*";
    } else if (root_label == "ID") {
        map<string, string> &sub_map = (symbol_map[current_procedure]).second;
        string var = (root->at(0))->get_root();
        string type = "";
        if (sub_map.find(var) == sub_map.end()) {
            cerr << "ERROR: Use of undeclared variable:" << var << endl; // p2
        } else {
            type = sub_map[var];
        }
        root->set_type(type);
        return type;
    } else if (root_label == "expr") {
        if (root->size() == 1) { // expr term
            type = set_expr_type(root->at(0));
        } else if (root->size() == 3) { // expr PLUS/MINUS term
            string type_1 = set_expr_type(root->at(0));
            string type_2 = set_expr_type(root->at(2));
            string op = (root->at(1))->get_root();
            if ((type_1 == "int") && (type_2 == "int")) {
                type = "int";
            } else if ((type_1 == "int*") && (type_2 == "int")) {
                type = "int*";
            } else if ((type_1 == "int*") && (type_2 == "int*")) {
                if (op == "PLUS") {
                    cerr << "ERROR: incorrect type of parameters: int*+int*" << endl; // p3
                } else if (op == "MINUS") {
                    type = "int";
                }
            } else if ((type_1 == "int") && (type_2 == "int*")) {
                if (op == "MINUS") {
                    cerr << "ERROR: incorrect type of parameters: int-int*" << endl; // p3
                } else if (op == "PLUS") {
                    type = "int*";
                }
            } 
            /*else {
                cerr << "ERROR: incorrect type of parameters: undefined operation between " 
                     << type_1 << " and " << type_2 << endl;
            }*/
        }
        root->set_type(type);
        return type;
    } else if (root_label == "term") {
        if (root->size() == 1) { // term factor
            type = set_expr_type(root->at(0));
        } else if (root->size() == 3) { // term STAR/SLASH/PCT factor
            string type_1 = set_expr_type(root->at(0));
            string type_2 = set_expr_type(root->at(2));
            if (type_1 == "int" && type_2 == "int") {
                type = "int";
            } else {
                cerr << "ERROR: incorrect type of parameters: undefined operation between " // p3
                     << type_1 << " " << root->at(0)->get_root() << " " << " and " 
                     << type_2 << " " << root->at(2)->get_root() << " " << endl;
            }
            root->set_type(type);
            return type;
        }
        root->set_type(type);
        return type;
    } else if (root_label == "factor") {
        if (root->size() == 1) { // ID/NUM/NULL
            type = set_expr_type(root->at(0));
        } else if (root->size() == 2) { // AMP lvalue, STAR factor
            string child_type = set_expr_type(root->at(1));
            string operation = (root->at(0))->get_root();
            if (operation == "AMP") {
                if (child_type == "int") {
                    type = "int*";
                } else {
                    cerr << "ERROR: incorrect type of parameters: referencing a non-integer" << endl; // p3
                }
            } else if (operation == "STAR") {
                if (child_type == "int*") {
                    type = "int";
                } else {
                    cerr << "ERROR: incorrect type of parameters: dereferencing a non-pointer" << endl; // p3
                }
            } 
            /*else {
                cerr << "ERROR: incorrect type of parameters: undefined unary operation, should be & or *" << endl;
            }*/
        } else if (root->size() == 3) { // LPAREN expr RPAREN, ID LPAREN RPAREN
            string left = (root->at(0))->get_root();
            string right = (root->at(2))->get_root();
            if (left == "LPAREN" && right == "RPAREN") {
                type = set_expr_type(root->at(1));
            } else if (left == "ID") {
                if (check_signature(root)) {
                    type = "int";
                }
            }
            /*else {
                cerr << "ERROR: incorrect type of parameters" << endl;
            }*/
        } else if (root->size() == 4) { // ID LPAREN arglist RPAREN
            if (check_signature(root)) {
                type = "int";
            }
        } else if (root->size() == 5) { // NEW INT LBRACK expr RBRACK
            string child_type = set_expr_type(root->at(3));
            if (child_type == "int") {
                type = "int*";
            } else {
                cerr << "ERROR: incorrect type of parameters: use non-integer to initialize new" << endl; // p3
            }
        }
        root->set_type(type);
        return type;
    } else if (root_label == "lvalue") {
        if (root->size() == 1) { // ID
            type = set_expr_type(root->at(0));
        } else if (root->size() == 2) { // STAR factor
            string child_type = set_expr_type(root->at(1));
            if (child_type == "int*") {
                type = "int";
            } else {
                cerr << "ERROR: incorrect type of parameters: lvalue version of dereferencing a non-pointer" << endl; // p3
            }
        } else if (root->size() == 3) { // LPAREN lvalue RPAREN
            type = set_expr_type(root->at(1));
        }
        root->set_type(type);
        return type;
    } else {
        cerr << "ERROR: incorrect type of parameters" << endl; // p3
        return "";
    }
}

void assign_type_return(shared_ptr<Node> root) {
    if (root->get_root() == "main" || root->get_root() == "procedure") {
        current_procedure = ((root->at(1))->at(0))->get_root();
        bool main = (root->get_root() == "main");
        string type = "";

        if (main) { // extra checl for wain
            shared_ptr<Node> dcl_1 = nullptr;
            shared_ptr<Node> dcl_2 = nullptr;
            for(int index = 0; index < root->size(); ++index) {
                if ((root->at(index))->get_root() == "dcl") {
                    if (!dcl_1) {
                        dcl_1 = root->at(index);
                    } else {
                        dcl_2 = root->at(index);
                    }
                }
            }

            //cerr << "dcl1: " << "type: " << dcl_type(dcl_1) << " id: " << dcl_id(dcl_1) << endl;
            //cerr << "dcl2: " << "type: " << dcl_type(dcl_2) << " id: " << dcl_id(dcl_2) << endl;

            if (dcl_id(dcl_1) == dcl_id(dcl_2)) cerr << "ERROR: The two parameters of wain have the same name." << endl; // p1
            if (dcl_type(dcl_2) != "int") cerr << "ERROR: The second parameter of wain is not int type." << endl; // p1,p2
        }

        int i = 0;
        for (; i < root->size(); ++i) {
            if ((root->at(i))->get_root() == "RETURN") break;
        }
        type = set_expr_type(root->at(i+1));
        if (type != "int") cerr << "ERROR: The return expression of procedure " << current_procedure << " is not int type." << endl; // p1,p2,p4
    } else {
        for (int i = 0; i < root->size(); ++i) {
            assign_type_return(root->at(i));
        }
    }
}

void declaration_check(shared_ptr<Node> root) {
    string root_label = root->get_root();
    if (root_label == "dcls") {
        if (root->size() == 1) { // ".EMPTY"
            return;
        } else if (root->size() == 5) { // dcls dcl BECOMES NULL/NUM SEMI
            string LHS_type = "", RHS_type = "";
            declaration_check(root->at(0));
            shared_ptr<Node> dcl = root->at(1);
            LHS_type = (dcl->at(1))->get_type();
            string RHS_label = (root->at(3))->get_root();
            if (RHS_label == "NUM") {
                RHS_type = "int";
            } else if (RHS_label == "NULL") {
                RHS_type = "int*";
            }
            (root->at(3))->set_type(RHS_type);
            if (LHS_type != RHS_type) {
                cerr << "ERROR: type of variable doesn't match with its initialization value:" 
                     << "LHS: " << LHS_type << "RHS: " << RHS_type << endl; // p2
            }
        }
    } else {
        for (int i = 0; i < root->size(); ++i) {
            declaration_check(root->at(i));
        }
    }
}

void count_param(shared_ptr<Node> root, vector<string> &param_list) {
    if (root->get_root() == "params") {
        if (root->size() == 1) {
            shared_ptr<Node> child = root->at(0);
            string child_label = child->get_root();
            if (child_label == ".EMPTY") {
                return;
            } else if (child_label == "paramlist") {
                return count_param(child, param_list);
            } 
            /* else {
                cerr << "ERROR: Unexpected transition" << endl;
                return;
            }*/
        } 
        /*else {
            cerr << "ERROR: Unexpeced transition" << endl;
            return;
        }*/
    } else if (root->get_root() == "paramlist") {
        if (root->size() == 1) {
            shared_ptr<Node> dcl = root->at(0);
            string type = "";
            if (dcl->size() == 2) {
                shared_ptr<Node> id = dcl->at(1);
                param_list.emplace_back(id->get_type());
            }
        } else if (root->size() == 3) {
            shared_ptr<Node> dcl = root->at(0);
            string type = "";
            if (dcl->size() == 2) {
                shared_ptr<Node> id = dcl->at(1);
                param_list.emplace_back(id->get_type());
            }
            count_param(root->at(2), param_list);
            return;
        } 
        /* else {
            cerr << "ERROR: Unexpected transition" << endl;
            return;
        }*/
    } 
    /* else {
        return;
    }*/
}

void add_symbol(shared_ptr<Node> root) {
    if (root->get_root() == "dcl" && root->size() == 2) {
        shared_ptr<Node> variable = (root->at(1))->at(0);
        string type = variable->get_type();
        string id = variable->get_root();
        map<string,string> &sub_map = symbol_map[current_procedure].second;
        if (sub_map.find(id) == sub_map.end()) {
            sub_map[id] = type;
        } else {
            cerr << "ERROR: Redeclaration of variable: " << id << endl; // p2
        }
    } else {
        if (root->get_root() == "expr") {
            set_expr_type(root);
        } else if (root->get_root() == "main") {
            current_procedure = ((root->at(1))->at(0))->get_root();
            if (symbol_map.find(current_procedure) != symbol_map.end()) {
                cerr << "ERROR: Redeclaration of procedure: " << current_procedure << endl; // p5
            }
            vector<string> &parameter_vec = symbol_map[current_procedure].first;
            shared_ptr<Node> dcl_1 = root->at(3);
            shared_ptr<Node> dcl_2 = root->at(5);
            string type_1 = (dcl_1->at(1))->get_type();
            string type_2 = (dcl_2->at(1))->get_type();
            parameter_vec.emplace_back(type_1);
            parameter_vec.emplace_back(type_2);
        } else if (root->get_root() == "procedure") {
            current_procedure = ((root->at(1))->at(0))->get_root();
            if (symbol_map.find(current_procedure) != symbol_map.end()) {
                cerr << "ERROR: Redeclaration of procedure: " << current_procedure << endl; // p5
            }
            vector<string> &parameter_vec = symbol_map[current_procedure].first;
            count_param(root->at(3), parameter_vec);
        } else if (root->get_root() == "statements") {
            if (root->size() == 1) { // .Empty
                ;
            } else if (root->size() == 2) { // statements statement
                ;
            }
        } else if (root->get_root() == "statement") {
            if (root->size() == 4) { // lvalue BECOMES expr SEMI
                shared_ptr<Node> LHS = root->at(0);
                shared_ptr<Node> RHS = root->at(2);
                string type_1 = "", type_2 = "";
                type_1 = set_expr_type(LHS);
                type_2 = set_expr_type(RHS);
                if (type_1 != type_2) {
                    cerr << "ERROR: Can't assign value of type " << type_2 << " to variable of type " << type_1 << endl;
                }
                return;
            } else if (root->size() == 5) {
                if (root->at(0)->get_root() == "PRINTLN") { // PRINTLN LPAREN expr RPAREN SEMI
                    shared_ptr<Node> expr = root->at(2);
                    string type = set_expr_type(expr);
                    if (type != "int") {
                        cerr << "ERROR: Can only print int, gives " << type << endl;
                    }
                } else if (root->at(0)->get_root() == "DELETE") { // DELETE LBRACK RBRACK expr SEMI
                    shared_ptr<Node> expr = root->at(3);
                    string type = set_expr_type(expr);
                    if (type != "int*") {
                        cerr << "ERROR: Can only delete int*, gives " << type << endl;
                    }
                }
                return;
            } else { // IF and WHILE, recursion will automatically checks
                ;
            }
        } else if (root->get_root() == "test") {
            if (root->size() == 3) { // expr EQ/NE/LT/LE/GE/GT expr
                shared_ptr<Node> LHS = root->at(0);
                shared_ptr<Node> RHS = root->at(2);
                string type_1 = set_expr_type(LHS);
                string type_2 = set_expr_type(RHS);
                if (type_1 != type_2) {
                    cerr << "ERROR: Can't compare value of type " << type_1 << " and value of type " << type_2 << endl;
                }
            }
            return;
        }
        for (int i = 0; i < root->size(); ++i) {
            add_symbol(root->at(i)); // this adds the paramters into map by recursion
        }
    }
}


void add_procedures(shared_ptr<Node> root) {
    if (root->get_root() == "main" || root->get_root() == "procedure") {
        add_symbol(root);
        assign_type_return(root);
    } else if (root->get_root() == "procedures") {
        if (root->size() == 1) {
            add_procedures(root->at(0));
        } else if (root->size() == 2) {
            add_procedures(root->at(0));
            add_procedures(root->at(1));
        }
    }
}

bool params_equal(shared_ptr<Node> arg_list, vector<string> &param_list, int index) {
    if (arg_list->get_root() == "arglist") {
        int size = param_list.size();
        if (arg_list->size() == 1) {
            if (index != size - 1) {
                if (index > size - 1) cerr << "ERROR: Too much arguments, expecting " << size << endl;
                if (index < size - 1) cerr << "ERROR: Too few arguments, expecting " << size << endl;
                return false;
            } else {
                string type = set_expr_type(arg_list->at(0));
                if (type == param_list[index]) {
                    return true;
                } else {
                    cerr << "ERROR: last argument doesn't match" << endl;
                    return false;
                }
            }
        } else if (arg_list->size() == 3) {
            if (!(index < size)) {
                cerr << "Too much arguments, expecting " << size << endl;
                return false;
            } else {
                string type = set_expr_type(arg_list->at(0));
                if (type != param_list[index]) {
                    cerr << "ERROR: the " << index << "-th argument doesn't match" << endl;
                    return false;
                } else {
                    return params_equal(arg_list->at(2), param_list, index+1);
                }
            }
        }
    }
    return false;
}

bool check_signature(shared_ptr<Node> root) { // ID LPAREN RPAREN, ID LPAREN arglist RPAREN
    shared_ptr<Node> id = root->at(0);
    map<string, string> &sub_map = symbol_map[current_procedure].second;
    if (sub_map.find(id->at(0)->get_root()) != sub_map.end()) {
        cerr << "ERROR: attempting to use local variable as a procedure" << endl;
        return false;
    } else if (symbol_map.find(id->at(0)->get_root()) == symbol_map.end()) {
        cerr << "ERROR: use of undefined procedure " << id->at(0)->get_root() << endl;
        return false;
    } else {
        vector<string> &param_list = symbol_map[id->at(0)->get_root()].first;

        /*
        cerr << "param_list: ";
        for (auto &it: param_list) cerr << it << " ";
        cerr << endl;
        */

        if (root->size() == 3) {
            if (param_list.size() != 0) {
                cerr << "ERROR: Too few arguments, expecting " << param_list.size() << endl;
                cerr << "ERROR: the arguments of procedure don't match with its declaration for procedure: " << id->at(0)->get_root() << endl;
                return false;
            } else {
                return true;
            }
        } else if (root->size() == 4) {
            shared_ptr<Node> arg_list = root->at(2);
            if (!params_equal(arg_list, param_list, 0)) {
                cerr << "ERROR: the arguments of procedure don't match with its declaration for procedure: " << id->at(0)->get_root() << endl;
                return false;
            } else {
                return true;
            }
        } else {
            return false;
        }
    }
}

int main() {

    //std::ifstream in("practice.wlp4i");

    string non_terminal;
    std::istringstream ss(non_terminals);
    while (ss >> non_terminal) {
        non_terminal_map[non_terminal] = 1;
    }
    

    shared_ptr<Node> tree = build_tree(nullptr, cin);
    assign_type_dcl(tree); // problem 1
    declaration_check(tree); // problem 2

    add_procedures(tree->at(1));
    //add_symbol(tree); // problem 2
    //assign_type_return(tree); // problem 1


    tree->print();

    /*
    cerr << "map size: " << symbol_map.size() << endl;
    for (auto &it : symbol_map) {
        cerr << "map " << it.first << ":";
        for (auto &itt : it.second.first) {
            cerr << itt << " ";
        }
        cerr << endl;
        for (auto &ittt : it.second.second) {
            cerr << "(" << ittt.first << "," << ittt.second << ")" << " ";
        }
        cerr << endl;
    }
    */

}