// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "examples.h"

using namespace std;
using namespace seal;

/*
Both the BFV and BGV schemes (with BatchEncoder) as well as the CKKS scheme support
native vectorized computations on encrypted numbers. In addition to computing slot-wise,
it is possible to rotate the encrypted vectors cyclically.

Simply changing `scheme_type::bfv` to `scheme_type::bgv` will make this example work for
the BGV scheme.
*/
void example_rotation_bfv_mp()
{
   print_example_banner("Example: Multiparty Rotation / Rotation in BFV");


   EncryptionParameters parms(scheme_type::bfv);


   size_t poly_modulus_degree = 4096;
   parms.set_poly_modulus_degree(poly_modulus_degree);
   parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
   parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
   // cout << PlainModulus::Batching(poly_modulus_degree, 20)<< endl;
   SEALContext context(parms);
   print_parameters(context);
   cout << endl;






   //Generate common public keys


   KeyGenerator keygen(context);
   int party_num = 2;
   vector<SecretKey> SKS(party_num);
   vector<PublicKey> PKS(party_num);
   keygen.gen_secret_key(SKS[0]);
   keygen.create_public_key_with_sk(PKS[0],SKS[0]);
   for (int i = 1; i <party_num; ++i) {
       keygen.gen_secret_key(SKS[i]);
       keygen.create_public_key_with_same_c1(PKS[0],PKS[i],SKS[i]);
   }
   PublicKey CPK;
   // SecretKey CSK;
   keygen.create_common_public_key(CPK,PKS,party_num);


   // Target public key and secret key TPK, TSK
   SecretKey TSK;
   PublicKey TPK;
   keygen.gen_secret_key(TSK);
   keygen.create_public_key_with_sk(TPK, TSK);


   // keygen.create_common_secret_key(CSK,SKS,party_num);
   cout <<"Generate CPK, SKS"<< endl;




   for (int i = 0; i <party_num; ++i) {
       PKS[i].data().release();
   }






   // SecretKey secret_key = keygen.secret_key();
   // PublicKey public_key;
   // keygen.create_public_key(public_key);
   // RelinKeys relin_keys;
   // keygen.create_relin_keys(relin_keys);
   // Encryptor encryptor(context, public_key);
   // Evaluator evaluator(context);
   // Decryptor decryptor(context, secret_key);


   //Set encryptor & evaluator




   // cout << "    + Noise budget in fresh encryption: " << decryptor.invariant_noise_budget(encrypted_matrix) << " bits"
   //      << endl;
   // cout << endl;


   /*
   Rotations require yet another type of special key called `Galois keys'. These
   are easily obtained from the KeyGenerator.
   */
   // GaloisKeys galois_keys;
   // keygen.create_galois_keys(galois_keys);




   vector<GaloisKeys> galois_keys_set(party_num);
   keygen.create_galois_keys_with_sk(galois_keys_set[0],SKS[0]);
   cout <<"Generate other galois key"<< endl;
   for (int i = 1; i <party_num; ++i) {
       keygen.create_galois_keys_with_sk_c1(galois_keys_set[i],SKS[i],galois_keys_set[0]);
   }
   // cout <<"Generate galois key"<< endl;
   GaloisKeys cRotKeys;
   keygen.gen_common_galois_keys(galois_keys_set,party_num,cRotKeys);
   // for (int i = 0; i <party_num; ++i) {
   //     SKS[i].data().release();
   // }
   cout <<"Generate common galois key"<< endl;
    vector<RelinKeys> relin_keys_round_one_set(party_num);
    vector<RelinKeys> relin_keys_round_two_set(party_num);

    cout <<"Generate Relin keys Round One"<< endl;
    for (int i = 0; i <party_num; ++i) {
        keygen.gen_relin_keys_round_one(SKS[i], relin_keys_round_one_set[i]);
    }

    RelinKeys relin_keys_round_one;
    keygen.gen_agg_relin_keys_round_one(relin_keys_round_one_set, party_num, relin_keys_round_one);

    cout <<"Generate Relin keys Round Two"<< endl;
    for (int i = 0; i <party_num; ++i) {
        keygen.gen_relin_keys_round_two(relin_keys_round_one, relin_keys_round_two_set[i]);
    }

    cout <<"Generate common Relin keys"<< endl;
    RelinKeys cRelinKeys;
    keygen.gen_comm_relin_keys(relin_keys_round_two_set, relin_keys_round_one, party_num, cRelinKeys);


   Encryptor encryptor(context, CPK);
   Evaluator evaluator(context);


   BatchEncoder batch_encoder(context);
   size_t slot_count = batch_encoder.slot_count();
   size_t row_size = slot_count / 2;
   cout << "Plaintext matrix row size: " << row_size << endl;


   vector<uint64_t> pod_matrix(slot_count, 0ULL);
   pod_matrix[0] = 0ULL;
   pod_matrix[1] = 1ULL;
   pod_matrix[2] = 2ULL;
   pod_matrix[3] = 3ULL;


   cout << "Input plaintext matrix:" << endl;
   print_matrix(pod_matrix, row_size);


   /*
   First we use BatchEncoder to encode the matrix into a plaintext. We encrypt
   the plaintext as usual.
   */
   Plaintext plain_matrix;
   print_line(__LINE__);
   cout << "Encode and encrypt." << endl;
   batch_encoder.encode(pod_matrix, plain_matrix);
   Ciphertext encrypted_matrix;
   encryptor.encrypt(plain_matrix, encrypted_matrix);
   // /*
   // Now rotate both matrix rows 3 steps to the left, decrypt, decode, and print.
   // */
   print_line(__LINE__);
   cout << "Rotate rows 3 steps left." << endl;
   // evaluator.rotate_rows_inplace(encrypted_matrix, 3, galois_keys);
   evaluator.rotate_rows_inplace(encrypted_matrix, 3, galois_keys_set[0]);
//    vector<Decryptor> decryptor_set;
//    for (int i = 0; i < party_num; ++i) {
//        decryptor_set.emplace_back(context, SKS[i]);
//    }

 


   // Aggregate partial decryptions to obtain the final plaintext.
   // aggregator.aggregate_partial_decryption(encrypted_matrix, partial_encrypted, aggregated_plain, party_num);


   // Decode the aggregated plaintext
//    vector<uint64_t> result;
//    batch_encoder.decode(plain_matrix, result);
//    print_matrix(pod_matrix, row_size);


   /*
   We can also rotate the columns, i.e., swap the rows.
   */
   // print_line(__LINE__);
   // cout << "Rotate columns." << endl;
   // evaluator.rotate_columns_inplace(encrypted_matrix, galois_keys);
   // cout << "    + Noise budget after rotation: " << decryptor.invariant_noise_budget(encrypted_matrix) << " bits"
   //      << endl;
   // cout << "    + Decrypt and decode ...... Correct." << endl;
   // decryptor.decrypt(encrypted_matrix, plain_result);
   // batch_encoder.decode(plain_result, pod_matrix);
   // print_matrix(pod_matrix, row_size);


   /*
   Finally, we rotate the rows 4 steps to the right, decrypt, decode, and print.
   */
   // print_line(__LINE__);
   // cout << "Rotate rows 4 steps right." << endl;
   // evaluator.rotate_rows_inplace(encrypted_matrix, -4, galois_keys);
   // cout << "    + Noise budget after rotation: " << decryptor.invariant_noise_budget(encrypted_matrix) << " bits"
   //      << endl;
   // cout << "    + Decrypt and decode ...... Correct." << endl;
   // decryptor.decrypt(encrypted_matrix, plain_result);
   // batch_encoder.decode(plain_result, pod_matrix);
   // print_matrix(pod_matrix, row_size);


   /*
   Note that rotations do not consume any noise budget. However, this is only
   the case when the special prime is at least as large as the other primes. The
   same holds for relinearization. Microsoft SEAL does not require that the
   special prime is of any particular size, so ensuring this is the case is left
   for the user to do.
   */
}

