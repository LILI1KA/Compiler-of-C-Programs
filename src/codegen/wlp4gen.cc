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
using std::to_string;
using std::stoi;

int stack_change = 0;

void out(string line) {
    if (line == "add $30, $30, $4") {
        stack_change += 4;
    } else if (line == "sub $30, $30, $4") {
        stack_change -= 4;
    } else {
        if (stack_change != 0) {
            if (stack_change == 4) {
                cout << "add $30, $30, $4" << endl;
                stack_change = 0;
            } else if (stack_change == -4) {
                cout << "sub $30, $30, $4" << endl;
                stack_change = 0;
            } else if (stack_change > 0) {
                cout << "lis $12" << endl;
                cout << ".word " << stack_change << endl;
                cout << "add $30, $30, $12" << endl;
                stack_change = 0;
            } else if (stack_change < 0) {
                cout << "lis $12" << endl;
                cout << ".word " << 0-stack_change << endl;
                cout << "sub $30, $30, $12" << endl;
                stack_change = 0;
            }
        }
        cout << line << endl;
    }
        
    
}

// map<string, int> non_terminal_map;

map<string, int> last_map;

map<string, int> import_map;

map<string, int> function_will_be_used;
int checking_function = 0;

map<string,pair<int,map<string,pair<string, int>>>> symbol_map; 
// map<function name, pair(number, map(symbol, pair(type, efficiency)))>
map<string,map<string, string>> value_map;

string current_procedure = "";

int label_index = 0;

string get_label() {
    string label = "L" + to_string(label_index);
    ++label_index;
    return label;
}

void code_start(shared_ptr<Node> root);
void code_procedures(shared_ptr<Node> root);
void code_procedure(shared_ptr<Node> root);
void code_params(shared_ptr<Node> root);
void code_paramlist(shared_ptr<Node> root);
void code_main(shared_ptr<Node> root);
void code_type(shared_ptr<Node> root);
void code_dcl(shared_ptr<Node> root);
void code_dcls(shared_ptr<Node> root);
void code_statements(shared_ptr<Node> root);
void code_statement(shared_ptr<Node> root);
void code_test(shared_ptr<Node> root);
void code_expr(shared_ptr<Node> root);
void code_term(shared_ptr<Node> root);
void code_factor(shared_ptr<Node> root);
void code_NUM(shared_ptr<Node> root);
void code_ID(shared_ptr<Node> root);
void code_arglist(shared_ptr<Node> root);
void code_arglist_helper(shared_ptr<Node> root);
void code_return(shared_ptr<Node> root);
void code_lvalue(shared_ptr<Node> root);

string calculate(shared_ptr<Node> root);

int testing = 0; // never treat id as constant for testing in while as control flow can go back

int get_symbol_location(string str) {
    map<string,pair<string, int>> &sub_map = symbol_map[current_procedure].second;
    return sub_map[str].second;
}


void init_value(string key) {
    value_map[current_procedure][key] = "false";
}

void set_value(string key, string value) {
    value_map[current_procedure][key] = value;
}

string get_value(string str) {
    return value_map[current_procedure][str];
}

void reset_values() {
    for (auto &itt:value_map[current_procedure]) {
        itt.second = "false";
    }
}

void add_symbol_table(shared_ptr<Node> root) {
    if (root->get_root() == "dcl") {
        shared_ptr<Node> id = root->at(1)->at(0);
        string id_name = id->get_root();
        string id_type = id->get_root();
        int symbol_location = symbol_map[current_procedure].first;
        map<string,pair<string, int>> &sub_map = symbol_map[current_procedure].second;
        sub_map[id_name] = make_pair(id_type, symbol_location);
        symbol_map[current_procedure].first = symbol_location - 4;
        init_value(id_name);
        return;
    } else if (root->get_root() == "procedure" || root->get_root() == "main") {
        string procedure_name = root->at(1)->at(0)->get_root();
        current_procedure = procedure_name;
        symbol_map[current_procedure].first = 0;
        value_map[current_procedure];
    }
    for (int i = 0; i < root->size(); ++i) {
        add_symbol_table(root->at(i));
    }
}


void push(string reg) {
    out("sw "+reg+", -4($30)");
    out("sub $30, $30, $4");
}

void pop(string reg) {
    out("add $30, $30, $4");
    out("lw "+reg+", -4($30)");
}

bool same(shared_ptr<Node> left, shared_ptr<Node> right) {
    if (left->get_root() != right->get_root() || (left->size() > 1 && left->at(1)->get_root() == "LPAREN" )) { // can't be function
        return false;
    } else {
        if (left->size() != right->size()) {
            return false;
        } else {
            for (int i = 0; i < left->size(); ++i) {
                if (!same(left->at(i),right->at(i))) return false;
            }
            return true;
        }
    }
}


