/* 
 * File:   tcc.h
 * Author: tehdog
 *
 * Created on 22. Oktober 2014, 22:49
 */

#ifndef TCC_H
#define	TCC_H
#include <string>
#include <libtcc.h>
#include <vector>
using string=std::string;
struct TCCSymbol { string name; void* func;};
class TCC {
public:
    template<typename T>
    void addfun(string name, string signature, T* func) {
        //TODO memory leak
        if(!tcc) tcc = tcc_new();
        headers += signature +";\n";
        symbols.push_back({name, (void*) func});
    }

    template<typename T> T* compile(string code, string symbol) {
        if(!tcc) tcc = tcc_new();
        for(TCCSymbol& s:symbols) tcc_add_symbol(tcc, s.name.c_str(), s.func);
        code = headers + code;
        int state = tcc_compile_string(tcc, code.c_str());
        if(state == -1) throw std::invalid_argument("Error compiling code.");
        int size = tcc_relocate(tcc, TCC_RELOCATE_AUTO);
        if(size == -1) throw std::invalid_argument("Error compiling code 2.");
        T* fn = (T*) tcc_get_symbol(tcc, symbol.c_str());
        tcc = 0;
        return fn;
    }
    // returns a pointer to a function called "fun" in the given c code
    template<typename T> T* eval(string code) {
        return compile<T>(code, "fun");
    }
private:
    TCCState* tcc;
    std::string headers;
    std::vector<TCCSymbol> symbols;
};

#endif	/* TCC_H */

