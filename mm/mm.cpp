#include <chrono>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace std;

vector<string> tick_from_stream(char *buf) { return {}; }

int main() {
    int fd;

    // FIFO file path
    filesystem::path ofpath = filesystem::current_path() / "market/ORDERBOOK";
    const char *myfifo = ofpath.c_str();

    // Creating the named file(FIFO)
    // mkfifo(<pathname>, <permission>)
    mkfifo(myfifo, 0644);

    char buf[1028];
    while (1) {

        // Open FIFO for Read only
        fd = open(myfifo, O_RDONLY);

        // Read from FIFO
        read(fd, buf, sizeof(buf));

        cout << strlen(buf) << " ";

        // Print the read message
        cout << buf << endl;
    }
    close(fd);

    return 0;
}