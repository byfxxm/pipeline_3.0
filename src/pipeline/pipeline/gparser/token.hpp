#ifndef _BYFXXM_TOKEN_HPP_
#define _BYFXXM_TOKEN_HPP_

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>

namespace byfxxm {
namespace token {
enum class Kind {
  CON,     // ����
  IDN,     // ��ʶ��
  FOR,     // for
  IF,      // IF
  ELSE,    // ELSE
  LB,      // [
  RB,      // ]
  POS,     // +
  NEG,     // -
  PLUS,    // +
  MINUS,   // -
  MUL,     // *
  DIV,     // /
  ASSIGN,  // =
  EQ,      // ==
  NE,      // !=
  LT,      // <
  LE,      // <=
  GT,      // >
  GE,      // >=
  KEOF,    // EOF
  SHARP,   // #
  WHILE,   // WHILE
  ELSEIF,  // ELSEIF
  ENDIF,   // ENDIF
  THEN,    // THEN
  DO,      // DO
  END,     // END
  NEWLINE, // \n
  SEMI,    // ;
  COMMA,   // ,
  MAX,     // MAX
  MIN,     // MIN
  STRING,  // "..."
  NOT,     // NOT
  GOTO,    // GOTO
  G,
  M,
  X,
  Y,
  Z,
  A,
  B,
  C,
  I,
  J,
  K,
  N,
  F,
  S,
  O,
};

// �ֱ���
struct Token {
  Kind kind;
  std::optional<std::variant<double, std::string>> value;
};

using Dictionary = std::pmr::unordered_map<std::string, Kind>;

inline const Dictionary keywords = {
    {"IF", Kind::IF},       {"ELSEIF", Kind::ELSEIF}, {"ELSE", Kind::ELSE},
    {"ENDIF", Kind::ENDIF}, {"THEN", Kind::THEN},     {"WHILE", Kind::WHILE},
    {"DO", Kind::DO},       {"END", Kind::END},       {"GT", Kind::GT},
    {"GE", Kind::GE},       {"LT", Kind::LT},         {"LE", Kind::LE},
    {"EQ", Kind::EQ},       {"NE", Kind::NE},         {"MAX", Kind::MAX},
    {"MIN", Kind::MIN},     {"NOT", Kind::NOT},       {"GOTO", Kind::GOTO}};

inline const Dictionary symbols = {
    {"[", Kind::LB},     {"]", Kind::RB},   {"+", Kind::PLUS},
    {"-", Kind::MINUS},  {"*", Kind::MUL},  {"/", Kind::DIV},
    {"=", Kind::ASSIGN}, {";", Kind::SEMI}, {",", Kind::COMMA},
};

inline const Dictionary gcodes = {
    {"G", Kind::G}, {"M", Kind::M}, {"X", Kind::X}, {"Y", Kind::Y},
    {"Z", Kind::Z}, {"A", Kind::A}, {"B", Kind::B}, {"C", Kind::C},
    {"I", Kind::I}, {"J", Kind::J}, {"K", Kind::K}, {"N", Kind::N},
    {"F", Kind::F}, {"S", Kind::S}, {"O", Kind::O},
};

inline bool _IsMapping(const Dictionary &dict, const std::string &word) {
  return dict.contains(word);
}

inline bool _IsMapping(const Dictionary &dict, char ch) {
  return std::ranges::find_if(dict, [&](auto &&elem) {
           return ch == elem.first[0];
         }) != dict.end();
}

inline bool IsKeyword(char ch) { return _IsMapping(keywords, ch); }

inline bool IsKeyword(const std::string &word) {
  return _IsMapping(keywords, word);
}

inline bool IsSymbol(char ch) { return _IsMapping(symbols, ch); }

inline bool IsSymbol(const std::string &word) {
  return _IsMapping(symbols, word);
}

inline bool IsGcode(char ch) { return _IsMapping(gcodes, ch); }

inline bool IsGcode(Kind kind) {
  return std::ranges::find_if(gcodes, [&](auto &&pair) {
           return pair.second == kind;
         }) != gcodes.end();
}

inline bool IsGcode(Token tok) { return IsGcode(tok.kind); }

inline bool IsGcode(const std::string &word) {
  return _IsMapping(gcodes, word);
}

inline constexpr char spaces[] = {
    ' ',
    '\t',
};

inline bool IsSpace(char ch) {
  return std::ranges::find(spaces, ch) != std::end(spaces);
}

inline constexpr bool IsSharp(char ch) { return ch == '#'; }

inline constexpr bool IsNewline(char ch) { return ch == '\n'; }

inline constexpr bool IsNewStatement(const Token &tok) {
  return tok.kind == Kind::NEWLINE || tok.kind == Kind::SEMI ||
         tok.kind == Kind::KEOF;
}

inline constexpr bool IsEndOfFile(const Token &tok) {
  return tok.kind == Kind::KEOF;
}
} // namespace token
} // namespace byfxxm

#endif
