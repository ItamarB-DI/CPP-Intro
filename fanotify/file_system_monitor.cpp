#include "file_system_monitor.hpp"


FileSystemMonitor::FileSystemMonitor(FaNotifyHandler& fa_handler, std::vector<std::filesystem::path>& valid_files, std::vector<std::filesystem::path>& valid_exe)
: m_fa_handler(fa_handler),
  m_valid_files(valid_files),
  m_valid_exe(valid_exe),
  m_stop(false) {
    //empty
}

void FileSystemMonitor::run() {

    while (!m_stop) {

    }
}

void FileSystemMonitor::stop() {
    m_stop = true;
}