//
//  ucenter_authcode.h
// C++ version of the encryption and decryption functions of the Authcode in the UCenter Home   
//
//  Created by jadder on 16/3/26.
//  E-Mail: jadderbao@163.com
//  Copyright (c) 2016 . All rights reserved.
//

#ifndef UCENTER_AUTHCODE_H
#define UCENTER_AUTHCODE_H

#include <string>

class ucenter_authcode
{
public:
	enum {
		UCENTER_AUTHCODE_ENCODE = 0,		
		UCENTER_AUTHCODE_DECODE = 1	
	};

	ucenter_authcode(const std::string& key);
	~ucenter_authcode();
	std::string encode(const std::string& str);
	std::string decode(const std::string& str);
private:
	std::string cut_string(const std::string& str, size_t start, size_t length);
	std::string random_string(size_t length);
	std::string authcode(const std::string& str, const std::string& key,
		int operation, int expiry);
	std::string get_key(const std::string& pass, size_t kLen);
	std::string RC4(const std::string& data, const std::string& pass);
	std::string md5(const std::string& data);
	bool is_valid_auth_result(const std::string& result, const std::string& keyb);
private:
	const std::string _key;
};

std::string ucenter_authcode_encode(const std::string& str, const std::string key);
std::string ucenter_authcode_decode(const std::string& str, const std::string key);


#endif // !UCENTER_AUTHCODE_H

