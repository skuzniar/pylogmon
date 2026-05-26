#ifndef File_Monitor_dot_h
#define File_Monitor_dot_h

#include "Types.h"

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

namespace O3 {

// File Monitor monitors a single file and receives notificstions when the file changes
class FileMonitor
{
public:
    FileMonitor(const FileMonitor& other) = delete;
    FileMonitor(FileMonitor&& other)      = default;

    FileMonitor(std::string path)
      :m_fd(open(path.c_str(), O_RDONLY))
    {
        if (m_fd < 0) {
            throw std::runtime_error("Failed to open " + path + " for reading. Error: " + strerror(errno));
        }
        if (lseek(m_fd, 0, SEEK_END) < 0) {
            throw std::runtime_error("Failed to move to the end of " + path + ". Error: " + strerror(errno));
        }
        //std::clog << "Added file monitor for: " << m_fd << ' ' << path << std::endl;
    }

    ~FileMonitor()
    {
        //std::clog << "Removed file monitor for fd: " << m_fd << std::endl;
        close(m_fd);
    }

    std::pair<std::string_view, bool> notify(O3::Event e)
    {
        if (auto count = read(m_fd, m_buffer.data(), m_buffer.size()); count > 0) {
            return { { m_buffer.data(), static_cast<std::size_t>(count) }, true };
        }
        return { {}, false };
    }

private:
    // File descriptor
    int m_fd = -1;

    // Input buffer
    std::array<char, 4096> m_buffer;
};

} // namespace O3

#endif
