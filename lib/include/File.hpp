#pragma once
#include <string>
#include <unistd.h>
#include <fcntl.h>

/// File class
/// Automatically handles a file descriptor (closing/opening)
class File {
public:
    File();

    /// Constructs File object and opens a file
    /// \param path path to a file
    /// \param mode flags used with open function (eg. O_WRONLY)
    /// \param perm permissions of a file, works only with O_CREAT flag
    /// \throws std::runtime_error on failure
    File(const std::string& path, int mode, int perm = 0666);
    ~File();

    /// Manually closes the file descriptor
    /// \param path path to a file
    /// \param mode flags used with open function (eg. O_WRONLY)
    /// \param perm permissions of a file, used only if O_CREAT flag was specified
    /// \returns false if file is already open (does not close old file descriptor)
    /// \throws std::runtime_error on failure
    bool Open(const std::string& path, int mode, int perm = 0666);

    /// Closes the file descriptor
    /// \returns false if file isn't open
    bool Close();

    /// Writes to a file
    /// \param buf data pointer
    /// \param len length of data to be written
    /// \returns false if file isn't open or if writing fails
    bool Write(const void* buf, int len);

    /// Writes a string to a file
    /// \param str string to be written
    /// \returns false if file isn't open or if writing fails
    bool Write(const std::string& str);

    /// Gets file descriptor
    /// \returns file descriptor or -1 if file isn't open
    int GetFD();

    File(const File&) = delete;
    File& operator=(const File&) = delete;

    File(File&& other) noexcept;
    File& operator=(File&& other) noexcept;
private:
    int m_fd;
};