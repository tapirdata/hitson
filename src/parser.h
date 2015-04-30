#ifndef WSON_PARSER_H_
#define WSON_PARSER_H_

#include "parser_source.h"

class Parser: public node::ObjectWrap {
  
  friend class ParserSource;

  public:
    static void Init(v8::Handle<v8::Object>);
    v8::Local<v8::Object> createError(int argc, v8::Local<v8::Value> argv[]) const;

  private:
    Parser(v8::Local<v8::Function>, v8::Local<v8::Object>);
    ~Parser();

    static v8::Persistent<v8::Function> constructor;
    static NAN_METHOD(New);
    static NAN_METHOD(Unescape);
    static NAN_METHOD(Parse);

    v8::Persistent<v8::Function> errorClass_;
    ParserSource::ConnectorMap connectors_;
    ParserSource ps_;
};

#endif // WSON_PARSER_H_


