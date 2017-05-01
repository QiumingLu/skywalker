#include "skywalker/file.h"

#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>

namespace skywalker{

namespace {

static Status IOError(const std::string& context, int err_number) {
  return Status::IOError(context, strerror(err_number));
}

}  // end namespace

class PosixSequentialFile : public SequentialFile {
 private:
  std::string filename_;
  FILE* file_;

 public:
  PosixSequentialFile(const std::string& fname, FILE* f)
      : filename_(fname),
        file_(f) {
  }

  virtual ~PosixSequentialFile() {
    fclose(file_);
  }

  virtual Status Read(size_t n, Slice* result, char* scratch) {
    Status s;
    size_t r =  fread(scratch, 1, n, file_);
    *result = Slice(scratch, r);
    if (r < n) {
      if (feof(file_)) {
        // We leave status as ok if we hit the end of the file
      } else {
        // A partial read with an error: return a non-ok status
        s = IOError(filename_, errno);
      }
    }
    return s;
  }

  virtual Status Skip(uint64_t n) {
    if (fseek(file_, n, SEEK_CUR)) {
      return IOError(filename_, errno);
    }
    return Status::OK();
  }
};

class PosixRandomAccessFile : public RandomAccessFile {
 private:
   std::string filename_;
   int fd_;

 public:
  PosixRandomAccessFile(const std::string& fname, int fd)
      : filename_(fname), fd_(fd) {
  }

  virtual ~PosixRandomAccessFile() {
    close(fd_);
  }

  virtual Status Read(uint64_t offset, size_t n, Slice* result,
                      char* scratch) const {
    Status s;
    ssize_t r = pread(fd_, scratch, n, static_cast<off_t>(offset));
    *result = Slice(scratch, (r < 0) ? 0 : r);
    if (r < 0) {
      s = IOError(filename_, errno);
    }
    return s;
  }
};

class PosixMmapReadableFile : public RandomAccessFile {
 private:
  std::string filename_;
  void* mmaped_region_;
  size_t length_;

 public:
  // base[0, length-1] contains the mmapped contents of the file
  PosixMmapReadableFile(const std::string& fname, void* base, size_t length)
      : filename_(fname),
        mmaped_region_(base),
        length_(length) {
  }

  virtual ~PosixMmapReadableFile() {
    munmap(mmaped_region_, length_);
  }

  virtual Status Read(uint64_t offset, size_t n, Slice* result,
                      char* scratch) const {
    Status s;
    if (offset + n > length_) {
      *result = Slice();
      s = IOError(filename_, EINVAL);
    } else {
      *result = Slice(reinterpret_cast<char*>(mmaped_region_) + offset, n);
    }
    return s;
  }
};

class PosixWritableFile : public WritableFile {
 private:
  std::string filename_;
  FILE* file_;

 public:
  PosixWritableFile(const std::string& fname, FILE* f)
      : filename_(fname), file_(f) { }

  ~PosixWritableFile() {
    if (file_ != NULL) {
      fclose(file_);
    }
  }

  virtual Status Append(const Slice& data) {
    size_t r = fwrite(data.data(), 1, data.size(), file_);
    if (r != data.size()) {
      return IOError(filename_, errno);
    }
    return Status::OK();
  }

  virtual Status Close() {
    Status result;
    if (fclose(file_) != 0) {
      result = IOError(filename_, errno);
    }
    file_ = NULL;
    return result;
  }

  virtual Status Flush() {
    if (fflush(file_) != 0) {
      return IOError(filename_, errno);
    }
    return Status::OK();
  }

