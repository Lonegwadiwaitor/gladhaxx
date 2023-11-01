#pragma once

#include "../src/pch.hpp"

#include <lz4.h>

namespace lz4 {
	enum class format {
		txt,
		bin
	};

	template<format fmt = format::txt>
	std::string compress(const std::string& source) {
		const std::size_t max_size = LZ4_compressBound(source.size() + (fmt == format::txt));
		std::string compressed(max_size, 0);

		compressed.resize(LZ4_compress_default(source.data(), compressed.data(), source.size() + (fmt == format::txt), max_size));
		compressed.shrink_to_fit();

		return compressed;
	}

	template<format fmt = format::txt>
	std::string decompress(const std::string& compressed, const std::size_t& original_length) {
		std::string decompressed(original_length, 0);
		LZ4_decompress_safe(compressed.data(), decompressed.data(), compressed.size(), original_length + (fmt == format::txt));

		return decompressed;
	}
}
