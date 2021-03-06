/*
* Copyright (C) 2011 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include "TypeFactory.h"
#include "VarType.h"
#include <string>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include "strUtils.h"


TypeFactory * TypeFactory::m_instance = NULL;

static Var0 g_var0;
static Var8 g_var8;
static Var16 g_var16;
static Var32 g_var32;

typedef std::map<std::string, VarType> TypeMap;
static  TypeMap g_varMap;
static bool g_initialized = false;
static int g_typeId = 0;


static VarConverter * getVarConverter(int size)
{
    VarConverter *v = NULL;

    switch(size) {
    case 0: v =  &g_var0; break;
    case 8: v =  &g_var8; break;
    case 16:    v =  &g_var16; break;
    case 32:    v =  &g_var32; break;
    }
    return v;
}

#define ADD_TYPE(name, size, printformat)                                           \
    g_varMap.insert(std::pair<std::string, VarType>(name, VarType(g_typeId++, name, &g_var##size,printformat)));

void TypeFactory::initBaseTypes()
{
    g_initialized = true;
    ADD_TYPE("UNKNOWN", 0, "0x%x");
    ADD_TYPE("void", 0, "0x%x");
    ADD_TYPE("char", 8, "%c");
    ADD_TYPE("int", 32, "%d");
    ADD_TYPE("float", 32, "%d");
    ADD_TYPE("short", 16, "%d");
}

int TypeFactory::initFromFile(const std::string &filename)
{
    if (!g_initialized) {
        initBaseTypes();
    }

    FILE *fp = fopen(filename.c_str(), "rt");
    if (fp == NULL) {
        perror(filename.c_str());
        return -1;
    }
    char line[1000];
    int lc = 0;
    while(fgets(line, sizeof(line), fp) != NULL) {
        lc++;
        std::string str = trim(line);
        if (str.size() == 0 || str.at(0) == '#') {
            continue;
        }
        size_t pos = 0, last;
        std::string name;
        name = getNextToken(str, pos, &last, WHITESPACE);
        if (name.size() == 0) {
            fprintf(stderr, "Error: %d : missing type name\n", lc);
            return -2;
        }
        pos = last + 1;
        std::string size;
        size = getNextToken(str, pos, &last, WHITESPACE);
        if (size.size() == 0) {
            fprintf(stderr, "Error: %d : missing type width\n", lc);
            return -2;
        }
        pos = last + 1;
        std::string printString;
        printString = getNextToken(str, pos, &last, WHITESPACE);
        if (printString.size() == 0) {
            fprintf(stderr, "Error: %d : missing print-string\n", lc);
            return -2;
        }

        VarConverter *v = getVarConverter(atoi(size.c_str()));
        if (v == NULL) {
            fprintf(stderr, "Error: %d : unknown var width: %d\n", lc, atoi(size.c_str()));
            return -1;
        }

        if (getVarTypeByName(name)->id() != 0) {
            fprintf(stderr,
                    "Warining: %d : type %s is already known, definition in line %d is taken\n",
                    lc, name.c_str(), lc);
        }
        g_varMap.insert(std::pair<std::string, VarType>(name, VarType(g_typeId++, name, v ,printString)));
    }
    g_initialized = true;
    return 0;
}


const VarType * TypeFactory::getVarTypeByName(const std::string & type)
{
    if (!g_initialized) {
        initBaseTypes();
    }
    TypeMap::iterator i = g_varMap.find(type);
    if (i == g_varMap.end()) {
        i = g_varMap.find("UNKNOWN");
    }
    return &(i->second);
}

