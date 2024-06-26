#ifndef __LowMC_h__
#define __LowMC_h__

#include "database_constants.h"
#include <bitset>
#include <vector>

namespace utils {

const unsigned numofboxes = 8;    // Number of Sboxes
const unsigned blocksize = 64;   // Block size in bits
const unsigned keysize = 128; // Key size in bits
const unsigned rounds = 24; // Number of rounds

const unsigned prefixsize = blocksize-DatabaseConstants::InputLength; 
const unsigned datablock_size = DatabaseConstants::InputLength + DatabaseConstants::OutputLength;

const unsigned identitysize = blocksize - 3*numofboxes;
                  // Size of the identity part in the Sbox layer

typedef std::bitset<blocksize> block; // Store messages and states
typedef std::bitset<keysize> keyblock;
typedef std::bitset<datablock_size>  datablock;
typedef std::bitset<prefixsize> prefixblock;
typedef std::bitset<DatabaseConstants::InputLength> rawinputblock;
typedef std::bitset<DatabaseConstants::OutputLength> rawdatablock;

class LowMC {
public:
    LowMC (keyblock k, prefixblock prefix) {
        key = k;
        prefix_ = prefix;
        instantiate_LowMC();
        keyschedule();
    };

    block encrypt (const block message);
    block decrypt (const block message);
    void set_key (keyblock k);
    prefixblock get_prefix () {return prefix_; }

    void print_matrices();
    
private:
// LowMC private data members //
    // The Sbox and its inverse    
    const std::vector<unsigned> Sbox =
        {0x00, 0x01, 0x03, 0x06, 0x07, 0x04, 0x05, 0x02};
    const std::vector<unsigned> invSbox =
        {0x00, 0x01, 0x07, 0x02, 0x05, 0x06, 0x03, 0x04};
    std::vector<std::vector<block>> LinMatrices;
        // Stores the binary matrices for each round
    std::vector<std::vector<block>> invLinMatrices;
        // Stores the inverses of LinMatrices
    std::vector<block> roundconstants;
        // Stores the round constants
    keyblock key = 0;
        //Stores the master key
    prefixblock prefix_ = 0;
        //Stores the prefix
    std::vector<std::vector<keyblock>> KeyMatrices;
        // Stores the matrices that generate the round keys
    std::vector<block> roundkeys;
        // Stores the round keys

    
// LowMC private functions //
    block Substitution (const block message);
        // The substitution layer
    block invSubstitution (const block message);
        // The inverse substitution layer

    block MultiplyWithGF2Matrix
        (const std::vector<block> matrix, const block message);    
        // For the linear layer
    block MultiplyWithGF2Matrix_Key
        (const std::vector<keyblock> matrix, const keyblock k);
        // For generating the round keys

    void keyschedule ();
        //Creates the round keys from the master key

    void instantiate_LowMC ();
        //Fills the matrices and roundconstants with pseudorandom bits 
   
// Binary matrix functions //   
    unsigned rank_of_Matrix (const std::vector<block> matrix);
    unsigned rank_of_Matrix_Key (const std::vector<keyblock> matrix);
    std::vector<block> invert_Matrix (const std::vector<block> matrix);

// Random bits functions //
    block getrandblock ();
    keyblock getrandkeyblock ();
    bool  getrandbit ();

};
} // namespace utils

#endif
