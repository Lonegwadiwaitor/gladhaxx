#include "../../src/pch.hpp"
#include "aes.hpp"

using crypto_string = std::basic_string<CryptoPP::byte, std::char_traits<CryptoPP::byte>, std::allocator<CryptoPP::byte>>;

std::string encode_hex(const std::string& text) {
	using CryptoPP::StringSource, CryptoPP::HexEncoder, CryptoPP::StringSink;

	std::string encoded;

	StringSource(text, true,
		new HexEncoder(
			new StringSink(encoded)
		)
	);

	return encoded;
}

std::string aes_128::enc(std::string text, std::string key, std::string iv) {
	using CryptoPP::HexEncoder, CryptoPP::HexDecoder;
	using CryptoPP::StringSink, CryptoPP::StringSource;
	using CryptoPP::AES, CryptoPP::CBC_Mode;
	using CryptoPP::StreamTransformationFilter;

	crypto_string key_bytes = reinterpret_cast<CryptoPP::byte*>(key.data());
	crypto_string iv_bytes = reinterpret_cast<CryptoPP::byte*>(iv.data());
	std::string cipher;

	CBC_Mode<AES>::Encryption e;
	e.SetKeyWithIV(key_bytes.data(), key_bytes.size(), iv_bytes.data());

	StringSource s(text, true,
		new StreamTransformationFilter(e,
			new StringSink(cipher)
		)
	);

	return encode_hex(cipher);
}

std::string aes_128::dec(std::string text, std::string key, std::string iv) {
	using CryptoPP::HexEncoder, CryptoPP::HexDecoder;
	using CryptoPP::StringSink, CryptoPP::StringSource;
	using CryptoPP::AES, CryptoPP::CBC_Mode;
	using CryptoPP::StreamTransformationFilter;

	crypto_string key_bytes = reinterpret_cast<CryptoPP::byte*>(key.data());
	crypto_string iv_bytes = reinterpret_cast<CryptoPP::byte*>(iv.data());
	std::string cipher;

	CBC_Mode<AES>::Decryption e;
	e.SetKeyWithIV(key_bytes.data(), key_bytes.size(), iv_bytes.data());

	StringSource s(text, true,
		new HexDecoder(
			new StreamTransformationFilter(e,
				new StringSink(cipher)
			)
		)
	);

	return cipher;
}
