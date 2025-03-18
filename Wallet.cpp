/*-- Wallet.cpp ------------------------------------------------------------
   This file implements Wallet member functions.
-------------------------------------------------------------------------*/
#include "Wallet.h"

Wallet::Wallet() {

    /* Generated Key Pairs */
    locktimeUTXO = 5;
    versionUTXO = 1.0;
    keyPair = generateECDSAKeyPair();
    get_key_values(keyPair);
    std::vector<uint8_t> test;
    test = utility.shaHash((unsigned char*)"Hello!");
    ecDoSign(keyPair, test);
    //address
    genAddress(pubKey);
}

EVP_PKEY* Wallet::generateECDSAKeyPair() {
    /*
     * The libctx and propq can be set if required, they are included here
     * to show how they are passed to EVP_PKEY_CTX_new_from_name().
     */
    OSSL_LIB_CTX* libctx = NULL;
    const char* propq = NULL;
    EVP_PKEY_CTX* genctx = NULL;
    EVP_PKEY* key = NULL;
    OSSL_PARAM params[3];
    const char* curvename = "P-256";
    int use_cofactordh = 1;

    genctx = EVP_PKEY_CTX_new_from_name(libctx, "EC", propq);
    if (genctx == NULL) {
        std::cout << "EVP_PKEY_CTX_new_from_name() failed\n";
        goto cleanup;
    }

    if (EVP_PKEY_keygen_init(genctx) <= 0) {
        std::cout << "EVP_PKEY_keygen_init() failed\n";
        goto cleanup;
    }

    params[0] = OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME,
        (char*)curvename, 0);
    /*
     * This is an optional parameter.
     * For many curves where the cofactor is 1, setting this has no effect.
     */
    params[1] = OSSL_PARAM_construct_int(OSSL_PKEY_PARAM_USE_COFACTOR_ECDH,
        &use_cofactordh);
    params[2] = OSSL_PARAM_construct_end();
    if (!EVP_PKEY_CTX_set_params(genctx, params)) {
        std::cout << "EVP_PKEY_CTX_set_params() failed\n";
        goto cleanup;
    }

    if (EVP_PKEY_generate(genctx, &key) <= 0) {
        std::cout << "EVP_PKEY_generate() failed\n";
        goto cleanup;
    }
cleanup:
    EVP_PKEY_CTX_free(genctx);
    return key;
}

/*
 * The following code shows how retrieve key data from the generated
 * EC key. See doc/man7/EVP_PKEY-EC.pod for more information.
 *
 * EVP_PKEY_print_private() could also be used to display the values.
 */
int Wallet::get_key_values(EVP_PKEY* pkey) {
    int result = 0;
    size_t curve_name_len = 0;
    char* out_curvename = nullptr;
    BIGNUM* out_priv = NULL;
    size_t out_pubkey_len = 0, out_privkey_len = 0;

    /* Get Curve Name Length */
    if (!EVP_PKEY_get_utf8_string_param(pkey, OSSL_PKEY_PARAM_GROUP_NAME, NULL, 0, &curve_name_len)) {
        std::cout << "Failed to get curve name length\n";
        goto cleanup;
    }

    /* Get Curve Name */
    out_curvename = new char[curve_name_len + 1];
    if (!EVP_PKEY_get_utf8_string_param(pkey, OSSL_PKEY_PARAM_GROUP_NAME,
        out_curvename, curve_name_len + 1, NULL)) {
        std::cout << "Failed to get curve name\n";
        goto cleanup;
    }

    /* Get Public Key Length */
    if (!EVP_PKEY_get_octet_string_param(pkey, OSSL_PKEY_PARAM_PUB_KEY, NULL, 0, &out_pubkey_len)) {
        std::cout << "Failed to get public key length\n";
        result = -1;
        goto cleanup;
    }

    /* Get Pub Key */
    pubKey = new unsigned char[out_pubkey_len];
    if (!EVP_PKEY_get_octet_string_param(pkey, OSSL_PKEY_PARAM_PUB_KEY, pubKey, out_pubkey_len, &out_pubkey_len)) {
        std::cout << "Failed to get public key\n";
        result = -1;
        goto cleanup;
    }

    /* Get Priv Key */
    if (!EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_PRIV_KEY, &out_priv) || out_priv == nullptr) {
        std::cout << "Failed to get private key\n";
        result = -1;
        goto cleanup;
    }

    /* Get Private Key Length */
    out_privkey_len = BN_num_bytes(out_priv);
    if (out_privkey_len <= 0) {
        std::cout << "Invalid private key length\n";
        result = -1;
        goto cleanup;
    }

    /* Priv Key to unsigned char* */
    privKey = new unsigned char[out_privkey_len];
    if (BN_bn2bin(out_priv, privKey) <= 0) {
        std::cout << "BN_bn2bin failed\n";
        result = -1;
        goto cleanup;
    }

    std::cout << "Curve name:\n" << out_curvename << std::endl;
    std::cout << "Public key: ";
    std::cout << utility.toString(pubKey) << std::endl;
    std::cout << "Private Key: ";
    std::cout << utility.toString(privKey) << std::endl;

    result = 1;
