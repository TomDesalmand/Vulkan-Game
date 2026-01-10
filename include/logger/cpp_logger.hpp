// Amalgamated single-header for CPP-Logger
#ifndef CPP_LOGGER_SINGLE_HPP
#define CPP_LOGGER_SINGLE_HPP

// ---- Public headers (include/*.hpp) ----
// ===== Begin: include/utils.hpp =====

#include <cstdint>
#include <string>

struct RGB {
    std::uint8_t r{};
    std::uint8_t g{};
    std::uint8_t b{};
};

#ifdef _WIN32
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <windows.h>
#endif

void enable_virtual_terminal_processing_once();
std::string ansi_fg_rgb(const RGB& color);
const char* ansi_reset() noexcept;
void log_test();

// ===== End: include/utils.hpp =====

// ===== Begin: include/logging.hpp =====

#include <unordered_map>
#include <string>
#include <utility>
#include <sstream>


namespace cpp_logging {

    class Logger {
        public:
            struct Type {
                int code{};
                std::string label;
                RGB color;
                std::string format = "[{label}]: {context}";
            };
        
            Logger();
            void create_type(const std::string& type_name, int code, RGB color, std::string label = {}, std::string format = {});
            void create_type(const std::string& type_name, int code, int r, int g, int b, std::string label = {}, std::string format = {});
            void set_format(const std::string& type_name, const std::string& format);
            template <typename... Args>
            void log_by_type(const std::string& type_name, Args&&... args);
        
        private:
            std::unordered_map<std::string, Type> _types;
            static std::string trim(const std::string& s);
            static std::string render_format(const Type& type, const std::string& message, const std::string& fmt);
            void log_by_type_message(const std::string& type_name, const std::string& message);
    };
    
    Logger& implicit_logger();
    
    template <typename... Args>
    void Logger::log_by_type(const std::string& type_name, Args&&... args) {
        std::ostringstream oss;
        (oss << ... << std::forward<Args>(args));
        log_by_type_message(type_name, oss.str());
    }

}

#if defined(LOG_LEVEL)
  #define LOG_SHOULD_LOG(CODE) ((CODE) <= (LOG_LEVEL))
#else
  #define LOG_SHOULD_LOG(CODE) (true)
#endif

