#ifndef TSON_PARSER_H_
#define TSON_PARSER_H_

#include "parser_source.h"

class Parser: public node::ObjectWrap {
  
  friend class ParserSource;

  public:
    static void Init(v8::Handle<v8::Object>);

  private:
    Parser(v8::Local<v8::Function>);
    ~Parser();

    static v8::Persistent<v8::Function> constructor;
    static NAN_METHOD(New);
    static NAN_METHOD(Parse);

    v8::Persistent<v8::Function> errorClass_;
    ParserSource ps_;
};

#endif // TSON_PARSER_H_


