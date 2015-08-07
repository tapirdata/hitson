
#include "parser.h"

using v8::Local;
using v8::Handle;
using v8::Value;
using v8::String;
using v8::Object;
using v8::Function;
using v8::FunctionTemplate;


Parser::Parser(Local<Function> errorClass, v8::Local<v8::Object> options) {
  errorClass_.Reset(errorClass);
  Local<Value> conDefsValue = options->Get(Nan::New("connectors").ToLocalChecked());
  if (conDefsValue->IsObject()) {
    Local<Object> conDefs = conDefsValue.As<Object>();
    v8::Local<v8::Array> names = conDefs->GetOwnPropertyNames();
    uint32_t len = names->Length();
    // connectors_.resize(len);
    for (uint32_t i=0; i<len; ++i) {
      Local<String> name = names->Get(i).As<v8::String>();
      Local<Object> conDef = conDefs->Get(name).As<Object>();
      ParseConnector *connector = new ParseConnector();
      connector->self.Reset(conDef);
      Local<Value> hasCreateValue = conDef->Get(Nan::New("hasCreate").ToLocalChecked());
      connector->hasCreate = hasCreateValue->IsBoolean() && hasCreateValue->BooleanValue();
      if (connector->hasCreate) {
        connector->create.Reset(
          conDef->Get(Nan::New(sCreate)).As<Function>()
        );
      } else {
        connector->precreate.Reset(
          conDef->Get(Nan::New(sPrecreate)).As<Function>()
        );
        connector->postcreate.Reset(
          conDef->Get(Nan::New(sPostcreate)).As<Function>()
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
  // std::cout << "Parser::~Parser" << std::endl;
  errorClass_.Reset();
  for (ConnectorMap::iterator it=connectors_.begin(); it != connectors_.end(); ++it) {
    delete it->second;
  }
  connectors_.clear();
  for (std::vector<ParserSource*>::iterator it=psPool_.begin(); it != psPool_.end(); ++it) {
    delete *it;
  }
};

ParserSource* Parser::acquirePs() {
  // std::cout << "Parser::acquirePs #=" << psPool_.size() << std::endl;
  ParserSource* ps;
  if (psPool_.size() > 0) {
    ps = psPool_.back();
    psPool_.pop_back();
  } else {
    ps = new ParserSource(*this);
  }
  return ps;
}

void Parser::releasePs(ParserSource* ps) {
  // std::cout << "Parser::releasePs #=" << psPool_.size() << std::endl;
  psPool_.push_back(ps);
}


Nan::Persistent<Function> Parser::constructor;
Nan::Persistent<String> Parser::sEmpty;
Nan::Persistent<String> Parser::sCreate;
Nan::Persistent<String> Parser::sPrecreate;
Nan::Persistent<String> Parser::sPostcreate;

NAN_METHOD(Parser::New) {
  Nan::HandleScope();
  if (info.Length() < 1 || !(info[0]->IsFunction())) {
    return Nan::ThrowTypeError("First argument should be an error constructor");
  }
  if (info.Length() < 2 || !(info[0]->IsObject())) {
    return Nan::ThrowTypeError("Second argument should be an option object");
  }
  Local<Function> errorClass = info[0].As<Function>();
  Local<Object> options = info[1].As<Object>();
  if (info.IsConstructCall()) {
    Parser* obj = new Parser(errorClass, options);
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    const int argc = 2;
    Local<Value> argv[argc] = {errorClass, errorClass};
    Local<Function> cons = Nan::New<Function>(constructor);
    info.GetReturnValue().Set(cons->NewInstance(argc, argv));
  }
}

NAN_METHOD(Parser::Unescape) {
  Nan::HandleScope();
  TargetBuffer target;
  if (info.Length() < 1 || !(info[0]->IsString())) {
    return Nan::ThrowTypeError("First argument should be a string");
  }
  v8::Local<v8::String> s = info[0].As<v8::String>();

  Parser* self = node::ObjectWrap::Unwrap<Parser>(info.This());
  int errPos = target.appendHandleUnescaped(s);
  if (errPos >= 0) {
    const int argc = 2;
    v8::Local<v8::Value> argv[argc] = { s, Nan::New<v8::Number>(errPos) };
    return Nan::ThrowError(self->createError(argc, argv));
  }
  info.GetReturnValue().Set(target.getHandle());
}


NAN_METHOD(Parser::Parse) {
  Nan::HandleScope();
  if (info.Length() < 1 || !(info[0]->IsString())) {
    return Nan::ThrowTypeError("First argument should be a string");
  }
  Local<String> s = info[0].As<String>();

  Nan::Callback *backrefCb = NULL;
  if (info.Length() >= 2 && (info[1]->IsFunction())) {
    Local<Function> backrefCbHandle = info[1].As<Function>();
    backrefCb = new Nan::Callback(backrefCbHandle);
  }

  Parser* self = node::ObjectWrap::Unwrap<Parser>(info.This());
  ParserSource *ps = self->acquirePs();
  ps->init(s, backrefCb);
  Local<Value> result = ps->getValue(NULL);
  self->releasePs(ps);
  delete backrefCb;
  if (ps->hasError) {
    return Nan::ThrowError(ps->error);
  } else {
    info.GetReturnValue().Set(result);
  }
}

NAN_METHOD(Parser::ParsePartial) {
  Nan::HandleScope();
  if (info.Length() < 1 || !(info[0]->IsString())) {
    return Nan::ThrowTypeError("First argument should be a string");
  }
  if (info.Length() < 3 || !(info[2]->IsFunction())) {
    return Nan::ThrowTypeError("Second argument should be a function");
  }

  Local<String> s = info[0].As<String>();
  Handle<Value> howNext = info[1];

  Local<Function> cbHandle = info[2].As<Function>();
  Nan::Callback *cb = new Nan::Callback(cbHandle);

  Nan::Callback *backrefCb = NULL;
  if (info.Length() >= 4 && (info[3]->IsFunction())) {
    Local<Function> backrefCbHandle = info[3].As<Function>();
    backrefCb = new Nan::Callback(backrefCbHandle);
  }

  Parser* self = node::ObjectWrap::Unwrap<Parser>(info.This());
  ParserSource *ps = self->acquirePs();
  ps->init(s, backrefCb);
  bool reqAbort = false;
  Local<Value> error;
  while (true) {
    Handle<Value> nextRaw;
    if (howNext->IsArray()) {
      Handle<v8::Array> howNextArr = howNext.As<v8::Array>();
      nextRaw = howNextArr->Get(0);
      Local<Value> nSkip = howNextArr->Get(1);
      if (nSkip->IsNumber()) {
        int nSkipVal = nSkip->NumberValue();
        ps->skip(nSkipVal);
      }
    } else if (howNext->IsObject()) {
      error = howNext;
      break;
    } else {
      nextRaw = howNext;
    }
    if (ps->isEnd()) {
      break;
    }
    if (!nextRaw->IsBoolean()) {
      reqAbort = true;
      break;
    }
    bool isValue = true;
    Handle<Value> result = nextRaw->IsTrue() ? ps->getRawValue(&isValue): ps->getValue(&isValue);
    if (ps->hasError) {
      error = ps->error;
      break;
    }
    size_t pos = ps->getPos();
    v8::Local<Value> cbArgv[] = {
      Nan::New(isValue),
      result,
      Nan::New<v8::Number>(pos)
    };
    howNext = cb->Call(3, cbArgv);
  }
  self->releasePs(ps);
  delete cb;
  delete backrefCb;
  if (error.IsEmpty()) {
    info.GetReturnValue().Set(Nan::New<v8::Boolean>(!reqAbort));
  } else {
    return Nan::ThrowError(error);
  }
}

NAN_METHOD(Parser::ConnectorOfCname) {
  Nan::HandleScope();
  if (info.Length() < 1 || !(info[0]->IsString())) {
    return Nan::ThrowTypeError("First argument should be a string");
  }
  Parser* self = node::ObjectWrap::Unwrap<Parser>(info.This());
  BaseBuffer name;
  name.appendHandle(info[0].As<String>());
  const ParseConnector* connector = self->getConnector(name.getBuffer());
  Local<Value> result;
  if (connector) {
    result = Nan::New<Object>(connector->self);
  } else {
    result = Nan::Null();
  }
  info.GetReturnValue().Set(result);
}

void Parser::Init(v8::Handle<v8::Object> exports) {
  Nan::HandleScope();

  Local<FunctionTemplate> newTpl = Nan::New<FunctionTemplate>(New);
  newTpl->SetClassName(Nan::New("Parser").ToLocalChecked());
  newTpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(newTpl, "unescape", Unescape);
  Nan::SetPrototypeMethod(newTpl, "parse", Parse);
  Nan::SetPrototypeMethod(newTpl, "parsePartial", ParsePartial);
  Nan::SetPrototypeMethod(newTpl, "connectorOfCname", ConnectorOfCname);

  constructor.Reset(newTpl->GetFunction());
  sEmpty.Reset(Nan::New("").ToLocalChecked());
  sCreate.Reset(Nan::New("create").ToLocalChecked());
  sPrecreate.Reset(Nan::New("precreate").ToLocalChecked());
  sPostcreate.Reset(Nan::New("postcreate").ToLocalChecked());

  exports->Set(Nan::New("Parser").ToLocalChecked(), newTpl->GetFunction());
}


Local<Value> Parser::createError(int argc, Local<Value> *argv) const {
  return Nan::New<v8::Function>(errorClass_)->NewInstance(argc, argv);
}


