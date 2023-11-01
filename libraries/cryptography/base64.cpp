#include "../../src/pch.hpp"
#include "base64.hpp"

using CryptoPP::Base64Encoder, CryptoPP::Base64Decoder;
using CryptoPP::StringSink, CryptoPP::StringSource;

std::string base64::enc(const std::string &text) {
	std::string cipher{};

	StringSource s(text, true,
		new Base64Encoder(
			new StringSink(cipher)
		)
	);
	
	return cipher;
}

std::string base64::dec(const std::string &text) {
	std::string cipher{};

	StringSource s(text, true,
		new Base64Decoder(
			new StringSink(cipher)
		)
	);

	return cipher;
}
