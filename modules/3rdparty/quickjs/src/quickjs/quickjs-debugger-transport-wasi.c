#include "quickjs-debugger.h"
#include <assert.h>
#include <poll.h>

struct js_debugger_target
{
    char *address;
    int in_handle, out_handle;
};

static size_t js_transport_read(void *udata, char *buffer, size_t length)
{
    struct js_debugger_target *data = (struct js_debugger_target *)udata;
    if (data->in_handle <= 0)
        return -1;

    if (length == 0)
        return -2;

    if (buffer == NULL)
        return -3;

    ssize_t ret = read(data->in_handle, (void *)buffer, length);
    if (ret < 0)
        return -4;

    if (ret == 0)
        return -5;

    if (ret > length)
        return -6;

    return ret;
}

static size_t js_transport_write(void *udata, const char *buffer, size_t length)
{
    struct js_debugger_target *data = (struct js_debugger_target *)udata;
    if (data->out_handle <= 0)
        return -1;

    if (length == 0)
        return -2;

    if (buffer == NULL)
        return -3;

    size_t ret = write(data->out_handle, (const void *)buffer, length);
    if (ret <= 0 || ret > (ssize_t)length)
        return -4;

    return ret;
}

static size_t js_transport_peek(void *udata)
{
    struct pollfd fds[1];
    int poll_rc;

    struct js_debugger_target *data = (struct js_debugger_target *)udata;
    if (data->in_handle <= 0)
        return -1;

    fds[0].fd = data->in_handle;
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    poll_rc = poll(fds, 1, 0);
    if (poll_rc < 0)
        return -2;
    if (poll_rc > 1)
        return -3;
    // no data
    if (poll_rc == 0)
        return 0;
    // has data
    return 1;
}

static void js_transport_close(JSRuntime *rt, void *udata)
{
    struct js_debugger_target *data = (struct js_debugger_target *)udata;
    // if (data->in_handle <= 0 || data->out_handle <= 0)
    //     return;
    // close(data->handle);
    // data->handle = 0;
    // free(data->address);
    // free(udata);
}

static struct js_debugger_target js_debugger_parse_debugger_target(const char *address)
{
    struct js_debugger_target ret = {NULL, -1, -1};

    int address_length = strnlen(address, 128);
    assert(address_length < 128);

    char lines[3][64];
    int cur_line = 0;
    int i = 0, line_i = 0;

    while (address[i] != 0)
    {
        char c = address[i];
        if (c == '\n')
        {
            lines[cur_line][line_i] = 0;
            cur_line++;
            line_i = 0;
            if (cur_line >= 3)
            {
                break;
            }
        }
        else
        {
            lines[cur_line][line_i++] = c;
            if (line_i > 63)
            {
                assert(0);
            }
        }
        i++;
    }

    assert(cur_line == 3);

    ret.address = malloc(strnlen(lines[0], 64));
    strncpy(ret.address, lines[0], 64);

    char *fd_string = strstr(lines[1], ":");
    assert(fd_string);
    ret.in_handle = atoi(fd_string + 1);

    fd_string = strstr(lines[2], ":");
    assert(fd_string);
    ret.out_handle = atoi(fd_string + 1);

    return ret;
}

void js_debugger_connect(JSContext *ctx, const char *address)
{
    struct js_debugger_target result = js_debugger_parse_debugger_target(address);

    struct js_debugger_target *data = malloc(sizeof(struct js_debugger_target));
    data->address = NULL;
    data->in_handle = result.in_handle;
    data->out_handle = result.out_handle;

    js_debugger_attach(ctx, js_transport_read, js_transport_write, js_transport_peek, js_transport_close, data);
}

void js_debugger_wait_connection(JSContext *ctx, const char *address)
{
    // struct sockaddr_in addr = js_debugger_parse_sockaddr(address);

    // int server = socket(AF_INET, SOCK_STREAM, 0);
    // assert(server >= 0);

    // int reuseAddress = 1;
    // assert(setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuseAddress, sizeof(reuseAddress)) >= 0);

    // assert(bind(server, (struct sockaddr *) &addr, sizeof(addr)) >= 0);

    // listen(server, 1);

    // struct sockaddr_in client_addr;
    // socklen_t client_addr_size = (socklen_t) sizeof(addr);
    // int client = accept(server, (struct sockaddr *) &client_addr, &client_addr_size);
    // close(server);
    // assert(client >= 0);

    // struct js_transport_data *data = (struct js_transport_data *)malloc(sizeof(struct js_transport_data));
    // memset(data, 0, sizeof(js_transport_data));
    // data->handle = client;
    // js_debugger_attach(ctx, js_transport_read, js_transport_write, js_transport_peek, js_transport_close, data);
}
