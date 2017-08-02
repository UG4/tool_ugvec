/*
* Base64 encoding/decoding (RFC1341)
* Copyright (c) 2005-2011, Jouni Malinen <j@w1.fi>
*
* This software may be distributed under the terms of the BSD license.
*/

#ifndef __H__base64
#define __H__base64

#include <string>

std::string base64_encode(const unsigned char *src, size_t len);
std::string base64_decode(const unsigned char *src, size_t len);

#endif	//__H__base64
