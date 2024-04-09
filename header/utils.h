#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fmt/core.h>
#include <iostream>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include "LowMC.h"
#include "database_constants.h"
#include "seal/seal.h"

typedef  std::vector<seal::Ciphertext> PIRQuery;
typedef  seal::Ciphertext PIRResponse;
typedef  std::vector<seal::Ciphertext> PIRResponseList;
typedef  std::vector<uint64_t> Row;
typedef  std::vector<Row> PirDB;
namespace utils {

typedef  std::vector<rawdatablock>  RawDB;
typedef  std::vector<std::bitset<datablock_size>>  EncodedDB;
typedef  std::vector<std::bitset<datablock_size>>  RawResponses;
using namespace std;
using namespace seal;
using namespace DatabaseConstants;


    // Returns the next power of 2 for a given number
    inline size_t next_power_of_two(size_t n) {
        return pow(2, ceil(log2(n)));
    }

    // Generates a random number between 0 and max_value
    inline uint32_t generate_random_number(uint32_t max_value) {
        return rand() % (max_value + 1);
    }


    // Prints an error message and exits the program with an error code
    inline void check(bool condition, const std::string& error_message="Encounter errors!") {
        if (!condition) {
            throw std::runtime_error(error_message);
        }
    }

    // Prints an error message and exits the program with an error code
    inline void error_exit(const std::string& error_message, int error_code = 1) {
        std::cerr << "Error: " << error_message << std::endl;
        exit(error_code);
    }

    // Prints a message to the console
    inline void print_message(const std::string& message) {
        std::cout << message << std::endl;
    }


    inline std::vector<uint64_t> rotate_vector_row(std::vector<uint64_t>& vec, int rotation_Amount) {
        if (vec.empty()) {
            return {};
        }

        const size_t row_size = vec.size()/2;
        rotation_Amount = rotation_Amount % row_size;

        std::vector<uint64_t> temp(vec.size(), 0ULL);
        for (size_t i = 0; i < row_size; ++i) {
            temp[(i + rotation_Amount) % row_size] = vec[i];
            temp[(i + rotation_Amount) % row_size + row_size] = vec[i + row_size];
        }
        return temp;
    }

    inline std::vector<uint64_t> rotate_vector_col(std::vector<uint64_t>& vec) {
        if (vec.empty()) {
            return {};
        }

        const size_t row_size = vec.size()/2;
        
        uint64_t tmp_slot = 0;
        for (size_t i = 0; i < row_size; ++i) {
            tmp_slot = vec[i];
            vec[i] = vec[row_size + i];
            vec[row_size + i] = tmp_slot;
        }
      
    return vec;
    }
    
    inline std::size_t hash_mod(size_t id, size_t nonce, size_t data, size_t total_buckets){
        std::hash<std::string> hasher1;
        return hasher1(std::to_string(id) + std::to_string(nonce) + std::to_string(data)) % total_buckets;
    }

    inline std::vector<size_t> get_candidate_buckets(size_t data, size_t num_candidates , size_t total_buckets){
        std::vector<size_t> candidate_buckets;
         
        for (int i = 0; i < num_candidates; i++){
            size_t nonce = 0;
            auto bucket = hash_mod( i, nonce, data, total_buckets);
            while (std::find(candidate_buckets.begin(), candidate_buckets.end(), bucket) != candidate_buckets.end()){
                nonce += 1;
                bucket = hash_mod( i, nonce, data, total_buckets);
            }
            candidate_buckets.push_back(bucket);
        }

        return candidate_buckets;
    }

    template< size_t size = blocksize>
    typename std::bitset<size> random_bitset( double p = 0.5) {
        typename std::bitset<size> bits;
        std::random_device rd;
        std::mt19937 gen( rd());
        std::bernoulli_distribution d( p);

        for( int n = 0; n < size; ++n) {
            bits[ n] = d( gen);
        }

        return bits;
    }
    
    template< size_t size1, size_t size2>
    inline typename std::bitset<size1+size2> concatenate(std::bitset<size1> b1, std::bitset<size2> b2) {
        return std::bitset<size1+size2>(b1.to_string() + b2.to_string());
    }

    
    template< size_t psize=prefixsize, size_t osize=blocksize>
    inline typename std::bitset<psize> get_prefix(std::bitset<osize> b) {
        return std::bitset<psize>(b.to_string().substr(0, psize));
    }
    
