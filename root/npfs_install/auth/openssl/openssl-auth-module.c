#include <stdio.h>
#include "npfs.h"
#include "openssl-auth-module.h"
#include "access-config.h"
#include "fifo_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <pthread.h>
#include "openssl-crypt.h"

/** Data relevant for the symmetric encryption */
struct symmetric_cipher_data {
	/** Name of the cipher in openssl */
	char* openssl_cipher_name;
	/** Name of the cipher in kernel cryptoapi */
	char* kernel_cipher_name;
	/** Length of the key for symmetric encryption */
	int key_length;
};

/** Structure represeting global settings of this module */
struct openssl_auth_context {
	/** CA for checking the clients certificate */
	char* path_to_ca;
	/** Public key of this server */
	char* path_to_publickey;
	/** Private key of this server */
	char* path_to_privatekey;
	/** Password of private key of this server */
	char* privatekey_password;
	/** Data relevant for the symmetric encryption */
	struct symmetric_cipher_data symmetric_cipher;
};

enum auth_status {
	AS_NONE,
	AS_REJECTED,
	AS_ACCEPTED,
	AS_INPROGRESS
};

/** Structure representing session authentication context of each connection */
struct openssl_session_auth_context {
	struct fifo_buffer input_buffer; /* Data from remote computer */
	struct fifo_buffer output_buffer; /* Data for remote computer */
	/* Authentication status */
	enum auth_status status;
	/* Associated authentication pseudofile handle */
	Npfid* fid;
	/* User being authenticated */
	char* uname;
	/* Thread executing auth protocol */
	pthread_t auth_thread;

	/* For synchronization at the end of auth protocol */
	pthread_mutex_t lock;
	pthread_cond_t cond;
};

static struct openssl_auth_context* module_context = NULL;

static void print_usage() {
	printf("Openssl auth module requires:\n");
	printf("   ca .. Path to certificate authority\n");
	printf("   publickey .. Public key\n");
	printf("   privatekey .. Private key\n");	
	printf(" Optional params:\n");	
	printf("   password .. Password of private key\n");	
	printf("   s_cipher .. Name of the symmetric cipher to be used (openssl format name)\n");	
}

static int parse_symmetric_cipher(const char* name) {
	if ( strcmp(name, "aes-128-ecb") == 0 ) {
		module_context->symmetric_cipher.openssl_cipher_name = "aes-128-ecb";
		module_context->symmetric_cipher.kernel_cipher_name = "aes";
		module_context->symmetric_cipher.key_length = 16;
		return 0;
	} 

	if ( strcmp(name, "aes-192-ecb") == 0 ) {
		module_context->symmetric_cipher.openssl_cipher_name = "aes-192-ecb";
		module_context->symmetric_cipher.kernel_cipher_name = "aes";
		module_context->symmetric_cipher.key_length = 24;
		return 0;
	} 

	if ( strcmp(name, "aes-256-ecb") == 0 ) {
		module_context->symmetric_cipher.openssl_cipher_name = "aes-256-ecb";
		module_context->symmetric_cipher.kernel_cipher_name = "aes";
		module_context->symmetric_cipher.key_length = 32;
		return 0;
	} 

	if ( strcmp(name, "bf-ecb") == 0 ) {
		module_context->symmetric_cipher.openssl_cipher_name = "bf-ecb";
		module_context->symmetric_cipher.kernel_cipher_name = "blowfish";
		module_context->symmetric_cipher.key_length = 16;
		return 0;
	} 
	
	printf("Unknown alogorithm: %s\n", name);

	return -EINVAL;
}