void code_start(shared_ptr<Node> root) {
    out("lis $4");
    out(".word 4");
    out("lis $11");
    out(".word 1");
    if (root->get_root() == "start") {
        if (root->size() == 3) { // BOF procedures EOF
            code_procedures(root->at(1));
        }
    }
}

void code_procedures(shared_ptr<Node> root) {
    if (root->get_root() == "procedures") {
        if (root->size() == 1) { // main
            code_main(root->at(0));
        } else if (root->size() == 2) {
            code_procedures(root->at(1));
            code_procedure(root->at(0));
        }
    }
}


void code_params(shared_ptr<Node> root) {
    if (root->get_root() == "params") {
        if (root->size() == 1) { // .EMPTY
            if (root->at(0)->get_root() == ".EMPTY") {    
            } else if (root->at(0)->get_root() == "paramlist") { // paramlist
                code_paramlist(root->at(0));
            }
        }
    }
}

void code_paramlist(shared_ptr<Node> root) {
    if (root->get_root() == "paramlist") {
        if (root->size() == 1) { // dcl
            shared_ptr<Node> dcl = root->at(0);
            //code_dcl(dcl);
        } else if (root->size() == 3) { // dcl COMMA paramlist
            shared_ptr<Node> dcl = root->at(0);
            shared_ptr<Node> paramlist = root->at(2);
            //code_dcl(dcl);
            code_paramlist(paramlist);
        }
    }
}

bool need_allocate(shared_ptr<Node> root) {
    if (root->get_root() == "new" || root->get_root() == "delete") {
        return true;
    } else {
        for (int i = 0; i < root->size(); ++i) {
            if (need_allocate(root->at(i))) return true;
        }
        return false;
    }
}

void code_main(shared_ptr<Node> root) {
    if (root->get_root() == "main") { // only one main
        testing = 0;
        checking_function = 1;
        current_procedure = root->at(1)->at(0)->get_root();
        reset_values();
        int total_shift = 0 - symbol_map[current_procedure].first;
        out("sub $29, $30, $4");
        int count = total_shift;
        while (count != 0) {
            out("sub $30, $30, $4");
            count -= 4;
        }
        shared_ptr<Node> dcl_1 = root->at(3);
        shared_ptr<Node> dcl_2 = root->at(5);
        shared_ptr<Node> dcls = root->at(8);
        shared_ptr<Node> statements = root->at(9);
        shared_ptr<Node> expr = root->at(11);
        int symbol_location_1 = get_symbol_location(dcl_1->at(1)->at(0)->get_root());
        int symbol_location_2 = get_symbol_location(dcl_2->at(1)->at(0)->get_root());
        out("sw $1, "+to_string(symbol_location_1)+"($29)");
        out("sw $2, "+to_string(symbol_location_2)+"($29)");
        string expr1_type = dcl_1->at(1)->at(0)->get_type();

        if (need_allocate(root)) {
            if (expr1_type == "int") {
            out("lis $2");
            out(".word 0");
            }
            int in_test = testing;
            testing = 1;
            out(".import init");
            if (!in_test) testing = 0;
            push("$31");
            out("lis $5");
            out(".word init");
            out("jalr $5");
            pop("$31");
        }

        code_dcls(dcls);
        code_statements(statements);
        code_return(expr);
    }
}

void code_dcl(shared_ptr<Node> root) {
    if (root->get_root() == "dcl") { // type ID
        shared_ptr<Node> id = root->at(1)->at(0);
        string id_name = id->get_root();
        int symbol_location = get_symbol_location(id_name);
        out("sw $0, "+to_string(symbol_location)+"($29)");
    }
}

void code_dcls(shared_ptr<Node> root) {
    if (root->get_root() == "dcls") {
        if (root->size() == 1) { // .EMPTY
            return;
        } else if (root->size() == 5) { // dcls dcl BECOMES NUM/NULL SEMI
            code_dcls(root->at(0));
            shared_ptr<Node> dcl = root->at(1);
            shared_ptr<Node> value = root->at(3);
            string id_name = dcl->at(1)->at(0)->get_root();
            string val = "";
            if (value->get_root() == "NUM") {
                val = value->at(0)->get_root();
                out("lis $3");
                out(".word "+val);
                if (!testing) {
                    set_value(id_name, val);
                } else {
                    init_value(id_name);
                }
            } else if (value->get_root() == "NULL") {
                out("add $3, $0, $11");
                val = value->at(0)->get_root();
                if (val == "NULL") val = "1";
                if (!testing) {
                    set_value(id_name, val);
                } else {
                    init_value(id_name);
                }
            }
            int symbol_location = get_symbol_location(id_name);
            //code_dcl(dcl);
            out("sw $3, "+to_string(symbol_location)+"($29)");
        }
    }
}

void code_statements(shared_ptr<Node> root) {
    if (root->get_root() == "statements") {
        if (root->size() == 1) { // .EMPTY
            return;
        } else if (root->size() == 2) { // statements statement
            code_statements(root->at(0));
            code_statement(root->at(1));
        }
    }
}

