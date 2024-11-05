#pragma once

#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <syncstream>
#include <chrono>
#include <thread>
#include <mutex>
#include <string>
#include <iomanip>

struct Logger {
  enum class Level {
    DEBUG,
    TRACE,
    INFO,
    WARN,
    ERROR,
    FATAL
  };

  static Logger& get_instance() noexcept {
    static Logger instance;
    return instance;
  }

  template<typename... Args>
  void log(const char* file, int line, Level level, Args&&... args) {
    auto hdr = format_header(file, line, level);

    std::ostringstream os{};

    os << hdr;

    (write(os, std::forward<Args>(args)), ...);

    std::osyncstream out{std::cerr};
    out << os.str() << "\n";
  }

  template<typename... Args>
  void debug(const char* file, int line, Args&&... args) {
    log(file, line, Level::DEBUG, std::forward<Args>(args)...);
  }
  template<typename... Args>
  void trace(const char* file, int line, Args&&... args) {
    log(file, line, Level::TRACE, std::forward<Args>(args)...);
  }

  template<typename... Args>
  void info(const char* file, int line, Args&&... args) {
    log(file, line, Level::INFO, std::forward<Args>(args)...);
  }

  template<typename... Args>
  void warn(const char* file, int line, Args&&... args) {
    log(file, line, Level::WARN, std::forward<Args>(args)...);
  }

  template<typename... Args>
  void error(const char* file, int line, Args&&... args) {
    log(file, line, Level::ERROR, std::forward<Args>(args)...);
  }

  template<typename... Args>
  void fatal(const char* file, int line, Args&&... args) {
    log(file, line, Level::FATAL, std::forward<Args>(args)...);
    abort();
  }

  Level get_level() const noexcept {
    return m_current_level;
  }

  void set_level(Level level) noexcept {
    m_current_level = level;
  }

private:
  Logger() = default;
  ~Logger() = default;
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  std::string format_header(const char* file, int line, Level level) noexcept {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    auto len = strlen(file);
    assert(len > 0);

    auto ptr = file + (len - 1);

    while (ptr != file && *ptr != '/') {
      --ptr;
    }

    assert(ptr != file && *ptr == '/');

    std::string location{ptr + 1};
    location.push_back(':');
    location.append(std::format("{}", line));

    std::ostringstream os{};

    os << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
       << '.' << std::setfill('0') << std::setw(3) << ms.count()
       << " [" << level_to_string(level) << "] "
       << location << " "
       << std::this_thread::get_id() << " ";

    return os.str();
  }

  template<typename... Args>
  void write(std::ostream& out, Args&&... args) {
    ((out << std::forward<Args>(args)), ...);
  }

  const char* level_to_string(Level level) const {
    switch (level) {
      case Level::DEBUG: return "DBG";
      case Level::TRACE: return "TRC";
      case Level::INFO:  return "INF";
      case Level::WARN:  return "WRN";
      case Level::ERROR: return "ERR";
      case Level::FATAL: return "FTL";
      default:           return "UNK";
    }
  }

  Level m_current_level{Level::INFO};
};

#define set_log_level_debug() Logger::get_instance().set_level(Logger::Level::DEBUG)
#define set_log_level_info() Logger::get_instance().set_level(Logger::Level::INFO)
#define set_log_level_warn() Logger::get_instance().set_level(Logger::Level::WARN)
#define set_log_level_error() Logger::get_instance().set_level(Logger::Level::ERROR)
#define set_log_level_fatal() Logger::get_instance().set_level(Logger::Level::FATAL)

#define get_log_level() Logger::get_instance().get_level()

#define is_log_level(level) (get_log_level() <= level)

#define LOG(level, ...) Logger::get_instance().log(__FILE__, __LINE__, Logger::Level::level, __VA_ARGS__)

#define log_debug(...) \
  do { \
    if (is_log_level(Logger::Level::DEBUG)) { \
      Logger::get_instance().debug(__FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (false)

#define log_trace(...) \
  do { \
    if (is_log_level(Logger::Level::TRACE)) { \
      Logger::get_instance().trace(__FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (false)

#define log_info(...) \
  do { \
    if (is_log_level(Logger::Level::INFO)) { \
      Logger::get_instance().info(__FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (false)

#define log_warn(...) \
  do { \
    if (is_log_level(Logger::Level::WARN)) { \
    Logger::get_instance().warn(__FILE__, __LINE__, __VA_ARGS__)

#define log_error(...) \
  do { \
    if (is_log_level(Logger::Level::ERROR)) { \
      Logger::get_instance().error(__FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (false)

#define log_fatal(...) \
  do { \
    if (is_log_level(Logger::Level::FATAL)) { \
      Logger::get_instance().fatal(__FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (false)
