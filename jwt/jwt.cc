#include <alloca.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <memory>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "jwt.h"
#include "base64.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/time_utils.h"

static jwt_malloc_t pfn_malloc = NULL;
static jwt_realloc_t pfn_realloc = NULL;
static jwt_free_t pfn_free = NULL;

static int __append_str(char **buf, const char *str)
{
	char *new_;

	if (*buf == NULL) {
		new_ = (char *)jwt_calloc(1, strlen(str) + 1);
	} else {
		new_ = (char *)jwt_realloc(*buf, strlen(*buf) + strlen(str) + 1);
	}

	if (new_ == NULL)
		return ENOMEM;

	strcat(new_, str);

	*buf = new_;

	return 0;
}

#define APPEND_STR(__buf, __str) do {		\
	int ret = __append_str(__buf, __str);	\
	if (ret)				\
		return ret;			\
} while(0)

void *jwt_malloc(size_t size)
{
	if (pfn_malloc)
		return pfn_malloc(size);
	return malloc(size);
}

void *jwt_realloc(void* ptr, size_t size)
{
	if (pfn_realloc)
		return pfn_realloc(ptr, size);
	return realloc(ptr, size);
}

void jwt_freemem(void *ptr)
{
	if (pfn_free)
		pfn_free(ptr);
	else
		free(ptr);
}

char *jwt_strdup(const char *str)
{
	size_t len;
	char *result;

	len = strlen(str);
	result = (char *)jwt_malloc(len + 1);
	if (!result)
		return NULL;

	memcpy(result, str, len);
	result[len] = '\0';
	return result;
}

void *jwt_calloc(size_t nmemb, size_t size)
{
	size_t total_size;
	void* ptr;

	total_size = nmemb * size;
	if (!total_size)
		return NULL;

	ptr = jwt_malloc(total_size);
	if (ptr)
		memset(ptr, 0, total_size);

	return ptr;
}

const char *jwt_alg_str(jwt_alg_t alg)
{
	switch (alg) {
	case JWT_ALG_NONE:
		return "none";
	case JWT_ALG_HS256:
		return "HS256";
	case JWT_ALG_HS384:
		return "HS384";
	case JWT_ALG_HS512:
		return "HS512";
	case JWT_ALG_RS256:
		return "RS256";
	case JWT_ALG_RS384:
		return "RS384";
	case JWT_ALG_RS512:
		return "RS512";
	case JWT_ALG_ES256:
		return "ES256";
	case JWT_ALG_ES384:
		return "ES384";
	case JWT_ALG_ES512:
		return "ES512";
	default:
		return NULL;
	}
}

jwt_alg_t jwt_str_alg(const char *alg)
{
	if (alg == NULL)
		return JWT_ALG_INVAL;

	if (!strcasecmp(alg, "none"))
		return JWT_ALG_NONE;
	else if (!strcasecmp(alg, "HS256"))
		return JWT_ALG_HS256;
	else if (!strcasecmp(alg, "HS384"))
		return JWT_ALG_HS384;
	else if (!strcasecmp(alg, "HS512"))
		return JWT_ALG_HS512;
	else if (!strcasecmp(alg, "RS256"))
		return JWT_ALG_RS256;
	else if (!strcasecmp(alg, "RS384"))
		return JWT_ALG_RS384;
	else if (!strcasecmp(alg, "RS512"))
		return JWT_ALG_RS512;
	else if (!strcasecmp(alg, "ES256"))
		return JWT_ALG_ES256;
	else if (!strcasecmp(alg, "ES384"))
		return JWT_ALG_ES384;
	else if (!strcasecmp(alg, "ES512"))
		return JWT_ALG_ES512;

	return JWT_ALG_INVAL;
}

void jwt_base64uri_encode(char *str)
{
	int len = (int)strlen(str);
	int i, t;

	for (i = t = 0; i < len; i++) {
		switch (str[i]) {
		case '+':
			str[t++] = '-';
			break;
		case '/':
			str[t++] = '_';
			break;
		case '=':
			break;
		default:
			str[t++] = str[i];
		}
	}

	str[t] = '\0';
}

