#include <SHA256.h>

using namespace std;

// Converts the ASCII string to a binary representation.
vector<unsigned long> convert_to_binary(const string);

// Pads the messages to make sure they are a multiple of 512 bits.
vector<unsigned long> pad_to_512bits(const vector<unsigned long>);

// Changes the n 8 bit segments representing every ASCII character to 32 bit words.
vector<unsigned long> resize_block(vector<unsigned long>);

// The actual hash computing.
string compute_hash(const vector<unsigned long>);

// Used for debugging. Decided to leave this in for if I ever will expand on this.
        string show_as_hex(unsigned long);
        void cout_block_state(vector<unsigned long>);
        string show_as_binary(unsigned long);
        const bool show_block_state_add_1 = 0;
        const bool show_distance_from_512bit = 0;
        const bool show_padding_results = false;
        const bool show_working_vars_for_t = 0;
        const bool show_T1_calculation = false;
        const bool show_T2_calculation = false;
        const bool show_hash_segments = false;
        const bool show_Wt = false;

// Taken from NIST spec. I find it amazing how this can all be done by just
// bit rotations.
        #define ROTRIGHT(word,bits) (((word) >> (bits)) | ((word) << (32-(bits))))
        #define SSIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
        #define SSIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))
        #define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
        #define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

// Supposed incorrect implimentation from NIST.
// BSIG0 is replaced with EP0 and BSIG1 is replaced with EP0 in the implimentation.
        #define BSIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
        #define BSIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))

//
/* Function : main
The beginning of my program for this extra credit implimentation of SHA-256.
First it checks to see if testing is enabled by lack of command line arguments,
causing it to hash "abc" and comparing it to a hard coded hash I know is correct.
1) Converts the ASCII string into n 8 bit segments representing the ASCII value
of each character.
2) Pads the message so it will be 512 bits long.
3) Combines seperate 8 bit ASCII values to 32 bit words.
4) Computes the hash.
5) If testing, compare result with expected result. Yell if result is not as
   expected.
Notes: Should be using Boost.Program_options here.
Input : Message as a command line argument or nothing indicating to test.
Output : Encrypted SHA-256 hash and debugging information if requested.
Errors : The message length should be no more than 55 characters.*/
string sha256( std::string message )
{

        // abc in decimal is 97 98 99
        // abc in hex is 0x61 0x62 0x63

        // This will hold all the blocks.
        vector<unsigned long> block;

        // First convert this guy to a vector of strings representing 8 bit variables.
        block = convert_to_binary(message);

        // Pad it so that the message will be a full 512 bits long.
        block = pad_to_512bits(block);

        // Combine the seperate 8 bit sections into single 32 bit sections.
        block = resize_block(block);

        // This is what does the actual hashing.
        string hash = compute_hash(block);

        return hash;
}

/* Function : resize_block
Resize the blocks from 64 8 bit sections to 16 32 bit sections.
Input : Vector of individual 8 bit ascii values
Output : Vector of 32 bit words which are combined ascii values.
Errors : The message length should be no more than 55 characters.*/
vector<unsigned long> resize_block(vector<unsigned long> input)
{
        vector<unsigned long> output(16);

        // Loop through the 64 sections by 4 steps and merge those 4 sections.
        for(int i = 0; i < 64; i = i + 4)
        {
                // Lets make a big 32 bit section first.
                bitset<32> temp(0);

                // Shift the blocks to their assigned spots and OR them with the original
                // to combine them.
                temp = (unsigned long) input[i] << 24;
                temp |= (unsigned long) input[i + 1] << 16;
                temp |= (unsigned long) input[i + 2] << 8;
                temp |= (unsigned long) input[i + 3];

                // Puts the new 32 bit word into the correct output array location.
                output[i/4] = temp.to_ulong();
        }

        return output;
}

/* Function : cout_block_state
Shows the current contents of all the blocks in binary. This is used solely
for debugging and has no actual use in the implimentation.
Input : Vector of the current blocks.
Output : Contents of each block in binary and hex.*/
void cout_block_state(vector<unsigned long> block)
{
        cout << "---- Current State of block ----\n";
        for (int i = 0; i < block.size(); i++)
        {
                cout << "block[" << i << "] binary: " << show_as_binary(block[i])
                        << "     hex y: 0x" << show_as_hex(block[i]) << endl;
        }
}

