#include "base_buffer.h"
#include "target_buffer.h"

class SourceBuffer: public BaseBuffer {

  public:

    static inline Ctype getCtype(uint16_t c) {
      switch (c) {
        case '{':
          return OBJECT;
        case '}':
          return ENDOBJECT;
        case '[':
          return ARRAY;
        case ']':
          return ENDARRAY;
        case ':':
          return IS;
        case '#':
          return LITERAL;
        case '|':
          return PIPE;
        case '`':
          return QUOTE;
      }
      return TEXT;
    }

    SourceBuffer():
      nextIdx(0)
    {}

    inline void next() {
      size_t len = buffer_.size();
      if (nextIdx >= len) {
        nextType = END;
      } else {
        nextChar = buffer_[nextIdx++];
        nextType = SourceBuffer::getCtype(nextChar);
        /*
        if (nextType == QUOTE) {
          if (nextIdx == len) {
            return SYNTAX_ERROR;
          }
          uint16_t c = buffer_[nextIdx++];
          nextChar = getUnescapeChar(c);
          if (!nextChar) {
            return SYNTAX_ERROR;
          }
          nextType = TEXT;
        }
        */
      }
    }

    inline int pullUnescaped(TargetBuffer& target) {
      size_t len = buffer_.size();
      while (true) {
        if (nextType == QUOTE) {
          if (nextIdx == len) {
            return SYNTAX_ERROR;
          }
          nextChar = getUnescapeChar(buffer_[nextIdx++]);
          if (!nextChar) {
            return SYNTAX_ERROR;
          }
        }  
        target.push(nextChar);
        next();
        if (nextType != TEXT && nextType != QUOTE) {
          break;
        }
      }
      return 0;
    }

    inline int pullUnescaped(std::string& target) {
      size_t len = buffer_.size();
      while (true) {
        if (nextType == QUOTE) {
          if (nextIdx == len) {
            return SYNTAX_ERROR;
          }
          nextChar = getUnescapeChar(buffer_[nextIdx++]);
          if (!nextChar) {
            return SYNTAX_ERROR;
          }
        }  
        target.push_back(nextChar);
        next();
        if (nextType != TEXT && nextType != QUOTE) {
          break;
        }
      }
      return 0;
    }

    inline int pullUnescapedBuffer() {
      nextBuffer.clear();
      return pullUnescaped(nextBuffer);
    }

    inline int pullUnescapedString() {
      nextString.clear();
      return pullUnescaped(nextString);
    }

    void clear() {
      BaseBuffer::clear();
      nextIdx = 0;
    }

    void init(v8::Local<v8::String> s) {
      clear();
      appendHandle(s);
      next();
    }  

    size_t nextIdx;
    uint16_t nextChar;
    Ctype nextType;
    TargetBuffer nextBuffer;
    std::string nextString;
};


