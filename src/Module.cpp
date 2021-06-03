//
// Created by dy on 6/30/20.
//

#include "Module.h"

void Module::init(FIFO ** &input, FIFO ** &output, int input_cnt, int output_cnt, int packet_cnt=0)
{
    this->input_cnt = input_cnt;
    this->output_cnt = output_cnt;
    this->input = input;
    this->output = output;
    this->packet_cnt = packet_cnt;
    this->data_in = new uint8_t[MAX_FIFO_SIZE];
    this->data_out = new uint8_t[MAX_FIFO_SIZE];
}


void Module::loop()
{
    int cnt = 0;
    //uint8_t * raw_data;
    long long raw_data_len;
    printf("Module loop start.");
    while (true) {
        //Data_Pack * data_in = this->input[0]->get();
        //data_in->get_data(raw_data, raw_data_len, cnt);
        //this->input[0]->set_destroy_time(data_in->destroy());
        //delete data_in;
        this->input[0]->get(this->data_in, raw_data_len, cnt);
        //Data_Pack * data_out = new Data_Pack();
        //data_out->init(raw_data_len, cnt);
       // data_out->put_data(raw_data, raw_data_len);
        //delete [] raw_data;
        memcpy(this->data_out, this->data_in, raw_data_len);
        usleep(10000);
        this->output[0]->put(this->data_out, raw_data_len, cnt);

        if (this->packet_cnt > 0 && cnt >= this->packet_cnt)
            break;
    }
}
