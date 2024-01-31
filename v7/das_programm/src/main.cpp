#include "common.hpp"
#include "send.hpp"
#include "receive.hpp"
#include "pack.hpp"

#include <vector>
#include <string>
#include <fstream>
#include <b15f/b15f.h>

// Helper sleep function
#include <chrono>
#include <thread>

class Timer 
{	// Courtesy of ChatGPT
public:
    // Constructor, takes the duration in milliseconds
    explicit Timer(std::chrono::milliseconds duration) : duration_(duration) {};

    // Start the timer
    void start() {
        start_time_ = std::chrono::high_resolution_clock::now();
    }

    // Check if the timer has elapsed
    bool hasElapsed() const {
        auto current_time = std::chrono::high_resolution_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time_);
        return elapsed_time >= duration_;
    }

    // Reset the timer
    void reset() {
        start_time_ = std::chrono::high_resolution_clock::now();
    }

private:
    std::chrono::high_resolution_clock::time_point start_time_;
    std::chrono::milliseconds duration_;
};

void sleep_ms(unsigned int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// Remember, we split our 8 bits in half, 4 for sending, 4 for receiving.
// The following three functions help with this matter.
unsigned char get_sender_bits(unsigned char channel_state, unsigned int sender_offset) {
    return (channel_state >> (4 * sender_offset)) & 0xf;
}

unsigned char get_receiver_bits(unsigned char channel_state, unsigned int sender_offset) {
    return (channel_state >> (4 * (1 - sender_offset))) & 0xf;
}

unsigned char assemble_channel_bits(unsigned char sender_bits, unsigned char receiver_bits, unsigned int sender_offset) {
    unsigned char sender_byte   = sender_bits   << (sender_offset * 4);
    unsigned char receiver_byte = receiver_bits << ((1 - sender_offset) * 4);

    return sender_byte | receiver_byte;
}

// Utility functions to read and write byte vectors from/to filestreams.
std::vector<unsigned char> read_bytes_from_file(std::istream& file_stream, std::size_t num_bytes) {
    std::vector<char> buffer(num_bytes);

    // Read bytes from the file stream into the buffer
    file_stream.read(buffer.data(), num_bytes);

    // Resize the vector to the actual number of bytes read
    buffer.resize(file_stream.gcount());

    return std::vector<unsigned char>(buffer.begin(), buffer.end());
}

void write_bytes_to_file(std::ostream& file_stream, const std::vector<unsigned char>& bytes) {
    // Have to perform this cast... unfortunately
    std::vector<char> buffer(bytes.begin(), bytes.end());

    // Write out all the bytes
    file_stream.write(buffer.data(), bytes.size());
}


int main(int argc, char *argv[]) {

	if(argc != 4) {
		fprintf(stderr, "Usage: %s <sender_offset> <block_size> <delay>\n", argv[0]);
		std::exit(1);
	}


    // What channel to send on?
    unsigned char sender_offset = std::stoi(argv[1]);
	if(!(sender_offset == 1 || sender_offset == 0)) {
		fprintf(stderr, "Sender offset must be 0 or 1!\n");
		std::exit(1);
	}

	// Arbitrarily chosen number of bytes of data packed to each frame.
	const std::size_t BLOCK_SIZE = std::stoi(argv[2]);

    std::istream &infile  = std::cin;
    std::ostream &outfile = std::cout;

    // B15F setup here
	B15F& drv = B15F::getInstance();
	
	// Reset B15F pins before transmission begin
	drv.setRegister(&DDRA, 0xFF);
	drv.setRegister(&PORTA, 0);

	// Set the real ddra mask
	unsigned char ddra_mask = (sender_offset == 0 ? 0b01001011 : 0b10110100);
	drv.setRegister(&DDRA, ddra_mask);

	// Little helper print...
	std::cerr << "Sending with sender channel  offset " << std::to_string(sender_offset) << std::endl;
	std::cerr << "DDRA " << std::bitset<8>(ddra_mask) << std::endl;
	if(sender_offset == 1)
		std::cerr << "High bits send, low bits receive" << std::endl;
	else
		std::cerr << "High bits receive, low bits send" << std::endl;


    Sender sender;
    Receiver receiver;

    unsigned char channel_state = 0x0,
                  sender_bits = 0x0,
                  receiver_bits = 0x0;

    bool sender_done = false;
    bool sender_done_after_this_frame = false;
    bool receiver_done = false;

    static std::vector<unsigned char> inframe, outframe;

    float delay = std::stof(argv[3]);
    unsigned int sender_delay_ticks = int(3 * (1.0f / delay));
    sender_delay_ticks = std::clamp(int(sender_delay_ticks), 10, 100);
    unsigned int delay_ms = (delay * 1000);
    unsigned long n_ticks_elapsed = 0;
    Timer timer{std::chrono::milliseconds(delay_ms)};
    timer.start();

    while( !(sender_done && receiver_done) ) {

	channel_state = drv.getRegister(&PINA);
	// std::cerr << std::bitset<8>(channel_state) << "\t";

        if(sender.need_frame() && !sender_done) {
            auto data = read_bytes_from_file(infile, BLOCK_SIZE);
            pack_frame(data, inframe);
            sender.read_frame(inframe);

            // TODO: What if end of infile is reached?
	    // If the data we attempt to send is of length zero, assume we are done.
	    if(data.size() == 0) {
		    if(sender_done_after_this_frame) sender_done = true;
		    sender_done_after_this_frame = true;
		    fprintf(stderr, "Sender is done.\n");
	    }
        }
        if(receiver.frame_available() && !receiver_done) {
            std::vector<unsigned char> data;
            receiver.frame_pull(outframe);
            unpack_frame(outframe, data);
            write_bytes_to_file(outfile, data);
	    outfile.flush();

	    

            // TODO: What if end of outfile is reached?
	    // If the data we receive is of length zero, assume we are done.
	    // (Three including control sequences.)
	    if(data.size() < BLOCK_SIZE) {
		    receiver_done = true;
		    fprintf(stderr, "Receiver is done.\n");
	    }
        }

	// Wait for timer, then reset
	while(!timer.hasElapsed());
	timer.reset();

        if((n_ticks_elapsed++ > sender_delay_ticks) && !sender_done)  {
		// std::cerr << "\nSender bits read: " << 
			// std::bitset<4>(get_sender_bits(channel_state, sender_offset))
		       	// << std::endl;
		sender_bits   = sender.tick(get_sender_bits(channel_state, sender_offset));
	} else {
		sender_bits = 0x0;
	}

	// std::cerr << "S" << std::bitset<4>(sender_bits) << "\t";
        if(!receiver_done) 
		receiver_bits = receiver.tick(get_receiver_bits(channel_state, sender_offset));
	else
		receiver_bits = get_receiver_bits(channel_state, sender_offset) | 0b0100;

	// std::cerr << "R" << std::bitset<4>(receiver_bits) << std::endl;

        channel_state = assemble_channel_bits(sender_bits, receiver_bits, sender_offset);
	drv.setRegister(&PORTA, channel_state);
    }

    fprintf(stderr, "Transmission done. Flushing streams and exiting.\n");

    // Reset B15F pins after transmission end
    // drv.setRegister(&DDRA, 0xFF);
    // drv.setRegister(&PORTA, 0);
    outfile.flush();

    return 0;
}
