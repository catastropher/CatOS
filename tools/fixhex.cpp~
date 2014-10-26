#include <iostream>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

struct Line {
	int size;
	int address;
	unsigned char type;
	vector<unsigned char> data;
	unsigned char checksum;
};

int fromhex(string &s, int &pos, int count) {
	int val = 0;
	
	while(pos < s.length() && count-- > 0) {
		int num;
		
		if(s[pos] >= '0' && s[pos] <= '9')
			num = s[pos] - '0';
		else
			num = toupper(s[pos]) - 'A' + 10;
		
		pos++;
		val = val*16 + num;
	}
	
	return val;
}

string tohex(int val, char width) {
	char tab[] = "0123456789ABCDEF";
	string res;
	
	for(int i = 0;i < width;i++) {
		res = tab[val & 0x0F] + res;
		val >>= 4;
	}
	
	return res;
}

int main(int argc, char *argv[]) {
	vector<Line> lines;
	ifstream in;
	ofstream out;
	
	if(argc < 2) {
		cout << "Usage: fixhex [hexfile]" << endl;
		return -1;
	}
	
	in.open(argv[1]);
	
	if(!in.is_open()) {
		cout << "Error: could not open input" << argv[1] << endl;
	}
	
	string line;
	
	while(getline(in, line)) {
		Line l;
		int pos = 1;
		
		l.size = fromhex(line, pos, 2);
		
		l.address = fromhex(line, pos, 4);
		l.type = fromhex(line, pos, 2);
		
		for(int i = 0;i < l.size;i++) {
			l.data.push_back(fromhex(line, pos, 2));
		}
		
		l.checksum = fromhex(line, pos, 2);
		
		lines.push_back(l);
	}
	
	in.close();
	
	for(int i = 0;i < lines.size() - 1;i++) {
#if 0
		if(lines[i].type == 0 && lines[i + 1].type == 0) {
			while(lines[i + 1].address - lines[i].address > lines[i].data.size())
				lines[i].data.push_back(0);
		}
#endif
	}
	
	out.open(argv[1]);
	
	if(!out.is_open()) {
		cout << "Error: could not open output" << argv[1] << endl;
		return -1;
	}
	
	for(int i = 0;i < lines.size();i++) {
		out << ":" << tohex(lines[i].data.size(), 2) << tohex(lines[i].address, 4) <<
			tohex(lines[i].type, 2);
			
		for(int d = 0;d < lines[i].data.size();d++) {
			out << tohex(lines[i].data[d], 2);
		}
		
		unsigned char sum = lines[i].data.size() + (lines[i].address & 0xFF) +
			(lines[i].address >> 8) + lines[i].type;

		for(int d = 0;d < lines[i].data.size();d++) {
			sum += lines[i].data[d];
		}
		
		sum = ~sum + 1;
		
		
		out << tohex(sum, 2) << "\r\n";
	}
	
	out.close();
	
	
	
	
	return 0;
	
}