  virtual Status Sync() {
    if (fflush(file_) != 0 ||
        fsync(fileno(file_)) != 0) {
      return IOError(filename_, errno);
    }
    return Status::OK();
  }
};

SequentialFile::~SequentialFile() {
}

RandomAccessFile::~RandomAccessFile() {
}

WritableFile::~WritableFile() {
}

Status FileManager::NewSequentialFile(const std::string& fname,
                                      SequentialFile** result) {
  FILE* f = fopen(fname.c_str(), "r");
  if (f == NULL) {
    *result = NULL;
    return IOError(fname, errno);
  } else {
    *result = new PosixSequentialFile(fname, f);
    return Status::OK();
  }
}

Status FileManager::NewRandomAccessFile(const std::string& fname,
                                        RandomAccessFile** result) {
  *result = NULL;
  Status s;
  int fd = open(fname.c_str(), O_RDONLY);
  if (fd < 0) {
    s = IOError(fname, errno);
  } else if (sizeof(void*) >= 8) {
    // Use mmap when virtual address-space is plentiful.
    uint64_t size;
    s = GetFileSize(fname, &size);
    if (s.ok()) {
      void* base = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
      if (base != MAP_FAILED) {
        *result = new PosixMmapReadableFile(fname, base, size);
      } else {
        s = IOError(fname, errno);
      }
    }
    close(fd);
  } else {
    *result = new PosixRandomAccessFile(fname, fd);
  }
  return s;
}

Status FileManager::NewWritableFile(const std::string& fname,
                                    WritableFile** result) {
  Status s;
  FILE* f = fopen(fname.c_str(), "w");
  if (f == NULL) {
    *result = NULL;
    s = IOError(fname, errno);
  } else {
    *result = new PosixWritableFile(fname, f);
  }
  return s;
}

Status FileManager::NewAppendableFile(const std::string& fname,
                                      WritableFile** result) {
  Status s;
  FILE* f = fopen(fname.c_str(), "a");
  if (f == nullptr) {
    *result = nullptr;
    s = IOError(fname, errno);
  } else {
    *result = new PosixWritableFile(fname, f);
  }
  return s;
}

Status FileManager::DeleteFile(const std::string& fname) {
  Status result;
  if (unlink(fname.c_str()) != 0) {
    result = IOError(fname, errno);
  }
  return result;
}

Status FileManager::CreateDir(const std::string& dirname) {
  Status result;
  if (mkdir(dirname.c_str(), 0755) != 0) {
    result = IOError(dirname, errno);
  }
  return result;
}

Status FileManager::DeleteDir(const std::string& dirname) {
  Status result;
  if (rmdir(dirname.c_str()) != 0) {
    result = IOError(dirname, errno);
  }
  return result;
}

Status FileManager::GetChildren(const std::string& dir,
                                std::vector<std::string>* result) {
  result->clear();
  DIR* d = opendir(dir.c_str());
  if (d == nullptr) {
    return IOError(dir, errno);
  }
  struct dirent* entry;
  while ((entry = readdir(d)) != nullptr) {
    result->push_back(entry->d_name);
  }
  closedir(d);
  return Status::OK();
}

bool FileManager::FileExists(const std::string& fname) {
  return access(fname.c_str(), F_OK) == 0;
}

Status FileManager::RenameFile(const std::string& src,
                               const std::string& target) {
  Status result;
  if (rename(src.c_str(), target.c_str()) != 0) {
    result = IOError(src, errno);
  }
  return result;
}

Status FileManager::GetFileSize(const std::string& fname, uint64_t* size) {
  Status s;
  struct stat sbuf;
  if (stat(fname.c_str(), &sbuf) != 0) {
    *size = 0;
    s = IOError(fname, errno);
  } else {
    *size = sbuf.st_size;
  }
  return s;
}

Status ReadFileToString(FileManager* manager,
                        const std::string& fname,
                        std::string* data) {
  data->clear();
  SequentialFile* file;
  Status s = manager->NewSequentialFile(fname, &file);
  if (!s.ok()) {
    return s;
  }
  static const int kBufferSize = 8192;
  char* space = new char[kBufferSize];
  while (true) {
    Slice fragmenet;
    s = file->Read(kBufferSize, &fragmenet, space);
    if (!s.ok()) {
      break;
    }
    data->append(fragmenet.data(), fragmenet.size());
    if (fragmenet.empty()) {
      break;
    }
  }
  delete[] space;
  delete file;
  return s;
}

static Status DoWriteStringToFile(FileManager* manager,
                                  const Slice& data,
                                  const std::string& fname,
                                  bool should_sync) {
  WritableFile* file;
  Status s = manager->NewWritableFile(fname, &file);
  if (!s.ok()) {
    return s;
  }
  s = file->Append(data);
  if (s.ok() && should_sync) {
    s = file->Sync();
  }
  if (s.ok()) {
    s = file->Close();
  }
  delete file;
  if (!s.ok()) {
    manager->DeleteFile(fname);
  }
  return s;
}

Status WriteStringToFile(FileManager* manager, const Slice& data,
                         const std::string& fname) {
  return DoWriteStringToFile(manager, data, fname, false);
}

Status WriteStringToFileSync(FileManager* manager, const Slice& data,
                             const std::string& fname) {
  return DoWriteStringToFile(manager, data, fname, true);
}

pthread_once_t FileManager::once_ = PTHREAD_ONCE_INIT;
FileManager* FileManager::file_manager_ = nullptr;
void FileManager::InitFileManager() { file_manager_ =  new FileManager(); }

FileManager* FileManager::Instance() {
  pthread_once(&once_, &FileManager::InitFileManager);
  return file_manager_;
}

}  // namespace skywalker