string get_lvalue_id(shared_ptr<Node> root) {
    if (root->get_root() == "lvalue") {
        if (root->size() == 1) { // ID
            return root->at(0)->at(0)->get_root();
        } else if (root->size() == 3) { // LPAREN lvalue RPAREN
            return get_lvalue_id(root->at(1));
        } else {
            return "";
        }
    } else {
        return "";
    }
}

string test_true_forever = "";

void code_statement(shared_ptr<Node> root) {
    if (root->get_root() == "statement") {
        if (root->size() == 4) { // lvalue BECOMES expr SEMI
            shared_ptr<Node> lvalue = root->at(0);
            shared_ptr<Node> expr = root->at(2);
            if (lvalue->size() == 1) { // lvalue->ID
                size1:
                string lvalue_id = get_lvalue_id(lvalue);
                init_value(lvalue_id);
                int symbol_location = get_symbol_location(lvalue_id);
                code_expr(expr);
                out("sw $3, "+to_string(symbol_location)+"($29)");
                string value = calculate(expr);
                if (!testing) {
                    set_value(lvalue_id, value);
                } else {
                    init_value(lvalue_id);
                }
            } else if (lvalue->size() == 2) { // lvalue->STAR factor
                size2:
                code_expr(expr);
                push("$3");
                code_factor(lvalue->at(1));
                pop("$5");
                out("sw $5, 0($3)");
                reset_values();
            } else if (lvalue->size() == 3) { // lvalue->LPAREN lvalue RPAREN
                while (lvalue->size() == 3) {
                    lvalue = lvalue->at(1);
                }
                if (lvalue->size() == 1) {
                    goto size1;
                } else if (lvalue->size() == 2) {
                    goto size2;
                }
            }
        } else if (root->size() == 11) { // if-else statement
            shared_ptr<Node> test = root->at(2);
            shared_ptr<Node> true_statements = root->at(5);
            shared_ptr<Node> false_statements = root->at(9);
            string comparison = test->at(1)->get_root();
            string true_label = get_label();
            string false_label = get_label();
            test_true_forever = "";
            code_test(test);
            if (test_true_forever != "") {
                if (test_true_forever == "true") {
                    code_statements(true_statements);
                } else if (test_true_forever == "false") {
                    code_statements(false_statements);
                } else {
                    test_true_forever = "";
                    goto normal;
                }
                test_true_forever = "";
                return;
            }
            normal:
            int in_test = testing;
            testing = 1;
            if (comparison == "NE" || comparison == "LT" || comparison == "GT") {
                out("bne $3, $0, "+true_label);
            } else if (comparison == "EQ" || comparison == "GE" || comparison == "LE") {
                out("beq $3, $0, "+true_label);
            }
            code_statements(false_statements);
            out("beq $0, $0, "+false_label);
            out(true_label+":");
            code_statements(true_statements);
            out(false_label+":");
            if (!in_test) testing = 0;
        } else if (root->size() == 7) { // while loop statement
            shared_ptr<Node> test = root->at(2);
            shared_ptr<Node> true_statements = root->at(5);
            string comparison = test->at(1)->get_root();
            string continue_label = get_label();
            string end_label = get_label();
            out(continue_label+":");
            int in_test = testing;
            testing = 1;
            code_test(test);
            if (test_true_forever != "") {
                if (test_true_forever == "true") {
                    out("beq $0, $0, 0");
                } else if (test_true_forever == "false") {
                } else {
                    test_true_forever = "";
                    goto normal_while;
                }
                test_true_forever = "";
                return;
            }
            normal_while:
            if (comparison == "NE" || comparison == "LT" || comparison == "GT") {
                out("beq $3, $0, "+end_label);
            } else if (comparison == "EQ" || comparison == "GE" || comparison == "LE") {
                out("bne $3, $0, "+end_label);
            }
            code_statements(true_statements);
            out("beq $0, $0, "+continue_label);
            out(end_label+":");
            if (!in_test) testing = 0;
        } else if (root->size() == 5) { // PRINTLN LPAREN expr RPAREN SEMI / DELETE LBRACK RBRACK expr SEMI
            if (root->at(0)->get_root() == "PRINTLN") { // PRINTLN LPAREN expr RPAREN SEMI
                if (import_map.find("print") == import_map.end()) {
                    int in_test = testing;
                    testing = 1;
                    out(".import print");
                    import_map["print"];
                    if (!in_test) testing = 0;
                }
                push("$31");
                code_expr(root->at(2));
                out("add $1, $3, $0");
                out("lis $10");
                out(".word print");
                out("jalr $10");
                pop("$31");
            } else if (root->at(0)->get_root() == "DELETE") { // DELETE LBRACK RBRACK expr SEMI
                if (import_map.find("delete") == import_map.end()) {
                    int in_test = testing;
                    testing = 1;
                    out(".import delete");
                    import_map["delete"];
                    if (!in_test) testing = 0;
                }
                code_expr(root->at(3));
                string skip = get_label();
                out("beq $11, $3, "+skip);
                out("add $1, $3, $0");
                push("$31");
                out("lis $5");
                out(".word delete");
                out("jalr $5");
                pop("$31");
                out(skip+":");

            }
        }
    }
}

