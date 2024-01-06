#include "common.hpp"
#include "send.hpp"
#include "receive.hpp"

#include <vector>
#include <string>
#include <fstream>

unsigned char get_sender_bits(unsigned char channel_state, unsigned int sender_offset) {
    return (channel_state >> (4 * sender_offset)) & 0xff;
}

unsigned char get_receiver_bits(unsigned char channel_state, unsigned int sender_offset) {
    return (channel_state >> (4 * (1 - sender_offset))) & 0xff;
}

unsigned char assemble_channel_bits(unsigned char sender_bits, unsigned char receiver_bits, unsigned int sender_offset) {
    unsigned char sender_byte   = sender_bits   << (sender_offset * 4);
    unsigned char receiver_byte = receiver_bits << ((1 - sender_offset) * 4);

    return sender_byte | receiver_byte;
}

int main(int argc, char *argv[]) {
    // What channel to send on?
    unsigned char sender_offset = std::stoi(argv[3]);

    std::fstream infile(argv[1]);
    std::fstream outfile(argv[2]);

    // TODO: B15F setup here

    Sender sender;
    Receiver receiver;

    unsigned char channel_state = 0x0,
                  sender_bits = 0x0,
                  receiver_bits = 0x0;

    std::vector<unsigned char> inframe, outframe;

    while(true) {

        if(sender.need_frame()) {
            // TODO: Read data from infile and pack it to frame here.
            sender.read_frame(inframe);
        }
        if(receiver.frame_available()) {
            receiver.frame_pull(outframe);

        }

        sender_bits   = sender.tick(get_sender_bits(channel_state, sender_offset));
        receiver_bits = receiver.tick(get_receiver_bits(channel_state, sender_offset));

        channel_state = assemble_channel_bits(sender_bits, receiver_bits, sender_offset);

        // TODO: B15F code to read and set channel state here
    }

    return 0;
}
