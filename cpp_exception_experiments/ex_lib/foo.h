#pragma once

#include <map>
#include <string>
#include <functional>
#include <typeinfo>
#include <typeindex>


typedef std::function<void()> f_t;

void lib_try_catch(f_t);


struct type_info_t
{
    std::string local_name;
    std::string mangled_name;
    size_t type_info_hash;
    size_t size;

    f_t throwing_f;
};

#define TYPE_MAP_ENTRY(_S_) {#_S_, {#_S_, typeid(_S_).name(), typeid(_S_).hash_code(), sizeof(_S_), throw_ ## _S_}}

const std::map<std::string, type_info_t>& get_lib_types();

