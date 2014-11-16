#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <cstdlib>

using namespace std;

struct SysCall {
	string name;
	string decl;
};


map<string, SysCall> calls;

vector<string> load_file(string name) {
	vector<string> lines;
	
	ifstream file(name.c_str());
	if(!file.is_open()) {
		cout << "Error: failed to open file '" << name << "'" << endl;
		exit(-1);
	}
	
	while(!file.eof()) {
		string line;
		getline(file, line);
		lines.push_back(line);
	}
	
	return std::move(lines);
}

void get_system_calls(vector<string> &lines, char type) {
	for(string &i : lines) {
		size_t pos = i.find("SYSCALL");
		size_t save_pos = pos;
		
		if(pos != string::npos) {
			pos += 7;
			
			while(pos < i.length() && i[pos] == ' ')
				pos++;
			
			string name;
			
			while(pos < i.length() && i[pos] != '(')
				pos++;
			
			size_t func_pos = pos;
			
			pos--;
			
			while(i[pos] != ' ' && i[pos] != '*') {
				name = i[pos--] + name;
			}
			
			if(type == 'c') {
			
				string type;
				
				while(pos > save_pos + 7)
					type = i[pos--] + type;
				
				//cout << "Name: '" << name << "'" << endl;
				//cout << "Type: '" << type << "'" << endl;
				
				string func;
				
				while(func_pos < i.length() && (i[func_pos] != '{')) {
					if(func_pos + 1 < i.length() && i[func_pos] == ' ' && i[func_pos + 1] == '{')
						break;
						
					if(func_pos + 1 < i.length() && i[func_pos] == '_' && i[func_pos + 1] == '_') {
						while(func_pos < i.length() && isalpha(i[func_pos]) || i[func_pos] == '_')
							func_pos++;
						
						continue;
					}
					
					func += i[func_pos++];
				}
				
				//cout << "Func: '" << func << "'" << endl;
				SysCall s;
				
				s.name = "_" + name + "_start";
				s.decl = "const " + type + "(*" + name + ")" + func + " = (void *)0x";
				calls[s.name] = s;
			}
			else {
				i = calls["_" + name + "_start"].decl;
			}
		}	
	}
}

void resolve_system_calls() {
	vector<string> lines = load_file("../src/output.map");
	map<string, string> symbols;
	
	for(string &i : lines) {
		int pos = 5;
		string addr;
		string name;
		
		if(i.length() > 6 && i[0] == ' ' && i[1] == ' ' && i[2] == ' ' && i[3] == ' ' && isdigit(i[5])) {
			while(i[pos] == '0')
				pos++;
		
			while(pos < i.length() && i[pos] != ' ') {
				addr += i[pos++];
			}
			
			if(addr == "")
				addr = "0";
			
			while(i[pos] == ' ')
				pos++;
			
			while(i[pos] != ' ')
				name += i[pos++];
			
			//cout << "'" << name << "' = '" << addr << "'" << endl;
			symbols[name] = addr;
		}
	}
	
	for(auto &i : calls) {
		if(symbols.count(i.first) == 0) {
			cout << "Error: failed to resolve symbol: '" << i.first << "'" << endl;
			exit(-1);
		}
		
		i.second.decl += symbols[i.first] + ";";
		cout << i.second.decl << endl;
	}
}
	

void process_file(string name) {
	vector<string> lines = load_file(name);
	
	get_system_calls(lines, name[name.length() - 1]);
}

int main(int argc, char *argv[]) {
	process_file(argv[1]);
	resolve_system_calls();
}










