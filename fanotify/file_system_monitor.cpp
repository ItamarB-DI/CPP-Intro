#include "file_system_monitor.hpp"

FileSystemMonitor::FileSystemMonitor(FaNotifyHandler& fa_handler,
                                     const std::vector<std::filesystem::path>& valid_files, 
                                     const std::vector<std::filesystem::path>& valid_process)
: m_fa_handler(fa_handler),
  m_stop(false) {

    for (auto file: valid_files) {
        std::string str = file.c_str();
        std::vector<char> temp(str.begin(), str.end());

        m_valid_files.push_back(temp);
    }

    for (auto process: valid_process) {
        std::string str = process.c_str();
        std::vector<char> temp(str.begin(), str.end());

        m_valid_processes.push_back(temp);
    }    
}

void FileSystemMonitor::run() {

    while (!m_stop) {
        FaNotifyHandler::EventItem event =  m_fa_handler.getTopEvent();

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
    for (auto valid_process: m_valid_processes) {

        if (valid_process == process_name) {
            return FAN_ACCESS;
        }
    }

    auto path = event.m_path;
    for (auto valid_file: m_valid_files) {

        if (std::string(valid_file.data())  == std::string(path.data()) ) {
            return FAN_ACCESS;
        }
    }


    return FAN_DENY;
}