void* jwt_b64_decode(const char *src, int *ret_len)
{
	char *buf;
	char *new_buf;
	int len, i, z;

	/* Decode based on RFC-4648 URI safe encoding. */
	len = (int)strlen(src);
	new_buf = (char *)alloca(len + 4);
	if (!new_buf)
		return NULL;

	for (i = 0; i < len; i++) {
		switch (src[i]) {
		case '-':
			new_buf[i] = '+';
			break;
		case '_':
			new_buf[i] = '/';
			break;
		default:
			new_buf[i] = src[i];
		}
	}
	z = 4 - (i % 4);
	if (z < 4) {
		while (z--)
			new_buf[i++] = '=';
	}
	new_buf[i] = '\0';

	buf = (char *)malloc(i);
	if (buf == NULL)
		return NULL;

	*ret_len = jwt_Base64decode(buf, new_buf);

	return buf;
}

int jwt_read_key(const char* key_file,unsigned char* out)
{
	if (key_file == NULL || out == NULL)
		return -1;
	
	struct stat buf;
	int fd;
	fd = open(key_file, O_RDONLY);
	fstat(fd, &buf);
	close(fd);
	if(!out)
		return ENOMEM;
	int len = 0;
	FILE *fp = fopen(key_file, "r");
	if (fp != NULL) {
		len = fread(out, 1, buf.st_size, fp);
		fclose(fp);
		out[len] = '\0';	
	}
	return len;
}

static Json::Value jwt_b64_decode_json(char *src)
{
	char *buf;
	int len;
	
	buf = (char *)jwt_b64_decode(src, &len);
	if (buf == NULL)
		return Json::Value();

	buf[len] = '\0';

	Json::Reader reader;
  Json::Value root;
  if (!reader.parse(buf, root))
		return Json::Value();

	jwt_freemem(buf);

	return std::move(root);
}


JwtToken::JwtToken(
	jwt_alg_t jalg,
	unsigned char* pkey,
	const Json::Value& grants)
 : key(pkey),	
   key_len(0),
   alg(jalg),
   grants_(std::move(grants))
{
	if (key != NULL)
		key_len = strlen((char *)key);
}

JwtToken::JwtToken(const std::string& token,unsigned char* pkey)
	: key(pkey), 
		key_len(0),
		token_(token),
		signatureOk(false)
{
	if (key != NULL)
		key_len = strlen((char *)key);
}

JwtToken::~JwtToken() = default;

JwtToken::JwtToken(const JwtToken&) = default;
JwtToken::JwtToken(JwtToken&&) = default;
JwtToken& JwtToken::operator=(const JwtToken&) = default;
JwtToken& JwtToken::operator=(JwtToken&&) = default;

JwtToken::Builder::Builder() = default;

JwtToken::Builder::~Builder() = default;

JwtToken JwtToken::Builder::build() 
{
  return JwtToken(alg,key,std::move(grants));
}

JwtToken::Builder& JwtToken::Builder::add_alg(
	jwt_alg_t alg,unsigned char* key)
{
	RTC_DCHECK(alg != JWT_ALG_INVAL);
	this->alg = alg;
	this->key = key;
	return *this;
}

JwtToken::Builder& JwtToken::Builder::add_grant(
		const std::string& grant, const std::string& val)
{
	RTC_DCHECK(!grant.empty());
	if (!grants.isMember(grant))
		grants[grant] = val;
	return *this;
}

JwtToken::Builder& JwtToken::Builder::add_grant_int(
		const std::string& grant, int val)
{
	RTC_DCHECK(!grant.empty());
	if (!grants.isMember(grant))
		grants[grant] = val;	
	return *this;
}

JwtToken::Builder& JwtToken::Builder::add_grant_int64(
		const std::string& grant, int64_t val)
{
	RTC_DCHECK(!grant.empty());
	if (!grants.isMember(grant))
		grants[grant] = val;	
	return *this;
}

JwtToken::Builder& JwtToken::Builder::add_grant_bool(
		const std::string& grant, bool val)
{
	RTC_DCHECK(!grant.empty());
	if (!grants.isMember(grant))
		grants[grant] = val;	
	return *this;
}

void JwtToken::checkHead()
{
	if (!headers_.empty()) {
		RTC_DCHECK(headers_.isMember("alg"));
		RTC_DCHECK(headers_.isMember("typ"));
		std::string alg_;
		std::string typ;
		rtc::GetStringFromJsonObject(headers_,"alg",&alg_);
		rtc::GetStringFromJsonObject(headers_,"typ",&typ);
		RTC_DCHECK(jwt_str_alg(alg_.c_str()) == alg);
		RTC_DCHECK(strcasecmp(typ.c_str(), "JWT") == 0);
	}	
}

