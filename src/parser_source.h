#ifndef WSON_PARSER_SOURCE_H_
#define WSON_PARSER_SOURCE_H_

#include "source_buffer.h"
#include <map>
#include <memory>

class Parser;

struct ParseFrame {
  v8::Local<v8::Object> value;
  ParseFrame *parent;
  bool isBackreffed;
  bool vetoBackref;

  inline ParseFrame(v8::Local<v8::Object> v, ParseFrame *p):
    value(v),
    parent(p),
    isBackreffed(false),
    vetoBackref(false)
  {}
};

class ParserSource {
  public:
    friend class Parser;

    ParserSource(Parser& parser): parser_(parser) {
      // std::cout << "ParserSource::ParserSource" << std::endl;
    }
    ~ParserSource() {
      // std::cout << "ParserSource::~ParserSource" << std::endl;
    }
    void init(v8::Local<v8::String> s) {
      hasError=false;
      source.init(s);
    }
    inline void next() { source.next(); }
    inline bool isEnd() { return source.nextType == END; }
    inline v8::Local<v8::String> getText();
    inline v8::Local<v8::Value> getLiteral();
    inline v8::Local<v8::Object> getBackreffed(ParseFrame& frame);
    inline v8::Local<v8::Object> getArray(ParseFrame* parentFrame);
    inline v8::Local<v8::Object> getObject(ParseFrame* parentFrame);
    inline v8::Local<v8::Object> getCustom(ParseFrame* parentFrame);
    v8::Local<v8::Value> getValue(bool* isValue);
    void makeError(int pos = -1, const BaseBuffer* cause=NULL);
  private:
    Parser& parser_;
    SourceBuffer source;
    bool hasError;
    v8::Local<v8::Object> error;
};


#endif // WSON_PARSER_SOURCE_H_

