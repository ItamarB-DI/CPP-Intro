#include "file_system_monitor.hpp"

#include <iostream>

FileSystemMonitor::FileSystemMonitor(FaNotifyHandler& fa_handler,
                                     const std::vector<std::filesystem::path>& tracked_files, 
                                     const std::vector<std::filesystem::path>& valid_process)
: m_fa_handler(fa_handler),
  m_tracked_files(tracked_files),
  m_valid_processes(valid_process),
  m_stop(false) {
    //empty
}

void FileSystemMonitor::run() {

    while (m_stop == false) {
        FaNotifyHandler::EventItem event = m_fa_handler.getTopEvent();

        if (event == FaNotifyHandler::EMPTY_EVENT) { continue; }

        auto status = checkEvent(event);

        struct fanotify_response res = {event.m_fd, status};

        m_fa_handler.addNewReply(res);
    }
}

void FileSystemMonitor::stop() {
    m_stop = true;
}

unsigned int FileSystemMonitor::checkEvent(const FaNotifyHandler::EventItem& event) {

    auto process_name = event.m_process;
    auto path = event.m_path;
    
    bool process_is_valid = false;
    bool path_is_tracked = false;

    std::cout << process_name << std::endl;

    for (auto valid_process: m_valid_processes) {
        if (process_name == valid_process) {
            process_is_valid = true;
            break;
        }
    }

    for (auto tracked_file: m_tracked_files) {
        if (path == tracked_file) {
            path_is_tracked = true;
            break;
        }
    }

    if (path_is_tracked == true && process_is_valid == false) {
        return FAN_DENY;
    }

    if (path_is_tracked == false || process_is_valid == true) {
        return FAN_ACCESS;
    }

    return FAN_DENY;
}


