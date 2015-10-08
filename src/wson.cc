#include "stringifier.h"
#include "parser.h"

using v8::FunctionTemplate;

void Init(v8::Local<v8::Object> exports) {
  Stringifier::Init(exports);
  Parser::Init(exports);
}

NODE_MODULE(wson_addon, Init)