void code_test(shared_ptr<Node> root) {
    if (root->get_root() == "test") {
        if (root->size() == 3) {
            shared_ptr<Node> LHS = root->at(0);
            shared_ptr<Node> RHS = root->at(2);
            string left = calculate(LHS);
            string right = calculate(RHS);
            string comparison = root->at(1)->get_root();

            if (left != "false" && right != "false") {
                int num_1 = stoi(left);
                int num_2 = stoi(right);
                if (comparison == "EQ") {
                    if (num_1 == num_2) {
                        test_true_forever = "true";
                    } else {
                        test_true_forever = "false";
                    }
                    return;
                } else if (comparison == "NE") {
                    if (num_1 != num_2) {
                        test_true_forever = "true";
                    } else {
                        test_true_forever = "false";
                    }
                    return;
                } else if (comparison == "LT") {
                    if (num_1 < num_2) {
                        test_true_forever = "true";
                    } else {
                        test_true_forever = "false";
                    }
                    return;
                } else if (comparison == "GT") {
                    if (num_1 > num_2) {
                        test_true_forever = "true";
                    } else {
                        test_true_forever = "false";
                    }
                    return;
                } else if (comparison == "LE") {
                    if (!(num_1 > num_2)) {
                        test_true_forever = "true";
                    } else {
                        test_true_forever = "false";
                    }
                    return;
                } else if (comparison == "GE") {
                    if (!(num_1 < num_2)) {
                        test_true_forever = "true";
                    } else {
                        test_true_forever = "false";
                    }
                    return;
                }
                out("lis $5");
                out(".word "+left);
                out("lis $3");
                out(".word "+right);
            } else if (left != "false") {
                code_expr(RHS); // $3 already here
                out("lis $5");
                out(".word "+left);
            } else if (right != "false") {
                code_expr(LHS);
                out("add $5, $3, $0");
                out("lis $3");
                out(".word "+right);
            } else {
                code_expr(LHS);
                out("sw $3, -4($30)");
                out("sub $30, $30, $4");
                code_expr(RHS);
                out("add $30, $30, $4");
                out("lw $5, -4($30)");
            }

            shared_ptr<Node> expr = root->at(0);
            // now $5 is the first register, $3 is the second register
            if (comparison == "EQ" || comparison == "NE") { // expr EQ expr, expr NE expr
                out("sub $3, $5, $3");
                // EQ is true <=> $3 == $0, NE is true <=> $3 != $0
            } else if (comparison == "LT" || comparison == "GE") { // expr LT expr, expr GE expr
                if (expr->get_type() == "int*") {
                    out("sltu $3, $5, $3");
                } else if (expr->get_type() == "int") {
                    out("slt $3, $5, $3");
                }                
                // LT is true <=> $3 != $0, GE is true <=> $3 == $0
            } else if (comparison == "GT" || comparison == "LE") { // expr GT expr, expr LE expr
                if (expr->get_type() == "int*") {
                    out("sltu $3, $3, $5");
                } else if (expr->get_type() == "int") {
                    out("slt $3, $3, $5");
                }
                // GT is true <=> $3 != $0, LE is true <=> $3 == $0
            }
        }
    }
}



string calculate(shared_ptr<Node> root) {
    if (root->get_root() == "NUM") {
        return root->at(0)->get_root();
    } else if (root->get_root() == "ID") {
        if (!testing) {
            return get_value(root->at(0)->get_root());
        } else {
            return "false";
        }
    } else if (root->get_root() == "NULL") {
        return "1";
    } else if (root->get_root() == "expr") {
        if (root->size() == 1) { // term
            return calculate(root->at(0));
        } else if (root->size() == 3) { // expr PLUS/MINUS term
            string expr1 = calculate(root->at(0));
            string expr2 = calculate(root->at(2));
            string type1 = root->at(0)->get_type();
            string type2 = root->at(2)->get_type();
            if (expr1 == "false" || expr2 == "false") {
                return "false";
            } else {
                int num1 = stoi(expr1);
                int num2 = stoi(expr2);
                string operation = root->at(1)->get_root();
                if (operation == "PLUS") {
                    if (type1 == "int*") num1 = num1*4;
                    if (type2 == "int*") num2 = num2*4;
                    return to_string(num1+num2);
                } else if (operation == "MINUS") {
                    if (type2 == "int*") num2 = num2*4;
                    int result = num1-num2;
                    if (type1 == "int*" && type2 == "int*") result = result/4;
                    return to_string(result);
                } else {
                    return "false";
                }
            }
        } else {
            return "false";
        }
    } else if (root->get_root() == "term") {
        if (root->size() == 1) { // factor
            return calculate(root->at(0));
        } else if (root->size() == 3) { // term STAR/SLASH/PCT factor
            string expr1 = calculate(root->at(0));
            string expr2 = calculate(root->at(2));
            if (expr1 == "false" || expr2 == "false") {
                return "false";
            } else {
                int num1 = stoi(expr1);
                int num2 = stoi(expr2);
                string operation = root->at(1)->get_root();
                if (operation == "STAR") {
                    return to_string(num1*num2);
                } else if (operation == "SLASH" && num2 != 0) {
                    return to_string(num1/num2);
                } else if (operation == "PCT" && num2 != 0) {
                    return to_string(num1%num2);
                } else {
                    return "false";
                }
            }
        } else {
            return "false";
        }
    } else if (root->get_root() == "factor") {
        if (root->size() == 1) {
            return calculate(root->at(0));
        } else if (root->size() == 3) { // LPAREN expr RPAREN
            return calculate(root->at(1));
        } else {
            return "false";
        }
    } else {
        return "false";
    }
}


