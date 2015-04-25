#include "parser_source.h"
#include "parser.h"

v8::Local<v8::String> ParserSource::getText() {
 int err = source.pullUnescapedBuffer();
 if (err) {
   makeError();
   return NanNew("");
 }  
 return source.nextBuffer.getHandle();
}  

v8::Local<v8::Value> ParserSource::getLiteral() {
  int err = 0;
  v8::Local<v8::Value> value;
  if (hasError) return value;
  if (source.nextType == TEXT) {
    size_t litBeginIdx = source.nextIdx - 1;
    switch (source.nextChar) {
      case 'u':
        next();
        value = NanUndefined();
        break;
      case 'n':
        next();
        value = NanNull();
        break;
      case 'f':
        next();
        value = NanFalse();
        break;
      case 't':
        next();
        value = NanTrue();
        break;
      default: {
        err = source.pullUnescapedString();
        if (err) {
          makeError();
          break;
        }
        const char* begin = source.nextString.data();
        char* end;
        int x = strtol(begin, &end, 10);
        if (end == begin + source.nextString.size()) {
          value = NanNew<v8::Number>(x);
          break;
        } else {
          double x = strtod(begin, &end);
          if (end == begin + source.nextString.size()) {
            value = NanNew<v8::Number>(x);
            break;
          }
        }
        {
          TargetBuffer msg;
          msg.append(std::string("unexpected literal '"));
          msg.append(source.nextString);
          msg.append(std::string("'"));
          makeError(litBeginIdx, &msg);
        }  
      }
    }
  } else {
    value = NanNew<v8::String>();
  }
  return value;
}

    /*
      void makeError(TargetBuffer& msg) {
      size_t errIdx = nextIdx;
      uint16_t errChar = nextChar;
      if (nextType == END) {
        errChar = 0;
      } else {
        --errIdx;
      }
      msg.append(std::string("Unexpected '"));
      if (errChar) {
        msg.push(nextChar);
      }
      msg.append(std::string("' at '"));
      msg.append(getBuffer(), 0, errIdx);
      msg.push('^');
      msg.append(getBuffer(), errIdx);
      msg.append(std::string("'"));
    }
    */


v8::Local<v8::Array> ParserSource::getArray() {
  v8::Local<v8::Array> value = NanNew<v8::Array>();
  if (hasError) goto end;
  switch (source.nextType) {
    case ENDARRAY:
      next();
      break;
    default:
      goto stageNext;
  }
  goto end;

stageNext:
  switch (source.nextType) {
    case TEXT:
    case QUOTE:
      value->Set(value->Length(), getText());
      goto stageHave;
    case LITERAL:
      next();
      value->Set(value->Length(), getLiteral());
      if (hasError) goto end;
      goto stageHave;
    case ARRAY:
      next();
      value->Set(value->Length(), getArray());
      if (hasError) goto end;
      goto stageHave;
    case OBJECT:
      next();
      value->Set(value->Length(), getObject());
      if (hasError) goto end;
      goto stageHave;
    default:
      makeError();
  }
  goto end;

stageHave:
  switch (source.nextType) {
    case ENDARRAY:
      next();
      break;
    case PIPE:
      next();
      goto stageNext;
    default:
      makeError();
  }
  goto end;

end:
  return value;
}

v8::Local<v8::Object> ParserSource::getObject() {
  v8::Local<v8::Object> value = NanNew<v8::Object>();
  v8::Local<v8::String> key;
  if (hasError) goto end;

  switch (source.nextType) {
    case ENDOBJECT:
      next();
      break;
    default:
      goto stageNext;
  }
  goto end;

stageNext:
  switch (source.nextType) {
    case TEXT:
    case QUOTE:
      key = getText();
      goto stageHaveKey;
    case LITERAL:
      next();
      key = NanNew<v8::String>();
      goto stageHaveKey;
    default:
      makeError();
  }
  goto end;

stageHaveKey:
  switch (source.nextType) {
    case ENDOBJECT:
      next();
      value->Set(key, NanTrue());
      break;
    case PIPE:
      next();
      value->Set(key, NanTrue());
      goto stageNext;
    case IS:
      next();
      goto stageHaveColon;
    default:
      makeError();
  }
  goto end;

stageHaveColon:
  switch (source.nextType) {
    case TEXT:
    case QUOTE:
      value->Set(key, getText());
      goto stageHaveValue;
    case LITERAL:
      next();
      value->Set(key, getLiteral());
      if (hasError) goto end;
      goto stageHaveValue;
    case ARRAY:
      next();
      value->Set(key, getArray());
      if (hasError) goto end;
      goto stageHaveValue;
    case OBJECT:
      next();
      value->Set(key, getObject());
      if (hasError) goto end;
      goto stageHaveValue;
    default:
      makeError();
  }
  goto end;

stageHaveValue:
  switch (source.nextType) {
    case ENDOBJECT:
      next();
      break;
    case PIPE:
      next();
      goto stageNext;
    default:
      makeError();
  }
  goto end;

end:
  return value;
}

v8::Local<v8::Value> ParserSource::getValue() {
  v8::Local<v8::Value> value;
  if (hasError) return value;
  switch (source.nextType) {
    case TEXT:
    case QUOTE:
      value = getText();
      break;
    case LITERAL:
      next();
      value = getLiteral();
      break;
    case ARRAY:
      next();
      value = getArray();
      break;
    case OBJECT:
      next();
      value = getObject();
      break;
    default:
      makeError();
  }
  if (source.nextType != END) {
    makeError();
  }
  return value;
}

void ParserSource::makeError(int pos, const BaseBuffer* cause) {
  if (pos < 0) {
    pos = source.nextIdx;
    if (pos > 0)
      --pos;
  }
  const int argc = 3;
  v8::Local<v8::String> hCause;
  if (cause) {
    hCause = cause->getHandle();
  } else {  
    hCause = NanNew("");
  }
  v8::Local<v8::Value> argv[argc] = {
    source.getHandle(), 
    NanNew<v8::Number>(pos),
    hCause
  };
  error = parser_.createError(argc, argv);
  hasError = true;
}

