/*
 * Replace the following string of 0s with your student number
 * c0037826
 */
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <math.h>
#include "rft_client_logging.h"

/* 
 * is_corrupted - returns true with the given probability 
 * 
 * The result can be passed to the checksum function to "corrupt" a 
 * checksum with the given probability to simulate network errors in 
 * file transfer.
 *
 * DO NOT EDIT the is_corrupted function.
 */
static bool is_corrupted(float prob) {
    if (fpclassify(prob) == FP_ZERO)
        return false;
    
    float r = (float) rand();
    float max = (float) RAND_MAX;
    
    return (r / max) <= prob;
}

/* 
 * The verify_server function checks that the give server structure is 
 * equivalent to the server field of the proto struct. That is, the protocol
 * family, port and address of the two server structs are the same. 
 * It also checks that the given server_size is the same as the size of the
 * socket address size field of the proto struct.
 *
 * These checks can be used to ensure that an ACK is from the same server that 
 * a client sent data to.
 * 
 * If any of the checks fail, the function returns a corresponding error 
 * protocol state. If the servers are the same, the function returns the state 
 * of the protocol before the function was called.
 *
 * DO NOT EDIT the verify_server function.
 */
static proto_state verify_server(protocol_t* proto, struct sockaddr_in* server, 
    socklen_t server_size) {
    if (server_size != proto->sockaddr_size)
        return PS_BAD_S_SIZE;
    
    if (server->sin_port != proto->server.sin_port)
        return PS_BAD_S_PORT;
        
    if (server->sin_family != proto->server.sin_family)
        return PS_BAD_S_FAM;
        
    if (server->sin_addr.s_addr != proto->server.sin_addr.s_addr)
        return PS_BAD_S_ADDR;
        
    return proto->state;
}

/*
 * The setnlog_protocol function sets the protocol state to the given 
 * state, sets the proto src_line to the given line number and then 
 * logs output for the protocol.
 *
 * DO NOT EDIT the setnlog_protocol function.
 */
static void setnlog_protocol(protocol_t* proto, proto_state state, int line) {
    proto->state = state;
    proto->src_line = line;
    log_protocol(proto);
}

/* 
 * init_protocol - initialises fields of the given protocol struct to the 
 * values shown in its implementation.
 *
 * DO NOT EDIT the init_protocol function.
 */
void init_protocol(protocol_t* proto) {
    memset(proto, 0, sizeof(protocol_t));
    proto->state = PS_INIT;
    proto->in_file = NULL;
    proto->sockfd = -1;
    proto->seg_size = sizeof(segment_t);
    proto->sockaddr_size = (socklen_t) sizeof(struct sockaddr_in); 
    proto->timeout_sec = DEFAULT_TIMEOUT;
    proto->curr_retry = 0;
    proto->max_retries = DEFAULT_RETRIES;
    
    init_segment(proto, DATA_SEG, false);
    init_segment(proto, ACK_SEG, false);
}

/* 
 * TODO: you must implement this function.
 *
 * See documentation in rft_client_util.h and the assignment specification
 */
void init_segment(protocol_t* proto, seg_type type, bool payload_only) {
    
    if(type ==  DATA_SEG)
    {
        if(!payload_only)
        {
            memset(&proto->data, 0, proto->seg_size);
            proto->data.type = type;
        }
        else{
            for (int i = 0; i < PAYLOAD_SIZE; i++)
                proto->data.payload[i] = 0;
        }
        return;
    }
    
    if(type == ACK_SEG )
    {
        if(!payload_only)
        {
            memset(&proto->ack, 0, proto->seg_size);
            proto->ack.type = type;
        }
        else{
        
            for (int i = 0; i < PAYLOAD_SIZE; i++)
                proto->ack.payload[i] = 0;
        }
        return;
    }
    return;
}

/* 
 * TODO: you must implement this function.
 *
 * See documentation in rft_client_util.h and the assignment specification
 *
 * Hint:
 *  - you have to detect an error when reading from the proto's input file
 */
