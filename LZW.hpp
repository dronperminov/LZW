#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>

using namespace std;

class LZW {
	string input; // путь к входному файлу
	string output; // путь к выхдному файлу
	string phrase; // префикс фраза
	bool info;

	bool IsPow2(size_t n) const;
	int WriteBytes(ofstream &fout, string& code, bool end);

	string GetCode(int i, int bits);
	size_t GetIndex(const string& code, size_t start, int bits);

public:
	LZW(const string& input, const string& output, const string& phrase, bool info);

	void Compress(); // сдатие данных
	void Decompress(); // распаковка данных
};

// конструктор
LZW::LZW(const string& input, const string& output, const string& phrase, bool info) {
	this->input = input;
	this->output = output;
	this->phrase = phrase;
	this->info = info;
}

bool LZW::IsPow2(size_t n) const {
	return (n & (n - 1)) == 0;
}

int LZW::WriteBytes(ofstream &fout, string& code, bool end) {
	int mod = code.length() % 8;

	// пока длина кода больше 8 или дошли до последнего
	while (code.length() >= 8 || (end && code.length())) {
		char byte = 0;

		// формируем байт
		for (size_t i = 0; i < 8; i++)
			byte = (byte << 1) | (i < code.length() ? code[i] - '0' : 0);

		fout.write(&byte, 1); // записываем байт		
		code.erase(0, 8); // сокращаем строку
	}

	return mod;
}

string LZW::GetCode(int i, int bits) {
	string code = "";

	for (int j = bits - 1; j >= 0; j--)
		code += '0' + ((i >> j) & 1);

	return code;
}

size_t LZW::GetIndex(const string& code, size_t start, int bits) {
	size_t index = 0;

	for (int i = 0; i < bits; i++)
		index = index * 2 + ((start + i < code.length() ? code[start + i] : '0') - '0');

	return index;
}

// сжатие файла
void LZW::Compress() {
	ifstream fin(input, ios::binary);

	if (!fin)
		throw string("Invalid input file");

	char byte;
	unordered_map<string, int> table;
	string dictionary = "";

	while (fin.read(&byte, 1)) {
		string c(1, byte);

		if (table.find(c) == table.end()) {
			table[c] = table.size();
			dictionary += byte;			
		}
	}

	fin.close();

	ofstream fout(output, ios::binary);

	fout << phrase << " "; // записываем префикс
	fout << table.size() << " "; // записываем размер словаря
	fout.write(dictionary.c_str(), dictionary.length()); // записываем символы
	
	int bits = 0;

	if (table.size() == 1) {
		bits = 1;
	}
	else {
		while ((1 << bits) < (int) table.size())
			bits++;
	}

	long pos = fout.tellp(); // адрес для количества бит в последнем байте
	byte = 0;
	fout.write(&byte, 1);

	fin.open(input, ios::binary);

	string code = "";

	fin.read(&byte, 1);
	string P = string(1, byte);

	while (fin.read(&byte, 1)) {
		string C = string(1, byte);

		if (table.find(P + C) != table.end()) {
			P += C;
		}
		else {
			code += GetCode(table[P], bits);

			if (IsPow2(table.size()))
				bits++;

			table[P + C] = table.size();
			
			P = C;
		}

		WriteBytes(fout, code, false);
	}

	code += GetCode(table[P], bits);

	byte = WriteBytes(fout, code, true) % 8;
	fout.seekp(pos);
	fout.write(&byte, 1);

	fin.close();
	fout.close();

	if (info) {
		cout << "Dictionary: " << endl;
		for (int i = 0; i < (int) table.size(); i++) {
			auto j = table.begin();

			while (j != table.end() && j->second != i)
				j++;

			cout << j->second << ". " << j->first << endl;
		}
	}
}

// распаковка данных
void LZW::Decompress() {
	ifstream fin(input.c_str(), ios::binary);

	string phraseFile; // префикс фраза
	fin >> phraseFile; // считываем префикс фразу

	// если не совпадают фразы, то бросаем исключение
	if (phraseFile != phrase)
		throw string("phrases does not match");

	int n; // размер словаря
	fin >> n; // считываем размер словаря

	int bits = 0;

	if (n == 1) {
		bits = 1;
	}
	else {
		while ((1 << bits) < n)
			bits++;
	}

	char byte;
	fin.read(&byte, 1); // игнорируем пробел после длины

	unordered_map<int, string> table;

	for (int i = 0; i < n; i++) {
		fin.read(&byte, 1);
		table[i] = string(1, byte);
	}

	fin.read(&byte, 1);
	int mod = byte;

	string code = "";

	while (fin.read(&byte, 1))
		for (int i = 0; i < 8; i++)
			code += '0' + ((byte >> (7 - i)) & 1);

	if (mod != 0)
		code.erase(code.length() - 8 + mod, 8 - mod);

	ofstream fout(output, ios::binary);

	int OLD, NEW;
	string S;

	size_t start = 0;
	OLD = GetIndex(code, start, bits);
	start += bits;

	fout << table[OLD];
	string C = table[OLD];

	while (start < code.length()) {
		if (IsPow2(n))
			bits++;
		
		NEW = GetIndex(code, start, bits);
		start += bits;

		if (table.find(NEW) == table.end()) {
			S = table[OLD] + C;
		}
		else {
			S = table[NEW];
		}

		fout << S;
		C = string(1, S[0]);

		table[n++] = table[OLD] + C;
		OLD = NEW;
	}

	if (info) {
		cout << "Dictionary: " << endl;
		for (size_t i = 0; i < table.size(); i++) {
			cout << i << "." << table[i] << endl;
		}
	}

	fin.close();
	fout.close();
}