static int parse_args(const char* args) {
	int c;
	int has_all_required = 0;
	char* outer, *inner;
	char* argsdup = strdup(args);

	parse_symmetric_cipher("aes-128-ecb"); /* Set some default cipher for the case when no user conf is provided */

	char* token = strtok_r(argsdup,":", &outer);	
	while ( token ) {
		char* name = strtok_r(token,"=",&inner);
		char* value = strtok_r(NULL,"=",&inner);

		/* TODO: Check mallocs for NULLs here */
		if ( strcmp("ca",name) == 0 ) {
			module_context->path_to_ca = malloc(strlen(value) + 1);			
			memcpy(module_context->path_to_ca, value,strlen(value) + 1);
		} else if ( strcmp("publickey",name) == 0 ) {
			module_context->path_to_publickey = malloc(strlen(value) + 1);
			memcpy(module_context->path_to_publickey, value,strlen(value) + 1);
		} else if ( strcmp("privatekey",name) == 0 ) {
			module_context->path_to_privatekey = malloc(strlen(value) + 1);
			memcpy(module_context->path_to_privatekey, value,strlen(value) + 1);
		} else if ( strcmp("password",name) == 0) {
			module_context->privatekey_password = malloc(strlen(value) + 1);
			memcpy(module_context->privatekey_password, value,strlen(value) + 1);
		} else if ( strcmp("s_cipher",name) == 0) {
			if ( parse_symmetric_cipher(value) )
				goto failed;
		}

		token = strtok_r(NULL, ":", &outer);
	}

	has_all_required = module_context->path_to_ca != NULL && module_context->path_to_privatekey != NULL && module_context->path_to_publickey != NULL;
	if ( !has_all_required ) {
		printf("Some openssl module arguments are missing\n");
	}
failed:
	free(argsdup);

	return has_all_required ? 0 : -EINVAL;	
}

static int init_openssl_auth_module(const char* args) {
	int ret;

	module_context = malloc(sizeof(struct openssl_auth_context));
	if ( module_context == NULL ) {
		return -ENOMEM;
	}

	module_context->path_to_ca = NULL;
	module_context->path_to_publickey = NULL;
	module_context->path_to_privatekey = NULL;
	module_context->privatekey_password = NULL;

	if ( parse_args(args) ) {
		print_usage();
		ret = -EINVAL;
		goto failed;
	}

	ret = init_openssl_services();
	if ( ret )
		goto failed;

	ret = setup_ca(module_context->path_to_ca);
	if ( ret )
		goto failed;

	printf("CA ready: %s\n", module_context->path_to_ca);

	initialize_access_control("~/.9paccess");

	return 0;

failed:
	free(module_context->path_to_ca);
	free(module_context->path_to_privatekey);
	free(module_context->path_to_publickey);
	free(module_context->privatekey_password);
	free(module_context);
	module_context = NULL;
	return ret;	
}

static int destroy_openssl_auth_module() {
	free(module_context->path_to_ca);
	free(module_context->path_to_privatekey);
	free(module_context->path_to_publickey);
	free(module_context->privatekey_password);
	free(module_context);
	finalize_access_control();

	return 0;
}

static struct openssl_session_auth_context* new_session_context(Npfid * afid) {
	struct openssl_session_auth_context* ctx;
	
	ctx = malloc(sizeof(struct openssl_session_auth_context));
	if ( ctx == NULL )
		return NULL;
	
	init_buffer(&ctx->input_buffer);
	init_buffer(&ctx->output_buffer);

	pthread_mutex_init(&ctx->lock, NULL);
	pthread_cond_init(&ctx->cond, NULL);

	ctx->status = AS_NONE;
	ctx->fid = afid;
	return ctx;
}

static int execute_openssl_auth_protocol(struct openssl_session_auth_context* ctx);

static void* run_auth(void* args) {
	struct openssl_session_auth_context* ctx;

	//fprintf(stderr, "PID: %d\n",getpid());

	ctx = (struct openssl_session_auth_context*)args;
	execute_openssl_auth_protocol(ctx);
}

