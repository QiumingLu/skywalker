#ifndef VOYAGER_UTIL_STRING_UTIL_H_
#define VOYAGER_UTIL_STRING_UTIL_H_

#include <string>
#include <vector>

namespace voyager {

// Check if an ASCII character is alphanumberic.
inline bool ascii_isalnum(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9');
}

inline bool ascii_isdigit(char c) {
  return ('0' <= c && c <= '9');
}

inline bool ascii_isspace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\v' ||
         c == '\f' || c == '\r';
}

inline bool ascii_isupper(char c) {
  return c >= 'A' && c <= 'Z';
}

inline bool ascii_islower(char c) {
  return c >= 'a' && c <= 'z';
}

#pragma GCC diagnostic ignored "-Wconversion"
inline char ascii_toupper(char c) {
  return ascii_islower(c) ? c -('a' - 'A') : c;
}

#pragma GCC diagnostic ignored "-Wconversion"
inline char ascii_tolower(char c) {
  return ascii_isupper(c) ? c + ('a' - 'A') : c;
}

inline int hex_digit_to_int(char c) {
  // Assume ASCII
  int x = static_cast<unsigned char>(c);
  if (x  > '9') {
    x += 9;
  }
  return x & 0xf;
}

inline bool HasPrefixString(const std::string& str,
                            const std::string& prefix) {
  return str.size() >= prefix.size() &&
         str.compare(0, prefix.size(), prefix) == 0;
}

inline std::string StripPrefixString(const std::string& str,
                                     const std::string& prefix) {
  if (HasPrefixString(str, prefix)) {
    return str.substr(prefix.size());
  } else {
    return str;
  }
}

inline bool HasSuffixString(const std::string& str,
                            const std::string& suffix) {
  return str.size() >= suffix.size() &&
         str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

inline std::string StripSuffixString(const std::string& str,
                                     const std::string& suffix) {
  if (HasSuffixString(str, suffix)) {
    return str.substr(0, str.size() - suffix.size());
  } else {
    return str;
  }
}

void StripString(std::string* s, const char* remove, char replacewith);

void StripWhitespace(std::string* s);

#pragma GCC diagnostic ignored "-Wconversion"
inline void LowerString(std::string* s) {
  std::string::iterator end = s->end();
  for (std::string::iterator it = s->begin(); it != end; ++it) {
    if ('A' <= *it && *it <= 'Z') {
      *it += 'a' - 'A';
    }
  }
}

#pragma GCC diagnostic ignored "-Wconversion"
inline void UpperString(std::string* s) {
  std::string::iterator end = s->end();
  for (std::string::iterator it = s->begin(); it != end; ++it) {
    if ('a' <= *it && *it <= 'z') {
      *it += 'A' - 'a';
    }
  }
}

inline std::string ToLower(const std::string& s) {
  std::string result = s;
  LowerString(&result);
  return result;
}

inline std::string ToUpper(const std::string& s) {
  std::string result = s;
  UpperString(&result);
  return result;
}

std::string StringReplace(const std::string& s, const std::string& oldsub,
                          const std::string& newsub, bool replace_all);

void SplitStringUsing(const std::string& full,
                      const char* delim,
                      std::vector<std::string>* res);

void SplitStringAllowEmpty(const std::string& full,
                           const char* delim,
                           std::vector<std::string>* result);

// Split a string using a character delimiter.
inline std::vector<std::string> Split(const std::string& full,
                                      const char* delim,
                                      const bool skip_empty = true) {
  std::vector<std::string> result;
  if (skip_empty) {
    SplitStringUsing(full, delim, &result);
  } else {
    SplitStringAllowEmpty(full, delim, & result);
  }
  return result;
}

// These methods concatenate a vector of strings into a C++ string, using
// the C-string "delim" as a separator between components.
void JoinStrings(const std::vector<std::string>& components,
                 const char* delim,
                 std::string* result);

inline std::string JoinStrings(const std::vector<std::string>& components,
                               const char* delim) {
  std::string result;
  JoinStrings(components, delim, &result);
  return result;
}

}  // namespace voyager

#endif  // VOYAGER_UTIL_STRING_UTIL_H_
