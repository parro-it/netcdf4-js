#include "Attribute.h"
#include <inttypes.h>
#include <netcdf.h>
#include <iostream>
#include "netcdf4js.h"

namespace netcdf4js {

v8::Persistent<v8::Function> Attribute::constructor;

Attribute::Attribute(const char* name_, int var_id_, int parent_id_) : name(name_), var_id(var_id_), parent_id(parent_id_) {
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Object> obj = v8::Local<v8::Function>::New(isolate, constructor)->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
    Wrap(obj);
    int retval = nc_inq_atttype(parent_id, var_id_, name_, &type);
    if (retval != NC_NOERR) {
        throw_netcdf_error(isolate, retval);
    }
}

Attribute::Attribute(const char* name_, int var_id_, int parent_id_, int type_) : name(name_), var_id(var_id_), parent_id(parent_id_), type(type_) {
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Object> obj = v8::Local<v8::Function>::New(isolate, constructor)->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
    Wrap(obj);
}

void Attribute::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* isolate = exports->GetIsolate();
    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(isolate);
    tpl->SetClassName(v8::String::NewFromUtf8(isolate, "Attribute"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    NODE_SET_PROTOTYPE_METHOD(tpl, "delete", Attribute::Delete);
    NODE_SET_PROTOTYPE_METHOD(tpl, "inspect", Attribute::Inspect);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "name"), Attribute::GetName, Attribute::SetName);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "value"), Attribute::GetValue, Attribute::SetValue);
    constructor.Reset(isolate, tpl->GetFunction());
}

void Attribute::GetName(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
    v8::Isolate* isolate = info.GetIsolate();
    Attribute* obj = node::ObjectWrap::Unwrap<Attribute>(info.Holder());
    info.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, obj->name.c_str()));
}

void Attribute::SetName(v8::Local<v8::String> property, v8::Local<v8::Value> val, const v8::PropertyCallbackInfo<void>& info) {
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    Attribute* obj = node::ObjectWrap::Unwrap<Attribute>(info.Holder());
    v8::String::Utf8Value new_name_(
#if NODE_MAJOR_VERSION >= 8
        isolate,
#endif
        val->ToString());
    int retval = nc_rename_att(obj->parent_id, obj->var_id, obj->name.c_str(), *new_name_);
    if (retval != NC_NOERR) {
        throw_netcdf_error(isolate, retval);
        return;
    }
    obj->name = *new_name_;
}