static Npfcall* openssl_auth(Npfid *afid, Npstr *uname, Npstr *aname) {
	struct openssl_session_auth_context* ctx;
	Npqid aqid;
	
	ctx = new_session_context(afid);	
	if ( ctx == NULL )
		return np_create_rerror("No mem",ENOMEM,1);


	ctx->uname = malloc(uname->len);
	if ( !ctx->uname )
		return np_create_rerror("No mem",ENOMEM,1);

	memcpy(ctx->uname, uname->str, uname->len);

	afid->aux = ctx;

	np_fid_incref(afid);
	
	pthread_create(&ctx->auth_thread, NULL, run_auth, ctx);

	aqid.type = Qtauth;

	return np_create_rauth(&aqid);
	//return execute_openssl_auth_protocol(ctx);
}

static Npfcall* openssl_attach(Npfid *afid, Npstr *uname, Npstr *aname) {
	struct openssl_session_auth_context* ctx;
	
	if  ( !afid )
		return np_create_rerror("No afid",EINVAL,1);

	ctx = (struct 	openssl_session_auth_context*)afid->aux;
	if ( ctx == NULL )
		return np_create_rerror("No session context",EINVAL,1);


	if ( memcmp(ctx->uname,uname->str, uname->len) != 0 ) {
		printf("Auth protocol was executed for a different user. Expected %s, but was %s\n", uname->str, ctx->uname);
		return np_create_rerror("Different user",EPERM,1);
	}

	pthread_mutex_lock(&ctx->lock);
	printf("Status of auth: %d\n", ctx->status);
	while ( ctx->status == AS_INPROGRESS ) {
		pthread_cond_wait(&ctx->cond, &ctx->lock);
	}
	printf("FINAL Status of auth: %d\n", ctx->status);
	pthread_mutex_unlock(&ctx->lock);

	if ( ctx->status != AS_ACCEPTED )
		return np_create_rerror("Authentication failed",EPERM,1);

	// All is ok => return NULL so that standard server attach can respond
	return NULL;
}

static Npfcall* openssl_clunk(Npfid *afid) {
	struct openssl_session_auth_context* ctx;

	ctx = (struct 	openssl_session_auth_context*)afid->aux;
	if ( ctx == NULL ) {
		return np_create_rerror("No session context",EINVAL,1);
	}

	pthread_join(ctx->auth_thread, NULL);
	pthread_mutex_destroy(&ctx->lock);
	pthread_cond_destroy(&ctx->cond);

	free(ctx->uname);
	free(ctx);
done:
	np_fid_decref(afid);
	return np_create_rclunk();
}

/* Read from remote client */
static Npfcall* openssl_remote_read(Npfid *afid, u64 offset, u32 count) {
	struct openssl_session_auth_context* ctx;
	int read_length;
	u8* data;
	Npfcall* rc;
	
	ctx = (struct openssl_session_auth_context*)afid->aux;
	if ( ctx == NULL )
		return np_create_rerror("No session context",EINVAL,1);

	/* We do not support offsets in our protocol */
	if ( offset != 0 )
		return np_create_rerror("No offset supported",EINVAL,1);

	data = malloc(count);
	if ( !data )
		return np_create_rerror("No mem",ENOMEM,1);

	read_from_buffer(&ctx->output_buffer, count, (char*)data, &read_length);

	rc = np_create_rread(read_length, data);
	free(data);
	return rc;
}

/* Write from remote client */
static Npfcall* openssl_remote_write(Npfid *afid, u64 offset, u32 count, u8 *data) {
	struct openssl_session_auth_context* ctx;
	int res;
	
	ctx = (struct 	openssl_session_auth_context*)afid->aux;
	if ( ctx == NULL )
		return np_create_rerror("No session context",EINVAL,1);

	/* We do not support offsets in our protocol */
	if ( offset != 0 )
		return np_create_rerror("No offset supported",EINVAL,1);

	res = write_to_buffer(&ctx->input_buffer, data, count);

	if ( res != count )
		return np_create_rerror("Invalid count written",EINVAL,1);
	
	return np_create_rwrite(res);
}

