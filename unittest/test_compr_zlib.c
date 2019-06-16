// Copyright (C) 2019 Joel Rosdahl
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 51
// Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#include "../src/compression.h"
#include "framework.h"
#include "util.h"

TEST_SUITE(compr_zlib)

TEST(zlib_small_roundtrip)
{
	FILE *f = fopen("data.zlib", "w");
	struct compressor *compr_zlib = compressor_from_type(COMPR_TYPE_ZLIB);
	struct compr_state *c_state = compr_zlib->init(f, -1);
	CHECK(c_state);

	CHECK(compr_zlib->write(c_state, "foobar", 6));

	CHECK(compr_zlib->free(c_state));
	fclose(f);

	f = fopen("data.zlib", "r");
	struct decompressor *decompr_zlib = decompressor_from_type(COMPR_TYPE_ZLIB);
	struct decompr_state *d_state = decompr_zlib->init(f);
	CHECK(d_state);

	char buffer[4];
	CHECK(decompr_zlib->read(d_state, buffer, 4));
	CHECK(memcmp(buffer, "foob", 4) == 0);
	CHECK(decompr_zlib->read(d_state, buffer, 2));
	CHECK(memcmp(buffer, "ar", 2) == 0);

	// Nothing left to read.
	CHECK(!decompr_zlib->read(d_state, buffer, 1));

	// Error state is remembered.
	CHECK(!decompr_zlib->free(d_state));
	fclose(f);
}

TEST(zlib_large_compressible_roundtrip)
{
	char data[] = "The quick brown fox jumps over the lazy dog";

	FILE *f = fopen("data.zlib", "w");
	struct compressor *compr_zlib = compressor_from_type(COMPR_TYPE_ZLIB);
	struct compr_state *c_state = compr_zlib->init(f, 1);
	CHECK(c_state);

	for (size_t i = 0; i < 1000; i++) {
		CHECK(compr_zlib->write(c_state, data, sizeof(data)));
	}

	CHECK(compr_zlib->free(c_state));
	fclose(f);

	f = fopen("data.zlib", "r");
	struct decompressor *decompr_zlib = decompressor_from_type(COMPR_TYPE_ZLIB);
	struct decompr_state *d_state = decompr_zlib->init(f);
	CHECK(d_state);

	char buffer[sizeof(data)];
	for (size_t i = 0; i < 1000; i++) {
		CHECK(decompr_zlib->read(d_state, buffer, sizeof(buffer)));
		CHECK(memcmp(buffer, data, sizeof(data)) == 0);
	}

	// Nothing left to read.
	CHECK(!decompr_zlib->read(d_state, buffer, 1));

	// Error state is remembered.
	CHECK(!decompr_zlib->free(d_state));
	fclose(f);
}

TEST(zlib_large_uncompressible_roundtrip)
{
	char data[100000];
	for (size_t i = 0; i < sizeof(data); i++) {
		data[i] = rand() % 256;
	}

	FILE *f = fopen("data.zlib", "w");
	struct compressor *compr_zlib = compressor_from_type(COMPR_TYPE_ZLIB);
	struct compr_state *c_state = compr_zlib->init(f, 1);
	CHECK(c_state);

	CHECK(compr_zlib->write(c_state, data, sizeof(data)));

	CHECK(compr_zlib->free(c_state));
	fclose(f);

	f = fopen("data.zlib", "r");
	struct decompressor *decompr_zlib = decompressor_from_type(COMPR_TYPE_ZLIB);
	struct decompr_state *d_state = decompr_zlib->init(f);
	CHECK(d_state);

	char buffer[sizeof(data)];
	CHECK(decompr_zlib->read(d_state, buffer, sizeof(buffer)));
	CHECK(memcmp(buffer, data, sizeof(data)) == 0);

	CHECK(decompr_zlib->free(d_state));
	fclose(f);
}

TEST_SUITE_END
