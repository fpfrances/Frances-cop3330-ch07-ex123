/*
 *  UCF COP3330 Fall 2021 Assignment 6 Solution
 *  Copyright 2021 Filipe Frances
 */
#include "Header.h"

struct Token{
	char kind;
	double value;
	std::string name;
	Token(char ch) :kind(ch), value(0){ 
	}
	Token(char ch, double val) :kind(ch), value(val){
	}
	Token(char ch, std::string n) :kind(ch), name(n){
	}
};

class Token_stream{
	bool full;
	Token buffer;
	public:
	Token_stream() :full(0), buffer(0){
	}
	Token get();
	void unget(Token t){
		buffer = t; full = true;
	}
	void ignore(char);
};

const char let = 'L';
const char quit = 'Q';
const char print = ';';
const char number = '8';
const char name = 'a';
const char isConst = 'C';
const std::string constKey = "const";
const std::string prompt = "> ";
const std::string result = "= ";

Token Token_stream::get(){
	if (full){
		full = false; return buffer;
	}
	char ch;
	std::cin >> ch;
	switch (ch){
	case '(':
	case ')':
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case ';':
	case '=':
	return Token(ch);
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':{
	std::cin.unget();
	double val;
	std::cin >> val;
	return Token(number, val);
	}
	default:
		if (isalpha(ch) || ch == '_'){
			std::string s;
			s += ch;
			while (std::cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_')) s += ch;
			std::cin.unget();
			if (s == "let") return Token(let);
			if (s == "quit") return Token(quit);
			return Token(name, s);
		}
	error("Bad token");
	}
}

void Token_stream::ignore(char c){
	if (full && c == buffer.kind){
		full = false;
		return;
	}
	full = false;
	char ch;
	while (std::cin >> ch)
		if (ch == c) return;
}

struct Variable{
	std::string name;
	double value;
	bool isConst;
	Variable(std::string n, double v, bool ic) :name(n), value(v), isConst(ic){
	}
};
vector<Variable> names;

double get_value(std::string s){
	for (int i = 0; i < names.size(); ++i)
		if (names[i].name == s) return names[i].value;
	error("get: undefined name ", s);
}

void set_value(std::string s, double d){
	for (int i = 0; i <= names.size(); ++i){
		//if the variable is not const allow to be redefined
		if (names[i].name == s && names[i].isConst == false){
			names[i].value = d;
			return;
		}
	}
	error("set: undefined name ", s);
}

//check if the variable is already declared
bool is_declared(std::string s){
	for (int i = 0; i < names.size(); ++i){
		if (names[i].name == s && names[i].isConst == true)
			error("Cannot reassign const variable");
		else if (names[i].name == s && names[i].isConst == false)
			return true;
	}
	return false;
}

//Add (var,val) to variable vector
double define_name(std::string var, double val, bool isConst){
	if (is_declared(var))
		error(var, " declared twice");
	names.push_back(Variable(var, val, isConst));
	return val;
}

Token_stream ts;
double expression();

double primary(){
	Token t = ts.get();
	switch (t.kind){
	case '(':
	{	double d = expression();
	t = ts.get();
	if (t.kind != ')') error("'(' expected");
	}
	case '-':
		return -primary();
	case number:
		return t.value;
	case name:
		return get_value(t.name);
	default:
		error("primary expected");
	}
}

double term(){
	double left = primary();
	while (true){
		Token t = ts.get();
		switch (t.kind){
		case '*':
			left *= primary();
			break;
		case '/':
		{	double d = primary();
		if (d == 0) error("divide by zero");
		left /= d;
		break;
		}
		default:
			ts.unget(t);
			return left;
		}
	}
}

double expression(){
	double left = term();
	while (true){
		Token t = ts.get();
		switch (t.kind){
		case '+':
			left += term();
			break;
		case '-':
			left -= term();
			break;
		default:
			ts.unget(t);
			return left;
		}
	}
}

double declaration(){
	Token t = ts.get();
	bool isC;
	if (t.kind == 'C'){
		isC = true;
		t = ts.get();
	}
	else
		isC = false;
	if (t.kind != 'a')
		error("name expected in declaration;");
	std::string name = t.name;
	if (is_declared(name)){
		std::cout << name + ", declared twice. Would you like to reassign? (No need to print with ';') Y/N > ";
		std::cin.clear();
		std::cin.ignore(10000, '\n');
		std::string ans;
		getline(std::cin, ans);
		if (ans == "n" || ans == "N")
			error(name, ", will not be reassigned; ");
		if (ans == "y" || ans == "Y"){
			std::cout << "(No need to print with ';') Please enter new value: ";
			int val;
			std::cin >> val;
			set_value(name, val);
			double d = val;
			return d;
		}
	}
	Token t2 = ts.get();
	if (t2.kind != '=')
		error("= missing in declaration of ", name);
	double d = expression();
	names.push_back(Variable(name, d, isC));
	return d;
}

double statement(){
	Token t = ts.get();
	switch (t.kind){
	case let:
		return declaration();
	default:
		ts.unget(t);
		return expression();
	}
}

void clean_up_mess(){
	ts.ignore(print);
}


void calculate(){
	while (true) try{
		std::cout << prompt;
		Token t = ts.get();
		while (t.kind == print) t = ts.get();
		if (t.kind == quit) return;
		ts.unget(t);
		std::cout << result << statement() << std::endl;
	}
	catch (std::runtime_error& e){
		std::cerr << e.what() << std::endl;
		clean_up_mess();
	}
}

int main()
try{
	define_name("pi", 3.1415926535, false);
	define_name("e", 2.7182818284, false);
	calculate();
	return 0;
}
catch (std::exception& e){
	std::cerr << "exception: " << e.what() << std::endl;
	char c;
	while (std::cin >> c && c != ';');
	return 1;
}
catch (...){
	std::cerr << "exception\n";
	char c;
	while (std::cin >> c && c != ';');
	return 2;
}