cleanup:
    /* Zeroize the private key data when we free it */
    BN_clear_free(out_priv);
    return result;
}


/* A function that takes a keypair and a message digest and signs it */
unsigned char* Wallet::ecDoSign(EVP_PKEY* keypair, const std::vector<uint8_t>& mesdgst) {
    /* Method Variables */
    unsigned char* sig = nullptr;

    /* Convert the message digest vector to a byte array */
    const unsigned char* md = mesdgst.data();
    size_t mdlen = mesdgst.size();

    /* Create a digest sign context */
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (ctx == nullptr) {
        std::cerr << "ctx creation failed\n";
        return 0;
    }

    /* Initialize the digest sign operation */
    if (EVP_DigestSignInit(ctx, nullptr, nullptr, nullptr, keypair) <= 0) {
        std::cerr << "sign_init Failed\n";
        EVP_MD_CTX_free(ctx);
        return 0;
    }

    /* Update the digest sign operation with the message digest */
    if (EVP_DigestSignUpdate(ctx, md, mdlen) <= 0) {
        std::cerr << "sign_update Failed\n";
        EVP_MD_CTX_free(ctx);
        return 0;
    }

    /* Finalize the digest sign operation and get the signature length */
    size_t siglen;
    if (EVP_DigestSignFinal(ctx, nullptr, &siglen) <= 0) {
        std::cerr << "sign_final Failed\n";
        EVP_MD_CTX_free(ctx);
        return 0;
    }

    /* Allocate memory for the signature */
    sig = static_cast<unsigned char*>(OPENSSL_malloc(siglen));
    if (sig == nullptr) {
        std::cerr << "Memory allocation failed for sig\n";
        EVP_MD_CTX_free(ctx);
        return 0;
    }

    /* Get the signature */
    if (EVP_DigestSignFinal(ctx, sig, &siglen) <= 0) {
        std::cerr << "sign_final Failed\n";
        OPENSSL_free(sig);
        EVP_MD_CTX_free(ctx);
        return 0;
    }

    /* Now, you can use the 'sig' array as needed */
    return sig;

    /* Clean up memory */
    OPENSSL_free(sig);
    EVP_MD_CTX_free(ctx);
}


/* A function that takes a keypair, a message digest, and a signature and verifies it */
bool Wallet::ecDoVerify(EVP_PKEY* pkey, const std::vector<uint8_t>& mesdgst, std::vector<unsigned char> signature) {
    /* Method Variables */
    unsigned char* sig = nullptr;
    unsigned long err = 0; // Variable to store the error code //TEMPP

    /* Convert the message digest and signature vectors to byte arrays */
    const unsigned char* md = mesdgst.data();
    size_t mdlen = mesdgst.size();
    sig = reinterpret_cast<unsigned char*>(signature.data());
    size_t siglen = signature.size();

    /* Create a digest verify context */
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (ctx == nullptr) {
        std::cerr << "ctx creation failed\n";
        return false;
    }

    /* Initialize the digest verify operation */
    if (EVP_DigestVerifyInit(ctx, nullptr, nullptr, nullptr, pkey) <= 0) {
        std::cerr << "verify_init Failed\n";
        EVP_MD_CTX_free(ctx);
        return false;
    }

    /* Update the digest verify operation with the message digest */
    if (EVP_DigestVerifyUpdate(ctx, md, mdlen) <= 0) {
        std::cerr << "verify_update Failed\n";
        EVP_MD_CTX_free(ctx);
        return false;
    }

    /* Finalize the digest verify operation and get the verification result */
    int ret = EVP_DigestVerifyFinal(ctx, sig, siglen);
    std::cout << "Verified: " << ret << std::endl;

    /* Clean up memory */
    EVP_MD_CTX_free(ctx);

    /* Return the verification result as a boolean */
    return bool(ret);
}

