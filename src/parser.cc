
#include "parser.h"

using v8::Local;
using v8::Persistent;
using v8::Value;
using v8::String;
using v8::Object;
using v8::Function;
using v8::FunctionTemplate;


Parser::Parser(Local<Function> errorClass): ps_(*this) {
  NanAssignPersistent(errorClass_, errorClass);
};

Parser::~Parser() {
  NanDisposePersistent(errorClass_);
};

Persistent<Function> Parser::constructor;

NAN_METHOD(Parser::New) {
  NanScope();
  if (args.Length() < 1 || !(args[0]->IsFunction())) {
    return NanThrowTypeError("First argument should be an error constructor");
  }  
  Local<Function> errorClass = args[0].As<Function>();
  if (args.IsConstructCall()) {
    Parser* obj = new Parser(errorClass);
    obj->Wrap(args.This());
    NanReturnValue(args.This());
  } else {
    const int argc = 1;
    Local<Value> argv[argc] = {errorClass};
    Local<Function> cons = NanNew<Function>(constructor);
    NanReturnValue(cons->NewInstance(argc, argv));
  }  
}

NAN_METHOD(Parser::Parse) {
  NanScope();
  Parser* self = node::ObjectWrap::Unwrap<Parser>(args.This());
  if (args.Length() < 1 || !(args[0]->IsString())) {
    return NanThrowTypeError("First argument should be a string");
  }
  Local<String> s = args[0].As<String>();
  ParserSource &ps = self->ps_;
  ps.init(s);
  Local<Value> result = ps.getValue();
  if (ps.hasError) {
    // TargetBuffer errorMsg;
    // ps.source.makeError(errorMsg);
    // result = ps.error; 
    return NanThrowError(ps.error);
  }
  NanReturnValue(result);

}

void Parser::Init(v8::Handle<v8::Object> exports) {
  NanScope();

  Local<FunctionTemplate> newTpl = NanNew<FunctionTemplate>(New);
  newTpl->SetClassName(NanNew("Parser"));
  newTpl->InstanceTemplate()->SetInternalFieldCount(1);

  NODE_SET_PROTOTYPE_METHOD(newTpl, "parse", Parse);

  NanAssignPersistent(constructor, newTpl->GetFunction());
  exports->Set(NanNew("Parser"), newTpl->GetFunction());
}