/* Read from server side */
static int openssl_local_read(struct openssl_session_auth_context* ctx, int requested_length, char* result, int* result_length) {
	return read_from_buffer(&ctx->input_buffer, requested_length, result, result_length);
}

/* Write from server side */
static int openssl_local_write(struct openssl_session_auth_context* ctx, const char* data, int data_length) {
	return write_to_buffer(&ctx->output_buffer, data, data_length);
}

//typedef unsigned int u32;

static int write_u32(struct openssl_session_auth_context* ctx, u32 value) {
	char buf[4];

	buf[0] = value;
	buf[1] = value >> 8;
	buf[2] = value >> 16;
	buf[3] = value >> 24;

	return openssl_local_write(ctx, buf, 4);
}

#define READ_ERROR -1

static u32 read_u32(struct openssl_session_auth_context* ctx) {
	unsigned char buf[4];
	u32 ret;
	int len;
	
	if ( openssl_local_read(ctx, 4, buf, &len) != 4 )
		return READ_ERROR;

	ret = buf[0] |( buf[1] << 8 )| (buf[2] << 16) | (buf[3] << 24);

	return ret;	
}

/* Converts addres from spfs string format to the proper structure */
static void convert_addr(const char* address, struct in_addr* result) {	
	char* addr, *p, *name;
	
	/* First make copy of the address so that we do not modify original address */
	addr = strdup(address);
	/* Check for starting protocol string, remove it if required */
	if (strncmp(addr, "tcp!", 4) == 0)
		name = addr + 4;
	else
		name = addr;

	/* Now find last occurance of ! (which is separating port) and "terminate" string at that point */
	p = strrchr(name, '!');
	if ( p )
		p[0] = '\0'; 

//	printf("Remote addre original: %s mod: %s\n", address, name);

	/* Finally we can convert the string */
	inet_pton(AF_INET, name, result);
	//printf("ORIG ADDR: %s NEW ADDR %s\n", address, name);	
	free(addr);
}

#define MAX_SANE_CERTIFICATE_LENGTH 20000
#define MAX_SANE_KEY_LENGTH 11000
// TODO: Make this configurable?
#define XY_RANDOM_DATA_LENGTH 32

