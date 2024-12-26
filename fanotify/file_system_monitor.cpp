#include "file_system_monitor.hpp"

#include <iostream>

FileSystemMonitor::FileSystemMonitor(FaNotifyHandler& fa_handler,
                                     const std::vector<std::filesystem::path>& valid_files, 
                                     const std::vector<std::filesystem::path>& valid_process)
: m_fa_handler(fa_handler) {

    for (auto file: valid_files) {
        std::string str = file.c_str();
        std::vector<char> temp(str.begin(), str.end());

        m_tracked_files.push_back(temp);
    }

    for (auto process: valid_process) {
        std::string str = process.c_str();
        std::vector<char> temp(str.begin(), str.end());

        m_valid_processes.push_back(temp);
    }    
}

void FileSystemMonitor::run() {

    while (true) {
        FaNotifyHandler::EventItem event =  m_fa_handler.getTopEvent();

        auto status = checkEvent(event);

        struct fanotify_response res = {event.m_fd, status};

        m_fa_handler.addNewReply(res);
    }
}

unsigned int FileSystemMonitor::checkEvent(const FaNotifyHandler::EventItem& event) {

    auto process_name = event.m_process;
    auto path = event.m_path;
    
    bool process_is_valid = false;
    bool path_is_valid = false;
    std::cout << process_name.data() << std::endl;

    for (auto valid_process: m_valid_processes) {
        if (std::string(valid_process.data()) == std::string(process_name.data())) {
            process_is_valid = true;
            break;
        }
    }

    for (auto valid_file: m_tracked_files) {
        if (std::string(valid_file.data())  == std::string(path.data()) ) {
            path_is_valid = true;
            break;
        }
    }

    if (path_is_valid == true && process_is_valid == false) {
        return FAN_DENY;
    }

    if (path_is_valid == false || process_is_valid == true) {
        return FAN_ACCESS;
    }

    return FAN_DENY;
}


