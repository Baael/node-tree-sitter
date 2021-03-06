#include "./ast_node_array.h"
#include <nan.h>
#include <tree_sitter/runtime.h>
#include <v8.h>
#include <vector>
#include <string>
#include "./ast_node.h"

namespace node_tree_sitter {

using namespace v8;

Nan::Persistent<Function> ASTNodeArray::constructor;

void ASTNodeArray::Init(Local<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("ASTNodeArray").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetIndexedPropertyHandler(
    tpl->InstanceTemplate(),
    GetIndex,
    NULL);

  Nan::SetAccessor(
    tpl->InstanceTemplate(),
    Nan::New("length").ToLocalChecked(),
    Length, NULL);

  const char * array_methods[] = {
    "every",
    "filter",
    "forEach",
    "indexOf",
    "map",
    "reduce",
    "reduceRight",
    "some",
  };

  Local<Array> array = Nan::New<Array>();
  for (size_t i = 0; i < (sizeof(array_methods) / sizeof(array_methods[0])); i++)
    tpl->PrototypeTemplate()->Set(
      Nan::New(array_methods[i]).ToLocalChecked(),
      array->Get(Nan::New(array_methods[i]).ToLocalChecked()));

  constructor.Reset(Nan::Persistent<Function>(tpl->GetFunction()));
}

ASTNodeArray::ASTNodeArray(TSNode node, TSDocument *document, size_t parse_count, bool is_named) :
  parent_node_(node), document_(document), parse_count_(parse_count), is_named_(is_named)  {}

Local<Value> ASTNodeArray::NewInstance(TSNode node, TSDocument *document, size_t parse_count, bool is_named) {
  Local<Object> instance = Nan::New(constructor)->NewInstance(0, NULL);
  (new ASTNodeArray(node, document, parse_count, is_named))->Wrap(instance);
  return instance;
}

NAN_METHOD(ASTNodeArray::New) {
  info.GetReturnValue().Set(Nan::Null());
}

NAN_INDEX_GETTER(ASTNodeArray::GetIndex) {
  ASTNodeArray *array = ObjectWrap::Unwrap<ASTNodeArray>(info.This());
  TSNode child = array->is_named_ ?
    ts_node_named_child(array->parent_node_, index) :
    ts_node_child(array->parent_node_, index);
  if (child.data)
    info.GetReturnValue().Set(ASTNode::NewInstance(child, array->document_, array->parse_count_));
  else
    info.GetReturnValue().Set(Nan::Undefined());
}

NAN_GETTER(ASTNodeArray::Length) {
  ASTNodeArray *array = ObjectWrap::Unwrap<ASTNodeArray>(info.This());
  size_t length = array->is_named_ ?
    ts_node_named_child_count(array->parent_node_) :
    ts_node_child_count(array->parent_node_);
  info.GetReturnValue().Set(Nan::New<Number>(length));
}

}  // namespace node_tree_sitter
