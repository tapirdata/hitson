#ifndef WSON_STINGIFIER_H_
#define WSON_STINGIFIER_H_

#include "stringifier_target.h"

enum {
  TI_FAIL      = 0,
  TI_UNDEFINED = 1,
  TI_NULL      = 2,
  TI_BOOLEAN   = 4,
  TI_NUMBER    = 8,
  TI_DATE      = 16,
  TI_STRING    = 20,
  TI_ARRAY     = 24,
  TI_OBJECT    = 32
};

class Stringifier: public node::ObjectWrap {
  public:
    friend class StringifierTarget;
    static void Init(v8::Handle<v8::Object>);

  private:
    Stringifier(v8::Local<v8::Function>, v8::Local<v8::Object>);
    ~Stringifier();

    struct StringifyConnector {
      v8::Persistent<v8::Object> self;
      v8::Persistent<v8::Function> by;
      v8::Persistent<v8::Function> split;
      TargetBuffer name;

      ~StringifyConnector() {
        Nan::DisposePersistent(self);
        Nan::DisposePersistent(by);
        Nan::DisposePersistent(split);
      }
    };

    inline static int getTypeid(v8::Local<v8::Value> x);
    inline const StringifyConnector* findConnector(v8::Local<v8::Object>) const;

    static v8::Persistent<v8::Function> constructor;
    static v8::Persistent<v8::String> sBy;
    static v8::Persistent<v8::String> sSplit;
    static v8::Persistent<v8::String> sConstructor;
    static v8::Persistent<v8::Function> objectConstructor;

    static NAN_METHOD(New);
    static NAN_METHOD(Escape);
    static NAN_METHOD(GetTypeid);
    static NAN_METHOD(Stringify);
    static NAN_METHOD(ConnectorOfValue);

    typedef std::vector<StringifyConnector*> ConnectorVector;

    v8::Persistent<v8::Function> errorClass_;
    ConnectorVector connectors_;
    StringifierTarget st_;
};

const Stringifier::StringifyConnector* Stringifier::findConnector(v8::Local<v8::Object> x) const {
  v8::Local<v8::Value> constructor = x->Get(Nan::New(sConstructor));
  if (constructor->IsFunction() and constructor != objectConstructor) {
    for (ConnectorVector::const_iterator it=connectors_.begin(); it != connectors_.end(); ++it) {
      // std::cout << "findConnector" << std::endl;
      if ((*it)->by == constructor) {
        return (*it);
      }
    }
  }
  return NULL;
}

int Stringifier::getTypeid(v8::Local<v8::Value> x) {
  if (x->IsString()) {
    return TI_STRING;
  } else if (x->IsNumber()) {
    return TI_NUMBER;
  } else if (x->IsUndefined()) {
    return TI_UNDEFINED;
  } else if (x->IsNull()) {
    return TI_NULL;
  } else if (x->IsBoolean()) {
    return TI_BOOLEAN;
  } else if (x->IsDate()) {
    return TI_DATE;
  } else if (x->IsArray()) {
    return TI_ARRAY;
  } else if (x->IsObject()) {
    return TI_OBJECT;
  } else {
    return TI_FAIL;
  }
}


#endif // WSON_STINGIFIER_H_

