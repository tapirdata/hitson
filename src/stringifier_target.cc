
#include "stringifier_target.h"
#include "stringifier.h"

void StringifierTarget::Init() {
}

void StringifierTarget::putText(v8::Local<v8::String> s) {
  if (s->Length() == 0) {
    target.push('#');
  } else {
    target.appendHandleEscaped(s);
  }
}
void StringifierTarget::putText(const usc2vector& buffer, size_t start, size_t length) {
  if (length == 0) {
    target.push('#');
  } else {
    target.appendEscaped(buffer, start, length);
  }
}

bool StringifierTarget::putBackref(v8::Local<v8::Object> x) {
  handleVector::const_iterator haveIt = std::find(haves.begin(), haves.end(), x);
  size_t idx;
  if (haveIt != haves.end()) {
    idx = haves.end() - haveIt - 1;
  } else if (haverefCb) {
    v8::Local<v8::Value> cbArgv[] = {
      x
    };
    v8::Local<v8::Value> haveIdx = haverefCb->Call(1, cbArgv);
    if (!haveIdx->IsUint32()) {
      return false;
    }
    idx = haves.size() + Nan::To<uint32_t>(haveIdx).ToChecked();
  } else {
    return false;
  }
  std::stringstream idxBuf;
  idxBuf << idx;
  target.push('|');
  target.append(idxBuf.str());
  return true;
}

void StringifierTarget::putValue(v8::Local<v8::Value> x) {
  int ti = Stringifier::getTypeid(x);
  switch (ti) {
    case TI_UNDEFINED:
      target.push('#');
      target.push('u');
      break;
    case TI_NULL:
      target.push('#');
      target.push('n');
      break;
    case TI_BOOLEAN:
      target.push('#');
      if (Nan::To<bool>(x).ToChecked())
        target.push('t');
      else
        target.push('f');
      break;
    case TI_NUMBER:
      target.push('#');
      target.appendHandle(Nan::To<v8::String>(x).ToLocalChecked());
      break;
    case TI_DATE:
      target.push('#');
      target.push('d');
      target.appendHandle(Nan::To<v8::String>(Nan::To<v8::Number>(x).ToLocalChecked()).ToLocalChecked());
      break;
    case TI_STRING:
      putText(x.As<v8::String>());
      break;
    case TI_ARRAY: {
      if (putBackref(x.As<v8::Object>())) {
        return;
      }
      haves.push_back(x);
      v8::Local<v8::Array> array = x.As<v8::Array>();
      uint32_t len = array->Length();
      target.push('[');
      for (uint32_t i=0; i<len; ++i) {
        putValue(array->Get(Nan::GetCurrentContext(), i).ToLocalChecked());
        if (i + 1 != len) {
          target.push('|');
        }
      }
      target.push(']');
      haves.pop_back();
      break;
    }
    case TI_OBJECT: {
      v8::Local<v8::Object> xObj = x.As<v8::Object>();
      if (putBackref(xObj)) {
        return;
      }
      haves.push_back(x);

      const Stringifier::StringifyConnector* connector = stringifier_.findConnector(xObj);
      if (connector) {
        target.push('[');
        target.push(':');
        target.append(connector->name.getBuffer());
        const int argc = 1;
        v8::Local<v8::Value> argv[argc] = {x};
        v8::Local<v8::Function> split = Nan::New<v8::Function>(connector->split);
        if (!split.IsEmpty()) {
          v8::Local<v8::Value> args = split->Call(Nan::GetCurrentContext(), Nan::New<v8::Object>(connector->self), argc, argv).ToLocalChecked();
          if (!args.IsEmpty() && args->IsArray()) {
            v8::Local<v8::Array> argsArray = args.As<v8::Array>();
            uint32_t len = argsArray->Length();
            for (uint32_t i=0; i<len; ++i) {
              target.push('|');
              putValue(argsArray->Get(Nan::GetCurrentContext(), i).ToLocalChecked());
            }
          }
        }
        target.push(']');
      } else {
        ObjectAdaptor *oa = getOa();
        oa->putObject(xObj);
        oa->sort();
        oa->emit(*this);
        releaseOa(oa);
      }
      haves.pop_back();
      break;
    }
  }
}

void StringifierTarget::put(v8::Local<v8::Value> x) {
  putValue(x);
}

void ObjectAdaptor::putObject(v8::Local<v8::Object> obj) {
  v8::Local<v8::Array> keys = obj->GetOwnPropertyNames(Nan::GetCurrentContext()).ToLocalChecked();
  uint32_t len = keys->Length();
  entries.resize(len);
  entryIdxs.resize(len);
  keyBunch.clear();
  for (uint32_t i=0; i<len; ++i) {
    entryIdxs[i] = i;
    Entry& entry = entries[i];
    v8::Local<v8::Value> key = keys->Get(Nan::GetCurrentContext(), i).ToLocalChecked();
    v8::Local<v8::String> skey = key->IsString() ? key.As<v8::String>() : Nan::To<v8::String>(key).ToLocalChecked();
    entry.keyBeginIdx = keyBunch.size();
    entry.keyLength = skey->Length();
    entry.value = obj->Get(Nan::GetCurrentContext(), key).ToLocalChecked();
    keyBunch.appendHandle(skey);
  }
}

struct OaLess {
  OaLess(const ObjectAdaptor& oa): oa_(oa) {}
  const ObjectAdaptor& oa_;
  bool operator()(size_t idxA, size_t idxB) {
    const uint16_t* keyData = oa_.keyBunch.getBuffer().data();
    const ObjectAdaptor::Entry& entryA = oa_.entries[idxA];
    const ObjectAdaptor::Entry& entryB = oa_.entries[idxB];
    const uint16_t* itA = keyData + entryA.keyBeginIdx;
    const uint16_t* itB = keyData + entryB.keyBeginIdx;
    const uint16_t* endA = itA + entryA.keyLength;
    const uint16_t* endB = itB + entryB.keyLength;
    while (itA != endA) {
      if (itB == endB) {  // B ends -> extra A-tail
        return false;
      }
      uint16_t cA = *itA++;
      uint16_t cB = *itB++;
      if (cA < cB) {
        return true;
      } else if (cA > cB) {
        return false;
      }
    }
    // A ends;
    if (itB == endB) {
      // equal
      return false;
    } else {
      // extra B-tail
      return true;
    }
  }
};


void ObjectAdaptor::sort() {
  if (entryIdxs.size() > 1) {
    OaLess oaLess(*this);
    std::sort(entryIdxs.begin(), entryIdxs.end(), oaLess);
    // std::swap(entryIdxs[0], entryIdxs[1]);
  }
}

void ObjectAdaptor::emit(StringifierTarget& st) {
  // const uint16_t* keyData = keyBunch.getBuffer().data();
  const usc2vector& keyBuffer = keyBunch.getBuffer();
  st.target.push('{');
  uint32_t len = entries.size();
  for (uint32_t i=0; i<len; ++i) {
    size_t entryIdx = entryIdxs[i];
    Entry& entry = entries[entryIdx];
    st.putText(keyBuffer, entry.keyBeginIdx, entry.keyLength);
    if (!entry.value->IsBoolean() || !Nan::To<bool>(entry.value).ToChecked()) {
      st.target.push(':');
      st.putValue(entry.value);
    }
    if (i + 1 != len) {
      st.target.push('|');
    }
  }
  st.target.push('}');
}


