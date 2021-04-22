/*
 * Vector.tpp
 * Test1 
 *  Created on: April 07, 2021
 *      Author: Raavi Lagoo
 *
 * */

#pragma once

#include <cstring>

#include "Pufferfish/Util/Vector.h"
#include "catch2/catch.hpp"

namespace PF = Pufferfish;

SCENARIO( "Vectors are sized  properly and Vectors are resized correctly", "[vector]" ) {
constexpr size_t buffer_size = 252UL;
PF::Util::Vector<uint8_t,buffer_size> vector1;
PF::Util::Vector<uint8_t,buffer_size> vector2;
//constexpr size_t buffer_size = 252UL;
WHEN("Vector array size is 252UL");


auto vector_size = vector1.max_size();
REQUIRE(vector_size == 252);
auto re_vector = vector1.resize(256);
REQUIRE("re_vector = PF::IndexStatus::ok");


}

SCENARIO("Amending byte to Vector works correctly and string copy to vector gives expected results"){
constexpr size_t buffer_size = 252UL;   
PF::Util::Vector<uint8_t,buffer_size> vector2;
PF::Util::Vector<uint8_t,buffer_size> vector1;
auto push_back = vector1.push_back(0x08);
vector1.push_back(0x03);
REQUIRE(push_back == PF::IndexStatus::ok);
auto size = vector1.size();
REQUIRE(size == 2);
auto buff_position = vector1.operator[](0x03);
REQUIRE(buff_position == 0);

auto available = vector1.available();
auto data = std::string("123565468465468498485ytgdtjyxtryjrsdtrs");
for (auto& ch : data) {
        vector2.push_back(ch);
      }
auto vector_index = vector1.copy_from(
          vector2.buffer(), vector2.size(), 0);  
auto vecsize = vector1.size();   
REQUIRE(vector_index == PF::IndexStatus::ok);           
//REQUIRE(vecsize == 44);
auto full_size = vector1.full();
//REQUIRE(full_size == 44);
vector1.clear();
auto cl_size = vector1.size();
REQUIRE(cl_size == 0);

REQUIRE( push_back == PF::IndexStatus::ok );

REQUIRE(available == 250UL);


 auto array_size = vector1.max_size();
 REQUIRE(array_size == 252);




}