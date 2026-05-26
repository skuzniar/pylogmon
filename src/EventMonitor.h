#ifndef EventMonitor_dot_h
#define EventMonitor_dot_h

#include "Types.h"

#include <type_traits>
#include <cstddef>
#include <iostream>
#include <vector>
#include <map>
#include <filesystem>
#include <thread>

#include <sys/event.h>
#include <fcntl.h>
#include <unistd.h>

namespace O3 {

// Event Monitor manages the collection of File Monitors and performs kqueue registration and notification.
// A single File Monitor notification may produce zero or more results.
template<typename FileMonitor, std::size_t millis = 100>
class EventMonitor
{
public:
    // Return type of the notification callback is std::pair<T, bool>
    using RT = std::invoke_result_t<decltype(&FileMonitor::notify), FileMonitor*, Event>;
    using T  = RT::first_type;

    // Notification result processing callback
    std::function<void(const T& s)> m_callback;

    using event = struct kevent;

    EventMonitor(const EventMonitor& other) = delete;

    EventMonitor(const std::vector<std::string>& paths)
      : m_kqfd(kqueue())
    {
        if (m_kqfd < 0) {
            throw std::runtime_error(std::string("Failed to initialize kqueue. Error: ") + strerror(errno));
        }

        m_fmonitors.reserve(paths.size());

        for (int idx = 0; const auto& path : paths) {
            if (!std::filesystem::is_regular_file(path)) {
                throw std::runtime_error("Path: " + path + " is not present or is not a file.");
            }
            if (int fd = open(path.c_str(), O_EVTONLY); fd >= 0) {
                m_fds.emplace(fd, idx++);

                event e;
                // EV_SET(&e, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, NOTE_WRITE | NOTE_DELETE | NOTE_EXTEND | NOTE_ATTRIB, 0, NULL);
                EV_SET(&e, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, NOTE_WRITE | NOTE_EXTEND, 0, NULL);
                m_ievents.emplace_back(e);

                m_fmonitors.emplace_back(path);
            } else {
                dtor();
                throw std::runtime_error("Unable to open file: " + path + ". Error: " + strerror(errno));
            }
        }
        m_oevents.resize(m_ievents.size());

        auto result = std::div(millis, 1000);
        m_pooltime  = { result.quot, 1000L * result.rem };

        m_thread = std::thread([this] { run(); });
    }

    ~EventMonitor()
    {
        stop();
        m_thread.join();
        dtor();
    }

private:
    void dispatch(const RT& result)
    {
        if (m_callback && result.second) {
            m_callback(result.first);
        }
    }

    void run()
    {
        while (!m_stop) {
            int count = kevent(m_kqfd, m_ievents.data(), m_ievents.size(), m_oevents.data(), m_oevents.size(), &m_pooltime);
            if (count < 0) {
                throw std::runtime_error(std::string("Failure during kevent call. Error: ") + strerror(errno));
            }
            for (int i = 0; i < count; ++i) {
                const auto& event = m_oevents[i];
                std::size_t index = m_fds[event.ident];
                if ((event.fflags & NOTE_DELETE) != 0) {
                    dispatch(m_fmonitors[index].notify(Event::Delete));
                }
                if ((event.fflags & NOTE_WRITE) != 0) {
                    dispatch(m_fmonitors[index].notify(Event::Write));
                }
                if ((event.fflags & NOTE_ATTRIB) != 0) {
                    dispatch(m_fmonitors[index].notify(Event::Attribute));
                }
                if ((event.fflags & NOTE_EXTEND) != 0) {
                    dispatch(m_fmonitors[index].notify(Event::Extend));
                }
            }
        }
    }

    void stop()
    {
        m_stop = true;
    }

    // To be called when throwing from ctor
    void dtor()
    {
        close(m_kqfd);
        for (const auto& [fd, _] : m_fds) {
            close(fd);
        }
    }
    // kqueue registration file descriptor
    int m_kqfd = -1;

    // Pooling time
    struct timespec m_pooltime = { 0, 1000L * 100 };

    // Map of file descriptors to file indices
    std::map<int, int> m_fds;

    // List of events we are interested in and the events that occurred
    std::vector<event> m_ievents;
    std::vector<event> m_oevents;

    // Collection of File Monitors
    std::vector<FileMonitor> m_fmonitors;

    // Pooling control
    bool m_stop = false;

    // Monitor thread
    std::thread m_thread;
};

} // namespace O3

#endif
