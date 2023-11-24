#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <iostream>
#include <string>
#include <vector>
#include <chrono>

using std::string;
using std::vector;

// def eval_list(response: str) -> List[int]:

vector<int> eval_list(string response)
{
    vector<int> result;
    string temp = "";
    for (int i = 0; i < response.length(); i++) {
        if (response[i] == '[') {
            continue;
        }

        else if (response[i] == ',' || response[i] == ']') {
            result.push_back(std::stoi(temp));
            temp = "";
        }

        else {
            temp += response[i];
        }
    }

    return result;
}

vector<unsigned char> format_keys(string aes_key, string aes_iv)
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
long long capture_time()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

long long calculate_time(long long start_time)
{
    return capture_time() - start_time;
}

void print_path() {
    
}

#endif