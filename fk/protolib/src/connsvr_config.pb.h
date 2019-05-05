// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: connsvr_config.proto

#ifndef PROTOBUF_connsvr_5fconfig_2eproto__INCLUDED
#define PROTOBUF_connsvr_5fconfig_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3005000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3005000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace protobuf_connsvr_5fconfig_2eproto {
// Internal implementation detail -- do not use these members.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[1];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static const ::google::protobuf::uint32 offsets[];
};
void AddDescriptors();
void InitDefaultsConnsvrConfigImpl();
void InitDefaultsConnsvrConfig();
inline void InitDefaults() {
  InitDefaultsConnsvrConfig();
}
}  // namespace protobuf_connsvr_5fconfig_2eproto
namespace config {
class ConnsvrConfig;
class ConnsvrConfigDefaultTypeInternal;
extern ConnsvrConfigDefaultTypeInternal _ConnsvrConfig_default_instance_;
}  // namespace config
namespace config {

// ===================================================================

class ConnsvrConfig : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:config.ConnsvrConfig) */ {
 public:
  ConnsvrConfig();
  virtual ~ConnsvrConfig();

  ConnsvrConfig(const ConnsvrConfig& from);

  inline ConnsvrConfig& operator=(const ConnsvrConfig& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  ConnsvrConfig(ConnsvrConfig&& from) noexcept
    : ConnsvrConfig() {
    *this = ::std::move(from);
  }

  inline ConnsvrConfig& operator=(ConnsvrConfig&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ConnsvrConfig& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const ConnsvrConfig* internal_default_instance() {
    return reinterpret_cast<const ConnsvrConfig*>(
               &_ConnsvrConfig_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(ConnsvrConfig* other);
  friend void swap(ConnsvrConfig& a, ConnsvrConfig& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline ConnsvrConfig* New() const PROTOBUF_FINAL { return New(NULL); }

  ConnsvrConfig* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const ConnsvrConfig& from);
  void MergeFrom(const ConnsvrConfig& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(ConnsvrConfig* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional string listen_ip = 2;
  bool has_listen_ip() const;
  void clear_listen_ip();
  static const int kListenIpFieldNumber = 2;
  const ::std::string& listen_ip() const;
  void set_listen_ip(const ::std::string& value);
  #if LANG_CXX11
  void set_listen_ip(::std::string&& value);
  #endif
  void set_listen_ip(const char* value);
  void set_listen_ip(const char* value, size_t size);
  ::std::string* mutable_listen_ip();
  ::std::string* release_listen_ip();
  void set_allocated_listen_ip(::std::string* listen_ip);

  // optional bool open_svr = 1;
  bool has_open_svr() const;
  void clear_open_svr();
  static const int kOpenSvrFieldNumber = 1;
  bool open_svr() const;
  void set_open_svr(bool value);

  // optional int32 svr_inst_id = 3;
  bool has_svr_inst_id() const;
  void clear_svr_inst_id();
  static const int kSvrInstIdFieldNumber = 3;
  ::google::protobuf::int32 svr_inst_id() const;
  void set_svr_inst_id(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:config.ConnsvrConfig)
 private:
  void set_has_open_svr();
  void clear_has_open_svr();
  void set_has_listen_ip();
  void clear_has_listen_ip();
  void set_has_svr_inst_id();
  void clear_has_svr_inst_id();

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::HasBits<1> _has_bits_;
  mutable int _cached_size_;
  ::google::protobuf::internal::ArenaStringPtr listen_ip_;
  bool open_svr_;
  ::google::protobuf::int32 svr_inst_id_;
  friend struct ::protobuf_connsvr_5fconfig_2eproto::TableStruct;
  friend void ::protobuf_connsvr_5fconfig_2eproto::InitDefaultsConnsvrConfigImpl();
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// ConnsvrConfig

// optional bool open_svr = 1;
inline bool ConnsvrConfig::has_open_svr() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void ConnsvrConfig::set_has_open_svr() {
  _has_bits_[0] |= 0x00000002u;
}
inline void ConnsvrConfig::clear_has_open_svr() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void ConnsvrConfig::clear_open_svr() {
  open_svr_ = false;
  clear_has_open_svr();
}
inline bool ConnsvrConfig::open_svr() const {
  // @@protoc_insertion_point(field_get:config.ConnsvrConfig.open_svr)
  return open_svr_;
}
inline void ConnsvrConfig::set_open_svr(bool value) {
  set_has_open_svr();
  open_svr_ = value;
  // @@protoc_insertion_point(field_set:config.ConnsvrConfig.open_svr)
}

// optional string listen_ip = 2;
inline bool ConnsvrConfig::has_listen_ip() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void ConnsvrConfig::set_has_listen_ip() {
  _has_bits_[0] |= 0x00000001u;
}
inline void ConnsvrConfig::clear_has_listen_ip() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void ConnsvrConfig::clear_listen_ip() {
  listen_ip_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_listen_ip();
}
inline const ::std::string& ConnsvrConfig::listen_ip() const {
  // @@protoc_insertion_point(field_get:config.ConnsvrConfig.listen_ip)
  return listen_ip_.GetNoArena();
}
inline void ConnsvrConfig::set_listen_ip(const ::std::string& value) {
  set_has_listen_ip();
  listen_ip_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:config.ConnsvrConfig.listen_ip)
}
#if LANG_CXX11
inline void ConnsvrConfig::set_listen_ip(::std::string&& value) {
  set_has_listen_ip();
  listen_ip_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:config.ConnsvrConfig.listen_ip)
}
#endif
inline void ConnsvrConfig::set_listen_ip(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  set_has_listen_ip();
  listen_ip_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:config.ConnsvrConfig.listen_ip)
}
inline void ConnsvrConfig::set_listen_ip(const char* value, size_t size) {
  set_has_listen_ip();
  listen_ip_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:config.ConnsvrConfig.listen_ip)
}
inline ::std::string* ConnsvrConfig::mutable_listen_ip() {
  set_has_listen_ip();
  // @@protoc_insertion_point(field_mutable:config.ConnsvrConfig.listen_ip)
  return listen_ip_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* ConnsvrConfig::release_listen_ip() {
  // @@protoc_insertion_point(field_release:config.ConnsvrConfig.listen_ip)
  clear_has_listen_ip();
  return listen_ip_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ConnsvrConfig::set_allocated_listen_ip(::std::string* listen_ip) {
  if (listen_ip != NULL) {
    set_has_listen_ip();
  } else {
    clear_has_listen_ip();
  }
  listen_ip_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), listen_ip);
  // @@protoc_insertion_point(field_set_allocated:config.ConnsvrConfig.listen_ip)
}

// optional int32 svr_inst_id = 3;
inline bool ConnsvrConfig::has_svr_inst_id() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void ConnsvrConfig::set_has_svr_inst_id() {
  _has_bits_[0] |= 0x00000004u;
}
inline void ConnsvrConfig::clear_has_svr_inst_id() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void ConnsvrConfig::clear_svr_inst_id() {
  svr_inst_id_ = 0;
  clear_has_svr_inst_id();
}
inline ::google::protobuf::int32 ConnsvrConfig::svr_inst_id() const {
  // @@protoc_insertion_point(field_get:config.ConnsvrConfig.svr_inst_id)
  return svr_inst_id_;
}
inline void ConnsvrConfig::set_svr_inst_id(::google::protobuf::int32 value) {
  set_has_svr_inst_id();
  svr_inst_id_ = value;
  // @@protoc_insertion_point(field_set:config.ConnsvrConfig.svr_inst_id)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace config

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_connsvr_5fconfig_2eproto__INCLUDED
