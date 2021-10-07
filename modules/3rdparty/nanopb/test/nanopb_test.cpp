
#include "protomsg/test.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>

int foo() {
    FetchAPI_GetState message = FetchAPI_GetState_init_zero;
    
    uint8_t buffer[128];
    size_t message_length;

    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    
    message.has_id = 1;
    message.id = 13;
    
    /* Now we are ready to encode the message! */
    bool status = pb_encode(&stream, FetchAPI_GetState_fields, &message);
    message_length = stream.bytes_written;
    
    /* Then just check for any errors.. */
    if (!status)
    {
        // printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
        return 1;
    }
    return 0;
}