void Wallet::extract_public_key() {
    EVP_PKEY* pubKey = NULL;
    BIO* bio = BIO_new(BIO_s_mem());

    if (!bio) {
        fprintf(stderr, "Error creating BIO\n");
    }

    // Write the public key to the BIO
    if (PEM_write_bio_PUBKEY(bio, keyPair) != 1) {
        fprintf(stderr, "Error writing public key to BIO\n");
        BIO_free(bio);
    }

    // Read the public key from the BIO
    pubKeyP = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    if (!pubKeyP) {
        fprintf(stderr, "Error reading public key from BIO\n");
    }

    BIO_free(bio);
}

std::string Wallet::genAddress(const unsigned char* pubKey) {
    /* Hash the Public Key (SHA-256 and RIPEMD-160) */
    std::vector<uint8_t> publicKeyHash = utility.shaHash(utility.ripemd(pubKey));

    /* Step 3: Add a Version Byte(e.g., 0x00 for Bitcoin Mainnet) */
    uint8_t versionByte = 0x17;
    std::vector<uint8_t> extPubKeyHash = { versionByte };
    extPubKeyHash.insert(extPubKeyHash.end(), publicKeyHash.begin(), publicKeyHash.end());

    address = utility.base58_encode(extPubKeyHash.data(), extPubKeyHash.size());
    std::cout << "Wallet Address: " << address << std::endl;
    return address;
}

unsigned char* Wallet::calcTxid(unsigned long long ts) {
    /* Create a random number generator */
    std::random_device rd;  // Seed for random number generator
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<unsigned int> distrib(0, 4294967295); // Define the range

    /* Generate a random number */
    unsigned int random_number = distrib(gen);
    std::string randomn;
    /* Achieve Uniformity */
    if (random_number < 10) {
        randomn = "000000000" + utility.toString(random_number);
    }
    else if (random_number < 100) {
        randomn = "00000000" + utility.toString(random_number);
    }
    else if (random_number < 1000) {
        randomn = "0000000" + utility.toString(random_number);
    }
    else if (random_number < 10000) {
        randomn = "000000" + utility.toString(random_number);
    }
    else if (random_number < 100000) {
        randomn = "00000" + utility.toString(random_number);
    }

    std::string txid = ("0X0017" + randomn + utility.toString(ts));

    return utility.toUnsignedChar(txid);
}

utxout Wallet::outUTXO(double feee, std::vector<std::string> rwa, std::vector<EVP_PKEY*> rks,
    std::vector<double>amm) {
    /* Sructure UTXO */
    transactions utxo;
    utxo.setTimeStamp(utility.TimeStamp());
    utxo.setTxid(calcTxid(utxo.getTimeStamp()));
    utxo.setSendAddr(address);
    utxo.setRecieveAddr(rwa);
    utxo.setRecievePkeys(rks);
    utxo.setAmmount(amm);
    extract_public_key();
    utxo.setSendPkey(pubKeyP);
    utxo.setFee(feee);
    utxo.setLockTime(locktimeUTXO);
    utxo.setVersion(versionUTXO);

    /* Check for balance */
    setBalance();
    double bal = getBalance();
    double check = 0;
    std::vector<double> checkbal = utxo.getAmmount();
    for (int i = 0; i < checkbal.size(); i++) {
        check += checkbal[i];
    }
    check += feee;

    if (check < bal) {
        std::cout << "Ammount Exceeds Balance!\n";
        utxout failed;
        return failed;
    }

    /* Sign UTXO */
    unsigned char* utxoHash = utxo.serialize();
    std::vector<uint8_t> utxoHashed = utility.shaHash(utxoHash);
    unsigned char* utxoSignedHash = ecDoSign(keyPair, utxoHashed);

    /* Update UTXO */
    double utxoup = 0;
    for (int i = 0; i < UTXO.size(); i++) {
        std::vector<double> vecam = UTXO[i].getAmmount();
        for (int j = 0; j < UTXO[i].getAmmount().size(); j++) {
            utxoup += vecam[j];
        }

        if (utxoup <= check) {
            vecam.clear();
            vecam.push_back(check - utxoup);
            UTXO[i].setAmmount(vecam);
            break;
        }
        else {
            vecam.clear();
            vecam.push_back(0);
            check -= utxoup;
        }
    }
    setBalance();

    /* Send UTXO & Signed Message to mempool */
    utxout out;
    size_t utxo_size = utxo.getSize();;
    size_t sh_size = getSignSize(utxoHashed);

    out.txSize = utxo_size;
    out.shSize = sh_size;
    out.utxo = utxo.serialize();
    out.utxoSignedHash = utxoSignedHash;

    return out;
}

