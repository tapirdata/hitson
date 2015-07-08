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
   return NanNew(Parser::sEmpty);
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
    value = NanNew(Parser::sEmpty);
  }
  return value;
}

v8::Handle<v8::Object> ParserSource::getBackreffed(ParseFrame* frame) {
  v8::Handle<v8::Object> value = NanNew<v8::Object>();
  bool refErr = false;
  size_t refBeginIdx = source.nextIdx - 1;
  Ctype nextType = source.nextType;
  if (nextType != TEXT) {
    refErr = true;
  } else {
    if (source.pullUnescapedString()) {
      makeError();
    } else {
      int refIdx;
      if (!getInteger(source.nextString, refIdx) || refIdx < 0) {
        refErr = true;
      } else {
        if (frame) {
          --refIdx;
        }
        ParseFrame *idxFrame = frame;
        while (refIdx >= 0) {
          ParseFrame* parentIdxFrame = frame ? idxFrame->parent : NULL;
          if (parentIdxFrame) {
            idxFrame = parentIdxFrame;
          } else if (backrefCb) {
            v8::Handle<v8::Value> cbArgv[] = {
              NanNew<v8::Number>(refIdx)
            };
            v8::Handle<v8::Value> brValue = backrefCb->Call(1, cbArgv);
            if (brValue->IsObject()) {
              value = brValue.As<v8::Object>();
            } else {
              refErr = true;
            }
            idxFrame = NULL;
            break;
          } else {
            refErr = true;
            break;
          }
          --refIdx;
        }
        if (idxFrame) {
          if (idxFrame->vetoBackref) {
            refErr = true;
          } else {
            idxFrame->isBackreffed = true;
            value = idxFrame->value;
          }
        }
      }
    }
  }
  if (refErr) {
    // std::cout << "getBackreffed refErr nextType=" << source.nextType << std::endl;
    TargetBuffer msg;
    msg.append(std::string("unexpected backref '"));
    if (nextType == TEXT) {
      msg.append(source.nextString);
    } else {
      msg.push(source.nextChar);
    }
    msg.append(std::string("'"));
    makeError(refBeginIdx, &msg);
  }
  return value;
}

v8::Local<v8::Object> ParserSource::getArray(ParseFrame* parentFrame) {
  if (source.nextType == IS) {
    next();
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
      value->Set(value->Length(), getBackreffed(&frame));
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
      frame.value->Set(key, getBackreffed(&frame));
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
  // std::cout << "getCustom" << std::endl;
  bool stolenBackref = false;
  ParseFrame frame(NanNew<v8::Object>(), parentFrame);
  v8::Local<v8::Array> args = NanNew<v8::Array>();
  const Parser::ParseConnector* connector(NULL);
  size_t nameIdx = source.nextIdx - 1; // for error
  if (hasError) goto end;
  switch (source.nextType) {
    case TEXT:
    case QUOTE:
      if (source.pullUnescapedBuffer()) {
        makeError();
        break;
      }
      connector = parser_.getConnector(source.nextBuffer.getBuffer());
      if (!connector) {
        TargetBuffer msg;
        msg.append(std::string("no connector for '"));
        msg.append(source.nextBuffer.getBuffer());
        msg.append(std::string("'"));
        makeError(nameIdx, &msg);
        break;
      }
      {
        if (connector->hasCreate) {
          frame.vetoBackref = true;
        } else {
          v8::Local<v8::Function> precreate = NanNew<v8::Function>(connector->precreate);
          const int argc = 0;
          v8::Local<v8::Value> argv[argc] = {};
          frame.value = precreate->Call(NanNew<v8::Object>(connector->self), argc, argv).As<v8::Object>();
          // std::cout << "precreate" << std::endl;
        }
      }
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
      args->Set(args->Length(), getBackreffed(&frame));
      if (hasError) goto end;
      goto stageHave;
    default:
      makeError();
  }

stageHave:
  switch (source.nextType) {
    case ENDARRAY:
      next();
      {
        if (connector->hasCreate) {
          v8::Local<v8::Function> create = NanNew<v8::Function>(connector->create);
          const int argc = 1;
          v8::Local<v8::Value> argv[argc] = {args};
          frame.value = create->Call(NanNew<v8::Object>(connector->self), argc, argv).As<v8::Object>();
          // std::cout << "create" << std::endl;
        } else {
          v8::Local<v8::Function> postcreate = NanNew<v8::Function>(connector->postcreate);
          const int argc = 2;
          v8::Local<v8::Value> argv[argc] = {frame.value, args};
          v8::Local<v8::Value> newValue = postcreate->Call(NanNew<v8::Object>(connector->self), argc, argv);
          if (newValue->IsObject()) {
            if (newValue != frame.value) {
              if (frame.isBackreffed) {
                stolenBackref = true;
                goto end;
              }
              frame.value = newValue.As<v8::Object>();
            }
          }
          // std::cout << "postcreate" << std::endl;
        }
      }
      break;
    case PIPE:
      next();
      goto stageNext;
    default:
      makeError();
  }
  goto end;

end:
  if (stolenBackref) {
    TargetBuffer msg;
    msg.append(std::string("backreffed value is replaced by postcreate"));
    // msg.append(source.nextString);
    // msg.append(std::string("'"));
    // makeError(litBeginIdx, &msg);
    makeError(-1, &msg);
  }
  return frame.value;
}

v8::Local<v8::Value> ParserSource::getValue(bool* isValue) {
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
    case PIPE:
      next();
      value = getBackreffed(NULL);
      break;
    default:
      if (isValue) {
        *isValue = false;
        value = NanNew<v8::String>(&source.nextChar, 1);
        next();
      } else {
        makeError();
      }
  }
  if (!isValue && !hasError && !isEnd()) {
    makeError(); // extra chars after end
  }
  return value;
}

v8::Local<v8::Value> ParserSource::getRawValue(bool* isValue) {
  v8::Local<v8::Value> value;
  switch (source.nextType) {
    case TEXT:
    case QUOTE:
      value = getText();
      break;
    default:
      *isValue = false;
      value = NanNew<v8::String>(&source.nextChar, 1);
      next();
  }
  return value;
}

void ParserSource::makeError(int pos, const BaseBuffer* cause) {
  // std::cout << "makeError hasMsg=" << (cause != NULL) << std::endl;
  if (pos < 0) {
    pos = isEnd() ? source.size() : source.nextIdx - 1;
  }
  const int argc = 3;
  v8::Local<v8::String> hCause;
  if (cause) {
    hCause = cause->getHandle();
  } else {
    hCause = NanNew(Parser::sEmpty);
  }
  v8::Local<v8::Value> argv[argc] = {
    source.getHandle(),
    NanNew<v8::Number>(pos),
    hCause
  };
  error = parser_.createError(argc, argv);
  hasError = true;
}