int JwtToken::Signature(char **out, unsigned int *len, const char *str)
{
	switch (alg) {
	/* HMAC */
	case JWT_ALG_HS256:
	case JWT_ALG_HS384:
	case JWT_ALG_HS512:
		return jwt_sign_sha_hmac(this, out, len, str);

	/* RSA */
	case JWT_ALG_RS256:
	case JWT_ALG_RS384:
	case JWT_ALG_RS512:

	/* ECC */
	case JWT_ALG_ES256:
	case JWT_ALG_ES384:
	case JWT_ALG_ES512:
		return jwt_sign_sha_pem(this, out, len, str);

	/* You wut, mate? */
	default:
		return EINVAL;
	}
}

int JwtToken::Verify(const char *head, const char *sig)
{
	switch (alg) {
	/* HMAC */
	case JWT_ALG_HS256:
	case JWT_ALG_HS384:
	case JWT_ALG_HS512:
		return jwt_verify_sha_hmac(this, head, sig);

	/* RSA */
	case JWT_ALG_RS256:
	case JWT_ALG_RS384:
	case JWT_ALG_RS512:

	/* ECC */
	case JWT_ALG_ES256:
	case JWT_ALG_ES384:
	case JWT_ALG_ES512:
		return jwt_verify_sha_pem(this, head, sig);

	/* You wut, mate? */
	default:
		return EINVAL;
	}
}


int JwtToken::EncodeAndSignature(FILE *fp)
{
	int ret = 0;
	char *buf = NULL, *head, *body, *sig;
	int head_len, body_len;
	unsigned int sig_len;

	if (!token_.empty())
		token_.clear();
	
	/* First the header. */
	if (alg != JWT_ALG_NONE) {
		if (!headers_.empty())
			headers_.clear();
		headers_["typ"] = "JWT";
		headers_["alg"] = jwt_alg_str(alg);
	}

	head = 
		(char *)alloca(strlen(rtc::JsonValueToString(headers_).c_str()) * 2);
	if (head == NULL) {
		headers_.clear();
		return ENOMEM;
	}	

	jwt_Base64encode(head, 
			rtc::JsonValueToString(headers_).c_str(), 
			(int)strlen(rtc::JsonValueToString(headers_).c_str()));
	head_len = (int)strlen(head);		

	/* Now the payload. */
	if (grants_.empty())
		return EINVAL;

	body = 
		(char *)alloca(strlen(rtc::JsonValueToString(grants_).c_str()) * 2);
	if (body == NULL) {
		grants_.clear();
		return ENOMEM;
	}
	
	jwt_Base64encode(body, rtc::JsonValueToString(grants_).c_str(), 
		(int)strlen(rtc::JsonValueToString(grants_).c_str()));
	body_len = (int)strlen(body);

	jwt_base64uri_encode(head);
	jwt_base64uri_encode(body);	

	/* Allocate enough to reuse as b64 buffer. */
	buf = (char *)jwt_malloc(head_len + body_len + 2);
	if (buf == NULL)
		return ENOMEM;
	strcpy(buf, head);
	strcat(buf, ".");
	strcat(buf, body);
	
	char *out = NULL;
	ret = __append_str(&out, buf);
	if (ret == 0)
		ret = __append_str(&out, ".");
	if (ret) {
		if (buf)
			jwt_freemem(buf);
		return ret;
	}

	if (alg == JWT_ALG_NONE) {
		jwt_freemem(buf);
		return 0;
	}

	/* Now the signature. */
	ret = Signature(&sig, &sig_len, buf);
	jwt_freemem(buf);

	if (ret)
		return ret;	
	
	buf = (char *)jwt_malloc(sig_len * 2);
	if (buf == NULL) {
		jwt_freemem(sig);
		return ENOMEM;
	}

	jwt_Base64encode(buf, sig, sig_len);

	jwt_freemem(sig);

	jwt_base64uri_encode(buf);
	ret = __append_str(&out, buf);
	jwt_freemem(buf);

	if (fp != NULL)
		fputs(out, fp);

	token_ = std::move(out);
	
	return ret;	
}

