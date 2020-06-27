#include <nan.h>
#include <stdint.h>
#include <iostream>
#include "hex.h"

#if defined(__GNUC__)
#include <x86intrin.h>
#endif

#ifdef PROFILE_
#include <chrono>
#include <time.h>
#endif

using namespace v8;

bool bytesFromString(Local<Value> val, const uint8_t** data, size_t* length) {
  if (node::Buffer::HasInstance(val)) {
    *data = reinterpret_cast<uint8_t*>(node::Buffer::Data(val));
    *length = node::Buffer::Length(val);
    return false;
  }

  if (!val->IsString()) return false;

  Local<String> str = val.As<String>();
  if (str->IsExternalOneByte()) {
    const String::ExternalOneByteStringResource* ext = str->GetExternalOneByteStringResource();
    *data = (const uint8_t*)ext->data();
    *length = ext->length();
    return false;
  } else if (str->IsOneByte()) {
    *length = str->Length();
    *data = (const uint8_t*)_mm_malloc(*length, 64);
    str->WriteOneByte(Nan::GetCurrentContext()->GetIsolate(), const_cast<uint8_t*>(*data));
    return true;
  } else {
    std::cout << "external 2-byte string encountered" << std::endl;
    return false;
  }
}

template <int METHOD>
NAN_METHOD(decodeHex) {
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
  Nan::TypedArrayContents<uint8_t> decoded(destTa);


#ifdef PROFILE_
  end = std::chrono::high_resolution_clock::now();
  time_span = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << time_span.count() << " ns\n";
  start = std::chrono::high_resolution_clock::now();
#endif

  if (METHOD == 2) decodeHexVec(*decoded, value, outLen);

#ifdef PROFILE_
  end = std::chrono::high_resolution_clock::now();
  time_span = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << time_span.count() << " ns\n";
#endif

  if (needsFree) _mm_free((void*)value);
}

NAN_MODULE_INIT(Init) {
  Nan::Set(target, Nan::New("decodeHexVec").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(decodeHex<2>)).ToLocalChecked());
}

NODE_MODULE(strdecode, Init);
