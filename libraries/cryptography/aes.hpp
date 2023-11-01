#pragma once

#include <string>

#include "base64.hpp"

namespace aes_128 {
	template<typename T>
	__forceinline std::string encrypt_custom(const std::string& text, const std::string& key, const std::string& iv) // thanks Synapse!
	{
		try
		{
			std::string encrypted;

			T encryptor;
			encryptor.SetKeyWithIV((byte*)key.c_str(), key.size(), (byte*)iv.c_str(), iv.length());

			CryptoPP::StringSource ss(text, true,
				new CryptoPP::StreamTransformationFilter(encryptor,
					new CryptoPP::StringSink(encrypted)
				)
			);

			return base64::enc(encrypted.c_str());
		}
		catch (CryptoPP::Exception& e)
		{
			return "";
		}
	}
	template<typename T>
	__forceinline std::string decrypt_custom(const std::string& text, const std::string& key, const std::string& iv)
	{
		try
		{
			std::string decrypted;

			T decryptor;
			decryptor.SetKeyWithIV((byte*)key.c_str(), key.size(), (byte*)iv.c_str(), iv.length());

			const auto Base = base64::dec(text);

			CryptoPP::StringSource ss(Base, true,
				new CryptoPP::StreamTransformationFilter(decryptor,
					new CryptoPP::StringSink(decrypted)
				)
			);

			return decrypted;
		}
		catch (CryptoPP::Exception& e)
		{
			return "";
		}
	}

	template<typename T>
	__forceinline std::string hash_custom(const std::string& text)
	{
		T hash;
		std::string digest;

		CryptoPP::StringSource ss(text, true,
			new CryptoPP::HashFilter(hash,
				new CryptoPP::HexEncoder(
					new CryptoPP::StringSink(digest), false
				)));

		return digest;
	}
	std::string enc(std::string text, std::string key, std::string iv);
	std::string dec(std::string text, std::string key, std::string iv);
}
