#include "openssl-crypt.h"
#include "openssl-func.h"
#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "npfs.h"

struct openssl_crypt_data {
	char* cipher_name;
	char* key;
	int key_length;

	EVP_CIPHER_CTX encrypt_ctx;
	EVP_CIPHER_CTX decrypt_ctx;
};

static void ocd_destroy(Npcrypt* crypt) {
	struct openssl_crypt_data* priv;

	if ( !crypt )
		return;

	priv = (struct openssl_crypt_data*)crypt->priv;
	EVP_CIPHER_CTX_cleanup(&priv->encrypt_ctx);
	EVP_CIPHER_CTX_cleanup(&priv->decrypt_ctx);
	free(priv->cipher_name);
	free(priv->key);
	free(crypt->priv);
}

static int ocd_encrypt(Npcrypt* crypt, char* data, int length, char** result, int* result_length) {
	struct openssl_crypt_data* priv;
	priv = (struct openssl_crypt_data*)crypt->priv;
	return symmetric_encrypt(&priv->encrypt_ctx, data, length, result, result_length);
}

static int ocd_decrypt(Npcrypt* crypt, char* data, int length, char** result, int* result_length) {
	struct openssl_crypt_data* priv;
	priv = (struct openssl_crypt_data*)crypt->priv;

	//printf(" LAST BYTES: %d %d %d %d %d\n", data[length-5],data[length-4],data[length-3],data[length-2], data[length-1]);

	return symmetric_decrypt(&priv->decrypt_ctx, data, length, result, result_length);
}


struct Npcrypt* new_openssl_crypt(const char* cipher_name, const char* key, int key_length) {
	struct openssl_crypt_data* priv;
	Npcrypt* result;
	const EVP_CIPHER* cipher = NULL;

	priv = malloc(sizeof(struct openssl_crypt_data));
	if ( !priv )
		return NULL;

	result = malloc(sizeof(struct Npcrypt));
	if ( !result ) {
		free(priv);
		return NULL;
	}

	priv->cipher_name = strdup(cipher_name);
	if ( !priv->cipher_name ) {
		free(priv);
		free(result);
		return NULL;
	}
	priv->key = malloc(key_length);
	if ( !priv->key ) {
		free(priv->cipher_name);
		free(priv);
		free(result);
		return NULL;
	}
	memcpy(priv->key, key, key_length);

	priv->key_length = key_length;

	result->destroy = ocd_destroy;
	result->encrypt = ocd_encrypt;
	result->decrypt = ocd_decrypt;
	result->priv = priv;

	EVP_CIPHER_CTX_init(&priv->encrypt_ctx);
	EVP_CIPHER_CTX_init(&priv->decrypt_ctx);

	cipher = EVP_get_cipherbyname(cipher_name);
	if ( cipher == NULL ) {
		printf("Cannot find cipher: %s\n", cipher_name);
		goto error_full;
	}

	if ( !EVP_DecryptInit(&priv->decrypt_ctx, cipher, key, NULL) ) {
		printf("Cannot init decrypt\n");
		goto error_full;
	}

	// TODO: Remove this, just a test
	cipher = EVP_get_cipherbyname(cipher_name);
	if ( cipher == NULL ) {
		printf("Cannot find cipher: %s\n", cipher_name);
		goto error_full;
	}

	if ( !EVP_EncryptInit(&priv->encrypt_ctx, cipher, key, NULL) ) {
		printf("Cannot init encrypt\n");
		goto error_full;
	}
	
	return result;

error_full:
	free(priv->key);
	free(priv->cipher_name);
	free(priv);
	free(result);
	return NULL;
}
