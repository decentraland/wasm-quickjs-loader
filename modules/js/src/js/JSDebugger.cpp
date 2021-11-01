#include "quickjs/quickjs-debugger.h"
#include "JS.h"

uint32_t __dbg__write(
  const void* buf_ptr,
  uint32_t length
) __attribute__((
    __import_module__("env"),
    __import_name__("debugger_write"),
    __warn_unused_result__
));

uint32_t __dbg__read(
  void* buf_ptr,
  uint32_t length
) __attribute__((
    __import_module__("env"),
    __import_name__("debugger_read"),
    __warn_unused_result__
));

uint32_t __dbg__peek(
  void
) __attribute__((
    __import_module__("env"),
    __import_name__("debugger_peek"),
    __warn_unused_result__
));


uint16_t __DoEvents(
  uint32_t arg1,
  uint32_t arg2,
  uint32_t arg3
) __attribute__((
    __import_module__("env"),
    __import_name__("DoEvents"),
    __warn_unused_result__
));

static size_t js_transport_read(void *udata, char *buffer, size_t length)
{
  // uint16_t counter = 0;
  // while (static_cast<JS *>(udata)->debugger_peek() < length){
  //   __DoEvents(length, static_cast<JS *>(udata)->debugger_peek(), counter++);
  //   static_cast<JS *>(udata)->debugger_poll();
  // }

  // return static_cast<JS *>(udata)->debugger_read(buffer, length);
  uint32_t result = __dbg__read(buffer, length);
  return result;
}

static size_t js_transport_write(void *udata, const char *buffer, size_t length)
{
  // return static_cast<JS *>(udata)->debugger_write(buffer, length);
  uint32_t result = __dbg__write(static_cast<const void*>(buffer), length);
  return result;
}

size_t js_transport_peek(void *udata)
{
  // return static_cast<JS *>(udata)->debugger_peek();
  uint32_t result = __dbg__peek();
  return result;
}

static void js_transport_close(JSRuntime *rt, void *udata)
{
  //  struct js_debugger_target *data = (struct js_debugger_target *)udata;
}

void js_debugger_connect(JSContext *ctx, const char *address)
{
  int ptr_number = atoi(address);
  JS *js = reinterpret_cast<JS *>(ptr_number);

  if (js != nullptr)
  {
    js_debugger_attach(ctx, js_transport_read, js_transport_write, js_transport_peek, js_transport_close, js);
  }
}

void js_debugger_wait_connection(JSContext *ctx, const char *address)
{
}
