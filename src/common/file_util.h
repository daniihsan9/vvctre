// Copyright 2013 Dolphin Emulator Project / 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <cstdio>
#include <fstream>
#include <functional>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include "common/common_types.h"
#ifdef _MSC_VER
#include "common/string_util.h"
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

namespace FileUtil {

// User paths for GetUserPath
enum class UserPath {
    CheatsDir,
    DumpDir,
    LoadDir,
    LogDir,
    NANDDir,
    RootDir,
    SDMCDir,
    ShaderDir,
    SysDataDir,
    UserDir,
};

// FileSystem tree node/
struct FSTEntry {
    bool isDirectory;
    u64 size;                 // file length or number of entries from children
    std::string physicalName; // name on disk
    std::string virtualName;  // name in FST names table
    std::vector<FSTEntry> children;
};

// Returns true if file filename exists
bool Exists(const std::string& filename);

// Returns true if filename is a directory
bool IsDirectory(const std::string& filename);

// Returns the size of filename (64bit)
u64 GetSize(const std::string& filename);

// Overloaded GetSize, accepts file descriptor
u64 GetSize(const int fd);

// Overloaded GetSize, accepts FILE*
u64 GetSize(FILE* f);

// Returns true if successful, or path already exists.
bool CreateDir(const std::string& filename);

// Creates the full path of full_path returns true on success
bool CreateFullPath(const std::string& full_path);

// Deletes a given filename, return true on success
// Doesn't supports deleting a directory
bool Delete(const std::string& filename);

// Deletes a directory filename, returns true on success
bool DeleteDir(const std::string& filename);

// renames file src_filename to dest_filename, returns true on success
bool Rename(const std::string& src_filename, const std::string& dest_filename);

// copies file src_filename to dest_filename, returns true on success
bool Copy(const std::string& src_filename, const std::string& dest_filename);

// creates an empty file filename, returns true on success
bool CreateEmptyFile(const std::string& filename);

/**
 * @param num_entries_out to be assigned by the callable with the number of iterated directory
 * entries, never null
 * @param directory the path to the enclosing directory
 * @param virtual_name the entry name, without any preceding directory info
 * @return whether handling the entry succeeded
 */
using DirectoryEntryCallable = std::function<bool(
    u64* num_entries_out, const std::string& directory, const std::string& virtual_name)>;

/**
 * Scans a directory, calling the callback for each file/directory contained within.
 * If the callback returns failure, scanning halts and this function returns failure as well
 * @param num_entries_out assigned by the function with the number of iterated directory entries,
 * can be null
 * @param directory the directory to scan
 * @param callback The callback which will be called for each entry
 * @return whether scanning the directory succeeded
 */
bool ForeachDirectoryEntry(u64* num_entries_out, const std::string& directory,
                           DirectoryEntryCallable callback);

/**
 * Scans the directory tree, storing the results.
 * @param directory the parent directory to start scanning from
 * @param parent_entry FSTEntry where the filesystem tree results will be stored.
 * @param recursion Number of children directories to read before giving up.
 * @return the total number of files/directories found
 */
u64 ScanDirectoryTree(const std::string& directory, FSTEntry& parent_entry,
                      unsigned int recursion = 0);

/**
 * Recursively searches through a FSTEntry for files, and stores them.
 * @param directory The FSTEntry to start scanning from
 * @param parent_entry FSTEntry vector where the results will be stored.
 */
void GetAllFilesFromNestedEntries(FSTEntry& directory, std::vector<FSTEntry>& output);

// deletes the given directory and anything under it. Returns true on success.
bool DeleteDirRecursively(const std::string& directory, unsigned int recursion = 256);

// Create directory and copy contents (does not overwrite existing files)
void CopyDir(const std::string& source_path, const std::string& dest_path);

// Returns a pointer to a string with a vvctre data dir
const std::string& GetUserPath(UserPath path);

std::size_t WriteStringToFile(bool text_file, const std::string& filename, std::string_view str);

std::size_t ReadFileToString(bool text_file, const std::string& filename, std::string& str);

/**
 * Splits the filename into 8.3 format
 * Loosely implemented following https://en.wikipedia.org/wiki/8.3_filename
 * @param filename The normal filename to use
 * @param short_name A 9-char array in which the short name will be written
 * @param extension A 4-char array in which the extension will be written
 */
void SplitFilename83(const std::string& filename, std::array<char, 9>& short_name,
                     std::array<char, 4>& extension);

// Gets the extension of the filename
std::string_view GetExtensionFromFilename(std::string_view name);

// Removes the final '/' or '\' if one exists
std::string_view RemoveTrailingSlash(std::string_view path);

enum class DirectorySeparator { ForwardSlash, BackwardSlash, PlatformDefault };

// Removes trailing slash, makes all '\\' into '/', and removes duplicate '/'. Makes '/' into '\\'
// depending if directory_separator is BackwardSlash or PlatformDefault and running on Windows
std::string SanitizePath(std::string_view path,
                         DirectorySeparator directory_separator = DirectorySeparator::ForwardSlash);

// Simple wrapper for cstdlib file functions to
// hopefully make error checking easier
// and make forgetting an fclose() harder
class IOFile : public NonCopyable {
public:
    IOFile();