static int exchange_certificates(struct openssl_session_auth_context* ctx, char** client_certificate, u32* client_certificate_length, char** server_certificate, u32* server_certificate_length, struct in_addr* remote_addr) {
	int ret, server_cert_verification_result_code, client_cert_verification_result_code;
	char* common_name = NULL;
	u32 common_name_length;	

	/* Load server certificate */
	if( load_cert(module_context->path_to_publickey, server_certificate, server_certificate_length) || *server_certificate_length <= 0 ) {
		printf("Failed to load public key: %s\n", module_context->path_to_publickey);
		goto failed;
	}

	/* Just check if we are able to verify certificate against our own CA */
	if ( verify_cert(*server_certificate, *server_certificate_length,  &common_name, &common_name_length) ) {
		printf("Failed to verify own certificate against the CA. Cert file: %s\n", module_context->path_to_publickey);
		goto failed;
	}

	/* Send server certificate length */
	if ( write_u32(ctx, *server_certificate_length) != 4) {
		printf("Failed to send server certificate length\n");
		goto failed;
	}

	/* Send server certificate */
	if ( openssl_local_write(ctx, *server_certificate, *server_certificate_length) != *server_certificate_length ) {
		printf("Failed to send server certificate\n");	
		goto failed;
	}

	/* Reads verification result of server certificate validation by the client */
	server_cert_verification_result_code = read_u32(ctx);

	if ( server_cert_verification_result_code == READ_ERROR ) {
		printf("Failed to read client response on server certificate validation\n");	
		goto failed;
	} else if ( server_cert_verification_result_code == 0 ) {
		printf("Client failed to validate server certificate\n");	
		goto failed;
	}

	/* Reads client certificate length */
	*client_certificate_length = read_u32(ctx);
	if ( *client_certificate_length == READ_ERROR ) {
		printf("Failed to read client certificate length\n");	
		goto failed;
	}	

	/* Reads client certificate */
	*client_certificate = malloc(*client_certificate_length);
	if ( *client_certificate == NULL )
		goto failed;
	if ( openssl_local_read(ctx, *client_certificate_length, *client_certificate, client_certificate_length) < *client_certificate_length ) {
		printf( "Failed to read client certificate\n");
		goto failed;
	}	

	/* Verifies client certificate against CA */
	ret = verify_cert(*client_certificate, *client_certificate_length, &common_name, &common_name_length);
	if ( ret ) {
		client_cert_verification_result_code = 0;
		printf("Failed to validate client certificate against CA\n");
	} else {
		/* Certificate is signed by the trusted CA, now we have to check whether the remote node ip matches certificate ip */
		char remote_ip[100];
		inet_ntop(AF_INET, remote_addr, remote_ip, 100);
		//printf("Remote addr: %s\n",remote_ip);
		if ( strcmp(remote_ip, common_name) != 0 ) {			
			client_cert_verification_result_code = 0;
			printf("Client certificate is signed by CA, but is issued for a different client. Current client is %s, but the certificate is for %s\n", remote_ip, common_name);
		} else {
			client_cert_verification_result_code = 1;
		}
	}

	if ( client_cert_verification_result_code == 1) {
		ret = 0;
	} else {
		ret = -1;
	}

	/* Send client certificate verification result */
	if ( write_u32(ctx, client_cert_verification_result_code) != 4 ) {
		printf("Failed to send server certificate length\n");
		goto failed;
	}	

	goto done;

failed:
	ret = -1;
done:
	free(common_name);
	return ret;
};

static int verify_private_key_ownership(struct openssl_session_auth_context* ctx, const char* private_key, int private_key_length, const char* client_certificate, int client_certificate_length) {
	char* x_buffer = NULL, *y_buffer = NULL, *concat_buffer = NULL;	
	int ret, tmp;
	char* yx_signature = NULL, *xy_signature = NULL;
	int yx_signature_length, xy_signature_length;

	x_buffer = malloc(XY_RANDOM_DATA_LENGTH);
	y_buffer = malloc(XY_RANDOM_DATA_LENGTH);
	concat_buffer = malloc(2*XY_RANDOM_DATA_LENGTH);
	if ( !x_buffer || !y_buffer || !concat_buffer )
		goto failed;

	get_random_key(x_buffer, XY_RANDOM_DATA_LENGTH);

	if ( write_u32(ctx, XY_RANDOM_DATA_LENGTH) != 4 ) {
		printf("Failed to x buffer size\n");
		goto failed;
	}	

	if ( openssl_local_write(ctx, x_buffer, XY_RANDOM_DATA_LENGTH) != XY_RANDOM_DATA_LENGTH ) {
		printf("Failed to send x buffer\n");	
		goto failed;
	}

	if ( openssl_local_read(ctx, XY_RANDOM_DATA_LENGTH, y_buffer, &tmp) < XY_RANDOM_DATA_LENGTH ) {
		printf( "Failed to read y buffer\n");
		goto failed;
	}

	memcpy(concat_buffer,y_buffer, XY_RANDOM_DATA_LENGTH);
	memcpy(concat_buffer+XY_RANDOM_DATA_LENGTH,x_buffer, XY_RANDOM_DATA_LENGTH);
	
	yx_signature_length = read_u32(ctx);
	if ( yx_signature_length == READ_ERROR ) {
		printf("Failed to read yx signature length\n");	
		goto failed;
	}

	yx_signature = malloc(yx_signature_length);
	if ( !yx_signature )
		goto failed;

	if ( openssl_local_read(ctx,  yx_signature_length, yx_signature, &tmp) < yx_signature_length ) {
		printf("Failed to read yx signature\n");
		goto failed;
	}
	
	ret = verify_signature(client_certificate, client_certificate_length, concat_buffer, 2*XY_RANDOM_DATA_LENGTH, yx_signature, yx_signature_length);
	if ( ret ) {
		printf("Failed to verify yx signature\n");
		goto failed;
	}

	memcpy(concat_buffer,x_buffer, XY_RANDOM_DATA_LENGTH);
	memcpy(concat_buffer+XY_RANDOM_DATA_LENGTH,y_buffer, XY_RANDOM_DATA_LENGTH);

	ret = sign_data(private_key, private_key_length, concat_buffer, 2*XY_RANDOM_DATA_LENGTH, &xy_signature, &xy_signature_length);
	if ( ret ) {
		printf("Failed to sign xy buffer\n");
		goto failed;
	}

	if ( write_u32(ctx, xy_signature_length) != 4) {
		printf("Failed to send xy signature length\n");
		goto failed;
	}

	/* Send server certificate */
	if ( openssl_local_write(ctx, xy_signature, xy_signature_length) != xy_signature_length ) {
		printf("Failed to send xy signature\n");	
		goto failed;
	}

	goto done;
failed:
	ret = -1;
done:
	free(x_buffer);
	free(y_buffer);
	free(concat_buffer);
	free(xy_signature);
	free(yx_signature);
	return ret;
};

