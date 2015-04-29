
#include "stringifier_target.h"
#include "stringifier.h"

void StringifierTarget::Init() {
}

const Connector* StringifierTarget::getConnector(v8::Local<v8::Object> x) {
  v8::Local<v8::Value> constructor = x->Get(NanNew("constructor"));
  if (constructor->IsFunction()) {
    // std::cout << "hasConstructor" << std::endl;
    for (connectorVector::const_iterator it=stringifier_.connectors_.begin(); it != stringifier_.connectors_.end(); ++it) {
      if (it->by == constructor) {
        return &(*it);
      }
    }
  }
  return NULL;
}


