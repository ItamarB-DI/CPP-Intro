#pragma once
#include <filesystem>
#include <vector>
#include <queue>
#include <mutex>
#include <sys/fanotify.h>

class FaNotifyHandler {
public:
    struct EventItem
    {
        std::vector<char> m_path; // the path that the event occured on.
        int m_fd; // the fd that the event occured on.
        std::vector<char> m_process; // the process name that made that event 
        int m_pid; // the process that made that event
    };
    

    FaNotifyHandler(std::vector<std::filesystem::path>& files);
    FaNotifyHandler(const FaNotifyHandler& other) = delete;
    FaNotifyHandler &operator=(const FaNotifyHandler& other) = delete;
    void listenForEvents();
    EventItem getTopEvent();
    void addNewReply(struct fanotify_response new_response); 

private:
    void handleEvent(struct fanotify_event_metadata *event_meta_data);
    void replyToFa();

    std::vector<std::filesystem::path>& m_files;
    int m_fanotify;
    std::queue<EventItem> m_events;
    std::queue<struct fanotify_response> m_replies;
    std::mutex m_lock_replies;
    std::mutex m_lock_events;
};

