#pragma once

#pragma warning(disable : 4244)
#pragma warning(disable : 4242)  // Disable warning C4242: 'argument': conversion from 'int' to '_Ty', possible loss of data
#pragma warning(disable : 4267)  // Disable warning C4267: '=': conversion from 'size_t' to 'int', possible loss of data

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include "Constants.h"
#include "NodeData.hpp"


using std::string;
using std::vector;

class Utility {
public:
	static vector<int64_t> eval_list(string response)
	{
		vector<int64_t> result;
		string temp = "";
		for (int i = 0; i < response.length(); i++) {
			if (response[i] == '[') {
				continue;
			}

			else if (response[i] == ',' || response[i] == ']') {
				result.push_back(std::stoll(temp));
				temp = "";
			}

			else {
				temp += response[i];
			}
		}

		return result;
	}

	static vector<unsigned char> format_keys(string aes_key, string aes_iv)
	{
		vector<unsigned char> result;
		string temp = "";
		for (int i = 0; i < aes_key.length(); i++) {
			if (aes_key[i] == '\\') {
				continue;
			}

			else if (aes_key[i] == ',') {
				result.push_back(std::stoi(temp));
				temp = "";
			}

			else {
				temp += aes_key[i];
			}
		}

		result.push_back(std::stoi(temp));
		temp = "";

		for (int i = 0; i < aes_iv.length(); i++) {
			if (aes_iv[i] == '\\') {
				continue;
			}

			else if (aes_iv[i] == ',') {
				result.push_back(std::stoi(temp));
				temp = "";
			}

			else {
				temp += aes_iv[i];
			}
		}

		result.push_back(std::stoi(temp));
		return result;
	}


	// capture time
	static unsigned long long capture_time()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	static long long calculate_time(long long start_time)
	{
		return capture_time() - start_time;
	}

	static void print_path() {

	}

	static vector<string> splitString(const string& input, const string& delimiter = Constants::SPLITER) {
		vector<string> tokens;
		size_t start = 0, end = 0;

		while ((end = input.find(delimiter, start)) != string::npos) {
			tokens.push_back(input.substr(start, end - start));
			start = end + delimiter.length();
		}

		tokens.push_back(input.substr(start));

		return tokens;
	}

	static vector<NodeData> generateRamdomPath(vector<NodeData>* nodes, int length = Constants::DEFAULT_PATH_LENGTH) {
		length = min(nodes->size(), length);

		vector<NodeData> path;

		// generate random path without duplicate
		while (path.size() < length) {
			int randomNum = rand() % nodes->size();

			if (find(path.begin(), path.end(), nodes->at(randomNum)) == path.end()) {
				path.push_back(nodes->at(randomNum));
			}
		}

		return path;
	}
};

