#pragma once

// All imports here
#include <filesystem>
#include <fstream>
#include <cstring>

// Vivi forward decl
struct Vivi {
    const char* version = "0.1";
    void processFlags(int argc, char* argv[], std::string& outScriptPath);
    void sourceExec(const std::string& source);
    void run(int argc, char* argv[]);
};
inline Vivi vivi;

// Include Vivi's headers
#include "viviToken.h"
#include <viviMemory.c>
#include <viviLexer.c>
#include "viviAst.h"
#include <viviAst.c>
#include "viviParser.h"
#include <viviParser.c>
#include <viviTooling.c>
#include <viviRuntime.c>