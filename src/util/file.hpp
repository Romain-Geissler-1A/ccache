// Copyright (C) 2021-2023 Joel Rosdahl and other contributors
//
// See doc/AUTHORS.adoc for a complete list of contributors.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 51
// Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#pragma once

#include <util/TimePoint.hpp>
#include <util/types.hpp>

#include <third_party/nonstd/expected.hpp>
#include <third_party/nonstd/span.hpp>

#include <cstddef>
#include <cstdint>
#include <ctime>
#include <optional>
#include <string>
#include <string_view>

namespace util {

// --- Interface ---

enum class InPlace { yes, no };
enum class LogFailure { yes, no };
enum class ViaTmpFile { yes, no };

// Copy a file from `src` to `dest`. If `via_tmp_file` is yes, `src` is copied
// to a temporary file and then renamed to dest.
nonstd::expected<void, std::string>
copy_file(const std::string& src,
          const std::string& dest,
          ViaTmpFile via_tmp_file = ViaTmpFile::no);

void create_cachedir_tag(const std::string& dir);

// Extends file size of `fd` to at least `new_size` by calling posix_fallocate()
// if supported, otherwise by writing zeros last to the file.
//
// Note that existing holes are not filled in case posix_fallocate() is not
// supported.
nonstd::expected<void, std::string> fallocate(int fd, size_t new_size);

// Return how much a file of `size` bytes likely would take on disk.
uint64_t likely_size_on_disk(uint64_t size);

// Read data from `fd` until end of file and call `data_receiver` with the read
// data. Returns an error if the underlying read(2) call returned -1.
nonstd::expected<void, std::string> read_fd(int fd, DataReceiver data_receiver);

// Return contents of file at  `path`.
//
// `T` should be `util::Bytes` or `std::vector<uint8_t>` for binary data and
// `std::string` for text data. If `T` is `std::string` and the content starts
// with a UTF-16 little-endian BOM on Windows then it will be converted to
// UTF-8.
//
// If `size_hint` is not 0 then it is assumed that `path` has this size (this
// saves system calls).
template<typename T>
nonstd::expected<T, std::string> read_file(const std::string& path,
                                           size_t size_hint = 0);

// Return (at most) `count` bytes from `path` starting at position `pos`.
//
// `T` should be `util::Bytes` or `std::vector<uint8_t>` for binary data and
// `std::string` for text data. If `T` is `std::string` and the content starts
// with a UTF-16 little-endian BOM on Windows then it will be converted to
// UTF-8.
template<typename T>
nonstd::expected<T, std::string>
read_file_part(const std::string& path, size_t pos, size_t count);

// Remove `path` (non-directory), NFS hazardous. Use only for files that will
// not exist on other systems.
//
// Returns whether the file was removed. A nonexistent `path` is considered
// successful.
nonstd::expected<bool, std::error_code>
remove(const std::string& path, LogFailure log_failure = LogFailure::yes);

// Remove `path` (non-directory), NFS safe.
//
// Returns whether the file was removed. A nonexistent `path` is considered a
// successful.
nonstd::expected<bool, std::error_code>
remove_nfs_safe(const std::string& path,
                LogFailure log_failure = LogFailure::yes);

// Set the FD_CLOEXEC on file descriptor `fd`. This is a NOP on Windows.
void set_cloexec_flag(int fd);

// Set atime/mtime of `path`. If `mtime` is std::nullopt, set to the current
// time. If `atime` is std::nullopt, set to what `mtime` specifies.
void set_timestamps(const std::string& path,
                    std::optional<util::TimePoint> mtime = std::nullopt,
                    std::optional<util::TimePoint> atime = std::nullopt);

// Write `size` bytes from binary `data` to `fd`.
nonstd::expected<void, std::string>
write_fd(int fd, const void* data, size_t size);

// Write text `data` to `path`. If `in_place` is no, unlink any existing file
// first (i.e., break hard links).
nonstd::expected<void, std::string> write_file(const std::string& path,
                                               std::string_view data,
                                               InPlace in_place = InPlace::no);

// Write binary `data` to `path`. If `in_place` is no, unlink any existing
// file first (i.e., break hard links).
nonstd::expected<void, std::string> write_file(const std::string& path,
                                               nonstd::span<const uint8_t> data,
                                               InPlace in_place = InPlace::no);

// --- Inline implementations ---

inline uint64_t
likely_size_on_disk(uint64_t size)
{
  return (size + 4095) & ~4095;
}

} // namespace util
