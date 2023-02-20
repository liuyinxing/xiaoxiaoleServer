#include <iostream>
#include <random>
#include <ctime>
#include <chrono>

namespace ShellRandom {
    static std::uniform_int_distribution<int> diProbability_;
    static std::default_random_engine dre_;

    template<class T>
    T DiRandom(T minT, T maxT);
    template<> int DiRandom<int>(int minT, int maxT);

    void DiRandomSeed(std::default_random_engine::result_type seed);
    std::string getRandomString(size_t stringsize, int minT, int maxT);
}
std::uniform_int_distribution<int> diProbability_;
std::default_random_engine dre_(std::default_random_engine::default_seed);
namespace ShellRandom {
    template<class T>
    T DiRandom(T minT, T maxT) {
        std::uniform_int_distribution<T> di(minT, maxT);
        return di(dre_);
    }
    template<>
    int DiRandom<int>(int minT, int maxT) {
        diProbability_.param(std::uniform_int_distribution<int>::param_type(minT, maxT));
        return diProbability_(dre_);
    }


    void DiRandomSeed(std::default_random_engine::result_type seed) {
        dre_.seed(seed);
    }
    std::string getRandomString(size_t stringsize, int minT, int maxT)
    {
        std::string str;
        str.reserve(stringsize);
        ShellRandom::DiRandomSeed(time(NULL));
        for (int i = 0; i < 20; ++i) {
            char r = (char)ShellRandom::DiRandom<int>(minT, maxT); // 指定字符生成范围
            str.append(1, r);
        }
        return str;
    }
}