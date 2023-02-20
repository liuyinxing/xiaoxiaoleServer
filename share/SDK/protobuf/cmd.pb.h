// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: cmd.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_cmd_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_cmd_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3021000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3021009 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_util.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_cmd_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_cmd_2eproto {
  static const uint32_t offsets[];
};
PROTOBUF_NAMESPACE_OPEN
PROTOBUF_NAMESPACE_CLOSE
namespace mycmd {

enum cmd_code : int {
  cmd_heart = 65000,
  cmd_rcode = 65001,
  cmd_security = 65002
};
bool cmd_code_IsValid(int value);
constexpr cmd_code cmd_code_MIN = cmd_heart;
constexpr cmd_code cmd_code_MAX = cmd_security;
constexpr int cmd_code_ARRAYSIZE = cmd_code_MAX + 1;

const std::string& cmd_code_Name(cmd_code value);
template<typename T>
inline const std::string& cmd_code_Name(T enum_t_value) {
  static_assert(::std::is_same<T, cmd_code>::value ||
    ::std::is_integral<T>::value,
    "Incorrect type passed to function cmd_code_Name.");
  return cmd_code_Name(static_cast<cmd_code>(enum_t_value));
}
bool cmd_code_Parse(
    ::PROTOBUF_NAMESPACE_ID::ConstStringParam name, cmd_code* value);
// ===================================================================


// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace mycmd

PROTOBUF_NAMESPACE_OPEN

template <> struct is_proto_enum< ::mycmd::cmd_code> : ::std::true_type {};

PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_cmd_2eproto