    IOFile(const std::string& filename, const char openmode[]);

    ~IOFile();

    IOFile(IOFile&& other) noexcept;
    IOFile& operator=(IOFile&& other) noexcept;

    void Swap(IOFile& other) noexcept;

    bool Open(const std::string& filename, const char openmode[]);
    bool Close();

    template <typename T>
    std::size_t ReadArray(T* data, std::size_t length) {
        static_assert(std::is_trivially_copyable_v<T>,
                      "Given array does not consist of trivially copyable objects");

        std::size_t items_read = ReadImpl(data, length, sizeof(T));
        if (items_read != length)
            m_good = false;

        return items_read;
    }

    template <typename T>
    std::size_t WriteArray(const T* data, std::size_t length) {
        static_assert(std::is_trivially_copyable_v<T>,
                      "Given array does not consist of trivially copyable objects");

        std::size_t items_written = WriteImpl(data, length, sizeof(T));
        if (items_written != length) {
            m_good = false;
        }

        return items_written;
    }

    template <typename T>
    std::size_t ReadBytes(T* data, std::size_t length) {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        return ReadArray(reinterpret_cast<char*>(data), length);
    }

    template <typename T>
    std::size_t WriteBytes(const T* data, std::size_t length) {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        return WriteArray(reinterpret_cast<const char*>(data), length);
    }

    template <typename T>
    std::size_t WriteObject(const T& object) {
        static_assert(!std::is_pointer_v<T>, "WriteObject arguments must not be a pointer");
        return WriteArray(&object, 1);
    }

    std::size_t WriteString(std::string_view str) {
        return WriteArray(str.data(), str.length());
    }

    bool IsOpen() const {
        return nullptr != m_file;
    }

    // m_good is set to false when a read, write or other function fails
    bool IsGood() const {
        return m_good;
    }
    explicit operator bool() const {
        return IsGood();
    }

    bool Seek(s64 off, int origin);
    u64 Tell() const;
    u64 GetSize() const;
    bool Resize(u64 size);
    bool Flush();

    // Clear error state
    void Clear() {
        m_good = true;
        std::clearerr(m_file);
    }

private:
    std::size_t ReadImpl(void* data, std::size_t length, std::size_t data_size);
    std::size_t WriteImpl(const void* data, std::size_t length, std::size_t data_size);

    std::FILE* m_file = nullptr;
    bool m_good = true;
};

} // namespace FileUtil

// To deal with Windows being dumb at unicode:
template <typename T>
void OpenFStream(T& fstream, const std::string& filename, std::ios_base::openmode openmode) {
#ifdef _MSC_VER
    fstream.open(Common::UTF8ToUTF16W(filename), openmode);
#else
    fstream.open(filename, openmode);
#endif
}