/* Function : show_as_hex
Shows the current contents of the block in hex.
This is mainly to avoid having to change the stream from hex to dec and back
again in the cout statement which would have been error prone.
Input : A 32 or less bit block
Output : Contents of the block in hex as a string. */
string show_as_hex(unsigned long input)
{
        bitset<32> bs(input);
        unsigned n = bs.to_ulong();

        stringstream sstream;
        sstream << std::hex << std::setw(8) << std::setfill('0') << n;
        string temp;
        sstream >> temp;

        return temp;
}



/* Function : show_as_binary
Shows the current contents of the block in hex.
This is mainly to avoid having to change the stream from hex to dec and back
again in the cout statement which would have been error prone.
Input : A 32 or less bit block
Output : Contents of the block in binary as a stromg. */
string show_as_binary(unsigned long input)
{
        bitset<8> bs(input);
        return bs.to_string();
}

/* Function : convert_to_binary
Takes the string and convers all characters to their ASCII Binary equivilents.
Input : A string of any length
Output : A vector consisting of one 8 bit value per ASCII character. */
vector<unsigned long> convert_to_binary(const string input)
{
        // Lets make a vector to hold all the ASCII character values.
        vector<unsigned long> block;

        // For each character, convert the ASCII chararcter to its binary
        // representation.
        for (int i = 0; i < input.size(); ++i)
        {
                // Make a temporary variable called B to store the 8 bit pattern
                // for the ASCII value.
                bitset<8> b(input.c_str()[i]);

                // Add that 8 bit pattern into the block.
                block.push_back(b.to_ulong());
        }

        return block;
}

/* Function : pad_to_512bits
Takes the vector of ASCII values in binary and pad it so that there will be a
total of 512 bits.
Padding specifically consist of having your message, adding 1, and adding 0's to
it so you will have the current length be 448 bits long, and then adding 64 bits
which say how long the original message was, giving you a total of 512 bits.
Input : The message in the form of a vector containing 8 bit binary ASCII values.
Output : A padded vector consisting of one 8 bit value per ASCII character. */
vector<unsigned long> pad_to_512bits(vector<unsigned long> block)
{
        // Variable names as per NIST.

        // l will represent the length of the message in terms of bits, with each
        // block representing one 8 bit ascii character.
        int l = block.size() * 8;

        // Equation for padding is l + 1 + k = 448 mod 512
        // Simplified to: l + 1 + k = 448
        //                448 - 1 - l = k
        //                447 - l = k
        // l = length of message in bits
        // k = how much zero's to add so new message will be a multiple of 512.
        int k = 447 - l;

        // First add in another 8 bit block with the first bit set to 1
        if(show_block_state_add_1)
                cout_block_state(block);

        unsigned long t1 = 0x80;
        block.push_back(t1);

        if(show_block_state_add_1)
                cout_block_state(block);

        // We just added in 7 zero's, so subtract 7 from k.
        k = k - 7;

        // Find how far away we are from a 512 bit message. Just debugging.
        if (show_distance_from_512bit)
        {
                cout << "l: " << l << endl;
                cout << "k: " << k + 7 << endl; // Plus 7 so this follows the paper.
        }

        // Just debugging.
        if (show_distance_from_512bit)
                cout << "adding " << k / 8 << " empty eight bit blocks!\n";

        // Add 8 bit blocks containing zero's
        for(int i = 0; i < k / 8; i++)
                block.push_back(0x00000000);

        // We are now at 488 bits out of 512. We need to add l in the binary
        // form of eight 8 bit blocks.
        bitset<64> big_64bit_blob(l);
        if (show_distance_from_512bit)
                cout << "l in a 64 bit binary blob: \n\t" << big_64bit_blob << endl;

        // Split up that 64 bit blob into 8 bit sections.
        string big_64bit_string = big_64bit_blob.to_string();

        // Push the first block into the 56th position.
        bitset<8> temp_string_holder1(big_64bit_string.substr(0,8));
        block.push_back(temp_string_holder1.to_ulong());

        // Push all the rest of the 8 bit blocks in.
        for(int i = 8; i < 63; i=i+8)
        {
                bitset<8> temp_string_holder2(big_64bit_string.substr(i,8));
                block.push_back(temp_string_holder2.to_ulong());
        }

        // Just display everything so we know what is going on.
        if (show_padding_results)
        {
                cout << "Current 512 bit pre-processed hash in binary: \n";
                        for(int i = 0; i < block.size(); i=i+4)
                                cout << i << ": " << show_as_binary(block[i]) << "     "
                                     << i + 1 << ": " << show_as_binary(block[i+1]) << "     "
                                     << i + 2 << ": " << show_as_binary(block[i+2]) << "     "
                                     << i + 3 << ": " << show_as_binary(block[i+3]) << endl;

                cout << "Current 512 bit pre-processed hash in hex: \n";
                for(int i = 0; i < block.size(); i=i+4)
                        cout << i << ": " << "0x" + show_as_hex(block[i]) << "     "
                             << i + 1 << ": " << "0x" + show_as_hex(block[i+1]) << "     "
                             << i + 2 << ": " << "0x" + show_as_hex(block[i+2]) << "     "
                             << i + 3 << ": " << "0x" + show_as_hex(block[i+3]) << endl;
        }
        return block;
}

