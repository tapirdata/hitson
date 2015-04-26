#ifndef WSON_STINGIFIER_H_
#define WSON_STINGIFIER_H_

#include "stringifier_target.h"

class Stringifier: public node::ObjectWrap {
  public:
    static void Init(v8::Handle<v8::Object>);

  private:
    Stringifier(v8::Local<v8::Function>);
    ~Stringifier();

    static v8::Persistent<v8::Function> constructor;
    static NAN_METHOD(New);
    static NAN_METHOD(Escape);
    static NAN_METHOD(Stringify);

    v8::Persistent<v8::Function> errorClass_;
    StringifierTarget st_;
};


#endif // WSON_STINGIFIER_H_