// Declare a logging callable as an inline templated function so it has proper
// linkage across translation units. This avoids the pitfalls of defining
// a lambda (or non-inline object) in a header which leads to multiple- or
// missing-definition issues when used from other TUs.
#define LOG_DECLARE_TYPE(NAME) \
    template<typename... Args> \
    inline void NAME(Args&&... _args) { \
        ::cpp_logging::implicit_logger().log_by_type(#NAME, std::forward<Args>(_args)...); \
    }

#define LOG_DEFINE_TYPE(NAME, CODE, R, G, B) \
    (::cpp_logging::implicit_logger().create_type(#NAME, (CODE), (R), (G), (B)));

#define LOG_DEFINE_TYPE_LABELED(NAME, CODE, R, G, B, LABEL) \
    (::cpp_logging::implicit_logger().create_type(#NAME, (CODE), (R), (G), (B), (LABEL)));

#define LOG_DEFINE_TYPE_RGB(NAME, CODE, RGBVAL) \
    (::cpp_logging::implicit_logger().create_type(#NAME, (CODE), (RGBVAL)));

#define LOG_DEFINE_TYPE_LABELED_RGB(NAME, CODE, RGBVAL, LABEL) \
    (::cpp_logging::implicit_logger().create_type(#NAME, (CODE), (RGBVAL), (LABEL)));

#define LOG_DEFINE_TYPE_FORMAT(NAME, CODE, R, G, B, FORMAT) \
    (::cpp_logging::implicit_logger().create_type(#NAME, (CODE), (R), (G), (B), std::string(), (FORMAT)))

#define LOG_DEFINE_TYPE_LABELED_FORMAT(NAME, CODE, R, G, B, LABEL, FORMAT) \
    (::cpp_logging::implicit_logger().create_type(#NAME, (CODE), (R), (G), (B), (LABEL), (FORMAT)))

#define LOG_DEFINE_TYPE_RGB_FORMAT(NAME, CODE, RGBVAL, FORMAT) \
    (::cpp_logging::implicit_logger().create_type(#NAME, (CODE), (RGBVAL), std::string(), (FORMAT)))

#define LOG_DEFINE_TYPE_LABELED_RGB_FORMAT(NAME, CODE, RGBVAL, LABEL, FORMAT) \
    (::cpp_logging::implicit_logger().create_type(#NAME, (CODE), (RGBVAL), (LABEL), (FORMAT)))

#define LOG_UPDATE_TYPE(NAME, CODE, R, G, B) \
    (::cpp_logging::implicit_logger().create_type(#NAME, (CODE), (R), (G), (B)))

#define LOG_UPDATE_TYPE_LABELED(NAME, CODE, R, G, B, LABEL) \
    (::cpp_logging::implicit_logger().create_type(#NAME, (CODE), (R), (G), (B), (LABEL)))

#define LOG_UPDATE_TYPE_RGB(NAME, CODE, RGBVAL) \
    (::cpp_logging::implicit_logger().create_type(#NAME, (CODE), (RGBVAL)))

#define LOG_UPDATE_TYPE_LABELED_RGB(NAME, CODE, RGBVAL, LABEL) \
    (::cpp_logging::implicit_logger().create_type(#NAME, (CODE), (RGBVAL), (LABEL)))

#define LOG_UPDATE_TYPE_FORMAT(NAME, CODE, R, G, B, FORMAT) \
    (::cpp_logging::implicit_logger().create_type(#NAME, (CODE), (R), (G), (B), std::string(), (FORMAT)))

#define LOG_UPDATE_TYPE_LABELED_FORMAT(NAME, CODE, R, G, B, LABEL, FORMAT) \
    (::cpp_logging::implicit_logger().create_type(#NAME, (CODE), (R), (G), (B), (LABEL), (FORMAT)))

#define LOG_UPDATE_TYPE_RGB_FORMAT(NAME, CODE, RGBVAL, FORMAT) \
    (::cpp_logging::implicit_logger().create_type(#NAME, (CODE), (RGBVAL), std::string(), (FORMAT)))

#define LOG_UPDATE_TYPE_LABELED_RGB_FORMAT(NAME, CODE, RGBVAL, LABEL, FORMAT) \
    (::cpp_logging::implicit_logger().create_type(#NAME, (CODE), (RGBVAL), (LABEL), (FORMAT)))

// ===== End: include/logging.hpp =====

// ---- Sources (src/*.cpp) ----
// ===== Begin: src/logging.cpp =====
// Header files includes //

// C++ standard library includes //
#include <utility>
#include <iostream>
#include <vector>

namespace cpp_logging {
    
    inline static std::string trim(const std::string& s) {
        const char* ws = " \t\n\r";
        std::size_t a = s.find_first_not_of(ws);
        if (a == std::string::npos) return {};
        std::size_t b = s.find_last_not_of(ws);
        return s.substr(a, b - a + 1);
    }

    inline Logger::Logger() {
        enable_virtual_terminal_processing_once();
    }

    inline void Logger::create_type(const std::string& type_name, int code, RGB color, std::string label, std::string format) {
        Type type;
        type.code  = code;
        type.color = color;
        type.label = label.empty() ? type_name : std::move(label);
        if (!format.empty()) {
            type.format = std::move(format);
        }
        _types[type_name] = std::move(type);
    }

    inline void Logger::create_type(const std::string& type_name, int code, int r, int g, int b, std::string label, std::string format) {
        create_type(
            type_name,
            code, 
            RGB{ 
                static_cast<std::uint8_t>(r), 
                static_cast<std::uint8_t>(g),
                static_cast<std::uint8_t>(b)
            },
            std::move(label),
            std::move(format)
        );
    }
    
    inline static bool parse_rgb_payload(const std::string& payload, RGB& out_rgb) {
        std::vector<int> nums;
        std::size_t p = 0;
        const std::string trimmed_payload = trim(payload);
        while (p < trimmed_payload.size()) {
            std::size_t comma = trimmed_payload.find(',', p);
            std::string tok = (comma == std::string::npos) ? trimmed_payload.substr(p) : trimmed_payload.substr(p, comma - p);
            tok = trim(tok);
            try {
                if (!tok.empty())
                    nums.push_back(std::stoi(tok));
            } catch (...) {
                nums.clear();
                break;
            }
            if (comma == std::string::npos) break;
            p = comma + 1;
        }
        if (nums.size() >= 3) {
            out_rgb.r = static_cast<std::uint8_t>(nums[0]);
            out_rgb.g = static_cast<std::uint8_t>(nums[1]);
            out_rgb.b = static_cast<std::uint8_t>(nums[2]);
            return true;
        }
        return false;
    }

    inline static void handle_rgb_token(const std::string& key, std::string& out) {
        std::string payload = (key.size() > 4) ? key.substr(4) : "";
        payload = trim(payload);
        if (payload == "reset") {
            out += ansi_reset();
        } else if (!payload.empty()) {
            RGB rgb{};
            if (parse_rgb_payload(payload, rgb)) {
                out += ansi_fg_rgb(rgb);
            } else {
                out.append("{").append(key).append("}");
            }
        } else {
            out.append("{").append(key).append("}");
        }
    }

    inline static std::string render_inline_tokens(const Logger::Type& /*type*/, const std::string& input, const std::string& label_colored) {
        std::string out;
        out.reserve(input.size());
        std::size_t pos = 0;
        while (pos < input.size()) {
            std::size_t brace_pos = input.find('{', pos);
            if (brace_pos == std::string::npos) {
                out.append(input, pos, input.size() - pos);
                break;
            }
            out.append(input, pos, brace_pos - pos);
            std::size_t end_brace = input.find('}', brace_pos + 1);
            if (end_brace == std::string::npos) {
                out.append(input, brace_pos, input.size() - brace_pos);
                break;
            }
            std::string key = input.substr(brace_pos + 1, end_brace - brace_pos - 1);
            if (key == "label") {
                out += label_colored;
            } else if (key == "rgb" || key.compare(0, 4, "rgb,") == 0) {
                handle_rgb_token(key, out);
            } else {
                out.append(input, brace_pos, end_brace - brace_pos + 1);
            }
            pos = end_brace + 1;
        }
        return out;
    }

    inline std::string Logger::render_format(const Type& type, const std::string& message, const std::string& fmt) {
        std::string out;
        out.reserve(fmt.size() + message.size() + type.label.size() + 16);

        const std::string label_colored = std::string(ansi_fg_rgb(type.color)) + type.label + ansi_reset();

        std::size_t pos = 0;
        while (pos < fmt.size()) {
            std::size_t brace_pos = fmt.find('{', pos);
            if (brace_pos == std::string::npos) {
                out.append(fmt, pos, fmt.size() - pos);
                break;
            }
            out.append(fmt, pos, brace_pos - pos);
            std::size_t end_brace = fmt.find('}', brace_pos + 1);
            if (end_brace == std::string::npos) {
                out.append(fmt, brace_pos, fmt.size() - brace_pos);
                break;
            }
            std::string key = fmt.substr(brace_pos + 1, end_brace - brace_pos - 1);

            if (key == "label") {
                out += label_colored;
            } else if (key == "context") {
                out += render_inline_tokens(type, message, label_colored);
            } else if (key == "rgb" || key.compare(0, 4, "rgb,") == 0) {
                handle_rgb_token(key, out);
            } else {
                out.append(fmt, brace_pos, end_brace - brace_pos + 1);
            }
            pos = end_brace + 1;
        }
        return out;
    }

    inline void Logger::set_format(const std::string& type_name, const std::string& format) {
        auto it = _types.find(type_name);
        if (it == _types.end())
            return;
        if (!format.empty())
            it->second.format = format;
    }

    inline void Logger::log_by_type_message(const std::string& type_name, const std::string& message) {
        auto it = _types.find(type_name);
        if (it == _types.end())
            return;
        const Type& type = it->second;
    #if defined(LOG_LEVEL)
        if (type.code > LOG_LEVEL)
            return;
    #endif
        const std::string& fmt = type.format;
        std::string formatted = render_format(type, message, fmt);
        std::ostream& os = std::cerr;
        os << formatted << '\n';
        os.flush();
    }

    inline Logger& implicit_logger() {
        static Logger logger{};
        return logger;
    }

}// ===== End: src/logging.cpp =====

// ===== Begin: src/utils.cpp =====
// Header files includes //

// C++ standard library includes //
#include <sstream>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

inline void enable_virtual_terminal_processing_once() {
#ifdef _WIN32
    static bool initialized = false;
    if (!initialized) {
        const DWORD enable = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        HANDLE handles[2] = { GetStdHandle(STD_OUTPUT_HANDLE), GetStdHandle(STD_ERROR_HANDLE) };
        for (HANDLE h : handles) {
            if (!h || h == INVALID_HANDLE_VALUE) continue;
            DWORD mode = 0;
            if (GetConsoleMode(h, &mode)) {
                mode |= enable;
                SetConsoleMode(h, mode);
            }
        }
        initialized = true;
    }
#else
    // No-op on non-Windows platforms
#endif
}

inline std::string ansi_fg_rgb(const RGB& c) {
    std::ostringstream oss;
    oss << "\x1b[38;2;"
        << static_cast<int>(c.r) << ';'
        << static_cast<int>(c.g) << ';'
        << static_cast<int>(c.b) << 'm';
    return oss.str();
}

inline const char* ansi_reset() noexcept {
    return "\x1b[0m";
}
// ===== End: src/utils.cpp =====

#endif // CPP_LOGGER_SINGLE_HPP
