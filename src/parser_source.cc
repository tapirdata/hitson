#include "parser_source.h"
#include "parser.h"
#include <cstdlib>



inline bool getNumber(const std::string& s, v8::Local<v8::Value>& value) {
  const char* begin = s.data();
  char* end;
  int x = strtol(begin, &end, 10);
  if (end == begin + s.size()) {
    value = NanNew<v8::Number>(x);
    return true;
  } else {
    double x = strtod(begin, &end);
    if (end == begin + s.size()) {
      value = NanNew<v8::Number>(x);
      return true;
    }
  }
  return false;
}

inline bool getDate(const std::string& s, v8::Local<v8::Value>& value) {
  // std::cout << "getDate: " << s << std::endl;
  const char* begin = s.data();
  char* end;
  double x = strtod(begin + 1, &end);
  if (end == begin + s.size()) {
    value = NanNew<v8::Date>(x);
    return true;
  }
  return false;
}

inline bool getInteger(const std::string& s, int& value) {
  const char* begin = s.data();
  char* end;
  int x = strtol(begin, &end, 10);
  if (end == begin + s.size()) {
    value = x;
    return true;
  }  
  return false;
}

v8::Local<v8::String> ParserSource::getText() {
 int err = source.pullUnescapedBuffer();
 if (err) {
   makeError();
   return NanNew("");
 }  
 return source.nextBuffer.getHandle();
}  

v8::Local<v8::Value> ParserSource::getLiteral() {
  v8::Local<v8::Value> value;
  if (hasError) return value;
  if (source.nextType == TEXT) {
    bool litErr = false;
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
      case 'd':
        if (source.pullUnescapedString()) {
          makeError();
          break;
        }
        if (!getDate(source.nextString, value)) {
          litErr = true;
        }  
        break;
      default: {
        if (source.pullUnescapedString()) {
          makeError();
          break;
        }
        if (!getNumber(source.nextString, value)) {
          litErr = true;
        }  
      }
    }
    if (litErr) {
      TargetBuffer msg;
      msg.append(std::string("unexpected literal '"));
      msg.append(source.nextString);
      msg.append(std::string("'"));
      makeError(litBeginIdx, &msg);
    }
  } else {
    value = NanNew<v8::String>();
  }
  return value;
}

v8::Local<v8::Object> ParserSource::getBackreffed(ParseFrame& frame) {
  v8::Local<v8::Object> value = NanNew<v8::Object>();
  bool refErr = false;
  size_t refBeginIdx = source.nextIdx - 1;
  if (source.nextType != TEXT) {
    refErr = true;
  } else { 
    if (source.pullUnescapedString()) {
      makeError();
    } else {
      int refIdx;
      if (!getInteger(source.nextString, refIdx) || refIdx < 0) {
        refErr = true;
      } else {
        ParseFrame *idxFrame = &frame;
        while (refIdx > 0) {
          idxFrame = idxFrame->parent;
          if (!idxFrame) {
            refErr = true;
            break;
          }
          --refIdx;
        }
        if (idxFrame) {
          value = idxFrame->value;
        }  
      }
    }
  }  
  if (refErr) {
    TargetBuffer msg;
    msg.append(std::string("unexpected backref '"));
    msg.append(source.nextString);
    msg.append(std::string("'"));
    makeError(refBeginIdx, &msg);
  }
  return value;
}  

v8::Local<v8::Object> ParserSource::getArray(ParseFrame* parentFrame) {
  if (source.nextType == IS) {
    return getCustom(parentFrame);
  }

  v8::Local<v8::Array> value = NanNew<v8::Array>();
  ParseFrame frame(value, parentFrame);
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
      value->Set(value->Length(), getArray(&frame));
      if (hasError) goto end;
      goto stageHave;
    case OBJECT:
      next();
      value->Set(value->Length(), getObject(&frame));
      if (hasError) goto end;
      goto stageHave;
    case PIPE:
      next();
      value->Set(value->Length(), getBackreffed(frame));
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

v8::Local<v8::Object> ParserSource::getObject(ParseFrame* parentFrame) {
  ParseFrame frame(NanNew<v8::Object>(), parentFrame);
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
      frame.value->Set(key, NanTrue());
      break;
    case PIPE:
      next();
      frame.value->Set(key, NanTrue());
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
      frame.value->Set(key, getText());
      goto stageHaveValue;
    case LITERAL:
      next();
      frame.value->Set(key, getLiteral());
      if (hasError) goto end;
      goto stageHaveValue;
    case ARRAY:
      next();
      frame.value->Set(key, getArray(&frame));
      if (hasError) goto end;
      goto stageHaveValue;
    case OBJECT:
      next();
      frame.value->Set(key, getObject(&frame));
      if (hasError) goto end;
      goto stageHaveValue;
    case PIPE:
      next();
      frame.value->Set(key, getBackreffed(frame));
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
  return frame.value;
}

v8::Local<v8::Object> ParserSource::getCustom(ParseFrame* parentFrame) {
  ParseFrame frame(NanNew<v8::Object>(), parentFrame);
  v8::Local<v8::Array> args = NanNew<v8::Array>();
  if (hasError) goto end;
  switch (source.nextType) {
    case TEXT:
      next();
      goto stageHave;
    default:
      makeError();
  }
  goto end;

stageNext:
  switch (source.nextType) {
    case TEXT:
    case QUOTE:
      args->Set(args->Length(), getText());
      goto stageHave;
    case LITERAL:
      next();
      args->Set(args->Length(), getLiteral());
      if (hasError) goto end;
      goto stageHave;
    case ARRAY:
      next();
      args->Set(args->Length(), getArray(&frame));
      if (hasError) goto end;
      goto stageHave;
    case OBJECT:
      next();
      args->Set(args->Length(), getObject(&frame));
      if (hasError) goto end;
      goto stageHave;
    case PIPE:
      next();
      args->Set(args->Length(), getBackreffed(frame));
      if (hasError) goto end;
      goto stageHave;
    default:
      makeError();
  }

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
  return frame.value;
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
      value = getArray(NULL);
      break;
    case OBJECT:
      next();
      value = getObject(NULL);
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