static int establish_symmetric_cipher(struct openssl_session_auth_context* ctx, const char* client_certificate, int client_certificate_length) {
	int ret = 0;
	char symmetric_key[MAX_SANE_KEY_LENGTH];
	char* encrypted_data = NULL;
	int encrypted_length;


	get_random_key(symmetric_key, module_context->symmetric_cipher.key_length);

	if ( encrypt_data(client_certificate, client_certificate_length, symmetric_key, module_context->symmetric_cipher.key_length, &encrypted_data, &encrypted_length) ) {
		printf("Failed to encrypt symmetric key\n");	
		goto failed;	
	}

	if ( write_u32(ctx, encrypted_length) != 4 ) {
		printf("Failed to send encrypted key length\n");
		goto failed;
	}

	if ( openssl_local_write(ctx, encrypted_data, encrypted_length) != encrypted_length ) {
		printf("Failed to encrypted symmetric key\n");	
		goto failed;
	}

	if ( write_u32(ctx,  strlen(module_context->symmetric_cipher.kernel_cipher_name) + 1) != 4 ) {
		printf("Failed to send alg name length\n");
		goto failed;
	}

	/* Set encryptor before sending last message to the client so that we are ready to encrypted enc messages when the auth protocol is done */
	ctx->fid->conn->encryptor = new_openssl_crypt(module_context->symmetric_cipher.openssl_cipher_name, symmetric_key, module_context->symmetric_cipher.key_length);

	if ( openssl_local_write(ctx, module_context->symmetric_cipher.kernel_cipher_name, strlen(module_context->symmetric_cipher.kernel_cipher_name) + 1) < 0) {
		printf("Failed to send symmetric cipher name\n");	
		goto failed;
	}

	goto done;
failed:
	ret = -1;
done:
	free(encrypted_data);
	return ret;
}

