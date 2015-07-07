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
  Local<Value> conDefsValue = options->Get(NanNew("connectors"));
  if (conDefsValue->IsObject()) {
    Local<Object> conDefs = conDefsValue.As<Object>();
    v8::Local<v8::Array> names = conDefs->GetOwnPropertyNames();
    uint32_t len = names->Length();
    connectors_.resize(len);
    for (uint32_t i=0; i<len; ++i) {
      Local<String> name = names->Get(i).As<v8::String>();
      Local<Object> conDef = conDefs->Get(name).As<Object>();
      StringifyConnector* connector = new StringifyConnector();
      NanAssignPersistent(connector->self, conDef);
      NanAssignPersistent(connector->by,
        conDef->Get(NanNew(sBy)).As<Function>()
      );
      NanAssignPersistent(connector->split,
        conDef->Get(NanNew(sSplit)).As<Function>()
      );
      connector->name.appendHandleEscaped(name);
      connectors_[i] = connector;
    }
  }
  // std::cout << "Stringifier connectors_.size()=" << connectors_.size() << std::endl;
};

Stringifier::~Stringifier() {
  NanDisposePersistent(errorClass_);
  for (ConnectorVector::iterator it=connectors_.begin(); it != connectors_.end(); ++it) {
    delete (*it);
  }
  connectors_.clear();
};

v8::Persistent<v8::Function> Stringifier::constructor;
v8::Persistent<v8::String> Stringifier::sBy;
v8::Persistent<v8::String> Stringifier::sSplit;
v8::Persistent<v8::String> Stringifier::sConstructor;

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
  if (args.Length() < 1) {
    return NanThrowTypeError("Missing first argument");
  }

  NanCallback *haverefCb = NULL;
  if (args.Length() >= 2 && (args[1]->IsFunction())) {
    Local<Function> haveCbHandle = args[1].As<Function>();
    haverefCb = new NanCallback(haveCbHandle);
  }

  st.clear(haverefCb);
  st.put(args[0]);

  Local<Value> result = st.target.getHandle();
  delete haverefCb;
  NanReturnValue(result);
}

void Stringifier::Init(v8::Handle<v8::Object> exports) {
  NanScope();

  Local<FunctionTemplate> newTpl = NanNew<FunctionTemplate>(New);
  newTpl->SetClassName(NanNew("Stringifier"));
  newTpl->InstanceTemplate()->SetInternalFieldCount(1);

  NODE_SET_PROTOTYPE_METHOD(newTpl, "escape", Escape);
  NODE_SET_PROTOTYPE_METHOD(newTpl, "stringify", Stringify);

  NanAssignPersistent(constructor, newTpl->GetFunction());
  NanAssignPersistent(sBy, NanNew("by"));
  NanAssignPersistent(sSplit, NanNew("split"));
  NanAssignPersistent(sConstructor, NanNew("constructor"));

  exports->Set(NanNew("Stringifier"), newTpl->GetFunction());

  StringifierTarget::Init();
}


