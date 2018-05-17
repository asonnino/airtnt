//
// blocking_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>
#include <boost/asio.hpp>


#include "misc.h"

#include "chat_message.hpp"
using boost::asio::ip::tcp;


#include <vector>
#include "Letter.h"
#include "image_util.h"
#include "processing.h"
using namespace std;


const int max_length = 20000;
chat_message read_msg_;


// input image
ocr_input_t* ocr_input;


////////////////////////////////
// EDIT
////////////////////////////////
#include "network_ra.h"
#include <time.h>

#ifndef SAFE_FREE
#define SAFE_FREE(ptr) {if (NULL != (ptr)) {free(ptr); (ptr) = NULL;}}
#endif

int steps=0;
int max_iterations=0;

int iter_counter=0;
////////////////////////////////
// END EDIT
////////////////////////////////

void session(tcp::socket sock)
{
    struct timeval t_connected, t_finished;
    gettimeofday(&t_connected,NULL);
    unsigned long m_connected = 1000000 * t_connected.tv_sec + t_connected.tv_usec;
  try
  {
    for (;;)
    {
      char data[max_length];

      boost::system::error_code error;
      size_t length = sock.read_some(boost::asio::buffer(data, sizeof(ra_samp_request_header_t)), error);
      if (error == boost::asio::error::eof){
        std::cout << "Connection close by peer\n";
        break; // Connection closed cleanly by peer.
      }else if (error){
        throw boost::system::system_error(error); // Some other error.
      }
      iter_counter++;
      /*
      if( !read_msg_.decode_header()){
        std::cout << "Error reading header\n";
        break;
      }
      */
      ra_samp_request_header_t *header = (ra_samp_request_header_t*) data;

      

      #ifdef MYDEBUG 
      printf("[header] length: %d\n", length);
      printf("[header] received size: %d\n", header->size);
      printf("[header] received type: %d\n", header->type);
      #endif
        
      int sum=0;
      int read_bytes=0;
      while(sum < header->size) {

        read_bytes += sock.read_some(boost::asio::buffer(
          data+sizeof(ra_samp_request_header_t)+sum, header->size - sum), error
        );
        sum += read_bytes;

        if (error == boost::asio::error::eof){
          break; // Connection closed cleanly by peer.
        }else if (error){
          throw boost::system::system_error(error); // Some other error.
        }
      }
      
      



      ////////////////////////////////
      // EDIT
      ////////////////////////////////
      // set message
      /*
      ra_samp_request_header_t *p_msg0_full = NULL;
      p_msg0_full = (ra_samp_request_header_t*)
        malloc(sizeof(ra_samp_request_header_t) + sizeof(uint32_t));
      p_msg0_full->type = read_msg_.type(); // e.g., TYPE_RA_MSG0;
      p_msg0_full->size = sizeof(uint32_t);
      uint32_t extended_epid_group_id = 0;
      *(uint32_t*)((uint8_t*)p_msg0_full + sizeof(ra_samp_request_header_t)) = extended_epid_group_id;

      // et buffer
      ra_samp_response_header_t *p_msg0_resp_full = NULL;
      */

      

      
      
      #ifdef MYDEBUG 
      ra_samp_request_header_t* tmp = (ra_samp_request_header_t*) data;
      printf("[body] length: %d\n", read_bytes);
      printf("[body] received type: %d\n", tmp->type);
      printf("[body] received size: %d\n", tmp->size);
      printf("\n");
      #endif

      // receive message
      ra_samp_response_header_t *p_msg0_resp_full = NULL;

      ra_network_send_receive(
        "http://example.com",
        (ra_samp_request_header_t*) data,
        &p_msg0_resp_full,
        steps, 
        max_iterations,
        ocr_input
      );

      #ifdef MYDEBUG 
      printf("p_att_result_msg pointer: %d\n", p_msg0_resp_full);

      printf("Response size: %d\n", p_msg0_resp_full->size);
      printf("Response type: %d\n", p_msg0_resp_full->type);
      #endif 


      // wirte to dump to socket
      int wrote_bytes=0;
      while(wrote_bytes < sizeof(ra_samp_response_header_t) + p_msg0_resp_full->size) {
        wrote_bytes += boost::asio::write(sock, boost::asio::buffer(
          p_msg0_resp_full + wrote_bytes, 
          sizeof(ra_samp_response_header_t) + p_msg0_resp_full->size - wrote_bytes)
        );
      }

      if((!p_msg0_resp_full->size) && (p_msg0_resp_full->type == 6)){
        gettimeofday(&t_finished,NULL);
        unsigned long m_finished = 1000000 * t_finished.tv_sec + t_finished.tv_usec;
        printf("Time connected [us] %lu, time finished [us] %lu, time diff [us] %lu, iterations %d\n", m_connected, m_finished, m_finished - m_connected, iter_counter);
        
        iter_counter = 0;


        ra_free_network_response_buffer(p_msg0_resp_full);

        exit(0);
      }

      ra_free_network_response_buffer(p_msg0_resp_full);

    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception in thread: " << e.what() << "\n";
  }
}

void server(boost::asio::io_context& io_context, unsigned short port)
{
  tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), port));
  for (;;)
  {
    std::thread(session, a.accept()).detach();
  }
}

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 4)
    {
      std::cerr << "Usage: blocking_tcp_echo_server <port>\n";
      return 1;
    }

    boost::asio::io_context io_context;


    // config 
    steps =  std::atoi(argv[2]);
    max_iterations = std::atoi(argv[3]);
    //

    /////////////////////////////////
    // START OCR
    /////////////////////////////////
    // load image
    char const *image_input = "./data/input_5_OK.png";
    vector< vector<int> > pixels;
    if (load_image(image_input, &pixels) != 0) {
      printf("Could not load input image: %s\n", image_input);
      return -1;
    }

    // convert input to C type 
    vector<int*> ptrs;
    transform(begin(pixels), end(pixels), back_inserter(ptrs), [](vector<int> &inner_vec) {
        return inner_vec.data();
    });
    int **input =  ptrs.data();
    int rows = pixels.size();
    int cols = pixels[0].size();

    // convert to 1d array
    unsigned long ocr_input_size = sizeof(ocr_input_t) + (rows * cols * sizeof(int));
    ocr_input = (ocr_input_t*)malloc(ocr_input_size);
    ocr_input->rows = rows;
    ocr_input->cols = cols;
    for (int i=0; i < rows; ++i) {
        for (int j=0; j < cols; ++j) {
            ocr_input->payload[i*rows+j] = input[i][j];
        }
    }
    /////////////////////////////////
    // END OCR
    /////////////////////////////////


    server(io_context, std::atoi(argv[1]));
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
    free(ocr_input);
  }

  free(ocr_input);
  return 0;
}
