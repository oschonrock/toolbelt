// for mmap:
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <sys/mman.h>
#include <sys/stat.h>

namespace os::fs {

class MemoryMappedFile {
public:
  explicit MemoryMappedFile(const std::string& filename) {
    int fd = open(filename.c_str(), O_RDONLY); // NOLINT
    if (fd == -1) throw std::logic_error("MemoryMappedFile: couldn't open file.");

    // obtain file size
    struct stat sb {};
    if (fstat(fd, &sb) == -1) throw std::logic_error("MemoryMappedFile: cannot stat file size");
    m_filesize = sb.st_size;

    m_map = static_cast<const char*>(mmap(nullptr, m_filesize, PROT_READ, MAP_PRIVATE, fd, 0U));
    if (m_map == MAP_FAILED) // NOLINT : doesn't work somehow?
      throw std::logic_error("MemoryMappedFile: cannot map file");
  }

  ~MemoryMappedFile() {
    if (munmap(static_cast<void*>(const_cast<char*>(m_map)), m_filesize) == -1) // NOLINT
      std::cerr << "Warnng: MemoryMappedFile: error in destructor during `munmap()`\n";
  }

  // no copies
  MemoryMappedFile(const MemoryMappedFile& other) = delete;
  MemoryMappedFile& operator=(MemoryMappedFile other) = delete;

  // default moves
  MemoryMappedFile(MemoryMappedFile&& other) = default;
  MemoryMappedFile& operator=(MemoryMappedFile&& other) = default;

  // char* pointers. up to callee to make string_views or strings
  [[nodiscard]] const char* begin() const { return m_map; }
  [[nodiscard]] const char* end() const { return m_map + m_filesize; } // NOLINT

  [[nodiscard]] std::string_view get_buffer() const {
    return std::string_view{begin(), static_cast<std::size_t>(end() - begin())} ;
  }

private:
  size_t      m_filesize = 0;
  const char* m_map      = nullptr;
};

} // namespace os::fs
