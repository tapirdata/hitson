#ifndef WSON_STINGIFIER_H_
#define WSON_STINGIFIER_H_

#include "stringifier_target.h"

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
        NanDisposePersistent(self);
        NanDisposePersistent(by);
        NanDisposePersistent(split);
      }
    };

    inline const StringifyConnector* findConnector(v8::Local<v8::Object>) const;

    static v8::Persistent<v8::Function> constructor;
    static v8::Persistent<v8::String> sBy;
    static v8::Persistent<v8::String> sSplit;
    static v8::Persistent<v8::String> sConstructor;
    static v8::Persistent<v8::Function> objectConstructor;

    static NAN_METHOD(New);
    static NAN_METHOD(Escape);
    static NAN_METHOD(Stringify);
    static NAN_METHOD(ConnectorOfValue);

    typedef std::vector<StringifyConnector*> ConnectorVector;

    v8::Persistent<v8::Function> errorClass_;
    ConnectorVector connectors_;
    StringifierTarget st_;
};

const Stringifier::StringifyConnector* Stringifier::findConnector(v8::Local<v8::Object> x) const {
  v8::Local<v8::Value> constructor = x->Get(NanNew(sConstructor));
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


#endif // WSON_STINGIFIER_H_

