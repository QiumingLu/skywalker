#ifndef SKYWALKER_UTIL_FILE_H_
#define SKYWALKER_UTIL_FILE_H_

#include <stdint.h>
#include <string>
#include <vector>
#include "skywalker/status.h"
#include "skywalker/slice.h"

namespace skywalker {

class SequentialFile;
class RandomAccessFile;
class WritableFile;

class FileManager {
 public:
  FileManager() { }

  // The returned file will only be accessed by one thread at a time.
  Status NewSequentialFile(const std::string& fname,
                           SequentialFile** result);

  // The returned file may be concurrently accessed by multiple threads.
  Status NewRandomAccessFile(const std::string& fname,
                             RandomAccessFile** result);

  // The returned file will only be accessed by one thread at a time.
  Status NewWritableFile(const std::string& fname,
                         WritableFile** result);

  // Delete the named file.
  Status DeleteFile(const std::string& fname);

  // Create the specified directory.
  Status CreateDir(const std::string& dirname);

  // Delete the specified directory.
  Status DeleteDir(const std::string& dirname);

  // Get files of the specified directory.
  Status GetFiles(const std::string& dir, std::vector<std::string>* files);

  // Store the size of fname in *file_size.
  Status GetFileSize(const std::string& fname, uint64_t* size);

 private:
  // No copying allowed
  FileManager(const FileManager&);
  void operator=(const FileManager&);
};

// A file abstraction for reading sequentially through a file
class SequentialFile {
 public:
  SequentialFile() { }
  virtual ~SequentialFile();

  virtual Status Read(size_t n, Slice* result, char* scratch) = 0;

  virtual Status Skip(uint64_t n) = 0;

 private:
  // No copying allowed
  SequentialFile(const SequentialFile&);
  void operator=(const SequentialFile&);
};

// A file abstraction for randomly reading the contents of a file.
class RandomAccessFile {
 public:
  RandomAccessFile() { }
  virtual ~RandomAccessFile();

  virtual Status Read(uint64_t offset, size_t n, Slice* result,
                      char* scratch) const = 0;

 private:
  // No copying allowed
  RandomAccessFile(const RandomAccessFile&);
  void operator=(const RandomAccessFile&);
};

// A file abstraction for sequential writing. The implementation
// must provide buffering since callers may append small fragments
// at a time to the file.
class WritableFile {
 public:
  WritableFile() { }
  virtual ~WritableFile();

  virtual Status Append(const Slice& data) = 0;
  virtual Status Close() = 0;
  virtual Status Flush() = 0;
  virtual Status Sync() = 0;

 private:
  // No copying allowed
  WritableFile(const WritableFile&);
  void operator=(const WritableFile&);
};

extern Status ReadFileToString(FileManager* manager,
                               const std::string& fname,
                               std::string* data);

extern Status WriteStringToFile(FileManager* manager,
                                const Slice& data,
                                const std::string& fname);

extern Status WriteStringToFileSync(FileManager* manager,
                                    const Slice& data,
                                    const std::string& fname);

}  // namespace skywalker

#endif  // SKYWALKER_UTIL_FILE_H_
