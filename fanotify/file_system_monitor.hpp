#pragma once
#include "fanotify_handler.hpp"

#include <filesystem>
#include <vector>


class FileSystemMonitor
{
public:
    explicit FileSystemMonitor(FaNotifyHandler& fa_handler,
                               const std::vector<std::filesystem::path>& valid_files, 
                               const std::vector<std::filesystem::path>& valid_process);

    void run(); // blocking
    void stop();

private:
    unsigned int checkEvent(const FaNotifyHandler::EventItem& event);

    FaNotifyHandler& m_fa_handler;
    std::vector<std::vector<char>> m_valid_files;
    std::vector<std::vector<char>> m_valid_processes;
    bool m_stop;
};
