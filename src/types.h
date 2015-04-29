#ifndef WSON_TYPES_H_
#define WSON_TYPES_H_

#include <nan.h>
#include <vector>

typedef std::vector<uint16_t> usc2vector;

enum Ctype {
  TEXT,
  OBJECT,
  ENDOBJECT,
  ARRAY,
  ENDARRAY,
  IS,
  LITERAL,
  PIPE,
  QUOTE,
  END,
};


#define SYNTAX_ERROR -1


#if (NODE_MODULE_VERSION > NODE_0_10_MODULE_VERSION)

template<typename T>
NAN_INLINE void NanAssignPersistent(
    v8::Persistent<T, v8::CopyablePersistentTraits<T> >& handle
  , v8::Handle<T> obj) {
    handle.Reset(v8::Isolate::GetCurrent(), obj);
}

template<typename T> NAN_INLINE void NanDisposePersistent(
    v8::Persistent<T, v8::CopyablePersistentTraits<T> >& handle) {
    handle.Reset();
}

typedef v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function> > PersistentFunction;

NAN_INLINE v8::Local<v8::Function> UnwrapPersistent(PersistentFunction fn) {
  return v8::Local<v8::Function>::New(v8::Isolate::GetCurrent(), fn);
}

#else

typedef v8::Persistent<v8::Function> PersistentFunction;

NAN_INLINE v8::Local<v8::Function> UnwrapPersistent(PersistentFunction fn) {
  return NanNew<v8::Function>(fn);
}


#endif


#endif // WSON_TYPES_H_

