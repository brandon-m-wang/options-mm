#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std;

int main() {

    filesystem::path ifpath =
        filesystem::current_path() / "data-sources/AMZN_OPTIONS_XFM.csv";
    filesystem::path ofpath = filesystem::current_path() / "market/ORDERBOOK";

    vector<string> row;
    string line;

    fstream ifile(ifpath, ios::in);
    fstream ofile(ofpath, ios::out);

    if (ifile.is_open() && ofile.is_open()) {
        while (getline(ifile, line)) {
            this_thread::sleep_for(chrono::milliseconds(3000));
            ofile.write(line.c_str(), line.size()) << endl;
        }
    }

    return 0;
}