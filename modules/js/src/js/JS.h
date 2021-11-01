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

    int debugger_read(void* buffer, size_t length);
    int debugger_write(const void* buffer, size_t length);
    int debugger_peek();
    int debugger_poll();

private:
    struct JSPimpl;
    std::unique_ptr<JSPimpl> pimpl;
};

#endif