/* Function : compute_hash
Note:	This is a very long function because a large portion of it is solely
                debugging assistance. Originally planned to be purged of those but in
                the end decided to keep them so it will be much easier to debug this
                in the future.
Input : The 512 bit padded message as a vector containing 8 bit binary ASCII values.
Output : A string representing the hash. */
string compute_hash(const vector<unsigned long> block)
{
        // Taken from the spec
        //   SHA-224 and SHA-256 use the same sequence of sixty-four constant
        //   32-bit words, K0, K1, ..., K63.  These words represent the first 32
        //   bits of the fractional parts of the cube roots of the first sixty-
        //   four prime numbers.
        unsigned long k[64] = {
                0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,
                0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
                0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,
                0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
                0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,
                0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
                0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,
                0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
                0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,
                0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
                0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
        };

        // Initial Hash Values, first thirty-two bits of the fractional parts of
        // the square roots of the first eight prime numbers.
        unsigned long static H0 = 0x6a09e667;
        unsigned long static H1 = 0xbb67ae85;
        unsigned long static H2 = 0x3c6ef372;
        unsigned long static H3 = 0xa54ff53a;
        unsigned long static H4 = 0x510e527f;
        unsigned long static H5 = 0x9b05688c;
        unsigned long static H6 = 0x1f83d9ab;
        unsigned long static H7 = 0x5be0cd19;

        unsigned long W[64];

        for(int t = 0; t <= 15; t++)
        {
                W[t] = block[t] & 0xFFFFFFFF;

                if (show_Wt)
                        cout << "W[" << t << "]: 0x" << show_as_hex(W[t]) << endl;
        }

        for(int t = 16; t <= 63; t++)
        {
                // Also taken from spec.
                W[t] = SSIG1(W[t-2]) + W[t-7] + SSIG0(W[t-15]) + W[t-16];

                // Have to make sure we are still dealing with 32 bit numbers.
                W[t] = W[t] & 0xFFFFFFFF;

                if (show_Wt)
                        cout << "W[" << t << "]: " << W[t];
        }

        unsigned long temp1;
        unsigned long temp2;
        unsigned long a = H0;
        unsigned long b = H1;
        unsigned long c = H2;
        unsigned long d = H3;
        unsigned long e = H4;
        unsigned long f = H5;
        unsigned long g = H6;
        unsigned long h = H7;

        if(show_working_vars_for_t)
                cout << "         A        B        C        D        "
                     << "E        F        G        H        T1       T2\n";

        for( int t = 0; t < 64; t++)
        {
                // Seems the Official spec is wrong!? BSIG1 is incorrect.
                // Replacing BSIG1 with EP1.
                temp1 = h + EP1(e) + CH(e,f,g) + k[t] + W[t];
                if ((t == 20) & show_T1_calculation)
                {
                        cout << "h: 0x" << hex << h << "  dec:" << dec << h
                             << "  sign:" << dec << (int)h << endl;
                        cout << "EP1(e): 0x" << hex << EP1(e) << "  dec:"
                             << dec << EP1(e) << "  sign:" << dec << (int)EP1(e)
                             << endl;
                        cout << "CH(e,f,g): 0x" << hex << CH(e,f,g) << "  dec:"
                             << dec << CH(e,f,g) << "  sign:" << dec
                             << (int)CH(e,f,g) << endl;
                        cout << "k[t]: 0x" << hex << k[t] << "  dec:" << dec
                             << k[t] << "  sign:" << dec << (int)k[t] << endl;
                        cout << "W[t]: 0x" << hex << W[t] << "  dec:" << dec
                             << W[t] << "  sign:" << dec << (int)W[t] << endl;
                        cout << "temp1: 0x" << hex << temp1 << "  dec:" << dec
                             << temp1 << "  sign:" << dec << (int)temp1 << endl;
                }


                // Seems the Official spec is wrong!? BSIG0 is incorrect.
                // Replacing BSIG0 with EP0.
                temp2 = EP0(a) + MAJ(a,b,c);

                // Shows the variables and operations for getting T2.
                if ((t == 20) & show_T2_calculation)
                {
                        cout << "a: 0x" << hex << a << "  dec:" << dec << a
                             << "  sign:" << dec << (int)a << endl;
                        cout << "b: 0x" << hex << b << "  dec:" << dec << b
                             << "  sign:" << dec << (int)b << endl;
                        cout << "c: 0x" << hex << c << "  dec:" << dec << c
                             << "  sign:" << dec << (int)c << endl;
                        cout << "EP0(a): 0x" << hex << EP0(a) << "  dec:"
                             << dec << EP0(a) << "  sign:" << dec << (int)EP0(a)
                             << endl;
                        cout << "MAJ(a,b,c): 0x" << hex << MAJ(a,b,c) << "  dec:"
                             << dec << MAJ(a,b,c) << "  sign:" << dec
                             << (int)MAJ(a,b,c) << endl;
                        cout << "temp2: 0x" << hex << temp2 << "  dec:" << dec
                             << temp2 << "  sign:" << dec << (int)temp2 << endl;
                }

                // Do the working variables operations as per NIST.
                h = g;
                g = f;
                f = e;
                e = (d + temp1) & 0xFFFFFFFF; // Makes sure that we are still using 32 bits.
                d = c;
                c = b;
                b = a;
                a = (temp1 + temp2) & 0xFFFFFFFF; // Makes sure that we are still using 32 bits.

                // Shows the contents of each working variable for the turn T.
                if (show_working_vars_for_t)
                {
                        cout << "t= " << t << " ";
                        cout << show_as_hex (a) << " " << show_as_hex (b) << " "
                             << show_as_hex (c) << " " << show_as_hex (d) << " "
                             << show_as_hex (e) << " " << show_as_hex (f) << " "
                             << show_as_hex (g) << " " << show_as_hex (h) << " "
                             << endl;
                }
        }

        // Shows the contents of each hash segment.
        if(show_hash_segments)
        {
                cout << "H0: " << show_as_hex (H0) << " + " << show_as_hex (a)
                         << " " << show_as_hex (H0 + a) << endl;
                cout << "H1: " << show_as_hex (H1) << " + " << show_as_hex (b)
                         << " " << show_as_hex (H1 + b) << endl;
                cout << "H2: " << show_as_hex (H2) << " + " << show_as_hex (c)
                         << " " << show_as_hex (H2 + c) << endl;
                cout << "H3: " << show_as_hex (H3) << " + " << show_as_hex (d)
                         << " " << show_as_hex (H3 + d) << endl;
                cout << "H4: " << show_as_hex (H4) << " + " << show_as_hex (e)
                         << " " << show_as_hex (H4 + e) << endl;
                cout << "H5: " << show_as_hex (H5) << " + " << show_as_hex (f)
                         << " " << show_as_hex (H5 + f) << endl;
                cout << "H6: " << show_as_hex (H6) << " + " << show_as_hex (g)
                         << " " << show_as_hex (H6 + g) << endl;
                cout << "H7: " << show_as_hex (H7) << " + " << show_as_hex (h)
                         << " " << show_as_hex (H7 + h) << endl;
        }

        // Add up all the working variables to each hash and make sure we are still
        // working with solely 32 bit variables.
        H0 = (H0 + a) & 0xFFFFFFFF;
        H1 = (H1 + b) & 0xFFFFFFFF;
        H2 = (H2 + c) & 0xFFFFFFFF;
        H3 = (H3 + d) & 0xFFFFFFFF;
        H4 = (H4 + e) & 0xFFFFFFFF;
        H5 = (H5 + f) & 0xFFFFFFFF;
        H6 = (H6 + g) & 0xFFFFFFFF;
        H7 = (H7 + h) & 0xFFFFFFFF;

        // Append the hash segments together one after the other to get the full
        // 256 bit hash.
        return show_as_hex(H0) + show_as_hex(H1) + show_as_hex(H2) +
                   show_as_hex(H3) + show_as_hex(H4) + show_as_hex(H5) +
                   show_as_hex(H6) + show_as_hex(H7);
}
