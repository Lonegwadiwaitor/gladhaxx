#include "../../src/pch.hpp"

#include "random.hpp"

std::string random::string(int len) {
    std::string temp;
    const char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    for (int i = 0; i < len; ++i)
        temp += chars[number(0, sizeof(chars) - 1)];

    return temp;
}

int random::number(int between, int between2) {
    std::random_device rd;
    std::uniform_int_distribution<int> rnum(between, between2);

    return rnum(rd);
}