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

    filesystem::path optionsPath =
        filesystem::current_path() / getenv("OPTIONS_PATH");
    filesystem::path orderbookPath =
        filesystem::current_path() / getenv("ORDERBOOK_PATH");
    const char *myfifo = orderbookPath.c_str();

    fstream optionsFstream(optionsPath, ios::in);

    vector<string> row;
    string line;

    mkfifo(myfifo, 0644);

    int fd = open(myfifo, O_WRONLY);

    while (getline(optionsFstream, line)) {
        this_thread::sleep_for(chrono::milliseconds(2));

        write(fd, line.c_str(), strlen(line.c_str()) + 1);
    }

    close(fd);
    optionsFstream.close();

    return 0;
}