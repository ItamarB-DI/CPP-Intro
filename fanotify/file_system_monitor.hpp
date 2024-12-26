#pragma once
#include "fanotify_handler.hpp"

#include <filesystem>
#include <vector>


class FileSystemMonitor
{
public:
    explicit FileSystemMonitor(FaNotifyHandler& fa_handler,
                               const std::vector<std::filesystem::path>& tracked_files, 
                               const std::vector<std::filesystem::path>& valid_process);

    void run(); // blocking

private:
    unsigned int checkEvent(const FaNotifyHandler::EventItem& event);

    FaNotifyHandler& m_fa_handler;
    std::vector<std::filesystem::path> m_tracked_files;
    std::vector<std::filesystem::path> m_valid_processes;
};
