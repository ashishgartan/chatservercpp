// 📁 utils.cpp
#include "utils.h"
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <string>
#include <stdio.h>
#include <iostream>

std::string hash_password(const std::string &password)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char *)password.c_str(), password.length(), hash);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return ss.str();
}

void print_progress_bar(size_t received, size_t total) {
    const int bar_width = 50;
    float progress = (float)received / total;
    int pos = bar_width * progress;

    std::cout << "\r[";
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << "%";
    std::cout.flush();
}
