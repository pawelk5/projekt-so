#include "File.hpp"
#include <stdexcept>

File::File() 
    :m_fd(-1)
{

}

File::File(const std::string& path, int mode, int perm) {
    Open(path, mode, perm);
}

File::~File() {
    Close();
}

bool File::Open(const std::string& path, int mode, int perm) {
    if (m_fd != -1)
        return false;

    m_fd = open(path.c_str(), mode, perm);
    if (m_fd == -1) {
        perror("open error");
        throw std::runtime_error("open error");
    }

    return true;
}

bool File::Close() {
    if (m_fd == -1)
        return false;

    close(m_fd);
    m_fd = -1;
    return true;
}

bool File::Write(const void* buf, int len) {
    if (m_fd == -1)
        return false;

    if (write(m_fd, buf, len) == -1) {
        perror("write error");
        return false;
    }
    return true;
}

bool File::Write(const std::string& str) {
    return Write((const void*)str.c_str(), str.size());
}

int File::GetFD() {
    return m_fd;
}