int JwtToken::parseHead(char *head)
{
	if (!headers_.empty()) 
		headers_.clear();
	
	headers_ = std::move(jwt_b64_decode_json(head));
	
	if (headers_.empty()) 
		return EINVAL;
	
	return 0;
}

int JwtToken::parseBody(char *body)
{
	if (!grants_.empty()) 
		grants_.clear();
	grants_ = std::move(jwt_b64_decode_json(body));
	if (grants_.empty()) 
		return EINVAL;
	return 0;
}

int JwtToken::verifyHead(char *head)
{
	int ret = EINVAL;
	if (head == NULL)
		return ret;
	
	if ((ret = parseHead(head))) {
		return ret;
	}
	
	std::string val;
	if (!headers_.isMember("alg") || 
			!rtc::GetStringFromJsonObject(headers_,"alg",&val))
		return EINVAL;

	if (val.empty())
		return EINVAL;

	alg = jwt_str_alg(val.c_str());
	if (alg == JWT_ALG_INVAL) {
		ret = EINVAL;
		goto verify_head_done;
	}
	
	if (alg != JWT_ALG_NONE) {
		/* If alg is not NONE, there may be a typ. */
		if (!headers_.isMember("typ") || 
				!rtc::GetStringFromJsonObject(headers_,"typ",&val)) 
			ret = EINVAL;	
		
		if (val.empty() || strcasecmp(val.c_str(), "JWT"))
			ret = EINVAL;	
		
		if (key != NULL) {
			if (key_len <= 0)
				ret = EINVAL;
		} 
	} else {
		/* If alg is NONE, there should not be a key */
		if (key){
			ret = EINVAL;
		}
	}
verify_head_done:
	return ret;	
}

int JwtToken::DecodeAndVerify()
{
	int ret = EINVAL;
	if (token_.empty())
		return ret;
	char *head = jwt_strdup(token_.c_str());
	char *body, *sig;
	
	if (!head)
		return ENOMEM;

	/* Find the components. */
	for (body = head; body[0] != '.'; body++) {
		if (body[0] == '\0')
			goto decode_done;
	}

	body[0] = '\0';
	body++;

	for (sig = body; sig[0] != '.'; sig++) {
		if (sig[0] == '\0')
			goto decode_done;
	}

	sig[0] = '\0';
	sig++;

	ret = verifyHead(head);
	if (ret) {
		RTC_LOG(LERROR) << "verifyHead Error";
		goto decode_done;
	}
		
	ret = parseBody(body);
	if (ret) {
		RTC_LOG(LERROR) << "parseBody Error";
		goto decode_done;
	}
	
	/* Check the signature, if needed. */
	if (alg != JWT_ALG_NONE) {
		/* Re-add this since it's part of the verified data. */
		body[-1] = '.';
		ret = Verify(head, sig);
	} else {
		ret = 0;
	}
	if (ret == 0)
		signatureOk = true;
	else {
		RTC_LOG(LERROR) << "Verify Error";
		signatureOk = false;
	}
		
decode_done:
	jwt_freemem(head);
	return ret;
}

bool JwtToken::VerifyInfo(const char* grant,const char* value)
{
	if (!signatureOk)
		return false;
	
	if (!grants_.isMember(grant))
		return false;
	
	std::string val;
	if (!rtc::GetStringFromJsonObject(grants_,grant,&val))
		return false;
	
	if (strcasecmp(val.c_str(),value))
		return false;
	
	return true;
}

bool JwtToken::VerifyContains(const char* grant,const char* value)
{
	if (!signatureOk)
		return false;
	
	if (!grants_.isMember(grant))
		return false;
	
	std::string val;
	if (!rtc::GetStringFromJsonObject(grants_,grant,&val))
		return false;
	
	if (val.find(value) == 
			std::string::npos)
		return false;
	
	return true;	
}

bool JwtToken::VerifyExpire()
{
	if (!signatureOk)
		return false;
	
	if (!grants_.isMember("exp"))
		return false;	
	
	int64_t val = 0;
	if (!rtc::GetInt64FromJsonObject(grants_,"exp",&val))
		return false;	
	
	if (rtc::TimeUTCMillis() > val) {
		RTC_LOG(INFO) << "time expiress";
		return false;
	}
	
	return true;
}
