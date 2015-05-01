#ifndef WSON_STINGIFIER_TARGET_H_
#define WSON_STINGIFIER_TARGET_H_

#include "target_buffer.h"
#include <algorithm>
#include <sstream>

class StringifierTarget;

class ObjectAdaptor {
  public:
    inline void putObject(v8::Local<v8::Object> obj);
    inline void sort();
    inline void emit(StringifierTarget&);
  private:
    struct Entry {
      size_t keyBeginIdx;
      size_t keyLength;
      v8::Local<v8::Value> value;
    };
    TargetBuffer keyBunch;
    std::vector<Entry> entries;
    std::vector<size_t> entryIdxs;
    friend struct OaLess;
};

class Stringifier;

class StringifierTarget {
  public:
    friend class Stringifier;

    StringifierTarget(Stringifier& stringifier): stringifier_(stringifier), oaIdx_(0) {}
    inline void putText(v8::Local<v8::String>);
    inline void putText(const usc2vector& buffer, size_t start, size_t length);
    inline bool putBackref(v8::Local<v8::Object> x);
    inline void putValue(v8::Local<v8::Value>);

    inline void clear() {
      target.clear();
      haves.clear();
      oaIdx_ = 0;
    };
    void put(v8::Local<v8::Value>);

    static void Init();

    typedef std::vector<v8::Local<v8::Value> > handleVector;

    TargetBuffer target;
    handleVector haves;

  private:
    Stringifier& stringifier_;
    enum {
      STATIC_OA_NUM = 8
    };
    ObjectAdaptor oas_[STATIC_OA_NUM];
    size_t oaIdx_;

    inline ObjectAdaptor* getOa() {
      if (oaIdx_ < STATIC_OA_NUM) {
        return &oas_[oaIdx_++];
      } else {
        oaIdx_++;
        return new ObjectAdaptor();
      }
    }

    inline void releaseOa(ObjectAdaptor* oa) {
      if (oaIdx_ > STATIC_OA_NUM) {
        delete oa;
      }
      --oaIdx_;
    }
};


#endif // WSON_STINGIFIER_TARGET_H_

