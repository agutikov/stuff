#include <iostream>
#include <fstream>
#include <limits>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
using namespace std;



vector<vector<int>> s_0 = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9}
};



std::string str_key(const vector<vector<int>>& s)
{
    std::string k;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            k += std::to_string(s[i][j]);
        }
    }
    return k;
}

uint64_t key(const vector<vector<int>>& s)
{
    uint64_t k = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            k <<= 4;
            k += s[i][j];
        }
    }
    return k;
}

void print(const vector<vector<int>>& s)
{
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf("%d ", s[i][j]);
        }
        printf("\n");
    }
    //printf("%9llX\n", key(s));
    //printf("\n");
}

vector<vector<int>> rotate_right(vector<vector<int>> s)
{
    vector<vector<int>> r = s;

    r[0][0] = s[2][0];
    r[0][2] = s[0][0];
    r[2][2] = s[0][2];
    r[2][0] = s[2][2];

    r[0][1] = s[1][0];
    r[1][2] = s[0][1];
    r[2][1] = s[1][2];
    r[1][0] = s[2][1];

    return r;
}

vector<vector<int>> rotate_left(vector<vector<int>> s)
{
    vector<vector<int>> r = s;

    r[0][0] = s[0][2];
    r[0][2] = s[2][2];
    r[2][2] = s[2][0];
    r[2][0] = s[0][0];

    r[0][1] = s[1][2];
    r[1][2] = s[2][1];
    r[2][1] = s[1][0];
    r[1][0] = s[0][1];

    return r;
}

vector<vector<int>> rotate(vector<vector<int>> s, int n)
{
    if (n > 0) {
        while (n--) {
            s = rotate_right(s);
        }
    } else {
        while (n++) {
            s = rotate_left(s);
        }
    }
    return s;
}

vector<vector<int>> mirror_h(vector<vector<int>> s)
{
    vector<vector<int>> r = s;

    r[0][0] = s[0][2];
    r[1][0] = s[1][2];
    r[2][0] = s[2][2];

    r[0][2] = s[0][0];
    r[1][2] = s[1][0];
    r[2][2] = s[2][0];

    return r;
}

vector<vector<int>> mirror_v(vector<vector<int>> s)
{
    vector<vector<int>> r = s;

    r[0][0] = s[2][0];
    r[0][1] = s[2][1];
    r[0][2] = s[2][2];

    r[2][0] = s[0][0];
    r[2][1] = s[0][1];
    r[2][2] = s[0][2];

    return r;
}

vector<vector<int>> transposition_main(vector<vector<int>> s)
{
    vector<vector<int>> r = s;

    r[0][1] = s[1][0];
    r[0][2] = s[2][0];
    r[1][2] = s[2][1];

    r[1][0] = s[0][1];
    r[2][0] = s[0][2];
    r[2][1] = s[1][2];

    return r;
}

vector<vector<int>> transposition_second(vector<vector<int>> s)
{
    vector<vector<int>> r = s;

    r[0][1] = s[1][2];
    r[0][0] = s[2][2];
    r[1][0] = s[2][1];

    r[1][2] = s[0][1];
    r[2][2] = s[0][0];
    r[2][1] = s[1][0];

    return r;
}



bool inc(std::vector<unsigned int>& v, unsigned int mod)
{
    bool r = false;
    unsigned int c = 0;
    v[0]++;
    for (auto& a : v) {
        a += c;
        c = a / mod;
        a = a % mod;
        r = r || a != 0;
    }
    return r;
}

bool seq_rep(const std::vector<unsigned int>& v)
{
    for (int i = 0; i < v.size() - 1; i++) {
        if (v[i] == v[i+1]) {
            return true;
        }
    }
    return false;
}

std::string get(const std::string& src, const std::vector<unsigned int>& indexes)
{
    string s;
    for (int j = indexes.size()-1; j >= 0; j--) {
        s += src[indexes[j]];
    }
    return s;
}

void append_mod(std::map<uint64_t, vector<string>>& m, const string& v, const vector<vector<int>>& s)
{
    auto k = key(s);
    auto it = m.find(k);
    if (it == m.end()) {
        m[k] = {v};
    } else {
        it->second.push_back(v);
    }
}


void append_state(std::map<uint64_t, vector<vector<int>>>& states, const vector<vector<int>>& s)
{
    states[key(s)] = s;
}


// TODO: reduction
// TODO: transitions between every state
// TODO: groups of sufficient transisions


// 1,2,3 - rotate 1*pi/2, 2*pi/2, 3*pi/2
// V - Vertical mirror
// H - Horizontal mirror
// T - Transposition
// S - transposition around Second diagonal
std::string modifications = "123VHTS";

vector<vector<int>> apply(vector<vector<int>> s, char c)
{
    if (c == 'V') {
        s = mirror_v(s);
    } else if (c == 'H') {
        s = mirror_h(s);
    } else if (c == 'T') {
        s = transposition_main(s);
    } else if (c == 'S') {
        s = transposition_second(s);
    } else {
        int r = c - '0';
        s = rotate(s, r);
    }
    return s;
}

vector<vector<int>> apply(vector<vector<int>> s, const std::string& seq)
{
    for (char c : seq) {
        s = apply(s, c);
    }
    return s;
}


int main()
{
    std::map<uint64_t, vector<string>> m;
    std::map<uint64_t, vector<vector<int>>> states;

    uint64_t k = key(s_0);
    m[k] = {"0"};
    states[k] = s_0;
    printf("0\n");
    print(s_0);
    printf("\n");

    for (int len = 1; len <= 2; len++) {
        std::vector<unsigned int> indexes(len);

        do {
            if (seq_rep(indexes)) {
                //continue;
            }

            auto seq = get(modifications, indexes);

            printf("%s\n", seq.c_str());
            auto s = apply(s_0, seq);
            print(s);
            printf("\n");
            append_mod(m, seq, s);
            append_state(states, s);

        } while(inc(indexes, modifications.size()));
    }

    printf("\n\n");
    for (auto& it : m) {
        for (const auto& seq: it.second) {
            printf("\"%s\" ", seq.c_str());
        }
        printf("\n");
        print(states.find(it.first)->second);
        printf("\n");
    }

    std::map<uint64_t, size_t> key2index;
    std::map<size_t, uint64_t> index2key;
    size_t index = 0;
    for (auto it : states) {
        auto k = it.first;
        key2index[k] = index;
        index2key[index] = k;
        index++;
    }

    vector<vector<char>> transisions(states.size(), vector<char>(states.size()));
    for (auto it : states) {
        auto k0 = it.first;
        auto s0 = it.second;
        for (char c : "0"s + modifications) {
            auto s1 = apply(s0, c);
            auto k1 = key(s1);

            transisions[key2index.find(key(s0))->second][key2index.find(key(s1))->second] = c;
        }
    }
    printf("\n");
    for (const auto& row : transisions) {
        for (char c : row) {
            printf(" %c ", c);
        }
        printf("\n");
    }
    printf("\n");


    return 0;
}
