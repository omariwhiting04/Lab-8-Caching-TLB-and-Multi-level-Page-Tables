#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
using namespace std;

int main() {
    ofstream fout("addresses.txt");
    if (!fout.is_open()) {
        cerr << "Error creating addresses.txt\n";
        return 1;
    }

    srand(time(0));
    const int NUM_ADDR = 1000;
    const int VIRT_SPACE = 1024 * 4096;  // 4MB

    for (int i = 0; i < NUM_ADDR; i++) {
        int addr = rand() % VIRT_SPACE;
        fout << addr << "\n";
    }

    fout.close();
    cout << "Generated " << NUM_ADDR << " addresses in addresses.txt\n";
    return 0;
}
