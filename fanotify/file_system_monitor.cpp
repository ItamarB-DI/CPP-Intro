#include "file_system_monitor.hpp"

#include <iostream>

FileSystemMonitor::FileSystemMonitor(FaNotifyHandler& fa_handler,
                                     const std::vector<std::filesystem::path>& tracked_files, 
                                     const std::vector<std::filesystem::path>& valid_process)
: m_fa_handler(fa_handler),
  m_tracked_files(tracked_files),
  m_valid_processes(valid_process) {
    //empty
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

    std::cout << process_name.c_str() << std::endl; //for testing

    for (auto valid_process: m_valid_processes) {
        if (process_name == valid_process) {
            process_is_valid = true;
            break;
        }
    }

    for (auto tracked_file: m_tracked_files) {
        if (path == tracked_file) {
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


