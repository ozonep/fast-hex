#include <napi.h>
#include <uv.h>
#include <stdint.h>
#include <iostream>
#include "hex.h"

#if defined(__GNUC__) // GCC, clang
#include <x86intrin.h> // _mm_malloc
#elif defined(_MSC_VER)
#include <intrin.h>
#endif

#ifdef PROFILE_
#include <chrono>
#include <time.h>
#endif

using namespace Napi;

// Returns true if data was allocated and should be freed (with _mm_free)
bool bytesFromString(Napi::Value val, const uint8_t** data, size_t* length) {
  Napi::Env env = val.Env();
  if (val.IsBuffer()) {
    *data = reinterpret_cast<uint8_t*>(val.As<Napi::Buffer<char>>().Data());
    *length = val.As<Napi::Buffer<char>>().Length();
    return false;
  }

  if (!val.IsString()) return false;

  Napi::String str = val.As<Napi::String>();
  if (str->IsExternalOneByte()) {
    //std::cout << "external one byte" << std::endl;
    const String::ExternalOneByteStringResource* ext = str->GetExternalOneByteStringResource();
    *data = (const uint8_t*)ext->data();
    *length = ext->length();
    return false;
  } else if (str->IsOneByte()) {
    //std::cout << "internal one byte" << std::endl;
    *length = str->Length();
    *data = (const uint8_t*)_mm_malloc(*length, 64);
    str->WriteOneByte(const_cast<uint8_t*>(*data));
    return true;
  } else {
    std::cout << "external 2-byte string encountered" << std::endl;
    return false;
  }
}

template <int METHOD>
Napi::Value decodeHex(const Napi::CallbackInfo& info) {
#ifdef PROFILE_
  std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
  std::chrono::nanoseconds time_span;
  start = std::chrono::high_resolution_clock::now();
#endif

  const uint8_t* value;
  size_t inLen;
  bool needsFree = bytesFromString(info[1], &value, &inLen);

  Local<Uint8Array> destTa = info[0].As<Uint8Array>();
  size_t outLen = inLen >> 1;
  Napi::TypedArrayContents<uint8_t> decoded(destTa);


#ifdef PROFILE_
  end = std::chrono::high_resolution_clock::now();
  time_span = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << time_span.count() << " ns\n";
  start = std::chrono::high_resolution_clock::now();
#endif

  if (METHOD == 0) decodeHexLUT(*decoded, value, outLen);
  if (METHOD == 1) decodeHexLUT4(*decoded, value, outLen);
  if (METHOD == 2) decodeHexVec(*decoded, value, outLen);

#ifdef PROFILE_
  end = std::chrono::high_resolution_clock::now();
  time_span = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << time_span.count() << " ns\n";
#endif

  if (needsFree) _mm_free((void*)value);
  //Local<v8::Object> buf = Napi::Buffer<char>::New(env, decoded, bufLength);
  //return buf;
}

template <int METHOD>
Napi::Value encodeHex(const Napi::CallbackInfo& info) {
  EscapableHandleScope scope(Isolate::GetCurrent());

  Local<Uint8Array> srcTa = info[0].As<Uint8Array>();
  Napi::TypedArrayContents<uint8_t> src(srcTa);
  size_t srcLen = srcTa->Length();

  size_t destLen = srcLen << 1;
  uint8_t* dst = (uint8_t*)_mm_malloc(destLen, 64);

#ifdef PROFILE_
  std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
  std::chrono::nanoseconds time_span;
  start = std::chrono::high_resolution_clock::now();
#endif

  if (METHOD == 0) encodeHex(dst, *src, srcLen);
  if (METHOD == 1) encodeHexVec(dst, *src, srcLen);

#ifdef PROFILE_
  end = std::chrono::high_resolution_clock::now();
  time_span = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << time_span.count() << " ns\n";
  start = std::chrono::high_resolution_clock::now();
#endif

  Napi::String str = v8::String::NewFromOneByte(
    Isolate::GetCurrent(),
    dst,
    v8::NewStringType::kNormal,
    destLen);

#ifdef PROFILE_
  end = std::chrono::high_resolution_clock::now();
  time_span = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << time_span.count() << " ns\n";
#endif

  _mm_free(dst);

  return scope.Escape(str);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  (target).Set(Napi::String::New(env, "decodeHexNode"),
    Napi::GetFunction(Napi::Napi::FunctionReference::New(env, decodeHex<0>)));
  (target).Set(Napi::String::New(env, "decodeHexNode2"),
    Napi::GetFunction(Napi::Napi::FunctionReference::New(env, decodeHex<1>)));
  (target).Set(Napi::String::New(env, "decodeHexVec"),
    Napi::GetFunction(Napi::Napi::FunctionReference::New(env, decodeHex<2>)));

  (target).Set(Napi::String::New(env, "encodeHex"),
    Napi::GetFunction(Napi::Napi::FunctionReference::New(env, encodeHex<0>)));
  (target).Set(Napi::String::New(env, "encodeHexVec"),
    Napi::GetFunction(Napi::Napi::FunctionReference::New(env, encodeHex<1>)));
}

NODE_API_MODULE(strdecode, Init);