void example_rotation_bfv()
{
    print_example_banner("Example: Rotation / Rotation in BFV");

    EncryptionParameters parms(scheme_type::bfv);

    size_t poly_modulus_degree = 8192;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));

    SEALContext context(parms);
    print_parameters(context);
    cout << endl;

    KeyGenerator keygen(context);
    SecretKey secret_key = keygen.secret_key();
    PublicKey public_key;
    keygen.create_public_key(public_key);
    RelinKeys relin_keys;
    keygen.create_relin_keys(relin_keys);
    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);

    BatchEncoder batch_encoder(context);
    size_t slot_count = batch_encoder.slot_count();
    size_t row_size = slot_count / 2;
    cout << "Plaintext matrix row size: " << row_size << endl;

    vector<uint64_t> pod_matrix(slot_count, 0ULL);
    pod_matrix[0] = 0ULL;
    pod_matrix[1] = 1ULL;
    pod_matrix[2] = 2ULL;
    pod_matrix[3] = 3ULL;
    pod_matrix[row_size] = 4ULL;
    pod_matrix[row_size + 1] = 5ULL;
    pod_matrix[row_size + 2] = 6ULL;
    pod_matrix[row_size + 3] = 7ULL;

    cout << "Input plaintext matrix:" << endl;
    print_matrix(pod_matrix, row_size);

    /*
    First we use BatchEncoder to encode the matrix into a plaintext. We encrypt
    the plaintext as usual.
    */
    Plaintext plain_matrix;
    print_line(__LINE__);
    cout << "Encode and encrypt." << endl;
    batch_encoder.encode(pod_matrix, plain_matrix);
    Ciphertext encrypted_matrix;
    encryptor.encrypt(plain_matrix, encrypted_matrix);
    cout << "    + Noise budget in fresh encryption: " << decryptor.invariant_noise_budget(encrypted_matrix) << " bits"
         << endl;
    cout << endl;

    /*
    Rotations require yet another type of special key called `Galois keys'. These
    are easily obtained from the KeyGenerator.
    */
    GaloisKeys galois_keys;
    keygen.create_galois_keys(galois_keys);

    /*
    Now rotate both matrix rows 3 steps to the left, decrypt, decode, and print.
    */
    print_line(__LINE__);
    cout << "Rotate rows 3 steps left." << endl;
    evaluator.rotate_rows_inplace(encrypted_matrix, 3, galois_keys);
    Plaintext plain_result;
    cout << "    + Noise budget after rotation: " << decryptor.invariant_noise_budget(encrypted_matrix) << " bits"
         << endl;
    cout << "    + Decrypt and decode ...... Correct." << endl;
    decryptor.decrypt(encrypted_matrix, plain_result);
    batch_encoder.decode(plain_result, pod_matrix);
    print_matrix(pod_matrix, row_size);
}