void code_expr(shared_ptr<Node> root) {
    if (root->get_root() == "expr") {
        if (root->size() == 1) { // term
            code_term(root->at(0));
        } else if (root->size() == 3) {
            string operation = root->at(1)->get_root();
            shared_ptr<Node> expr = root->at(0);
            shared_ptr<Node> term = root->at(2);
            string calculate_1 = calculate(expr);
            string calculate_2 = calculate(term);
            if (calculate_1 != "false" && calculate_2 != "false") {
                int num_1 = stoi(calculate_1);
                int num_2 = stoi(calculate_2);
                string type_1 = expr->get_type();
                string type_2 = term->get_type();
                int result = 0;
                if (operation == "PLUS") {
                    if (type_1 == "int*" && type_2 == "int") {
                        num_2 = num_2 * 4;
                    } else if (type_1 == "int" && type_2 == "int*") {
                        num_1 = num_1 * 4;
                    }
                    result = num_1+num_2;
                } else if (operation == "MINUS") {
                    if (type_1 == "int*" && type_2 == "int") num_2 = num_2 * 4;
                    result = num_1-num_2;
                    if (type_1 == "int*" && type_2 == "int*") result = result/4;
                }
                if (result == 0) {
                    out("add $3, $0, $0");
                } else if (result == 1) {
                    out("add $3, $0, $11");
                } else if (result == 4) {
                    out("add $3, $0, $4");
                } else if (result == 2) {
                    out("add $3, $11, $11");
                } else if (result == 8) {
                    out("add $3, $4, $4");
                } else if (result == -1) {
                    out("sub $3, $0, $11");
                } else if (result == -4) {
                    out("sub $3, $0, $4");
                } else {
                    out("lis $3");
                    out(".word "+to_string(result));
                }
                return;
            } else if (calculate_1 != "false") {
                code_term(term); // $3 is already here, may change $5
                int num = stoi(calculate_1);
                if (operation == "PLUS") { // num+term
                    if (term->get_type() == "int*") {
                        num = num * 4;
                    } else if (expr->get_type() == "int*") {
                        out("mult $3, $4");
                        out("mflo $3");
                    }
                    if (num == 0) {
                        return;
                    } else if (num == 1) {
                        out("add $3, $11, $3");
                        return;
                    } else if (num == 4) {
                        out("add $3, $4, $3");
                        return;
                    }
                } else if (operation == "MINUS") { // num-term
                    if (expr->get_type() == "int*" && term->get_type() == "int") {
                        out("mult $3, $4");
                        out("mflo $3");
                    }
                    if (num == 0) {
                        out("sub $3, $0, $3");
                        return;
                    } else if (num == 1) {
                        out("sub $3, $11, $3");
                    } else if (num == 4) {
                        out("sub $3, $4, $3");
                    } else {
                        goto normal_id_minus_number;
                    }
                    if (expr->get_type() == "int*" && term->get_type() == "int*") {
                        out("div $3, $4");
                        out("mflo $3");
                    }
                    return;
                }
                normal_id_minus_number:
                out("lis $5");
                out(".word "+calculate_1);
            } else if (calculate_2 != "false") {
                code_expr(expr);
                int num = stoi(calculate_2);
                if (operation == "PLUS") { // expr+num
                    if (expr->get_type() == "int*") {
                        num = num * 4;
                    } else if (term->get_type() == "int*") {
                        out("mult $3, $4");
                        out("mflo $3");
                    }
                    if (num == 0) {
                        return;
                    } else if (num == 1) {
                        out("add $3, $3, $11");
                        return;
                    } else if (num == 4) {
                        out("add $3, $3, $4");
                        return;
                    }
                } else if (operation == "MINUS") { // expr-num
                    if (expr->get_type() == "int*" && term->get_type() == "int") {
                        num = num*4;
                    }
                    if (num == 0) {
                        return;
                    } else if (num == 1) {
                        out("sub $3, $3, $11");
                    } else if (num == 4) {
                        out("sub $3, $3, $4");
                    } else {
                        goto normal_number_minus_id;
                    }
                    if (expr->get_type() == "int*" && term->get_type() == "int*") {
                        out("div $3, $4");
                        out("mflo $3");
                    }
                    return;
                }
                normal_number_minus_id:
                out("add $5, $3, $0");
                out("lis $3");
                out(".word "+calculate_2);
            } else {
                code_expr(expr);
                out("sw $3, -4($30)");
                out("sub $30, $30, $4");
                code_term(term);
                out("add $30, $30, $4");
                out("lw $5, -4($30)");
            }
            
            // now $5 is the first register, $3 is the second register
            if (operation == "PLUS") { // expr PLUS term
                if (expr->get_type() == "int*") {
                    out("mult $3, $4");
                    out("mflo $3");
                } else if (term->get_type() == "int*") {
                    out("mult $5, $4");
                    out("mflo $5");
                }
                out("add $3, $5, $3");
            } else if (operation == "MINUS") { // expr MINUS term
                if (expr->get_type() == "int*" && term->get_type() == "int") {
                    out("mult $3, $4");
                    out("mflo $3");
                }
                out("sub $3, $5, $3");
                if (expr->get_type() == "int*" && term->get_type() == "int*") {
                    out("div $3, $4");
                    out("mflo $3");
                }
            }
        }
    }
}

