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
#include "solace/exception.hpp"


#include <cstring>  // memcpy
#include <limits>


using namespace Solace;


static const byte kBase16Alphabet[] = "0123456789ABCDEF";
//static const byte kBase16Alphabet[] = "0123456789abcdef";

static const char kBase16Alphabet_2[256][2] = {
    {'0', '0'}, {'0', '1'}, {'0', '2'}, {'0', '3'}, {'0', '4'}, {'0', '5'}, {'0', '6'}, {'0', '7'}, {'0', '8'}, {'0', '9'}, {'0', 'A'}, {'0', 'B'}, {'0', 'C'}, {'0', 'D'}, {'0', 'E'}, {'0', 'F'},
    {'1', '0'}, {'1', '1'}, {'1', '2'}, {'1', '3'}, {'1', '4'}, {'1', '5'}, {'1', '6'}, {'1', '7'}, {'1', '8'}, {'1', '9'}, {'1', 'A'}, {'1', 'B'}, {'1', 'C'}, {'1', 'D'}, {'1', 'E'}, {'1', 'F'},
    {'2', '0'}, {'2', '1'}, {'2', '2'}, {'2', '3'}, {'2', '4'}, {'2', '5'}, {'2', '6'}, {'2', '7'}, {'2', '8'}, {'2', '9'}, {'2', 'A'}, {'2', 'B'}, {'2', 'C'}, {'2', 'D'}, {'2', 'E'}, {'2', 'F'},
    {'3', '0'}, {'3', '1'}, {'3', '2'}, {'3', '3'}, {'3', '4'}, {'3', '5'}, {'3', '6'}, {'3', '7'}, {'3', '8'}, {'3', '9'}, {'3', 'A'}, {'3', 'B'}, {'3', 'C'}, {'3', 'D'}, {'3', 'E'}, {'3', 'F'},
    {'4', '0'}, {'4', '1'}, {'4', '2'}, {'4', '3'}, {'4', '4'}, {'4', '5'}, {'4', '6'}, {'4', '7'}, {'4', '8'}, {'4', '9'}, {'4', 'A'}, {'4', 'B'}, {'4', 'C'}, {'4', 'D'}, {'4', 'E'}, {'4', 'F'},
    {'5', '0'}, {'5', '1'}, {'5', '2'}, {'5', '3'}, {'5', '4'}, {'5', '5'}, {'5', '6'}, {'5', '7'}, {'5', '8'}, {'5', '9'}, {'5', 'A'}, {'5', 'B'}, {'5', 'C'}, {'5', 'D'}, {'5', 'E'}, {'5', 'F'},
    {'6', '0'}, {'6', '1'}, {'6', '2'}, {'6', '3'}, {'6', '4'}, {'6', '5'}, {'6', '6'}, {'6', '7'}, {'6', '8'}, {'6', '9'}, {'6', 'A'}, {'6', 'B'}, {'6', 'C'}, {'6', 'D'}, {'6', 'E'}, {'6', 'F'},
    {'7', '0'}, {'7', '1'}, {'7', '2'}, {'7', '3'}, {'7', '4'}, {'7', '5'}, {'7', '6'}, {'7', '7'}, {'7', '8'}, {'7', '9'}, {'7', 'A'}, {'7', 'B'}, {'7', 'C'}, {'7', 'D'}, {'7', 'E'}, {'7', 'F'},
    {'8', '0'}, {'8', '1'}, {'8', '2'}, {'8', '3'}, {'8', '4'}, {'8', '5'}, {'8', '6'}, {'8', '7'}, {'8', '8'}, {'8', '9'}, {'8', 'A'}, {'8', 'B'}, {'8', 'C'}, {'8', 'D'}, {'8', 'E'}, {'8', 'F'},
    {'9', '0'}, {'9', '1'}, {'9', '2'}, {'9', '3'}, {'9', '4'}, {'9', '5'}, {'9', '6'}, {'9', '7'}, {'9', '8'}, {'9', '9'}, {'9', 'A'}, {'9', 'B'}, {'9', 'C'}, {'9', 'D'}, {'9', 'E'}, {'9', 'F'},
    {'A', '0'}, {'A', '1'}, {'A', '2'}, {'A', '3'}, {'A', '4'}, {'A', '5'}, {'A', '6'}, {'A', '7'}, {'A', '8'}, {'A', '9'}, {'A', 'A'}, {'A', 'B'}, {'A', 'C'}, {'A', 'D'}, {'A', 'E'}, {'A', 'F'},
    {'B', '0'}, {'B', '1'}, {'B', '2'}, {'B', '3'}, {'B', '4'}, {'B', '5'}, {'B', '6'}, {'B', '7'}, {'B', '8'}, {'B', '9'}, {'B', 'A'}, {'B', 'B'}, {'B', 'C'}, {'B', 'D'}, {'B', 'E'}, {'B', 'F'},
    {'C', '0'}, {'C', '1'}, {'C', '2'}, {'C', '3'}, {'C', '4'}, {'C', '5'}, {'C', '6'}, {'C', '7'}, {'C', '8'}, {'C', '9'}, {'C', 'A'}, {'C', 'B'}, {'C', 'C'}, {'C', 'D'}, {'C', 'E'}, {'C', 'F'},
    {'D', '0'}, {'D', '1'}, {'D', '2'}, {'D', '3'}, {'D', '4'}, {'D', '5'}, {'D', '6'}, {'D', '7'}, {'D', '8'}, {'D', '9'}, {'D', 'A'}, {'D', 'B'}, {'D', 'C'}, {'D', 'D'}, {'D', 'E'}, {'D', 'F'},
    {'E', '0'}, {'E', '1'}, {'E', '2'}, {'E', '3'}, {'E', '4'}, {'E', '5'}, {'E', '6'}, {'E', '7'}, {'E', '8'}, {'E', '9'}, {'E', 'A'}, {'E', 'B'}, {'E', 'C'}, {'E', 'D'}, {'E', 'E'}, {'E', 'F'},
    {'F', '0'}, {'F', '1'}, {'F', '2'}, {'F', '3'}, {'F', '4'}, {'F', '5'}, {'F', '6'}, {'F', '7'}, {'F', '8'}, {'F', '9'}, {'F', 'A'}, {'F', 'B'}, {'F', 'C'}, {'F', 'D'}, {'F', 'E'}, {'F', 'F'}
};