void example_rotation_ckks()
{
    print_example_banner("Example: Rotation / Rotation in CKKS");

    /*
    Rotations in the CKKS scheme work very similarly to rotations in BFV.
    */
    EncryptionParameters parms(scheme_type::ckks);

    size_t poly_modulus_degree = 8192;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 40, 40, 40, 40, 40 }));

    SEALContext context(parms);
    print_parameters(context);
    cout << endl;

    KeyGenerator keygen(context);
    SecretKey secret_key = keygen.secret_key();
    PublicKey public_key;
    keygen.create_public_key(public_key);
    RelinKeys relin_keys;
    keygen.create_relin_keys(relin_keys);
    GaloisKeys galois_keys;
    keygen.create_galois_keys(galois_keys);
    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);

    CKKSEncoder ckks_encoder(context);

    size_t slot_count = ckks_encoder.slot_count();
    cout << "Number of slots: " << slot_count << endl;
    vector<double> input;
    input.reserve(slot_count);
    double curr_point = 0;
    double step_size = 1.0 / (static_cast<double>(slot_count) - 1);
    for (size_t i = 0; i < slot_count; i++, curr_point += step_size)
    {
        input.push_back(curr_point);
    }
    cout << "Input vector:" << endl;
    print_vector(input, 3, 7);

    auto scale = pow(2.0, 50);

    print_line(__LINE__);
    cout << "Encode and encrypt." << endl;
    Plaintext plain;
    ckks_encoder.encode(input, scale, plain);
    Ciphertext encrypted;
    encryptor.encrypt(plain, encrypted);

    Ciphertext rotated;
    print_line(__LINE__);
    cout << "Rotate 2 steps left." << endl;
    evaluator.rotate_vector(encrypted, 2, galois_keys, rotated);
    cout << "    + Decrypt and decode ...... Correct." << endl;
    decryptor.decrypt(rotated, plain);
    vector<double> result;
    ckks_encoder.decode(plain, result);
    print_vector(result, 3, 7);

    /*
    With the CKKS scheme it is also possible to evaluate a complex conjugation on
    a vector of encrypted complex numbers, using Evaluator::complex_conjugate.
    This is in fact a kind of rotation, and requires also Galois keys.
    */
}

void example_rotation()
{
    print_example_banner("Example: Rotation");

    /*
    Run all rotation examples.
    */
    example_rotation_bfv_mp();
    // example_rotation_bfv();
    // example_rotation_ckks();
}
