#ifndef JS_H
#define JS_H

#include <memory>

class IIOModule;

class JS
{
public:
    JS(IIOModule* ioModule);
    ~JS();
    
    void loop(double dt);

private:
    struct JSPimpl;
    std::unique_ptr<JSPimpl> pimpl;
};

#endif
