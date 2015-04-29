#include "stringifier.h"

using v8::Local;
using v8::Value;
using v8::String;
using v8::Array;
using v8::Object;
using v8::Function;
using v8::FunctionTemplate;


Stringifier::Stringifier(Local<Function> errorClass, v8::Local<v8::Object> options): st_(*this) {
  NanAssignPersistent(errorClass_, errorClass);
  Local<Value> connectors = options->Get(NanNew("connectors"));
  if (connectors->IsObject()) {
    Local<Object> cons = connectors.As<Object>();
    v8::Local<v8::Array> names = cons->GetOwnPropertyNames();
    uint32_t len = names->Length();
    connectors_.resize(len);
    for (uint32_t i=0; i<len; ++i) {
      Local<String> name = names->Get(i).As<v8::String>();
      Local<Object> con = cons->Get(name).As<Object>();
      Local<Function> by = con->Get(NanNew("by")).As<Function>();
      Local<Function> split = con->Get(NanNew("split")).As<Function>();
      Connector &connector = connectors_[i];
      NanAssignPersistent(connector.by, by);
      NanAssignPersistent(connector.split, split);
      connector.name.appendHandleEscaped(name);
    }
  }
};

Stringifier::~Stringifier() {
  NanDisposePersistent(errorClass_);
  for (connectorVector::iterator it=connectors_.begin(); it != connectors_.end(); ++it) {
    NanDisposePersistent(it->by);
    NanDisposePersistent(it->split);
  }
};

v8::Persistent<v8::Function> Stringifier::constructor;

NAN_METHOD(Stringifier::New) {
  NanScope();
  if (args.Length() < 1 || !(args[0]->IsFunction())) {
    return NanThrowTypeError("First argument should be an error constructor");
  }
  if (args.Length() < 2 || !(args[0]->IsObject())) {
    return NanThrowTypeError("Second argument should be an option object");
  }
  Local<Function> errorClass = args[0].As<Function>();
  Local<Object> options = args[1].As<Object>();
  if (args.IsConstructCall()) {
    Stringifier* obj = new Stringifier(errorClass, options);
    obj->Wrap(args.This());
    NanReturnValue(args.This());
  } else {
    const int argc = 2;
    Local<Value> argv[argc] = {errorClass, options};
    Local<Function> cons = NanNew<Function>(constructor);
    NanReturnValue(cons->NewInstance(argc, argv));
  }
}

NAN_METHOD(Stringifier::Escape) {
  NanScope();
  TargetBuffer target;
  if (args.Length() < 1 || !(args[0]->IsString())) {
    return NanThrowTypeError("First argument should be a string");
  }
  v8::Local<v8::String> s = args[0].As<v8::String>();
  // Stringifier* self = node::ObjectWrap::Unwrap<Stringifier>(args.This());
  target.appendHandleEscaped(s);
  NanReturnValue(target.getHandle());
}


NAN_METHOD(Stringifier::Stringify) {
  NanScope();
  Stringifier* self = node::ObjectWrap::Unwrap<Stringifier>(args.This());
  StringifierTarget &st = self->st_;
  // st.target.reserve(128);
  st.clear();
  st.putValue(args[0]);
  NanReturnValue(st.target.getHandle());
}

void Stringifier::Init(v8::Handle<v8::Object> exports) {
  NanScope();

  Local<FunctionTemplate> newTpl = NanNew<FunctionTemplate>(New);
  newTpl->SetClassName(NanNew("Stringifier"));
  newTpl->InstanceTemplate()->SetInternalFieldCount(1);

  NODE_SET_PROTOTYPE_METHOD(newTpl, "escape", Escape);
  NODE_SET_PROTOTYPE_METHOD(newTpl, "stringify", Stringify);

  NanAssignPersistent(constructor, newTpl->GetFunction());
  exports->Set(NanNew("Stringifier"), newTpl->GetFunction());

  StringifierTarget::Init();
}


