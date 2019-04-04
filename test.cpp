#include <iostream>
#include <string>
#include <cstdlib>
#include <cassert>

#include "LZW.hpp"

using namespace std;

string GetString(int n) {
	string s = "";

	for (int i = 0; i < n; i++) {
		int k = rand() % 3;

		if (k == 0) {
			s += '0' + rand() % 10;
		}
		else if (k == 1) {
			s += 'a' + rand() % 26;
		}
		else {
			s += 'A' + rand() % 26;
		}
	}

	return s;
}

void WritetoFile(const string& path, const string& s) {
	ofstream f(path.c_str());
	f << s;
	f.close();
}

string ReadFromFile(const string& path) {
	string s;
	ifstream f(path.c_str());
	getline(f, s);
	f.close();

	return s;
}

int main() {
	for (int n = 1; n <= 10000; n++) {
		for (int i = 0; i < 1000; i++) {
			string s = GetString(n);
			WritetoFile("in.txt", s);

			LZW lzw1("in.txt", "compressed", "HF11", false);
			lzw1.Compress();

			LZW lzw2("compressed", "out.txt", "HF11", false);
			lzw2.Decompress();

			string s2 = ReadFromFile("out.txt");

			assert(s == s2);
		}

		cout << n << " - OK\n";
	}
}