void read_data(protocol_t* proto) {
    init_segment(proto, DATA_SEG, true);
    
        proto->data.file_data  = fread(proto->data.payload, 1, PAYLOAD_SIZE-1, proto->in_file);
        
    if (ferror(proto->in_file))
    {
        exit_err_state(proto, PS_BAD_READ, __LINE__);
        return;
    }
    proto->tfr_bytes=proto->tfr_bytes-proto->data.file_data;
    proto->data.payload[PAYLOAD_SIZE-1]='\0';
    return;
}

/* 
 * TODO: you must implement this function.
 *
 * See documentation in rft_client_util.h and the assignment specification
 * and see how to send data in the preliminary exercise.
 *
 * Hints:
 *  - you have to handle the case when the current retry count 
 *      exceeds the maximum number of retries allowed
 *  - you have to introduce a probability of corruption of the payload checksum
 *      if the tfr_mode is with timeout (wt)
 *  - you have to detect an error from sending data - see how the rft_server
 *      detects an error when sending an ACK
 */
void send_data(protocol_t* proto) {
  
    if(proto->send_file_op == send_file_normal)
    {
     
        proto->data.checksum =  checksum(proto->data.payload,false);
    }
   
    if (proto->send_file_op == send_file_with_timeout)
    {
      
        proto->data.checksum =  checksum(proto->data.payload,is_corrupted(proto->loss_prob));
    }
    proto->src_line =__LINE__;
    log_protocol(proto);
    if(proto->curr_retry>proto->max_retries)
    {
        exit_err_state(proto, PS_EXCEED_RETRY, __LINE__);
        return;
    }
    else{
        ssize_t bytes = sendto(proto->sockfd, &proto->data, proto->seg_size, 0,
                         (struct sockaddr*) &proto->server,
                         sizeof(struct sockaddr_in));
                        
        if (bytes != proto->seg_size) {
            exit_err_state(proto, PS_BAD_SEND, __LINE__);
            return;
        }
    }
    return;
}

/* 
 * send_file_normal - sends a file to a server according to the RFT protocol 
 * with positive acknowledgement, without retransmission.
 *
 * DO NOT EDIT the send_file_normal function.
 */
void send_file_normal(protocol_t* proto) { 
    proto->src_file = __FILE__;
    
    setnlog_protocol(proto, PS_START_SEND, __LINE__);
    while (proto->tfr_bytes) {
        read_data(proto);
        proto->state = PS_DATA_SEND;
        send_data(proto);
        proto->total_segments++;
        proto->total_file_data += proto->data.file_data;
        
        setnlog_protocol(proto, PS_ACK_WAIT, __LINE__);
        init_segment(proto, ACK_SEG, false);
        socklen_t server_size = proto->sockaddr_size;
        struct sockaddr_in server;
        memset(&server, 0, server_size);
        ssize_t nbytes = recvfrom(proto->sockfd, &proto->ack, proto->seg_size,
            0, (struct sockaddr *) &server, &server_size);

        if (nbytes != proto->seg_size)
            exit_err_state(proto, PS_BAD_ACK, __LINE__);
        
        if (proto->data.sq != proto->ack.sq)
            exit_err_state(proto, PS_BAD_ACK_SQ, __LINE__);

        proto_state state = verify_server(proto, &server, server_size);
        if (proto->state != state)
            exit_err_state(proto, state, __LINE__);

        setnlog_protocol(proto, PS_ACK_RECV, __LINE__);

        proto->data.sq++;
    }

    proto->state = proto->fsize ? PS_TFR_COMPLETE : PS_EMPTY_FILE;
    
    return;
}      

/* 
 * TODO: you must implement this function.
 *
 * See documentation in rft_client_util.h and the assignment specification
 */
