#include "stringifier.h"
#include "parser.h"

using v8::Handle;
using v8::Object;
using v8::FunctionTemplate;

void Init(Handle<Object> exports) {
  Stringifier::Init(exports);
  Parser::Init(exports);
}

NODE_MODULE(tson_addon, Init)


