#include "fanotify_handler.hpp"

#include <iostream>



int main() {

    std::filesystem::path pwd = std::filesystem::current_path();
    std::filesystem::path file1 = pwd / "test_file.txt";
    
    std::vector<std::filesystem::path> files;
    files.push_back(file1);

    FaNotifyHandler fa(files);

    fa.listenForEvents();

    return 0;
}

