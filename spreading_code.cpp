//
// Created by robert on 8/30/17.
//

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <set>
#include <list>

using namespace std;

#define CODE_BITS (20)

int main(int argc, char **argv) {
    list<long> codes;

    for(long i = 0; i < (1l << CODE_BITS); i++) {
        int dist = CODE_BITS;
        for(auto it2 = codes.begin(); it2 != codes.end(); it2++) {
            int d = __builtin_popcount(i ^ (*it2));
            if(d < dist) dist = d;
        }
        if(dist > 5) {
            codes.push_back(i);

            cout << setw(6) << (codes.size()-1) << "  ";
            for(int j = 0; j < CODE_BITS; j++) {
                cout << ((i >> j) & 1);
            }
            cout << endl;

        }
    }

    cout << endl << codes.size() << endl;
    int codeCount = (int)codes.size();

    long *gcodes = new long[codeCount];

    int k = 0;
    for(auto it = codes.begin(); it != codes.end(); it++) {
        int g = k ^ (k >> 1);
        gcodes[g] = *it;
        k++;
    }

    for(int i = 0; i < codeCount; i++) {
        for(int j = 0; j < CODE_BITS; j++) {
            cout << ((gcodes[i] >> j) & 1);
        }
        cout << endl;
    }

    return 0;
}
