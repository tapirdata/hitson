
#include "parser.h"

using v8::Local;
using v8::Persistent;
using v8::Value;
using v8::String;
using v8::Object;
using v8::Function;
using v8::FunctionTemplate;


Parser::Parser(Local<Function> errorClass, v8::Local<v8::Object> options): ps_(*this) {
  NanAssignPersistent(errorClass_, errorClass);
  Local<Value> conDefsValue = options->Get(NanNew("connectors"));
  if (conDefsValue->IsObject()) {
    Local<Object> conDefs = conDefsValue.As<Object>();
    v8::Local<v8::Array> names = conDefs->GetOwnPropertyNames();
    uint32_t len = names->Length();
    // connectors_.resize(len);
    for (uint32_t i=0; i<len; ++i) {
      Local<String> name = names->Get(i).As<v8::String>();
      Local<Object> conDef = conDefs->Get(name).As<Object>();
      ParseConnector *connector = new ParseConnector();
      NanAssignPersistent(connector->self, conDef);
      Local<Value> hasCreateValue = conDef->Get(NanNew("hasCreate"));
      connector->hasCreate = hasCreateValue->IsBoolean() && hasCreateValue->BooleanValue();
      if (connector->hasCreate) {
        NanAssignPersistent(connector->create,
          conDef->Get(NanNew(sCreate)).As<Function>()
        );
      } else {
        NanAssignPersistent(connector->precreate,
          conDef->Get(NanNew(sPrecreate)).As<Function>()
        );
        NanAssignPersistent(connector->postcreate,
          conDef->Get(NanNew(sPostcreate)).As<Function>()
        );
      }
      // std::cout << i << " hasCreate=" << connector.hasCreate << std::endl;
      connector->name.appendHandleEscaped(name);
      connectors_[connector->name.getBuffer()] = connector;
    }
  }
  // std::cout << "Parser connectors_.size()=" << connectors_.size() << std::endl;
};

Parser::~Parser() {
  NanDisposePersistent(errorClass_);
  for (ConnectorMap::iterator it=connectors_.begin(); it != connectors_.end(); ++it) {
    delete it->second;
  }
  connectors_.clear();
};

Persistent<Function> Parser::constructor;
Persistent<String> Parser::sEmpty;
Persistent<String> Parser::sCreate;
Persistent<String> Parser::sPrecreate;
Persistent<String> Parser::sPostcreate;

NAN_METHOD(Parser::New) {
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
    Parser* obj = new Parser(errorClass, options);
    obj->Wrap(args.This());
    NanReturnValue(args.This());
  } else {
    const int argc = 2;
    Local<Value> argv[argc] = {errorClass, errorClass};
    Local<Function> cons = NanNew<Function>(constructor);
    NanReturnValue(cons->NewInstance(argc, argv));
  }
}

NAN_METHOD(Parser::Unescape) {
  NanScope();
  TargetBuffer target;
  if (args.Length() < 1 || !(args[0]->IsString())) {
    return NanThrowTypeError("First argument should be a string");
  }
  v8::Local<v8::String> s = args[0].As<v8::String>();

  Parser* self = node::ObjectWrap::Unwrap<Parser>(args.This());
  int errPos = target.appendHandleUnescaped(s);
  if (errPos >= 0) {
    const int argc = 2;
    v8::Local<v8::Value> argv[argc] = { s, NanNew<v8::Number>(errPos) };
    return NanThrowError(self->createError(argc, argv));
  }
  NanReturnValue(target.getHandle());
}


NAN_METHOD(Parser::Parse) {
  NanScope();
  if (args.Length() < 1 || !(args[0]->IsString())) {
    return NanThrowTypeError("First argument should be a string");
  }
  Local<String> s = args[0].As<String>();

  Parser* self = node::ObjectWrap::Unwrap<Parser>(args.This());
  ParserSource &ps = self->ps_;
  ps.init(s);
  Local<Value> result = ps.getValue(NULL);
  if (ps.hasError) {
    return NanThrowError(ps.error);
  }
  NanReturnValue(result);

}

NAN_METHOD(Parser::ParsePartial) {
  NanScope();
  if (args.Length() < 1 || !(args[0]->IsString())) {
    return NanThrowTypeError("First argument should be a string");
  }
  Local<String> s = args[0].As<String>();

  if (args.Length() < 2 || !(args[1]->IsFunction())) {
    return NanThrowTypeError("Second argument should be a function");
  }
  Local<Function> cbHandle = args[1].As<Function>();
  NanCallback *cb = new NanCallback(cbHandle);

  Parser* self = node::ObjectWrap::Unwrap<Parser>(args.This());
  ParserSource &ps = self->ps_;
  ps.init(s);
  while (!ps.isEnd()) {
    bool isValue = true;
    Local<Value> result = ps.getValue(&isValue);
    if (ps.hasError) {
      break;
    }
    v8::Handle<Value> cbArgv[] = {
      NanNew(isValue),
      result
    };
    cb->Call(2, cbArgv);
  }
  if (!ps.hasError) {
    v8::Handle<Value> cbArgv[] = {
      NanFalse(),
      NanNull()
    };
    cb->Call(2, cbArgv);
  }  
  delete cb;
  if (ps.hasError) {
    return NanThrowError(ps.error);
  }
  NanReturnValue(NanUndefined());
}


void Parser::Init(v8::Handle<v8::Object> exports) {
  NanScope();

  Local<FunctionTemplate> newTpl = NanNew<FunctionTemplate>(New);
  newTpl->SetClassName(NanNew("Parser"));
  newTpl->InstanceTemplate()->SetInternalFieldCount(1);

  NODE_SET_PROTOTYPE_METHOD(newTpl, "unescape", Unescape);
  NODE_SET_PROTOTYPE_METHOD(newTpl, "parse", Parse);
  NODE_SET_PROTOTYPE_METHOD(newTpl, "parsePartial", ParsePartial);

  NanAssignPersistent(constructor, newTpl->GetFunction());
  NanAssignPersistent(sEmpty, NanNew(""));
  NanAssignPersistent(sCreate, NanNew("create"));
  NanAssignPersistent(sPrecreate, NanNew("precreate"));
  NanAssignPersistent(sPostcreate, NanNew("postcreate"));

  exports->Set(NanNew("Parser"), newTpl->GetFunction());
}


Local<Object> Parser::createError(int argc, Local<Value> *argv) const {
  return NanNew<v8::Function>(errorClass_)->NewInstance(argc, argv);
}


