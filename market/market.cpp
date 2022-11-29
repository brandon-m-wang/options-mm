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

int main() {

    filesystem::path ifpath =
        filesystem::current_path() / "data-sources/AMZN_OPTIONS_XFM.csv";
    filesystem::path ofpath = filesystem::current_path() / "market/ORDERBOOK";
    const char *myfifo = ofpath.c_str();

    vector<string> row;
    string line;

    mkfifo(myfifo, 0644);
    int fd;

    fstream ifile(ifpath, ios::in);

    if (ifile.is_open()) {
        fd = open(myfifo, O_WRONLY);
        while (getline(ifile, line)) {
            this_thread::sleep_for(chrono::milliseconds(15));

            // Now open in write mode and write
            // string taken from user.
            write(fd, line.c_str(), strlen(line.c_str()) + 1);
        }
        close(fd);
        ifile.close();
    }

    return 0;
}