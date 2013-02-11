#ifndef OPENSSL_CRYPT_H
#define OPENSSL_CRYPT_H

struct Npcrypt;

struct Npcrypt* new_openssl_crypt(const char* cipher_name, const char* key, int key_length);

#endif