static int execute_openssl_auth_protocol(struct openssl_session_auth_context* ctx) {
	u32 client_certificate_length, server_certificate_length, private_key_length;
	char* client_certificate = NULL;
	char* server_certificate = NULL;
	char* private_key = NULL;
	int auth_result = 0;
	node_security_mode required_security_mode;
	int auth_mode = 2;	
	uid_t user;
	struct in_addr remote_addr;
	int ret;

	ctx->status = AS_INPROGRESS;

	if ( ctx->fid->user == NULL ) {
		ctx->status = AS_REJECTED;
		pthread_cond_broadcast(&ctx->cond);
		return 0;
	}

	user = ctx->fid->user->uid;	
	convert_addr(ctx->fid->conn->address, &remote_addr);

	required_security_mode = get_security_mode(user, remote_addr);

	if ( required_security_mode == REJECT ) {
		ctx->status = AS_REJECTED;
		pthread_cond_broadcast(&ctx->cond);
		return 0;
	}

	if ( required_security_mode == OPEN || required_security_mode == ENCRYPTED ) /* TODO: only encryption with no auth is not supported yet */
		auth_mode= 0;
	else if ( required_security_mode == AUTHENTICATED )
		auth_mode = 1;
	else if ( required_security_mode == SECURED )
		auth_mode = 2;	

	if ( write_u32(ctx, auth_mode) != 4 ) {
		printf("Failed to send required mode to the client\n");
		ctx->status = AS_REJECTED;
		pthread_cond_broadcast(&ctx->cond);
		return 0;
	}

	if ( !auth_mode ) { /* Authentication is not required => we are done */
		ctx->status = AS_ACCEPTED;
		pthread_cond_broadcast(&ctx->cond);
		printf("Auth is not required for uid %d from host %s\n", user, ctx->fid->conn->address);
		return 1;
	}

	ret = exchange_certificates(ctx, &client_certificate, &client_certificate_length, &server_certificate, &server_certificate_length, &remote_addr);
	if ( ret )
		goto failed;

	if ( load_private_key(module_context->path_to_privatekey, module_context->privatekey_password, &private_key, &private_key_length) ) {
		printf("Failed to load private key\n");	
		goto failed;	
	}
	
	ret = verify_private_key_ownership(ctx, private_key, private_key_length, client_certificate, client_certificate_length);
	if ( ret )
		goto failed;

	if ( auth_mode == 2 )  { // If encryption is required
		ret = establish_symmetric_cipher(ctx, client_certificate, client_certificate_length);
		if ( ret )
			goto failed;
	}

	ctx->status = AS_ACCEPTED;
failed:
	free(private_key);
	free(server_certificate);
	free(client_certificate);
	if ( ctx->status == AS_INPROGRESS ) 
		ctx->status = AS_REJECTED;

	pthread_cond_broadcast(&ctx->cond);
	
	return auth_result;
}

static file_access_rights mode2rights(u8 mode) {
	file_access_rights rights = FAP_NONE;

	switch (mode & 3) {
	case Oread:
		rights = FAP_R;
		break;

	case Ordwr:
		rights = FAP_RW;
		break;

	case Owrite:
		rights = FAP_W;
		break;

	case Oexec:
		rights = FAP_RX;
		break;
	}

	return rights;
}

static int configurable_checkaccess(Npfid* fid, const char* path, u8 mode) {
	uid_t user;
	struct in_addr remote_addr;
/*	
char check_path[PATH_MAX];
	
	if ( !fid->path ) {
		memcpy(check_path, path, strlen(path) + 1);
	} else if ( !path ) {
		memcpy(check_path, fid->path, strlen(fid->path) + 1);
	} else {
		int fid_path_length = strlen(fid->path) + 1;
		memcpy(check_path, fid->path, fid_path_length);
		check_path[fid_path_length] = '/';
		memcpy(check_path + fid_path_length + 1, path, strlen(path) + 1);
	}
*/
	user = fid->user->uid;
	convert_addr(fid->conn->address, &remote_addr);

	return check_permission(user, remote_addr, path, mode2rights(mode)) == true ? 1 : 0; // 1 if access is ok
}

struct Npauth openssl_auth_module = {
	.initialize = init_openssl_auth_module,
	.destroy = destroy_openssl_auth_module,

	.auth = openssl_auth,
	.attach = openssl_attach,
	.read = openssl_remote_read,
	.write = openssl_remote_write,
	.clunk = openssl_clunk,

	.checkaccess = configurable_checkaccess
};