void code_lvalue(shared_ptr<Node> root) {
    if (root->get_root() == "lvalue") {
        if (root->size() == 1) { // ID
            code_ID(root->at(0));
        } else if (root->size() == 3) { // LPAREN lvalue RPAREN
            code_lvalue(root->at(1));
        } else if (root->size() == 2) { // STAR factor
            code_factor(root->at(1));
            out("lw $3, 0($3)");
        }
    }
}

void code_term(shared_ptr<Node> root) {
    if (root->get_root() == "term") {
        if (root->size() == 1) { // factor
            code_factor(root->at(0));
        } else if (root->size() == 3) {
            string operation = root->at(1)->get_root();
            shared_ptr<Node> term = root->at(0);
            shared_ptr<Node> factor = root->at(2);
            string calculate_1 = calculate(term);
            string calculate_2 = calculate(factor);

            if (calculate_1 != "false" && calculate_2 != "false") {
                int num_1 = stoi(calculate_1);
                int num_2 = stoi(calculate_2);
                int result = 0;
                if (operation == "STAR") {
                    result = num_1*num_2;
                } else if (operation == "SLASH" && num_2 != 0) {
                    result = num_1/num_2;
                    return;
                } else if (operation == "PCT" && num_2 != 0) {
                    result = num_1%num_2;
                    return;
                } else {
                    out("lis $5");
                    out(".word "+calculate_1);
                    out("lis $3");
                    out(".word "+calculate_2);
                    goto end;
                }
                if (result == 0) {
                    out("add $3, $0, $0");
                } else if (result == 1) {
                    out("add $3, $0, $11");
                } else if (result == 4) {
                    out("add $3, $0, $4");
                } else if (result == 2) {
                    out("add $3, $11, $11");
                } else if (result == 8) {
                    out("add $3, $4, $4");
                } else if (result == -1) {
                    out("sub $3, $0, $11");
                } else if (result == -4) {
                    out("sub $3, $0, $4");
                } else {
                    out("lis $3");
                    out(".word "+to_string(result));
                }
                return;
            } else if (calculate_1 != "false") {
                if (operation == "STAR" && calculate_1 == "2") {
                    code_factor(factor);
                    out("add $3, $3, $3");
                    return;
                } else if (operation == "STAR" && calculate_1 == "1") {
                    code_factor(factor);
                    out("add $3, $3, $0");
                    return;
                } else if (operation == "STAR" && calculate_1 == "0") {
                    out("add $3, $0, $0");
                    return;
                } else if (operation == "SLASH" && calculate_1 == "0") {
                    out("add $3, $0, $0");
                    return;
                } else if (operation == "PCT" && calculate_1 == "0") {
                    out("add $3, $0, $0");
                    return;
                }
                code_factor(factor); // $3 is already here
                out("lis $5");
                out(".word "+calculate_1);
            } else if (calculate_2 != "false") {
                if (operation == "STAR" && calculate_2 == "2") {
                    code_term(term);
                    out("add $3, $3, $3");
                    return;
                } else if (operation == "STAR" && calculate_2 == "1") {
                    code_term(term);
                    out("add $3, $3, $0");
                    return;
                } else if (operation == "STAR" && calculate_2 == "0") {
                    out("add $3, $0, $0");
                    return;
                } else if (operation == "SLASH" && calculate_2 == "1") {
                    code_term(term);
                    out("add $3, $3, $0");
                    return;
                } else if (operation == "PCT" && calculate_2 == "1") {
                    out("add $3, $0, $0");
                    return;
                }
                code_term(term);
                out("add $5, $3, $0");
                out("lis $3");
                out(".word "+calculate_2);
            } else {
                code_term(root->at(0));
                out("sw $3, -4($30)");
                out("sub $30, $30, $4");
                code_factor(root->at(2));
                out("add $30, $30, $4");
                out("lw $5, -4($30)");
            }
            
            end:
            // now $5 is the first register, $3 is the second register
            if (operation == "STAR") { // term STAR factor
                out("mult $5, $3");
                out("mflo $3");
            } else if (operation == "SLASH") { // term SLASH factor
                out("div $5, $3");
                out("mflo $3");
            } else if (operation == "PCT") { // term PCT factor
                out("div $5, $3");
                out("mfhi $3");
            }
        }
    }
}