    template< size_t ssize=prefixsize, size_t bsize = blocksize>
    inline auto split(bitset<bsize> b) {
        string str_repr = b.to_string();
        return make_pair(std::bitset<ssize>(str_repr.substr(0, ssize)), std::bitset<bsize-ssize>(str_repr.substr(ssize)));
    }

    void timing_start(string prefix);

    void timing_end(string prefix);

    uint64_t choose(uint64_t n, uint64_t k);
    std::vector<uint64_t> get_perfect_constant_weight_codeword(uint64_t __number, uint64_t encoding_size, uint64_t hamming_weight, bool __verbose=true);

    inline void append_non_collide_output(std::string hash_out, size_t mod, std::vector<size_t>& candidates) {
        for (int split = 0; split < hash_out.size() - 10; split++) {
            size_t idx = std::stoull(hash_out.substr(0, hash_out.size() - split), nullptr, 2) % mod;
            if (std::find(candidates.begin(), candidates.end(), idx) == candidates.end()) {
                candidates.emplace_back(idx);
                return;
            }
        }
        throw std::runtime_error("Error: candidate failed. ");
    }

    bool cuckoo_insert(uint64_t key, size_t attempt, std::unordered_map<uint64_t, std::vector<size_t>>& key_to_buckets, std::unordered_map<uint64_t, uint64_t> &bucket_to_key);
    
    inline void get_candidates_with_hash_values (size_t total_buckets, size_t bucket_size, std::vector<string>& ciphertexts, std::vector<size_t>& candidate_buckets, std::vector<size_t>& candidate_position) {
        for (auto& ciphertext: ciphertexts) {
            append_non_collide_output(ciphertext.substr(0, DatabaseConstants::BucketHashLength), total_buckets, candidate_buckets);
            append_non_collide_output(ciphertext.substr(DatabaseConstants::BucketHashLength), bucket_size, candidate_position);
        }
    }
    
    inline void multiply_acum(uint64_t op1, uint64_t op2, __uint128_t& product_acum) {
        product_acum = product_acum + static_cast<__uint128_t>(op1) * static_cast<__uint128_t>(op2); 
    }

    inline seal::EncryptionParameters create_encryption_parameters(string selection = "", BatchPirType type = PIRANA, bool verbose = false)
    {
        seal::EncryptionParameters seal_params(seal::scheme_type::bfv);

        // Generally this parameter selection will work
        // smaller p & bigger q -> higher depth
        int PlaintextModBitss = 22;
        vector<int> CoeffMods = vector<int>{55, 55, 48, 60};
        if (type == PIRANA)
            switch (LUT_OUTPUT_SIZE) {
                case 16:
                    PlaintextModBitss = LUT_OUTPUT_SIZE + 1;
                    CoeffMods = vector<int>{45, 45, 50};
                    break;
                case 18: 
                    PlaintextModBitss = LUT_OUTPUT_SIZE + 2;
                    CoeffMods = vector<int>{45, 45, 50};
                    break;
                case 20:
                    PlaintextModBitss = LUT_OUTPUT_SIZE + 1;
                    CoeffMods = vector<int>{45, 45, 50};
                    break;
                case 22:
                    PlaintextModBitss = LUT_OUTPUT_SIZE + 1;
                    CoeffMods = vector<int>{50, 50, 50};
                    break;
                case 24:
                    PlaintextModBitss = LUT_OUTPUT_SIZE + 1;
                    CoeffMods = vector<int>{55, 55, 55};
                    break;
                default:
                    throw std::runtime_error("Error: LUT_OUTPUT_SIZE not supported");
            }

        seal_params.set_poly_modulus_degree(PolyDegree);

        seal_params.set_coeff_modulus(CoeffModulus::Create(PolyDegree, CoeffMods));
        seal_params.set_plain_modulus(PlainModulus::Batching(PolyDegree, PlaintextModBitss));

        if (verbose) {
            std::cout << "+---------------------------------------------------+" << std::endl;
            std::cout << "|               ENCRYPTION PARAMETERS               |" << std::endl;
            std::cout << "+---------------------------------------------------+" << std::endl;
            std::cout << "|  seal_params_.poly_modulus_degree  = " << seal_params.poly_modulus_degree() << std::endl;

            auto coeff_modulus_size = seal_params.coeff_modulus().size();
            std::cout << "|  seal_params_.coeff_modulus().bit_count   = [";

            for (std::size_t i = 0; i < coeff_modulus_size - 1; i++)
            {
                std::cout << seal_params.coeff_modulus()[i].bit_count() << " + ";
            }

            std::cout << seal_params.coeff_modulus().back().bit_count();
            std::cout << "] bits" << std::endl;
            std::cout << "|  seal_params_.coeff_modulus().size = " << seal_params.coeff_modulus().size() << std::endl;
            std::cout << "|  seal_params_.plain_modulus().bit_count = " << seal_params.plain_modulus().bit_count() << std::endl;
            std::cout << "+---------------------------------------------------+" << std::endl;
        }
    return seal_params;
    }