void send_file_with_timeout(protocol_t* proto) {
    int lastErrorNum = errno;
    proto->src_file = __FILE__;
    
    setnlog_protocol(proto, PS_START_SEND, __LINE__);
    
    set_socket_timeout(proto);
    
    while (proto->tfr_bytes) {
        proto->curr_retry =  0;
        read_data(proto);
        proto->state = PS_DATA_SEND;
        
        send_data(proto);
        proto->total_segments++;
        
        setnlog_protocol(proto, PS_ACK_WAIT, __LINE__);
        init_segment(proto, ACK_SEG, false);
        socklen_t server_size = proto->sockaddr_size;
        struct sockaddr_in server;
        memset(&server, 0, server_size);
        ssize_t nbytes = recvfrom(proto->sockfd, &proto->ack, proto->seg_size,
            0, (struct sockaddr *) &server, &server_size);
        
        while (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            errno = lastErrorNum;
            setnlog_protocol(proto, PS_NO_ACK, __LINE__);
            proto->state = PS_DATA_RESEND;
            proto->curr_retry =  proto->curr_retry + 1;
            proto->resent_segments = proto->resent_segments + 1;
            proto->total_segments++;
            
            send_data(proto);
            
            init_segment(proto, ACK_SEG, false);
            nbytes = recvfrom(proto->sockfd, &proto->ack, proto->seg_size,
                0, (struct sockaddr *) &server, &server_size);
            
            setnlog_protocol(proto, PS_ACK_WAIT, __LINE__);
        }
        
        if (nbytes != proto->seg_size)
            exit_err_state(proto, PS_BAD_ACK, __LINE__);
        
        if (proto->data.sq != proto->ack.sq)
            exit_err_state(proto, PS_BAD_ACK_SQ, __LINE__);

        proto_state state = verify_server(proto, &server, server_size);
        
        if (proto->state != state)
            exit_err_state(proto, state, __LINE__);

        setnlog_protocol(proto, PS_ACK_RECV, __LINE__);
        proto->total_file_data += proto->data.file_data;
        proto->data.sq++;
    }

    proto->state = proto->fsize ? PS_TFR_COMPLETE : PS_EMPTY_FILE;
    
    return;
}

/* 
 * TODO: you must implement this function.
 *
 * See documentation in rft_client_util.h and the assignment specification
 *
 * Hint:
 *  - you must copy proto information to a metadata struct
 *  - you have to detect an error from sending metadata - see how the rft_server
 *      detects an error when sending an ACK
 */
bool send_metadata(protocol_t* proto) {
    metadata_t file_inf;
    memset(&file_inf, 0, sizeof(metadata_t));
    file_inf.size = proto->fsize;
    strncpy(file_inf.name, proto->out_fname, MAX_FILENAME_SIZE);
    
    ssize_t bytes = sendto(proto->sockfd, &file_inf,sizeof(metadata_t), 0,(struct sockaddr*) &proto->server,
                     proto->sockaddr_size);
                    
    if (bytes != sizeof(metadata_t)) {
    return false;
    }
    else{
        return true;
    }
}
  
/* 
 * TODO: you must implement this function.
 *
 * See documentation in rft_client_util.h and the assignment specification
 */
void set_socket_timeout(protocol_t* proto) {
    
    const struct timeval s_timeout={.tv_sec=proto->timeout_sec, .tv_usec=0};
    
    if(setsockopt(proto->sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&s_timeout, sizeof(s_timeout)) == -1)
    {
        exit_err_state(proto, PS_BAD_SOCKTOUT, __LINE__);
        return;
    }
    else{
        return;
    }
}

/* 
 * TODO: you must implement this function.
 *
 * See documentation in rft_client_util.h and the assignment specification,
 * and look at rft_server and preliminary exercise code.
 *
 * Hint:
 *  - Make sure you check the return values of system calls such as 
 *      socket and inet_aton
 */
void set_udp_socket(protocol_t* proto) {
    proto->sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (proto->sockfd  < 0) {
        return;
    }
    else{
        memset(&proto->server, 0, proto->sockaddr_size);
        proto->server.sin_family = AF_INET;
        proto->server.sin_port = htons(proto->server_port);
        if (!inet_aton(proto->server_addr, &proto->server.sin_addr)) {
            close (proto->sockfd);
            proto->sockfd = -1;
            return;
        }
        else
        {
            proto->state = PS_TFR_READY;
        }
    }
    return;
} 
