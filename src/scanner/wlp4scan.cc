#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
const std::string ALPHABET    = ".ALPHABET";
const std::string STATES      = ".STATES";
const std::string TRANSITIONS = ".TRANSITIONS";
const std::string INPUT       = ".INPUT";
const std::string EMPTY       = ".EMPTY";

bool isChar(std::string s) {
  return s.length() == 1;
}
bool isRange(std::string s) {
  return s.length() == 3 && s[1] == '-';
}

bool inRange(std::string s) {
  // test if s<=2147483647
  if (s.length() < 10) {
    return true;
  } else if (s.length() > 10) {
    return false;
  } else {
    std::string max = "2147483647";
    for (size_t i = 0; i < 10; ++i) {
      if (s[i] < max[i]) {
        return true;
      } else if (s[i] > max[i]) {
        return false;
      } 
    }
    return true;
  }
}

// Locations in the program that you should modify to store the
// DFA information have been marked with four-slash comments:
//// (Four-slash comment)
int main() {
  //std::istream& in = std::cin;
  std::ifstream in{"dfa.txt"};
  std::string s;

  std::map<char,int> symbol_set;
  std::map<std::string,int> normal_state_set;
  std::map<std::string,int> accepting_state_set;
  std::map<std::string,std::map<char,std::string>> transition_map;

  std::string initial_state = "";


  std::getline(in, s); // Alphabet section (skip header)
  // Read characters or ranges separated by whitespace
  while(in >> s) {
    if (s == STATES) { 
      break; 
    } else {
      if (isChar(s)) {
        //// Variable 's[0]' is an alphabet symbol
        symbol_set[s[0]];
      } else if (isRange(s)) {
        for(char c = s[0]; c <= s[2]; ++c) {
            //// Variable 'c' is an alphabet symbol
            symbol_set[c];
        }
      } else if (s == "space" || s == "tab" || s == "newline") {
        if (s == "space") {
          symbol_set[' '];
        } else if (s == "tab") {
          symbol_set['\t'];
        } else {
          symbol_set['\n'];
        }
      }
    }
  }

  std::getline(in, s); // States section (skip header)
  // Read states separated by whitespace
  while(in >> s) {
    if (s == TRANSITIONS) { 
      break; 
    } else {
      static bool initial = true;
      bool accepting = false;
      if (s.back() == '!' && !isChar(s)) {
        accepting = true;
        s.pop_back();
      }
      //// Variable 's' contains the name of a state
      if (initial) {
        //// The state is initial
        initial_state = s;
        initial = false;
      }
      if (accepting) {
        //// The state is accepting
        accepting_state_set[s];
      }
      
      else {
        //////// normal state
        normal_state_set[s];
      }
    }
  }

  std::getline(in, s); // Transitions section (skip header)
  // Read transitions line-by-line
  while(std::getline(in, s)) {
    if (s == INPUT) { 
      // Note: Since we're reading line by line, once we encounter the
      // input header, we will already be on the line after the header
      break; 
    } else {
      std::string fromState, symbols, toState;
      std::istringstream line(s);
      line >> fromState;
      //bool last;
      while(line >> s) {
        if(line.peek() == EOF) { // If we reached the last item on the line
          // Then it's the to-state
          toState = s;
        } else { // Otherwise, there is more stuff on the line
          // We expect a character or range
          if (isChar(s)) {
            symbols += s;
          } else if (isRange(s)) {
            for(char c = s[0]; c <= s[2]; ++c) {
              symbols += c;
            }
          } else if (s == "space" || s == "tab" || s == "newline") {
            if (s == "space") {
              symbols += ' ';
            } else if (s == "tab") {
              symbols += '\t';
            } else {
              symbols += '\n';
            }
          }
        }
      }
      if ((normal_state_set.find(fromState) == normal_state_set.end()) 
        && (accepting_state_set.find(fromState) == accepting_state_set.end())) {
        std::cerr << "ERROR: Invalid state: " << fromState << std::endl;
      }
      if ((normal_state_set.find(toState) == normal_state_set.end()) 
        && (accepting_state_set.find(toState) == accepting_state_set.end())) {
        std::cerr << "ERROR: Invalid state: " << toState << std::endl;
      }
      for ( char c : symbols ) {
        //// There is a transition from 'fromState' to 'toState' on 'c'
        if ((symbol_set.find(c) == symbol_set.end())) {
            std::cerr << "ERROR: Invalid transition: " << c << std::endl;
        }
        std::map<char,std::string> &temp_map = transition_map[fromState];
        temp_map[c] = toState;
      }
    }
  }

  s = "";

  std::map<std::string,int> intermediate_state_set;
  std::vector<std::string> intermediate_vector
    { "RETURN1", "RETURN2", "RETURN3", "RETURN4", "RETURN5", 
      "IF1", 
      "ELSE1", "ELSE2", "ELSE3",
      "WHILE1", "WHILE2", "WHILE3", "WHILE4", 
      "PRINTLN1", "PRINTLN2", "PRINTLN3", "PRINTLN4", "PRINTLN5", "PRINTLN6",  
      "WAIN2", "WAIN3", "WAIN4", 
      "INT2", 
      "NEW1", "NEW2", 
      "DELETE1", "DELETE2", "DELETE3", "DELETE4", "DELETE5", 
      "NULL1", "NULL2", "NULL3"};

  for (size_t i = 0; i < intermediate_vector.size(); ++i) {
    intermediate_state_set[intermediate_vector[i]];
  }

  // Input section (already skipped header)
  std::string token = "";
  std::string current_state = initial_state;
  std::string line = "";

  while (getline(std::cin, line)) {
    s = s + line + '\n';
  }

  while(s.length() > 0) {
    char current_char = s[0];
    //std::cout << "current state: " << current_state << std::endl;
    if (current_state == "COMMENT") {
      if (current_char == '\n') current_state = initial_state;
      s = s.substr(1);
      continue;
    }
    std::map<char,std::string> &current_map = transition_map[current_state];
    if (current_map.find(current_char) == current_map.end()) {
      if (accepting_state_set.find(current_state) == accepting_state_set.end()) {
        std::cerr << "ERROR: can't be tokenized using simplified maximal munch" << std::endl;
        return 0;
      } else {
        if (intermediate_state_set.find(current_state) != intermediate_state_set.end()) {
          current_state = "ID";
        }
        if (current_state == "NUM") {
          //check if num is <= 2147483647
          if (!inRange(token)) {
            std::cerr << "ERROR: number out of bound" << std::endl;
            return 0;
          }
        }
        if (current_state == "NUM_0") current_state = "NUM";
        if (current_state != "IGNORE") std::cout << current_state << " " << token << " " << std::endl;
        token = "";
        current_state = initial_state;
        continue;
      }
    }
    token += current_char;
    current_state = current_map[current_char];
    s = s.substr(1);
  }
  if (accepting_state_set.find(current_state) == accepting_state_set.end()) {
      std::cerr << "ERROR: can't be tokenized using simplified maximal munch" << std::endl;
  } else {
      if (current_state != "IGNORE") std::cout << current_state << " " << token << std::endl;
  }
  

  /*
  for (auto &it: symbol_set) std::cout << it.first << " ";
  std::cout << std::endl;
  for (auto &it: normal_state_set) std::cout << it.first << " ";
  std::cout << std::endl;
  for (auto &it: accepting_state_set) std::cout << it.first << " ";
  std::cout << std::endl;
  */
  /*for (auto &it_1: transition_map) {
    std::cout << "go from " << it_1.first << std::endl;
    for (auto &it_2: it_1.second) {
        std::cout << "      to " << it_2.second << " by " << it_2.first << std::endl;
    }
  }*/
  
  //std::cout << std::endl;
  
  
}