    inline void multiply_poly_acum(const uint64_t *ct_ptr, const uint64_t *pt_ptr, size_t size, uint128_t *result) {
        for (int cc = 0; cc < size; cc += 32) {
            multiply_acum(ct_ptr[cc], pt_ptr[cc], result[cc]);
            multiply_acum(ct_ptr[cc + 1], pt_ptr[cc + 1], result[cc + 1]);
            multiply_acum(ct_ptr[cc + 2], pt_ptr[cc + 2], result[cc + 2]);
            multiply_acum(ct_ptr[cc + 3], pt_ptr[cc + 3], result[cc + 3]);
            multiply_acum(ct_ptr[cc + 4], pt_ptr[cc + 4], result[cc + 4]);
            multiply_acum(ct_ptr[cc + 5], pt_ptr[cc + 5], result[cc + 5]);
            multiply_acum(ct_ptr[cc + 6], pt_ptr[cc + 6], result[cc + 6]);
            multiply_acum(ct_ptr[cc + 7], pt_ptr[cc + 7], result[cc + 7]);
            multiply_acum(ct_ptr[cc + 8], pt_ptr[cc + 8], result[cc + 8]);
            multiply_acum(ct_ptr[cc + 9], pt_ptr[cc + 9], result[cc + 9]);
            multiply_acum(ct_ptr[cc + 10], pt_ptr[cc + 10], result[cc + 10]);
            multiply_acum(ct_ptr[cc + 11], pt_ptr[cc + 11], result[cc + 11]);
            multiply_acum(ct_ptr[cc + 12], pt_ptr[cc + 12], result[cc + 12]);
            multiply_acum(ct_ptr[cc + 13], pt_ptr[cc + 13], result[cc + 13]);
            multiply_acum(ct_ptr[cc + 14], pt_ptr[cc + 14], result[cc + 14]);
            multiply_acum(ct_ptr[cc + 15], pt_ptr[cc + 15], result[cc + 15]);
            multiply_acum(ct_ptr[cc + 16], pt_ptr[cc + 16], result[cc + 16]);
            multiply_acum(ct_ptr[cc + 17], pt_ptr[cc + 17], result[cc + 17]);
            multiply_acum(ct_ptr[cc + 18], pt_ptr[cc + 18], result[cc + 18]);
            multiply_acum(ct_ptr[cc + 19], pt_ptr[cc + 19], result[cc + 19]);
            multiply_acum(ct_ptr[cc + 20], pt_ptr[cc + 20], result[cc + 20]);
            multiply_acum(ct_ptr[cc + 21], pt_ptr[cc + 21], result[cc + 21]);
            multiply_acum(ct_ptr[cc + 22], pt_ptr[cc + 22], result[cc + 22]);
            multiply_acum(ct_ptr[cc + 23], pt_ptr[cc + 23], result[cc + 23]);
            multiply_acum(ct_ptr[cc + 24], pt_ptr[cc + 24], result[cc + 24]);
            multiply_acum(ct_ptr[cc + 25], pt_ptr[cc + 25], result[cc + 25]);
            multiply_acum(ct_ptr[cc + 26], pt_ptr[cc + 26], result[cc + 26]);
            multiply_acum(ct_ptr[cc + 27], pt_ptr[cc + 27], result[cc + 27]);
            multiply_acum(ct_ptr[cc + 28], pt_ptr[cc + 28], result[cc + 28]);
            multiply_acum(ct_ptr[cc + 29], pt_ptr[cc + 29], result[cc + 29]);
            multiply_acum(ct_ptr[cc + 30], pt_ptr[cc + 30], result[cc + 30]);
            multiply_acum(ct_ptr[cc + 31], pt_ptr[cc + 31], result[cc + 31]);
            
        }
    }



} // namespace utils

#endif // UTILS_H
