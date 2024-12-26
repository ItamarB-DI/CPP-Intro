#pragma once
#include <filesystem>
#include <vector>
#include <queue>
#include <mutex>
#include <sys/fanotify.h>
#include <condition_variable>


class FaNotifyHandler {
public:
    struct EventItem
    {
        std::vector<char> m_path; // the path that the event occured on.
        int m_fd; // the fd that the event occured on.
        std::vector<char> m_process; // the process name that made that event 
        int m_pid; // the process that made that event
    };
    
    explicit FaNotifyHandler(const std::vector<std::filesystem::path>& files);
    FaNotifyHandler(const FaNotifyHandler& other) = delete; 
    FaNotifyHandler &operator=(const FaNotifyHandler& other) = delete; 
    ~FaNotifyHandler();

    void listenForEvents(); // this class run function (blocking)
    void stopListening();
    EventItem getTopEvent(); // returns the first event (blocking)
    void addNewReply(struct fanotify_response new_response); // abling to append a repliy for the FA

private:
    void handleEvent(struct fanotify_event_metadata *event_meta_data);
    void replyToFa();

    std::vector<std::filesystem::path> m_files;
    int m_fanotify;
    std::queue<EventItem> m_events;
    std::queue<struct fanotify_response> m_replies;
    std::mutex m_lock_replies;
    std::mutex m_lock_events;
    std::condition_variable m_events_cv;
    bool m_stop;
};


