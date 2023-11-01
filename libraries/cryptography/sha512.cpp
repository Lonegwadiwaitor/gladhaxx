#include "../../src/pch.hpp"

#include "sha512.hpp"

std::string sha512::hash(const std::string& text) {
	std::string cipher{};
	CryptoPP::SHA3_512 sha{};
	CryptoPP::ChannelSwitch channel_switch{};
	CryptoPP::HashFilter hash{ sha, new CryptoPP::HexEncoder(new CryptoPP::StringSink(cipher)) };
	
	channel_switch.AddDefaultRoute(hash);

	CryptoPP::StringSource source{ text.c_str(), true, new CryptoPP::Redirector(channel_switch) };

	return cipher;
}