void Wallet::inUTXO(transactions txin) {
    UTXO.push_back(txin);
    setBalance();
}

bool Wallet::verifyTx(const utxout& out) { //incoomplete
    /* Declare variables */
    transactions utxo = utxo.deserialize(out.utxo);
    EVP_PKEY* sendpk = utxo.getSendPkey();

    /* Verify Hash */
    unsigned char* utxoHash = utxo.serialize();
    std::vector<uint8_t> utxoHashed = utility.shaHash(utxoHash);
    unsigned char* utxoSignedHash = ecDoSign(sendpk, utxoHashed);

    if (utxoSignedHash != out.utxoSignedHash) {
        std::cout << "UTXO INVALID!\n";
        return false;
    }

    /* Verify UTXO amount > zero */
    double totalbal = 0.0;
    std::vector<double> tempBal = utxo.getAmmount();
    for (int i = 0; i < tempBal.size(); i++) {
        totalbal += tempBal[i];
    }
    if (!totalbal > 0) {
        std::cout << "UTXO amount is invalid (must be greater than 0)!\n";
        return false;
    }

    return true;
}

void Wallet::setBalance() {
    double amm = 0;
    unsigned short txsize = UTXO.size();
    for (int i = 0; i < txsize; i++) {
        amm += UTXO[i].totalAmm();
    }

    balance = amm;
}

double Wallet::getBalance() {
    return balance;
}

std::string Wallet::getWalletAddr() {
    return address;
}

EVP_PKEY* Wallet::getPubKey() {
    if (pubKeyP == nullptr) {
        extract_public_key();
    }
    return pubKeyP;
}

unsigned char* Wallet::serialize_utxout(const utxout& obj) {

    /* Calculate sizes */
    size_t tSize = 0;

    tSize += sizeof(size_t) + sizeof(size_t) + sizeof(size_t) + obj.txSize + obj.shSize;

    unsigned char* buffer = new unsigned char[tSize];
    size_t offset = 0;

    /* serialize utxout */

    /* Serialize tSize itself */
    std::memcpy(buffer + offset, &tSize, sizeof(tSize));
    offset += sizeof(tSize);

    /* Serialize txSize itself */
    std::memcpy(buffer + offset, &obj.txSize, sizeof(size_t));
    offset += sizeof(size_t);

    /* Serialize shSize itself */
    std::memcpy(buffer + offset, &obj.shSize, sizeof(size_t));
    offset += sizeof(size_t);

    /* Serialize utxo itself */
    std::memcpy(buffer + offset, &obj.utxo, obj.txSize);
    offset += obj.txSize;

    /* Serialize utxo itself */
    std::memcpy(buffer + offset, &obj.utxoSignedHash, obj.shSize);
    offset += obj.shSize;

    return buffer;
}


utxout Wallet::deserialize_utxout(const unsigned char* buffer) {
    utxout obj;
    size_t offset = 0;

    /* Deserialize tSize (not used in this function, but read to advance the offset) */
    size_t tSize;
    std::memcpy(&tSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize txSize */
    std::memcpy(&obj.txSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize shSize */
    std::memcpy(&obj.shSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    /* Deserialize utxo */
    obj.utxo = new unsigned char[obj.txSize];
    std::memcpy(obj.utxo, buffer + offset, obj.txSize);
    offset += obj.txSize;

    /* Deserialize utxoSignedHash */
    obj.utxoSignedHash = new unsigned char[obj.shSize];
    std::memcpy(obj.utxoSignedHash, buffer + offset, obj.shSize);
    offset += obj.shSize;

    return obj;
}
