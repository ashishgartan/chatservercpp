// 📁 utils.h
#pragma once
#include <string>


std::string hash_password(const std::string &password);

void print_progress_bar(size_t received, size_t total);
