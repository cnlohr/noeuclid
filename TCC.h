#ifndef TCC_H
#define	TCC_H
#include <string>
#include <libtcc.h>
#include <vector>
#include <stdexcept>

using string=std::string;
struct TCCSymbol { string name; void* func;};
class TCC {
public:
    template<typename T>
    void add(string name, T* func) {
        //TODO memory leak
        if(!tcc) tcc = tcc_new();
        symbols.push_back({name, (void*) func});
    }
    
    void addheader(string header) {
        headers += header+"\n";
    }

    template<typename T> T* compile(string code, string symbol) {
        if(!tcc) tcc = tcc_new();
        tcc_define_symbol(tcc, "IS_TCC_RUNTIME", nullptr);
        for(TCCSymbol& s:symbols) tcc_add_symbol(tcc, s.name.c_str(), s.func);
        code = headers + code;
        int state = tcc_compile_string(tcc, code.c_str());
        if(state == -1) {
            throw std::invalid_argument("Error compiling code (("+code+"))");
        }
        int size = tcc_relocate(tcc, TCC_RELOCATE_AUTO);
        if(size == -1) {
            throw std::invalid_argument("Error compiling code 2 (("+code+"))");
        }
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

