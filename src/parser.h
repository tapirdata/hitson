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

    struct ParseConnector {
      v8::Persistent<v8::Object> self;
      v8::Persistent<v8::Function> create;
      v8::Persistent<v8::Function> precreate;
      v8::Persistent<v8::Function> postcreate;
      TargetBuffer name;
      bool hasCreate;

      ~ParseConnector() {
        NanDisposePersistent(self);
        NanDisposePersistent(create);
        NanDisposePersistent(precreate);
        NanDisposePersistent(postcreate);
      }
    };

    inline const ParseConnector* getConnector(const usc2vector&) const;

    static v8::Persistent<v8::Function> constructor;
    static v8::Persistent<v8::String> sEmpty;
    static v8::Persistent<v8::String> sCreate;
    static v8::Persistent<v8::String> sPrecreate;
    static v8::Persistent<v8::String> sPostcreate;
    static NAN_METHOD(New);
    static NAN_METHOD(Unescape);
    static NAN_METHOD(Parse);

    typedef std::map<usc2vector, ParseConnector* > ConnectorMap;

    v8::Persistent<v8::Function> errorClass_;
    ConnectorMap connectors_;
    ParserSource ps_;
};

const Parser::ParseConnector* Parser::getConnector(const usc2vector& name) const {
  ConnectorMap::const_iterator it = connectors_.find(name);
  if (it == connectors_.end()) {
    return NULL;
  }  
  return it->second;
}

#endif // WSON_PARSER_H_


