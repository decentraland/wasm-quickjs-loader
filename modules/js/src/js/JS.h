#ifndef JS_H
#define JS_H

#include <string>

class IIOModule;

class JS
{
public:
    JS(IIOModule *ioModule);
    ~JS();

    void loop(double dt);
    void eval(const char *buf, size_t bufSize, std::string file = "index.js");

private:
    struct JSPimpl;
    std::unique_ptr<JSPimpl> pimpl;
};

#endif
