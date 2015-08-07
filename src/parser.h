#ifndef WSON_PARSER_H_
#define WSON_PARSER_H_

#include "parser_source.h"

class Parser: public node::ObjectWrap {

  friend class ParserSource;

  public:
    static void Init(v8::Handle<v8::Object>);
    v8::Local<v8::Value> createError(int argc, v8::Local<v8::Value> argv[]) const;

  private:
    Parser(v8::Local<v8::Function>, v8::Local<v8::Object>);
    ~Parser();

    struct ParseConnector {
      Nan::Persistent<v8::Object> self;
      Nan::Persistent<v8::Function> create;
      Nan::Persistent<v8::Function> precreate;
      Nan::Persistent<v8::Function> postcreate;
      TargetBuffer name;
      bool hasCreate;

      ~ParseConnector() {
        self.Reset();
        create.Reset();
        precreate.Reset();
        postcreate.Reset();
      }
    };

    inline const ParseConnector* getConnector(const usc2vector&) const;
    ParserSource* acquirePs();
    void releasePs(ParserSource*);

    static Nan::Persistent<v8::Function> constructor;
    static Nan::Persistent<v8::String> sEmpty;
    static Nan::Persistent<v8::String> sCreate;
    static Nan::Persistent<v8::String> sPrecreate;
    static Nan::Persistent<v8::String> sPostcreate;
    static NAN_METHOD(New);
    static NAN_METHOD(Unescape);
    static NAN_METHOD(Parse);
    static NAN_METHOD(ParsePartial);
    static NAN_METHOD(ConnectorOfCname);

    typedef std::map<usc2vector, ParseConnector* > ConnectorMap;

    Nan::Persistent<v8::Function> errorClass_;
    ConnectorMap connectors_;
    std::vector<ParserSource*> psPool_;
};

const Parser::ParseConnector* Parser::getConnector(const usc2vector& name) const {
  ConnectorMap::const_iterator it = connectors_.find(name);
  if (it == connectors_.end()) {
    return NULL;
  }
  return it->second;
}

#endif // WSON_PARSER_H_