void code_address(shared_ptr<Node> root) { // Only factor->AMP lvalue calls it, for case 1(a),(b),(c)
    if (root->get_root() == "lvalue") {
        if (root->size() == 1) { // ID
            string id_name = root->at(0)->at(0)->get_root();
            int symbol_location = get_symbol_location(id_name);
            out("lis $3");
            out(".word "+to_string(symbol_location));
            out("add $3, $3, $29");
        } else if (root->size() == 3) { // LPAREN lvalue RPAREN
            code_address(root->at(1));
        } else if (root->size() == 2) { // STAR FACTOR, STAR cancelled with AMP who calls it
            code_factor(root->at(1));
        }
    }
}


void code_factor(shared_ptr<Node> root) {
    if (root->get_root() == "factor") {
        if (root->size() == 1) {
            shared_ptr<Node> child = root->at(0);
            if (child->get_root() == "NUM") { // NUM
                code_NUM(child);
            } else if (child->get_root() == "ID") { // ID
                code_ID(child);
            } else if (child->get_root() == "NULL") { // NULL
                out("add $3, $0, $11");
            }
        } else if (root->size() == 2) {
            if (root->at(0)->get_root() == "STAR") { // STAR factor
                code_factor(root->at(1));
                out("lw $3, 0($3)");
            } else if (root->at(0)->get_root() == "AMP") { // AMP lvalue
                code_address(root->at(1)); // looking for the address of lvalue
            }
        } else if (root->size() == 3) { // LPAREN expr RPAREN
            if (root->at(0)->get_root() == "LPAREN") {
                code_expr(root->at(1));
            } else if (root->at(0)->get_root() == "ID") { // ID LPAREN RPAREN
                if (checking_function == 1) {
                    string function_name = root->at(0)->at(0)->get_root();
                    function_will_be_used[function_name] = 1;
                }
                // function without input: f()
                string procedure_name = root->at(0)->at(0)->get_root();
                out("sw $31, -4($30)");
                out("sw $29, -8($30)");
                out("sw $5, -12($30)");
                out("sub $30, $30, $4");
                out("sub $30, $30, $4");
                out("sub $30, $30, $4");
                

                out("sub $29, $30, $4");

                string procedure_label = "F" + procedure_name;
                out("lis $5");
                out(".word "+procedure_label);
                out("jalr $5");
                
                out("add $30, $30, $4");
                out("add $30, $30, $4");
                out("add $30, $30, $4");
                out("lw $5, -12($30)");
                out("lw $29, -8($30)");
                out("lw $31, -4($30)");
                

            }
        } else if (root->size() == 4) { // ID LPAREN arglist RPAREN
            if (checking_function == 1) {
                string function_name = root->at(0)->at(0)->get_root();
                function_will_be_used[function_name] = 1;
            }
            // function with inputs: f(a,b,c,...)
            shared_ptr<Node> arglist = root->at(2);

            out("sw $31, -4($30)");
            out("sw $29, -8($30)");
            out("sw $5, -12($30)");
            out("sub $30, $30, $4");
            out("sub $30, $30, $4");
            out("sub $30, $30, $4");

            code_arglist(arglist);
            
            int count = 4;
            while (arglist->size() == 3) {
                count = count+4;
                arglist = arglist->at(2);
            }
            int copy = count;
            while (copy != 0) {
                out("add $30, $30, $4");
                copy -= 4;
            }

            out("sub $29, $30, $4");

            string procedure_label = "F" + root->at(0)->at(0)->get_root();
            out("lis $5");
            out(".word "+procedure_label);
            out("jalr $5");


            out("add $30, $30, $4");
            out("add $30, $30, $4");
            out("add $30, $30, $4");
            out("lw $5, -12($30)");
            out("lw $29, -8($30)");
            out("lw $31, -4($30)");

        } else if (root->size() == 5) { // NEW INT LBRACK expr RBRACK
            if (import_map.find("new") == import_map.end()) {
                int in_test = testing;
                testing = 1;
                out(".import new");
                import_map["new"];
                if (!in_test) testing = 0;
            }
            code_expr(root->at(3));
            out("add $1, $3, $0");
            push("$31");
            out("lis $5");
            out(".word new");
            out("jalr $5");
            pop("$31");
            string skip = get_label();
            out("bne $3, $0, "+skip);
            out("lis $3");
            out(".word 1");
            out(skip+":");
        }
    }
}