void Attribute::GetValue(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
    v8::Isolate* isolate = info.GetIsolate();
    Attribute* obj = node::ObjectWrap::Unwrap<Attribute>(info.Holder());

    if ((obj->type < NC_BYTE || obj->type > NC_UINT64) && obj->type != NC_STRING) {
        isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Variable type not supported yet")));
        return;
    }

    size_t len;
    int retval = nc_inq_attlen(obj->parent_id, obj->var_id, obj->name.c_str(), &len);
    if (retval != NC_NOERR) {
        throw_netcdf_error(isolate, retval);
        return;
    }

    switch (obj->type) {
        case NC_BYTE: {
            int8_t* v = new int8_t[len];
            retval = nc_get_att(obj->parent_id, obj->var_id, obj->name.c_str(), v);
            if (len == 1) {
                info.GetReturnValue().Set(v8::Integer::New(isolate, v[0]));
            } else {
                info.GetReturnValue().Set(v8::Int8Array::New(v8::ArrayBuffer::New(isolate, v, len * 1), 0, len));
            }
            delete[] v;
        } break;
        case NC_SHORT: {
            int16_t* v = new int16_t[len];
            retval = nc_get_att(obj->parent_id, obj->var_id, obj->name.c_str(), v);
            if (len == 1) {
                info.GetReturnValue().Set(v8::Integer::New(isolate, v[0]));
            } else {
                info.GetReturnValue().Set(v8::Int16Array::New(v8::ArrayBuffer::New(isolate, v, len * 2), 0, len));
            }
            delete[] v;
        } break;
        case NC_INT: {
            int32_t* v = new int32_t[len];
            retval = nc_get_att(obj->parent_id, obj->var_id, obj->name.c_str(), v);
            if (len == 1) {
                info.GetReturnValue().Set(v8::Integer::New(isolate, v[0]));
            } else {
                info.GetReturnValue().Set(v8::Int32Array::New(v8::ArrayBuffer::New(isolate, v, len * 4), 0, len));
            }
            delete[] v;
        } break;
        case NC_FLOAT: {
            float* v = new float[len];
            retval = nc_get_att(obj->parent_id, obj->var_id, obj->name.c_str(), v);
            if (len == 1) {
                info.GetReturnValue().Set(v8::Number::New(isolate, v[0]));
            } else {
                info.GetReturnValue().Set(v8::Float32Array::New(v8::ArrayBuffer::New(isolate, v, len * 4), 0, len));
            }
            delete[] v;
        } break;
        case NC_DOUBLE: {
            double* v = new double[len];
            retval = nc_get_att(obj->parent_id, obj->var_id, obj->name.c_str(), v);
            if (len == 1) {
                info.GetReturnValue().Set(v8::Number::New(isolate, v[0]));
            } else {
                info.GetReturnValue().Set(v8::Float64Array::New(v8::ArrayBuffer::New(isolate, v, len * 8), 0, len));
            }
            delete[] v;
        } break;
        case NC_UBYTE: {
            uint8_t* v = new uint8_t[len];
            retval = nc_get_att(obj->parent_id, obj->var_id, obj->name.c_str(), v);
            if (len == 1) {
                info.GetReturnValue().Set(v8::Integer::New(isolate, v[0]));
            } else {
                info.GetReturnValue().Set(v8::Uint8Array::New(v8::ArrayBuffer::New(isolate, v, len * 1), 0, len));
            }
            delete[] v;
        } break;
        case NC_USHORT: {
            uint16_t* v = new uint16_t[len];
            retval = nc_get_att(obj->parent_id, obj->var_id, obj->name.c_str(), v);
            if (len == 1) {
                info.GetReturnValue().Set(v8::Integer::New(isolate, v[0]));
            } else {
                info.GetReturnValue().Set(v8::Uint16Array::New(v8::ArrayBuffer::New(isolate, v, len * 2), 0, len));
            }
            delete[] v;
        } break;
        case NC_UINT: {
            uint32_t* v = new uint32_t[len];
            retval = nc_get_att(obj->parent_id, obj->var_id, obj->name.c_str(), v);
            if (len == 1) {
                info.GetReturnValue().Set(v8::Integer::NewFromUnsigned(isolate, v[0]));
            } else {
                info.GetReturnValue().Set(v8::Uint32Array::New(v8::ArrayBuffer::New(isolate, v, len * 4), 0, len));
            }
            delete[] v;
        } break;
        case NC_INT64: {
            int64_t* v = new int64_t[len];
            retval = nc_get_att(obj->parent_id, obj->var_id, obj->name.c_str(), v);
            if (len == 1) {
                info.GetReturnValue().Set(v8::Integer::New(isolate, static_cast<int32_t>(v[0])));
            } else {
                info.GetReturnValue().Set(v8::Int32Array::New(v8::ArrayBuffer::New(isolate, v, len * 8), 0, len));
            }
            delete[] v;
        } break;
        case NC_UINT64: {
            uint64_t* v = new uint64_t[len];
            retval = nc_get_att(obj->parent_id, obj->var_id, obj->name.c_str(), v);
            if (len == 1) {
                info.GetReturnValue().Set(v8::Integer::NewFromUnsigned(isolate, static_cast<uint32_t>(v[0])));
            } else {
                info.GetReturnValue().Set(v8::Uint32Array::New(v8::ArrayBuffer::New(isolate, v, len * 8), 0, len));
            }
            delete[] v;
        } break;
        case NC_CHAR:
        case NC_STRING: {
            char* v = new char[len + 1];
            v[len] = 0;
            retval = nc_get_att_text(obj->parent_id, obj->var_id, obj->name.c_str(), v);
            info.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, v));
            delete[] v;
        } break;
    }
    if (retval != NC_NOERR) {
        throw_netcdf_error(isolate, retval);
    }
}

void Attribute::SetValue(v8::Local<v8::String> property, v8::Local<v8::Value> val, const v8::PropertyCallbackInfo<void>& info) {
    Attribute* obj = node::ObjectWrap::Unwrap<Attribute>(info.Holder());
    obj->set_value(val);
}

void Attribute::set_value(const v8::Local<v8::Value>& val) {
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    if ((type < NC_BYTE || type > NC_UINT) && type != NC_STRING) {
        isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Variable type not supported yet")));
        return;
    }

    int retval;
    if (val->IsUint32()) {
        uint32_t v = val->Uint32Value();
        retval = nc_put_att(parent_id, var_id, name.c_str(), NC_UINT, 1, &v);
    } else if (val->IsInt32()) {
        int32_t v = val->Int32Value();
        retval = nc_put_att(parent_id, var_id, name.c_str(), NC_INT, 1, &v);
    } else if (val->IsNumber()) {
        double v = val->NumberValue();
        retval = nc_put_att(parent_id, var_id, name.c_str(), NC_DOUBLE, 1, &v);
    } else {
        std::string v(*v8::String::Utf8Value(
#if NODE_MAJOR_VERSION >= 8
            isolate,
#endif
            val->ToString()));
        retval = nc_put_att_text(parent_id, var_id, name.c_str(), v.length(), v.c_str());
    }
    if (retval != NC_NOERR) {
        throw_netcdf_error(isolate, retval);
    }
}

void Attribute::Delete(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Attribute* obj = node::ObjectWrap::Unwrap<Attribute>(args.Holder());
    int retval = nc_del_att(obj->parent_id, obj->var_id, obj->name.c_str());
    if (retval != NC_NOERR) {
        throw_netcdf_error(args.GetIsolate(), retval);
    }
}

void Attribute::Inspect(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, "[object Attribute]"));
}
}  // namespace netcdf4js
