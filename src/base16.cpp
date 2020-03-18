/*
*  Copyright 2017 Ivan Ryabov
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*/
/*******************************************************************************
 * libSolace
 *	@file		base16.cpp
 *	@brief		Implementation of Base16 encoder and decoder.
 ******************************************************************************/
#include "solace/base16.hpp"
#include "solace/posixErrorDomain.hpp"


using namespace Solace;

// Not used
/*
static const char kBase16Alphabet_u[256][3] = {
    "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0A", "0B", "0C", "0D", "0E", "0F",
    "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "1A", "1B", "1C", "1D", "1E", "1F",
    "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "2A", "2B", "2C", "2D", "2E", "2F",
    "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "3A", "3B", "3C", "3D", "3E", "3F",
    "40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "4A", "4B", "4C", "4D", "4E", "4F",
    "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "5A", "5B", "5C", "5D", "5E", "5F",
    "60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "6A", "6B", "6C", "6D", "6E", "6F",
    "70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "7A", "7B", "7C", "7D", "7E", "7F",
    "80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "8A", "8B", "8C", "8D", "8E", "8F",
    "90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "9A", "9B", "9C", "9D", "9E", "9F",
    "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF",
    "B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF",
    "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9", "CA", "CB", "CC", "CD", "CE", "CF",
    "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "DA", "DB", "DC", "DD", "DE", "DF",
    "E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8", "E9", "EA", "EB", "EC", "ED", "EE", "EF",
    "F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "FA", "FB", "FC", "FD", "FE", "FF"
};
*/

static const char kBase16Alphabet_l[256][3] = {
    "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0a", "0b", "0c", "0d", "0e", "0f",
    "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "1a", "1b", "1c", "1d", "1e", "1f",
    "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "2a", "2b", "2c", "2d", "2e", "2f",
    "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "3a", "3b", "3c", "3d", "3e", "3f",
    "40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "4a", "4b", "4c", "4d", "4e", "4f",
    "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "5a", "5b", "5c", "5d", "5e", "5f",
    "60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "6a", "6b", "6c", "6d", "6e", "6f",
    "70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "7a", "7b", "7c", "7d", "7e", "7f",
    "80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "8a", "8b", "8c", "8d", "8e", "8f",
    "90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "9a", "9b", "9c", "9d", "9e", "9f",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "aa", "ab", "ac", "ad", "ae", "af",
    "b0", "b1", "b2", "b3", "b4", "b5", "b6", "b7", "b8", "b9", "ba", "bb", "bc", "bd", "be", "bf",
    "c0", "c1", "c2", "c3", "c4", "c5", "c6", "c7", "c8", "c9", "ca", "cb", "cc", "cd", "ce", "cf",
    "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8", "d9", "da", "db", "dc", "dd", "de", "df",
    "e0", "e1", "e2", "e3", "e4", "e5", "e6", "e7", "e8", "e9", "ea", "eb", "ec", "ed", "ee", "ef",
    "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "fa", "fb", "fc", "fd", "fe", "ff"
};


static const signed char kHexToBin[128] = {
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  //
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  //
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  //
          0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,  // 0..9
         -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // A..F
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  //
         -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // a..f
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1   //
};



Result<byte, Error>
charToBin(byte c) {
    auto const value = kHexToBin[c];

    if (value < 0) {
		return makeError(SystemErrors::ILSEQ, "charToBin");
    }

    return Ok(static_cast<byte>(value));
}


Base16Encoder::size_type
Base16Encoder::encodedSize(size_type len) {
    // FIXME(abyssoul): Does anybody care about size_type overflow?!
    return 2 * len;
}


Base16Encoder::size_type
Base16Encoder::encodedSize(MemoryView data) const {
    return encodedSize(data.size());
}


Result<void, Error>
Base16Encoder::encode(MemoryView src) {
    auto& dest = *getDestBuffer();

    for (auto value : src) {
        auto res = dest.write(wrapMemory(kBase16Alphabet_l[value], 2));
        if (!res) {
			return res.moveError();
        }
    }

    return Ok();
}


Base16Decoder::size_type
Base16Decoder::encodedSize(size_type len) {
    return len / 2;
}


Base16Decoder::size_type
Base16Decoder::encodedSize(MemoryView data) const {
    return encodedSize(data.size());
}




Result<void, Error>
Base16Decoder::encode(MemoryView src) {
    if (src.size() % 2 != 0) {
		return makeError(GenericError::DOM, "encode(): Input data size must be even");
    }

    auto& dest = *getDestBuffer();

    for (size_type i = 0; i < src.size(); i += 2) {
        auto high = charToBin(src[i]);
        auto low =  charToBin(src[i + 1]);

        if (!high)
			return high.moveError();
        if (!low)
			return low.moveError();

		auto const value = narrow_cast<byte>((high.unwrap() << 4) + low.unwrap());
        auto res = dest.write(value);
        if (!res)
			return res.moveError();
    }

    return Ok();
}


StringView
Base16Encoded_Iterator::operator* () const {
    return StringView{kBase16Alphabet_l[*_i], 2};
}


Result<byte, Error>
decode16(MemoryView::const_iterator i, MemoryView::const_iterator j) {
    auto high = charToBin(*i);
    auto low =  charToBin(*j);

    if (!high) {
		return high.moveError();
    }
    if (!low) {
		return low.moveError();
    }

	return Ok(static_cast<byte>((high.unwrap() << 4) + static_cast<byte>(low.unwrap())));
}


Base16Decoded_Iterator::Base16Decoded_Iterator(MemoryView::const_iterator rangeBegin,
											   MemoryView::const_iterator rangeEnd)
	: _i(mv(rangeBegin))
	, _end(mv(rangeEnd))
{

    MemoryView::const_iterator next = _i;
    if (_i != _end)
        ++next;

    if (next != _end) {
        auto r = decode16(_i, next);
        if (r) {
            _decodedValue = r.unwrap();
        } else {    // FIXME: Maybe we should throw here.
            _i = _end;
        }
    } else {
        _i = _end;
    }
}


Base16Decoded_Iterator&
Base16Decoded_Iterator::operator++ () {
    ++_i;
    ++_i;

    MemoryView::const_iterator next = _i;
    if (_i != _end)
        ++next;

    if (next != _end) {
        auto r = decode16(_i, next);
        if (r) {
            _decodedValue = r.unwrap();
        } else {    // FIXME: Maybe we should throw here.
            _i = _end;
        }
    } else {
        _i = _end;
    }

    return *this;
}
