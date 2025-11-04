#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cstdlib>
#include <ctime>
using namespace std;


#define PAGE_SIZE        4096
#define NUM_DIR_ENTRIES  1024
#define NUM_TABLE_ENTRIES 1024
#define NUM_FRAMES       256
#define TLB_SIZE         16
#define NUM_ACCESSES     1000


struct PageTableEntry {
    int frame;
    bool valid;
};

struct PageDirectoryEntry {
    vector<PageTableEntry>* table;
    bool valid;
};

struct TLBEntry {
    int vpn;
    int frame;
    int age;
};


vector<PageDirectoryEntry> pageDirectory(NUM_DIR_ENTRIES);
vector<TLBEntry> tlb(TLB_SIZE);
int pageFaults = 0, tlbHits = 0, accesses = 0;
int nextFrame = 0;


int searchTLB(int vpn) {
    for (int i = 0; i < TLB_SIZE; i++)
        if (tlb[i].vpn == vpn && tlb[i].frame != -1)
            return i;
    return -1;
}

void updateLRU() {
    for (auto &e : tlb)
        if (e.frame != -1) e.age++;
}

void insertTLB(int vpn, int frame) {
    int oldest = 0;
    for (int i = 1; i < TLB_SIZE; i++)
        if (tlb[i].age > tlb[oldest].age)
            oldest = i;
    tlb[oldest].vpn = vpn;
    tlb[oldest].frame = frame;
    tlb[oldest].age = 0;
}

int getPhysicalAddress(int virtAddr) {
    int offset = virtAddr & 0xFFF;
    int ptIndex = (virtAddr >> 12) & 0x3FF;
    int pdIndex = (virtAddr >> 22) & 0x3FF;
    int vpn = virtAddr >> 12; // 20-bit VPN

    updateLRU();
    int tlbIndex = searchTLB(vpn);
    int frame;

    if (tlbIndex != -1) {           // TLB hit
        tlbHits++;
        tlb[tlbIndex].age = 0;
        frame = tlb[tlbIndex].frame;
    } else {                        // TLB miss → page table lookup
        if (!pageDirectory[pdIndex].valid) {
            pageDirectory[pdIndex].table = new vector<PageTableEntry>(NUM_TABLE_ENTRIES);
            for (auto &e : *pageDirectory[pdIndex].table) {
                e.valid = false;
                e.frame = -1;
            }
            pageDirectory[pdIndex].valid = true;
        }

        vector<PageTableEntry> &pageTable = *pageDirectory[pdIndex].table;

        if (!pageTable[ptIndex].valid) {
            pageFaults++;
            pageTable[ptIndex].frame = nextFrame % NUM_FRAMES;
            pageTable[ptIndex].valid = true;
            nextFrame++;
        }

        frame = pageTable[ptIndex].frame;
        insertTLB(vpn, frame);
    }

    return (frame * PAGE_SIZE) + offset;
}


int main() {
    srand(time(0));

    // Initialize directory and TLB
    for (auto &dir : pageDirectory) {
        dir.valid = false;
        dir.table = nullptr;
    }
    for (auto &entry : tlb) {
        entry.vpn = -1;
        entry.frame = -1;
        entry.age = 0;
    }

    
    vector<int> addresses;
    long long virt_space = (long long)NUM_DIR_ENTRIES * NUM_TABLE_ENTRIES * PAGE_SIZE; // 4GB space
    for (int i = 0; i < NUM_ACCESSES; i++) {
        if (i % 5 == 0)  // 20% random (simulate non-local access)
            addresses.push_back(rand() % (virt_space / 2)); // keep within safe range
        else              // 80% localized (simulate locality)
            addresses.push_back((rand() % (128 * PAGE_SIZE)) + (rand() % 4) * PAGE_SIZE);
    }

    // ===== Simulate Accesses =====
    for (int i = 0; i < NUM_ACCESSES; i++) {
        accesses++;
        int phys = getPhysicalAddress(addresses[i]);
        if (i < 15) // show only first 15
            cout << "Virtual: " << setw(10) << addresses[i]
                 << " -> Physical: " << setw(10) << phys << endl;
    }

    
    cout << "\n=== Page Faults & Random Access Simulation ===\n";
    cout << "Accesses:     " << accesses << endl;
    cout << "TLB Hits:     " << tlbHits << endl;
    cout << "Page Faults:  " << pageFaults << endl;
    cout << fixed << setprecision(2);
    cout << "TLB Hit Rate: " << (100.0 * tlbHits / accesses) << "%\n";
    cout << "Fault Rate:   " << (100.0 * pageFaults / accesses) << "%\n";
    return 0;
}
