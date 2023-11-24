#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include <string>
#include <map>

using std::string;
using std::cout;
using std::endl;
using std::map;

string SERVER_COLOR = "blue";
string CLIENT_COLOR = "yellow";
string ACTION_COLOR = "green";
string ERROR_COLOR = "red";
string HIGH_LIGHT = "magenta";
string DATA_COLOR = "cyan";

map<string, string> colors = {
    {"black", "\033[30m"},
    {"red", "\033[31m"},
    {"green", "\033[32m"},
    {"yellow", "\033[33m"},
    {"blue", "\033[34m"},
    {"purple", "\033[35m"},
    {"cyan", "\033[36m"},
    {"white", "\033[37m"},
    {"dark_black", "\033[90m"},
    {"dark_red", "\033[91m"},
    {"dark_green", "\033[92m"},
    {"dark_yellow", "\033[93m"},
    {"dark_blue", "\033[94m"},
    {"dark_purple", "\033[95m"},
    {"dark_cyan", "\033[96m"},
    {"dark_white", "\033[97m"},
    {"reset", "\033[0m"}
};


void log(string message, string color)
{
    if (colors.find(color) != colors.end()) {
        string colored_message = colors[color] + message + colors["reset"];
        cout << colored_message << endl;
    }

    else {
        cout << message << endl;
    }
}

#endif