void code_procedure(shared_ptr<Node> root) {
    if (root->get_root() == "procedure") { // only one procedure
        testing = 0;
        string function_name = root->at(1)->at(0)->get_root();
        if (function_will_be_used.find(function_name) != function_will_be_used.end()) {
            checking_function = 1;
        } else {
            checking_function = 0;
            return;
        }
        current_procedure = root->at(1)->at(0)->get_root();
        reset_values();
        string procedure_label = "F" + current_procedure + ": ";
        out(procedure_label);

        int total_shift = 0 - symbol_map[current_procedure].first;
        int count = total_shift;
        while (count != 0) {
            out("sub $30, $30, $4");
            count -=4;
        }

        shared_ptr<Node> params = root->at(3);
        shared_ptr<Node> dcls = root->at(6);
        shared_ptr<Node> statements = root->at(7);
        shared_ptr<Node> expr = root->at(9);
        code_params(params);
        code_dcls(dcls);
        code_statements(statements);
        code_expr(expr);

        out("add $30, $29, $4");
        out("jr $31");
    }
}

void code_arglist(shared_ptr<Node> root) { code_arglist_helper(root); }

void code_arglist_helper(shared_ptr<Node> root) {
    if (root->size() == 1) { // expr
        code_expr(root->at(0));
        push("$3");
    } else if (root->size() == 3) { // expr COMMA arglist
        code_expr(root->at(0));
        push("$3");
        code_arglist_helper(root->at(2));
    }
}

void code_NUM(shared_ptr<Node> root) {
    if (root->get_root() == "NUM") {
        string num = root->at(0)->get_root();
        out("lis $3");
        out(".word "+num);
    }
}

void code_ID(shared_ptr<Node> root) {
    if (root->get_root() == "ID") {
        string id_name = root->at(0)->get_root();
        string value = calculate(root);
        if (value != "false") {
            out("lis $3");
            out(".word "+value);
        } else {
            int symbol_location = get_symbol_location(id_name);
            out("lw $3, "+to_string(symbol_location)+"($29)");
        }
    }
}

void code_return(shared_ptr<Node> root) {
    code_expr(root);
    out("add $30, $29, $4");
    out("jr $31");
}





shared_ptr<Node> build_tree(shared_ptr<Node> root, std::istream &in) {
    string word, line, label;
    in >> word;
    if (!root) root = make_shared<Node>(word);
    getline(in,line);
    std::istringstream words(line);

    while (words >> word) {
        //cerr << "word: " << word << " " << endl;
        if (word == ":") {
            words >> word;
            root->set_type(word);
            break;
        }
        shared_ptr<Node> child = make_shared<Node> (word);
        root->add(child);
    }

    //cerr << "current root: " << root->get_root() << endl;
    string root_label = root->get_root();
    for (int i = 0; i < root->size(); ++i) {
        label = (root->at(i))->get_root();
        //cerr << "label out: " << label << endl;
        if ((label != ".EMPTY") && (last_map.find(root_label) != last_map.end())) {
            //cerr << "map size: " << non_terminal_map.size() << endl;
            //cerr << "label in: " << label << endl;
            shared_ptr<Node> child = build_tree(root->at(i), in);
            root->at(i) = child;
        }
    }
    return root;
}

int main() {

    //std::ifstream cin("copy.wlp4ti");

    string non_terminal;
    std::istringstream ss(non_terminals);
    string line = "";
    while (getline(ss,line)) {
        std::istringstream liness(line);
        liness >> non_terminal;
        //non_terminal_map[non_terminal] = 1;
        last_map[non_terminal] = 1;
        //while (liness >> non_terminal) non_terminal_map[non_terminal] = 1;
    }

    shared_ptr<Node> tree = build_tree(nullptr, cin);
    
    //tree->print();

    
    add_symbol_table(tree);
    
    code_start(tree);
    
    /*
    // map<function name, pair(number, map(symbol, pair(type, offset)))>
    cerr << "symbol map:" << endl;
    cerr << "map size: " << symbol_map.size() << endl;
    for (auto &it : symbol_map) {
        cerr << "map " << it.first << ":";
        cerr << "total shift: " << it.second.first << "";
        cerr << endl;
        for (auto &ittt : it.second.second) {
            cerr << "(" << ittt.first << "," << ittt.second.second << ")" << " ";
        }
        cerr << endl;
    }
    cerr << "value map:" << endl;
    for (auto &it: value_map) {
        cerr << "map " << it.first << ":" << endl;
        for (auto &ittt : it.second) {
            cerr << "(" << ittt.first << "," << ittt.second << ")" << " ";
        }
        cerr << endl;
    }
    */
    
    
    
    
    

}