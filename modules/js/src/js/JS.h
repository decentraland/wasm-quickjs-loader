#ifndef JS_H
#define JS_H

class IIOModule;

class JS
{
public:
    JS(IIOModule* ioModule);

    void loop(double dt);

private:
    IIOModule* ioModule;
    
};

#endif
