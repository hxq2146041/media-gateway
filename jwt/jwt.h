#ifndef GATEWAY_JWT_H_
#define GATEWAY_JWT_H_
#include "rtc_base/strings/json.h"

//eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhdWQiOiJhdWQiLCJleHAiOjEyMzQ1Njc4LCJpc3MiOiJpc3MiLCJzdWIiOiJzdWIifQ.uS9SNCL0CQY5VpYP8zJgC1klgoO4hI0Lk3r6-rnc2pE

/** JWT algorithm types. */
typedef enum jwt_alg {
	JWT_ALG_NONE = 0,
	JWT_ALG_HS256,
	JWT_ALG_HS384,
	JWT_ALG_HS512,
	JWT_ALG_RS256,
	JWT_ALG_RS384,
	JWT_ALG_RS512,
	JWT_ALG_ES256,
	JWT_ALG_ES384,
	JWT_ALG_ES512,
	JWT_ALG_TERM
} jwt_alg_t;

class JwtToken {
 private:	 
	JwtToken(jwt_alg_t jalg,
		unsigned char* pkey,
		const Json::Value& grants); 
 public:
	unsigned char *key;
	int key_len;	 		
	jwt_alg_t alg;
	
	JwtToken(const std::string& token,unsigned char* pkey);	
  ~JwtToken();
  // Support move and copy.
  JwtToken(const JwtToken&);
  JwtToken(JwtToken&&);
  JwtToken& operator=(const JwtToken&);
  JwtToken& operator=(JwtToken&&);	 	
  // Preferred way of building JwtToken objects.
  class Builder {
   public:
    Builder();
    ~Builder();
    JwtToken build();
		Builder& add_alg(jwt_alg_t alg,unsigned char* key);
    Builder& add_grant(const std::string& grant, const std::string& val);
		Builder& add_grant_int(const std::string& grant, int val);
		Builder& add_grant_int64(const std::string& grant, int64_t val);
		Builder& add_grant_bool(const std::string& grant, bool val);
	 private:
	 	jwt_alg_t alg;
		unsigned char* key;
		Json::Value grants;
  };
	
	/**
   * return 0 success else error
	 */
	int EncodeAndSignature(FILE *fp=NULL);
	/**
   * return 0 success else error
	 */	
	int DecodeAndVerify();
	bool VerifyInfo(const char* grant,const char* value);
	bool VerifyContains(const char* grant,const char* value);
	bool VerifyExpire();
	
	std::string& token() {return token_;}
	Json::Value& headers() {return headers_;}
	Json::Value& grants() {return grants_;}
	
 protected:	
	void checkHead(); 
	int Signature(char **out, unsigned int *len, const char *str);
	int Verify(const char *head, const char *sig);
	int verifyHead(char *head);
	int parseHead(char *head);
	int parseBody(char *body);
 private: 	
	Json::Value headers_; 	
	Json::Value grants_;		
  std::string token_;
	bool signatureOk;

};

#define JWT_ALG_INVAL JWT_ALG_TERM

typedef JwtToken jwt_t;

const char *jwt_alg_str(jwt_alg_t alg);

jwt_alg_t jwt_str_alg(const char *alg);

typedef void *(*jwt_malloc_t)(size_t);
typedef void *(*jwt_realloc_t)(void *, size_t);
typedef void(*jwt_free_t)(void *);

/* Memory allocators. */
void *jwt_malloc(size_t size);
void jwt_freemem(void *ptr);
char *jwt_strdup(const char *str);
void *jwt_calloc(size_t nmemb, size_t size);
void *jwt_realloc(void *ptr, size_t size);

/* Helper routines. */
void jwt_base64uri_encode(char *str);
void *jwt_b64_decode(const char *src, int *ret_len);

/* These routines are implemented by the crypto backend. */
int jwt_sign_sha_hmac(jwt_t *jwt, char **out, unsigned int *len,
		      const char *str);

int jwt_verify_sha_hmac(jwt_t *jwt, const char *head, const char *sig);

int jwt_sign_sha_pem(jwt_t *jwt, char **out, unsigned int *len,
		     const char *str);

int jwt_verify_sha_pem(jwt_t *jwt, const char *head, const char *sig_b64);

int jwt_read_key(const char* key_file,unsigned char* out);

#endif
