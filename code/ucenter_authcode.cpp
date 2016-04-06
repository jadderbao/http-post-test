//
//  ucenter_authcode.h
// C++ version of the encryption and decryption functions of the Authcode  in the UCenter Home 
//
//  Created by jadder on 16/3/26.
//  E-Mail: jadderbao@163.com
//  Copyright (c) 2016 . All rights reserved.
//

#include "ucenter_authcode.h"
#include "md5.h"
#include "base64.h"

ucenter_authcode::ucenter_authcode(const std::string& key)
	:_key(key)
{
}


ucenter_authcode::~ucenter_authcode()
{
}

std::string ucenter_authcode::cut_string(const std::string& str,
	size_t start, size_t length)
{
	if (start >= 0) {
		if (length < 0) {
			length = length * -1;
			if (start - length < 0) {
				length = start;
				start = 0;
			}
			else {
				start = start - length;
			}
		}

		if (start > str.length()) {
			return std::string();
		}
	}
	else {
		if (length < 0) {
			return  std::string();
		}
		else {
			if (length + start > 0) {
				length = length + start;
				start = 0;
			}
			else {
				return  std::string();
			}
		}
	}

	if (str.length() - start < length) {
		length = str.length() - start;
	}

	return str.substr(start, length);

}

std::string ucenter_authcode::random_string(size_t length)
{
	const std::string random_str = "abcdefghijklmnopqrstuvwxyz0123456789";
	size_t random_len = random_str.size();

	std::string str(length, '\0');
	srand(length);
	for (size_t i = 0; i < length; i++) {
		str[i] = random_str[rand() % random_len];
	}

	return str;
}

std::string ucenter_authcode::encode(const std::string& str)
{
	return authcode(str, _key, UCENTER_AUTHCODE_ENCODE, 0);
}

std::string ucenter_authcode::decode(const std::string& str)
{
	return authcode(str, _key, UCENTER_AUTHCODE_DECODE, 0);
}

std::string ucenter_authcode::authcode(const std::string& source, 
	const std::string& key, int operation, int expiry)
{
	if (source.empty() || key.empty()) {
		return std::string();
	}

	size_t ckey_length = 4;

	std::string key_md5 = md5(key);
	std::string cut_str = cut_string(key_md5, 0, 16);
	std::string keya = md5(cut_str);
	cut_str = cut_string(key_md5, 16, 16);
	std::string keyb = md5(cut_str);
	std::string keyc;
	if (ckey_length > 0) {
		keyc = (operation == UCENTER_AUTHCODE_DECODE) ? cut_string(source, 0, ckey_length) 
			: random_string(ckey_length);
	}

	std::string cryp_key = keya + md5(keya+keyc);
	std::string result;

	if (operation == UCENTER_AUTHCODE_DECODE) {

		std::string temp = base64::decode(cut_string(source, ckey_length, source.size()));
		temp = RC4(temp, cryp_key);
		result = temp;
		if (is_valid_auth_result(result,keyb)){
			return cut_string(result, 26, result.size());
		}
		else {

			temp = base64::decode(cut_string(source+"=",ckey_length, source.size()+1));
			temp = RC4(temp, cryp_key);
			result = temp;
			if (is_valid_auth_result(result, keyb)) {
				return cut_string(result, 26, result.size());
			}
			else {
				temp = base64::decode(cut_string(source + "==", ckey_length, source.size()+2));
				temp = RC4(temp, cryp_key);
				result = temp;
				if (is_valid_auth_result(result, keyb)) {
					return cut_string(result, 26, result.size());
				}
				else {
					return std::string("2");
				}
			}
		}
	}
	else {

		result = "0000000000" + cut_string(md5(source + keyb), 0, 16) + source;
		result = RC4(result, cryp_key);
		return keyc + base64::encode(result);
	}

	return std::string();
}

std::string ucenter_authcode::get_key(const std::string& pass, size_t kLen)
{
	std::string key(kLen, '\0');
	for (size_t i = 0; i < kLen; i++) {
		key[i] = (char)i;
	}

	size_t pass_Len = pass.size();
	size_t j = 0;
	for (size_t i = 0; i < kLen; i++) {

		j = (j + (size_t)((key[i] + 256) % 256) + pass[i % pass_Len]) % kLen;

		char temp = key[i];
		key[i] = key[j];
		key[j] = temp;
	}

	return key;
}

static int to_Int(unsigned char b)
{
	return (int)((b + 256) % 256);
}

std::string ucenter_authcode::RC4(const std::string& data, const std::string& pass)
{
	if (data.empty() || pass.empty())
		return std::string();

	std::string output(data.size(), 0);
	std::string mbox = get_key(pass, 256);
	size_t mbox_Len = mbox.length();

	size_t dataLen = data.size();

	// 加密
	size_t i = 0;
	size_t j = 0;

	for (size_t offset = 0; offset < dataLen; offset++) {
		i = (i + 1) % mbox_Len;
		j = (j + (size_t)((mbox[i] + 256) % 256)) % mbox_Len;

		char temp = mbox[i];
		mbox[i] = mbox[j];
		mbox[j] = temp;
		char a = data[offset];

		// mbox[j] 一定比 mbox_Len 小，不需要再取模
		char b = mbox[(to_Int(mbox[i]) + to_Int(mbox[j])) % mbox_Len];

		output[offset] = (char)((int)a ^ to_Int(b));
	}

	return output;
}

std::string ucenter_authcode::md5(const std::string& data)
{
	return MD5(data).toStr();
}

bool ucenter_authcode::is_valid_auth_result(const std::string& result, const std::string& keyb)
{
	std::string md5_str = md5(cut_string(result, 26, result.size()) + keyb);
	return cut_string(result, 10, 16) == cut_string(md5_str, 0, 16);
}

std::string ucenter_authcode_encode(const std::string& str, const std::string key)
{
	ucenter_authcode ua(key);
	return ua.encode(str);
}

std::string ucenter_authcode_decode(const std::string& str, const std::string key)
{
	ucenter_authcode ua(key);
	return ua.decode(str);
}