static const char kHexToBin[128] = {
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  //
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  //
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  //
          0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,  // 0..9
         -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // A..F
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  //
         -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // a..f
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   //
};


Base16Encoder::size_type
Base16Encoder::encodedSize(size_type len) {
    // FIXME(abyssoul): Who cares about size_type overflow?!
    return 2 * len;
}


Base16Encoder::size_type
Base16Encoder::encodedSize(const ImmutableMemoryView& data) const {
    return encodedSize(data.size());
}


void Base16Encoder::encode(const ImmutableMemoryView& src) {
    auto& dest = *getDestBuffer();

    for (auto value : src) {
//        dest << kBase16Alphabet[value >> 4];
//        dest << kBase16Alphabet[value & 0x0F];
        dest << kBase16Alphabet_2[value][0];
        dest << kBase16Alphabet_2[value][1];

    }
}


Base16Decoder::size_type
Base16Decoder::encodedSize(const ImmutableMemoryView& data) const {
    return data.size() / 2;
}


char charToBin(byte c) {
    return kHexToBin[c];
}


void
Base16Decoder::encode(const ImmutableMemoryView& src) {
    auto& dest = *getDestBuffer();

    for (size_type i = 0; i < src.size(); i += 2) {
        const char high = charToBin(src[i]);
        const char low =  charToBin(src[i + 1]);

        if (high < 0 || low < 0) {
            raise<Exception>("Failed to decode base16 string: value is not in base16 alphabet");
        }

        const byte value = static_cast<byte>(high << 4) + static_cast<byte>(low);
        dest << value;
    }
}
