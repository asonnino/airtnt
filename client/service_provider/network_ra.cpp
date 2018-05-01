/*
 * Copyright (C) 2011-2018 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */



#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "network_ra.h"
#include "service_provider.h"

#include <boost/asio.hpp>
#include <iostream>
#include "chat_message.hpp"

// Used to send requests to the service provider sample.  It
// simulates network communication between the ISV app and the
// ISV service provider.  This would be modified in a real
// product to use the proper IP communication.
//
// @param server_url String name of the server URL
// @param p_req Pointer to the message to be sent.
// @param p_resp Pointer to a pointer of the response message.

// @return int

#define MAX_BUF_SIZE 2048

using boost::asio::ip::tcp;
boost::asio::io_context io_context;
tcp::socket s(io_context);
bool connected = false;

uint8_t* reply;

int connect(std::string url, std::string port){
    try
    {
        reply = (uint8_t*) malloc(MAX_BUF_SIZE);
        tcp::resolver resolver(io_context);
        boost::asio::connect(s, resolver.resolve(url, port));
        connected = true;
        return 0;
    }catch (std::exception& e){
        std::cerr << "Exception: " << e.what() << "\n";
        free(reply);
        return -1;
    }
}

int ra_network_send_receive(const char *server_url,
    const ra_samp_request_header_t *p_req,
    ra_samp_response_header_t **p_resp)
{
    int ret = 0;
    ra_samp_response_header_t* p_resp_msg;
    chat_message msg;    
    if((NULL == server_url) ||
        (NULL == p_req) ||
        (NULL == p_resp))
    {
        return -1;
    }

    if(!connected){
        int result = connect("localhost", "8000");
        if(result) return -1;
    }
    printf("Connected to the server\n");
    

    int wrote = 0;
    int read = 0;
    wrote = boost::asio::write(s, 
                               boost::asio::buffer(p_req, 
                               p_req->size + sizeof (ra_samp_request_header_t)));

    printf("Wrote %d bytes of Msg %d\n", wrote, p_req->type);

    if (wrote != (p_req->size + sizeof (ra_samp_request_header_t)))
    {
        fprintf(stderr, "\nError, sending the message [%s].",
                __FUNCTION__);
        ret = -1;
    }else{
        printf("Msg %d success\n", p_req->type);
    }

    read = boost::asio::read(s,
                             boost::asio::buffer(reply, sizeof(ra_samp_response_header_t)));
    printf("Read %d bytes \n", read);

    ra_samp_response_header_t* reply_header = (ra_samp_response_header_t*) reply;
    printf("Received a header, size: %d, type: %d\n", reply_header->size, reply_header->type);

    if(reply_header->size > 0){
        printf("Reading the rest of the message\n");
        read = boost::asio::read(s,
                                 boost::asio::buffer(reply + sizeof(ra_samp_response_header_t), reply_header->size)); 
        *p_resp = (ra_samp_response_header_t*) reply;
        printf("Setting the response pointer to %d\n", reply);
    }else{
        printf("Nothing else to read\n");
    }

/*    case TYPE_RA_MSG1:
        wrote = boost::asio::write(s,
                boost::asio::buffer(p_req
            + sizeof(ra_samp_request_header_t)),
            p_req->size));
        ret = sp_ra_proc_msg1_req((const sample_ra_msg1_t*)((uint8_t*)p_req
            + sizeof(ra_samp_request_header_t)),
            p_req->size,
            &p_resp_msg);
        if((wrote != p_req->size) || (read != 1))
        {
            fprintf(stderr, "\nError, call sp_ra_proc_msg1_req fail [%s].",
                __FUNCTION__);
            ret = -1;
        }
        else
        {
            *p_resp = (ra_samp_response_header_t*) reply;
        }
        break;

    case TYPE_RA_MSG3:

        wrote = boost::asio::write(s, boost::asio::buffer(((uint8_t*)p_req +
            sizeof(ra_samp_request_header_t)),
            p_req->size));

        ret =sp_ra_proc_msg3_req((const sample_ra_msg3_t*)((uint8_t*)p_req +
            sizeof(ra_samp_request_header_t)),
            p_req->size,
            &p_resp_msg);
        read = boost::asio::read(s,
        //TODO Get the actual size
            boost::asio::buffer(reply, 1)); 

        if((wrote != p_req->size) || (read != 1))
        {
            fprintf(stderr, "\nError, call sp_ra_proc_msg3_req fail [%s].",
                __FUNCTION__);
            ret = -1;
        }
        else
        {
            *p_resp = (ra_samp_response_header_t*) reply;
        }
        break;
    
    case TYPE_RA_OUTPUT:
        wrote = boost::asio::write(s, boost::asio::buffer(((uint8_t*)p_req + 
            sizeof(ra_samp_request_header_t)),
            p_req->size));

        ret = sp_ra_proc_msg_output_req((const life_input_t*) ((uint8_t*)p_req + 
                                sizeof(ra_samp_request_header_t)),
                                p_req->size);
        break;

    default:
        ret = -1;
        fprintf(stderr, "\nError, unknown ra message type. Type = %d [%s].",
            p_req->type, __FUNCTION__);
        break;
    }
*/
    return ret;
}

// Used to free the response messages.  In the sample code, the
// response messages are allocated by the SP code.
//
//
// @param resp Pointer to the response buffer to be freed.

void ra_free_network_response_buffer(ra_samp_response_header_t *resp)
{
    /*if(resp!=NULL)
    {
        free(reply);
    }*/
}
