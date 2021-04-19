/*
 * Copyright 2020, the Pez Globo team and the Pufferfish project contributors
 *
 * CRCElements.cpp
 *
 *  Created on: Nov 3, 2020
 *      Author: Rohan Purohit, Renji Panicker
 *
 * Unit tests to confirm behavior of CRCElement
 *
 */

#include "Pufferfish/Protocols/CRCElements.h"

#include "Pufferfish/HAL/CRCChecker.h"
#include "Pufferfish/Test/Util.h"
#include "Pufferfish/Util/Array.h"
#include "Pufferfish/Util/Endian.h"
#include "catch2/catch.hpp"
namespace PF = Pufferfish;

SCENARIO(
    "Protocols::CRCElement: The compute_body_crc method correctly computes the CRC of a payload "
    "from a CRCElement body given as input",
    "[CRCElement]") {
  constexpr size_t buffer_size = 254UL;
  using TestCRCElementProps = PF::Protocols::CRCElementProps<buffer_size>;
  using TestCRCElement = PF::Protocols::CRCElement<TestCRCElementProps::PayloadBuffer>;
  using TestCRCElementHeaderProps = PF::Protocols::CRCElementHeaderProps;
  using TestParsedCRCElement = PF::Protocols::ParsedCRCElement<buffer_size>;

  PF::HAL::SoftCRC32 crc32c{PF::HAL::crc32c_params};
  GIVEN("The CRCElement::compute_body_crc static method") {
    WHEN("The compute_body_crc is called on an input_buffer body with an empty payload") {
      PF::Util::ByteVector<buffer_size> buffer;
      TestCRCElementProps::PayloadBuffer input_payload;

      buffer.copy_from(
          input_payload.buffer(), input_payload.size(), TestCRCElementHeaderProps::payload_offset);

      auto computed_crc = TestCRCElement::compute_body_crc(buffer, crc32c);

      THEN("the computed CRC is '0x00000000") {
        // Calculated using the Sunshine Online CRC Calculator
        uint32_t expected_crc = 0x00000000;
        REQUIRE(computed_crc == expected_crc);
      }
    }

    WHEN("the compute_body_crc is called on an input_buffer body with payload '123456789'") {
      TestCRCElementProps::PayloadBuffer input_payload;
      auto data = std::string("123456789");
      for (auto& ch : data) {
        input_payload.push_back(ch);
      }

      PF::Util::ByteVector<buffer_size> buffer;
      buffer.resize(TestCRCElementHeaderProps::header_size + input_payload.size());
      buffer.copy_from(
          input_payload.buffer(), input_payload.size(), TestCRCElementHeaderProps::payload_offset);

      auto computed_crc = TestCRCElement::compute_body_crc(buffer, crc32c);

      THEN("the computed crc is '0xe3069283'") {
        // Calculated using the Sunshine Online CRC Calculator
        uint32_t expected_crc = 0xe3069283;
        REQUIRE(computed_crc == expected_crc);
      }
    }

    WHEN("the compute_body_crc is called on an input_buffer body with payload '0x00' ") {
      TestCRCElementProps::PayloadBuffer input_payload;
      input_payload.push_back(0x00);

      PF::Util::ByteVector<buffer_size> buffer;
      buffer.resize(TestCRCElementHeaderProps::header_size + input_payload.size());
      buffer.copy_from(
          input_payload.buffer(), input_payload.size(), TestCRCElementHeaderProps::payload_offset);

      auto computed_crc = TestCRCElement::compute_body_crc(buffer, crc32c);

      THEN("the computed crc is '0x527d5351'") {
        // Calculated using the Sunshine Online CRC Calculator
        uint32_t expected_crc = 0x527d5351;
        REQUIRE(computed_crc == expected_crc);
      }
    }

    WHEN("the compute_body_crc is called on an input_buffer body with payload '0x01'") {
      TestCRCElementProps::PayloadBuffer input_payload;
      input_payload.push_back(0x01);

      PF::Util::ByteVector<buffer_size> buffer;

      buffer.resize(TestCRCElementHeaderProps::header_size + input_payload.size());
      buffer.copy_from(
          input_payload.buffer(), input_payload.size(), TestCRCElementHeaderProps::payload_offset);

      auto computed_crc = TestCRCElement::compute_body_crc(buffer, crc32c);

      THEN("the computed crc is '0xa016d052") {
        // Calculated using the Sunshine Online CRC Calculator
        uint32_t expected_crc = 0xa016d052;
        REQUIRE(computed_crc == expected_crc);
      }
    }
  }
}

SCENARIO(
    "Protocols::ConstructedCRCElement: The write method correctly generates a body from the "
    "payload given in the constructor",
    "[CRCElement]") {
  constexpr size_t buffer_size = 254UL;
  using TestCRCElementProps = PF::Protocols::CRCElementProps<buffer_size>;
  using TestConstructedCRCElement = PF::Protocols::ConstructedCRCElement<buffer_size>;
  using TestCRCElementHeaderProps = PF::Protocols::CRCElementHeaderProps;

  PF::HAL::SoftCRC32 crc32c{PF::HAL::crc32c_params};
  GIVEN("A crc element constructed with payload=[0x01, 0x02, 0x05]") {
    TestCRCElementProps::PayloadBuffer input_payload;
    auto data = PF::Util::make_array<uint8_t>(0x01, 0x02, 0x05);
    for (auto& bytes : data) {
      input_payload.push_back(bytes);
    }

    TestConstructedCRCElement crc_element{input_payload};
    WHEN("The crc and payload are written to the output buffer") {
      PF::Util::ByteVector<buffer_size> output_buffer;

      THEN("the crc accessor method returns 0 before the write method is called") {
        REQUIRE(crc_element.crc() == 0x00000000);
      }

      auto write_status = crc_element.write(output_buffer, crc32c);
      auto crc_compute = crc32c.compute(input_payload.buffer(), input_payload.size());

      THEN("The write method reports ok status") { REQUIRE(write_status == PF::IndexStatus::ok); }
      THEN(
          "After the write method is called, the crc accessor method returns a value equal to the "
          "result of directly computing a CRC on the payload.") {
        REQUIRE(crc_element.crc() == crc_compute);
      }
      THEN(
          "The CRC field of the body's header matches the value returned by the crc accessor "
          "method.") {
        uint32_t crc_header = 0xd79115f6;
        REQUIRE(crc_header == crc_element.crc());
      }
      THEN("The body's payload section correctly stores the payload as 0x01 0x02 0x05") {
        REQUIRE(output_buffer.operator[](TestCRCElementHeaderProps::payload_offset) == data[0]);
        REQUIRE(output_buffer.operator[](TestCRCElementHeaderProps::payload_offset + 1) == data[1]);
        REQUIRE(output_buffer.operator[](TestCRCElementHeaderProps::payload_offset + 2) == data[2]);
      }

      THEN("The output buffer is as expected") {
        auto expected = std::string("\xD7\x91\x15\xF6\x01\x02\x05");
        REQUIRE(output_buffer == expected);
      }
    }
  }

  GIVEN("A crc element is constructed with an empty payload") {
    TestCRCElementProps::PayloadBuffer input_payload;
    TestConstructedCRCElement crc_element{input_payload};
    WHEN("The crc and payload are written to the output buffer") {
      PF::Util::ByteVector<buffer_size> output_buffer;
      THEN("the crc accessor method returns 0 before the write method is called") {
        uint32_t expected = 0x00000000;
        REQUIRE(crc_element.crc() == expected);
      }

      auto write_status = crc_element.write(output_buffer, crc32c);
      THEN("The write method reports ok status") { REQUIRE(write_status == PF::IndexStatus::ok); }
      THEN("After the write method is called, the crc accessor method returns 0") {
        uint32_t expected = 0x00000000;
        REQUIRE(crc_element.crc() == expected);
      }
      THEN(
          "The CRC field of the body's header matches the value returned by the crc accessor "
          "method.") {
        uint32_t crc_header = 0x00000000;
        REQUIRE(crc_header == crc_element.crc());
      }
      THEN("The body's payload section is empty") {
        constexpr size_t payload_size = 250UL;
        PF::Util::ByteVector<payload_size> expected;
        REQUIRE(crc_element.payload() == expected);
      }
      THEN("The output buffer is as expected") {
        auto expected_buffer = std::string("\x00\x00\x00\x00", 4);
        REQUIRE(output_buffer == expected_buffer);
      }
    }
  }
}

SCENARIO(
    "Protocols::CRCElement: The parse method correctly updates its internal crc value and the "
    "constructor's payload buffer from the input buffer",
    "[CRCElement]") {
  constexpr size_t buffer_size = 254UL;
  using TestCRCElementProps = PF::Protocols::CRCElementProps<buffer_size>;
  using TestCRCElement = PF::Protocols::CRCElement<TestCRCElementProps::PayloadBuffer>;
  PF::HAL::SoftCRC32 crc32c{PF::HAL::crc32c_params};

  GIVEN("A CRCElement constructed with an empty payload buffer with a capacity of 254 bytes") {
    TestCRCElementProps::PayloadBuffer payload;

    TestCRCElement crc_element{payload};

    REQUIRE(crc_element.crc() == 0);
    auto buffer = crc_element.payload();
    REQUIRE(buffer.empty() == true);

    WHEN("A body without a complete 4-byte header is parsed") {
      auto body = std::string("\x98\xdb\xe3", 3);

      PF::Util::ByteVector<buffer_size> input_buffer;
      PF::Util::convert_string_to_byte_vector(body, input_buffer);

      auto parse_status = crc_element.parse(input_buffer);

      THEN("the parse method reports out of bounds status") {
        REQUIRE(parse_status == PF::IndexStatus::out_of_bounds);
      }
      THEN(
          "After the parse method is called, The value returned by the crc accessor method remains "
          "unchanged") {
        REQUIRE(crc_element.crc() == 0);
      }
      THEN("the constructor's payload buffer remains unchanged") {
        REQUIRE(payload.empty() == true);
      }
    }

    WHEN(
        "A body with a complete 4-byte header, and a payload consistent with the CRC field of the "
        "header, is parsed") {
      auto body = std::string("\x98\xdb\xe3\x55\x01\x05\x01\x02\x03\x04\x05", 11);

      PF::Util::ByteVector<buffer_size> input_buffer;
      PF::Util::convert_string_to_byte_vector(body, input_buffer);

      auto parse_status = crc_element.parse(input_buffer);

      THEN("the parse method reports ok status") { REQUIRE(parse_status == PF::IndexStatus::ok); }
      THEN(
          "After the parse method is called, the value returned by the crc accessor method is "
          "equal to the crc field of the input_buffer body's header") {
        // Calculated using the Sunshine Online CRC Calculator
        uint32_t expected_crc = 0x98DBE355;
        REQUIRE(crc_element.crc() == expected_crc);
      }
      auto expected_payload = std::string("\x01\x05\x01\x02\x03\x04\x05", 7);
      THEN(
          "the payload returned from the payload accessor method is equal to the payload from the "
          "input_buffer body") {
        REQUIRE(crc_element.payload() == expected_payload);
      }
      THEN(
          "the payload accessor method returns a reference to the payload buffer given in the "
          "CRCElement constructor") {
        // change the payload buffer given to the constructor
        payload.push_back(0x06);

        // expected buffer returned by the payload accessor method
        auto expected = std::string("\x01\x05\x01\x02\x03\x04\x05\x06", 8);
        REQUIRE(crc_element.payload() == expected);
      }
      THEN("the payload given in the CRCElement constructor is independent of the input body") {
        // change the input_buffer
        input_buffer.push_back(0x06);

        REQUIRE(crc_element.payload() == expected_payload);
      }
    }

    WHEN(
        "A body with a complete 4-byte header, and a payload inconsistent with the CRC field of "
        "the header, is parsed") {
      auto body = std::string("\x12\x34\x56\x78\x03\x04\x00\xed\x30\x00", 10);

      PF::Util::ByteVector<buffer_size> input_buffer;
      PF::Util::convert_string_to_byte_vector(body, input_buffer);

      auto parse_status = crc_element.parse(input_buffer);

      THEN("the parse method reports ok status") { REQUIRE(parse_status == PF::IndexStatus::ok); }
      THEN(
          "After the parse method is called, the value returned by the crc accessor method is "
          "equal to the crc field of the input_buffer body's header") {
        uint32_t crc_body = 0x12345678;
        REQUIRE(crc_element.crc() == crc_body);
      }
      THEN("the value returned by the crc accessor method is inconsistent with the payload") {
        // Calculated using the Sunshine Online CRC Calculator
        uint32_t actual_crc = 0x6D551F4C;
        REQUIRE(crc_element.crc() != actual_crc);
      }
      auto expected_payload = std::string("\x03\x04\x00\xed\x30\x00", 6);
      THEN(
          "the payload returned from the payload accessor method is equal to the payload from the "
          "input_buffer body") {
        REQUIRE(crc_element.payload() == expected_payload);
      }
      THEN(
          "the payload accessor method returns a reference to the payload buffer given in the "
          "CRCElement constructor") {
        // change the payload buffer given to the constructor
        payload.push_back(0x06);

        // expected buffer returned by the payload accessor method
        auto expected = std::string("\x03\x04\x00\xed\x30\x00\x06", 7);
        REQUIRE(crc_element.payload() == expected);
      }
      THEN("the payload given in the CRCElement constructor is independent of the input body") {
        // change the input_buffer
        input_buffer.push_back(0x06);

        REQUIRE(crc_element.payload() == expected_payload);
      }
    }
  }

  GIVEN("A CRCElement constructed with a non-empty payload buffer of capacity 254 bytes") {
    TestCRCElementProps::PayloadBuffer payload;
    auto data = std::string("\x12\x13\x14\x15\x16", 5);
    PF::Util::convert_string_to_byte_vector(data, payload);

    TestCRCElement crc_element{payload};

    REQUIRE(crc_element.crc() == 0);
    REQUIRE(crc_element.payload() == data);

    WHEN("A body without a complete 4-byte header is parsed") {
      auto body = std::string("\x98\xdb\xe3", 3);

      PF::Util::ByteVector<buffer_size> input_buffer;
      PF::Util::convert_string_to_byte_vector(body, input_buffer);

      auto parse_status = crc_element.parse(input_buffer);

      THEN("the parse method reports out of bounds status") {
        REQUIRE(parse_status == PF::IndexStatus::out_of_bounds);
      }
      THEN(
          "After the parse method is called, the value returned by the crc accessor method remains "
          "unchanged") {
        REQUIRE(crc_element.crc() == 0);
      }
      THEN(
          "the payload accessor method returns a reference to the payload buffer given in the "
          "CRCElement constructor") {
        REQUIRE(crc_element.payload() == data);
      }
      THEN(
          "the payload given in the CRCElement constructor is independent of the input buffer "
          "body") {
        input_buffer.push_back(0x06);

        REQUIRE(crc_element.payload() == data);
      }
      THEN("the constructor's payload buffer remains unchanged") { REQUIRE(payload == data); }
    }

    WHEN(
        "A body with a complete 4-byte header, and a payload consistent with the CRC field of the "
        "header, is parsed") {
      auto body = std::string("\x2B\x01\xD0\x2C\x01\x06\x11\x22\x33\x44\x55\x66", 12);

      PF::Util::ByteVector<buffer_size> input_buffer;
      PF::Util::convert_string_to_byte_vector(body, input_buffer);

      auto parse_status = crc_element.parse(input_buffer);

      THEN("the parse method reports ok status") { REQUIRE(parse_status == PF::IndexStatus::ok); }
      THEN(
          "After the parse method is called, the value returned by the crc accessor method is "
          "equal to the crc field of the input_buffer body's header") {
        // Calculated using the Sunshine Online CRC Calculator
        uint32_t expected_crc = 0x2B01D02C;
        REQUIRE(crc_element.crc() == expected_crc);
      }
      auto expected_payload = std::string("\x01\x06\x11\x22\x33\x44\x55\x66", 8);
      THEN(
          "the payload returned from the payload accessor method is equal to the payload from the "
          "input_buffer body") {
        REQUIRE(crc_element.payload() == expected_payload);
      }
      THEN(
          "the payload accessor method returns a reference to the payload buffer given in the "
          "CRCElement constructor") {
        // change the payload buffer given to the constructor
        payload.push_back(0x06);

        // expected buffer returned by the payload accessor method
        auto expected = std::string("\x01\x06\x11\x22\x33\x44\x55\x66\x06", 9);
        REQUIRE(crc_element.payload() == expected);
      }
      THEN("the payload given in the CRCElement constructor is independent of the input body") {
        // change the input_buffer
        input_buffer.push_back(0x06);

        REQUIRE(crc_element.payload() == expected_payload);
      }
    }
  }

  GIVEN(
      "A CRCElement constructed with a non-empty payload buffer of capacity 254 bytes, and a "
      "non-zero crc member") {
    TestCRCElementProps::PayloadBuffer payload;
    auto data = std::string("\x0d\xf0\x07\xc3\xac", 5);
    PF::Util::convert_string_to_byte_vector(data, payload);

    TestCRCElement crc_element{payload};

    PF::Util::ByteVector<buffer_size> output_buffer;
    crc_element.write(output_buffer, crc32c);

    uint32_t expected_crc = 0x34CD15AD;
    REQUIRE(crc_element.crc() == expected_crc);
    REQUIRE(crc_element.payload() == data);
    WHEN("A body without a complete 4-byte header is parsed") {
      auto body = std::string("\x16\xbb\x77", 3);

      PF::Util::ByteVector<buffer_size> input_buffer;
      PF::Util::convert_string_to_byte_vector(body, input_buffer);

      auto parse_status = crc_element.parse(input_buffer);

      THEN("the parse method reports out of bounds status") {
        REQUIRE(parse_status == PF::IndexStatus::out_of_bounds);
      }
      THEN(
          "After the parse method is called, The value returned by the crc accessor method remains "
          "unchanged") {
        REQUIRE(crc_element.crc() == expected_crc);
      }
      THEN("the constructor's payload buffer remains unchanged") {
        input_buffer.push_back(0x06);

        REQUIRE(crc_element.payload() == data);
      }
    }

    WHEN(
        "A body with a complete 4-byte header, and a payload consistent with the CRC field of the "
        "header, is parsed") {
      auto body = std::string("\x1D\x83\x2D\x11\x40\xbe\x37\xcb\xac\x15\x4b\xd5", 12);

      PF::Util::ByteVector<buffer_size> input_buffer;
      PF::Util::convert_string_to_byte_vector(body, input_buffer);

      auto parse_status = crc_element.parse(input_buffer);
      THEN("the parse method reports ok status") { REQUIRE(parse_status == PF::IndexStatus::ok); }
      THEN(
          "After the parse method is called, the value returned by the crc accessor method is "
          "equal to the crc field of the input_buffer body's header") {
        uint32_t expected = 0x1d832d11;
        REQUIRE(crc_element.crc() == expected);
      }
      auto expected_payload = std::string("\x40\xbe\x37\xcb\xac\x15\x4b\xd5", 8);
      THEN(
          "the payload returned from the payload accessor method is equal to the payload from the "
          "input_buffer body") {
        REQUIRE(crc_element.payload() == expected_payload);
      }
      THEN(
          "the payload accessor method returns a reference to the payload buffer given in the "
          "CRCElement constructor") {
        // change the payload buffer given to the constructor
        payload.push_back(0x06);

        // expected buffer returned by the payload accessor method
        auto expected = std::string("\x40\xbe\x37\xcb\xac\x15\x4b\xd5\x06", 9);
        REQUIRE(crc_element.payload() == expected);
      }
      THEN(
          "the payload given in the CRCElement constructor is independent of the input buffer "
          "body") {
        // change the input_buffer
        input_buffer.push_back(0x06);

        REQUIRE(crc_element.payload() == expected_payload);
      }
    }
  }
}

SCENARIO("Protocols::CRCElement: correctly preserves data in roundtrip writing and parsing") {
  GIVEN("A CRC Element constructed with a non empty payload buffer with a capacity of 254 bytes") {
    constexpr size_t buffer_size = 254UL;
    using TestCRCElementProps = PF::Protocols::CRCElementProps<buffer_size>;
    using TestCRCElement = PF::Protocols::CRCElement<TestCRCElementProps::PayloadBuffer>;
    using TestParsedCRCElement = PF::Protocols::ParsedCRCElement<buffer_size>;
    using TestConstructedCRCElement = PF::Protocols::ConstructedCRCElement<buffer_size>;
    PF::HAL::SoftCRC32 crc32c{PF::HAL::crc32c_params};

    auto data = PF::Util::make_array<uint8_t>(0x01, 0x05, 0x01, 0x02, 0x03, 0x04, 0x05);

    TestCRCElementProps::PayloadBuffer payload;
    for (auto& bytes : data) {
      payload.push_back(bytes);
    }

    TestCRCElement crc_element{payload};

    WHEN(
        "A body with CRC and payload is generated, parsed into a new ParsedCRCElement object, and "
        "generated again with a new ConstructedCRCElement object") {
      // Write
      PF::Util::ByteVector<buffer_size> output_buffer;

      THEN("the crc accessor method returns 0 before the first write method is called") {
        REQUIRE(crc_element.crc() == 0x00000000);
      }
      auto write_status = crc_element.write(output_buffer, crc32c);

      THEN("The first write method call returns ok status") {
        REQUIRE(write_status == PF::IndexStatus::ok);
      }
      THEN(
          "After the first write method is called, the crc accessor method returns a value equal "
          "to the result of directly computing a CRC on the payload") {
        auto crc_compute = crc32c.compute(payload.buffer(), payload.size());
        REQUIRE(crc_element.crc() == crc_compute);
      }
      THEN(
          "The CRC field of the body's header matches the value returned by the crc accessor "
          "method.") {
        // Calculated using the Sunshine Online CRC Calculator
        uint32_t expected_crc = 0x98DBE355;
        REQUIRE(crc_element.crc() == expected_crc);
      }
      THEN(
          "The body's payload section correctly stores the payload as '0x01 0x05 0x01 0x02 0x03 "
          "0x04 0x05'") {
        auto expected_payload = std::string("\x01\x05\x01\x02\x03\x04\x05", 7);
        REQUIRE(crc_element.payload() == expected_payload);
      }
      THEN("The output buffer is as expected") {
        auto expected_buffer = std::string("\x98\xDB\xE3\x55\x01\x05\x01\x02\x03\x04\x05", 11);
        REQUIRE(output_buffer == expected_buffer);
      }

      // Parse
      TestCRCElementProps::PayloadBuffer parsed_payload;
      TestParsedCRCElement parsed_crc_element{parsed_payload};

      auto parse_status = parsed_crc_element.parse(output_buffer);

      THEN("The parse method reports ok status") { REQUIRE(parse_status == PF::IndexStatus::ok); }
      THEN(
          "After the parse method is called, the value returned by the crc accessor method is "
          "equal to the crc field of the input "
          "body's header") {
        uint32_t crc_body = 0x98DBE355;
        REQUIRE(parsed_crc_element.crc() == crc_body);
      }
      THEN(
          "the payload returned from the payload accessor method is equal to the payload from the "
          "body") {
        auto expected_payload = std::string("\x01\x05\x01\x02\x03\x04\x05", 7);
        REQUIRE(parsed_crc_element.payload() == expected_payload);
      }

      THEN(
          "the payload given in the ParsedCRCElement constructor is independent of the input "
          "body") {
        // change the input_buffer
        output_buffer.push_back(0x06);

        auto expected_payload = std::string("\x01\x05\x01\x02\x03\x04\x05", 7);
        REQUIRE(parsed_crc_element.payload() == expected_payload);
      }

      // Write
      auto final_payload = std::string("\x01\x05\x01\x02\x03\x04\x05", 7);
      TestCRCElementProps::PayloadBuffer const_payload;
      PF::Util::convert_string_to_byte_vector(final_payload, const_payload);

      TestConstructedCRCElement constructed_crc_element{const_payload};

      PF::Util::ByteVector<buffer_size> final_buffer;

      auto write = constructed_crc_element.write(final_buffer, crc32c);

      THEN("The second write method call returns ok status") {
        REQUIRE(write == PF::IndexStatus::ok);
      }
      THEN(
          "After the second write method is called, the crc accessor method returns a value equal "
          "to the result of directly computing a CRC on the payload") {
        auto crc_compute = crc32c.compute(const_payload.buffer(), const_payload.size());
        REQUIRE(constructed_crc_element.crc() == crc_compute);
      }
      THEN(
          "The CRC field of the body's header matches the value returned by the crc accessor "
          "method.") {
        // Calculated using the Sunshine Online CRC Calculator
        uint32_t expected_crc = 0x98DBE355;
        REQUIRE(constructed_crc_element.crc() == expected_crc);
      }
      THEN(
          "The body's payload section correctly stores the payload as '0x01 0x05 0x01 0x02 0x03 "
          "0x04 0x05'") {
        REQUIRE(constructed_crc_element.payload() == final_payload);
      }
      THEN("The output buffer is as expected") {
        auto expected_buffer = std::string("\x98\xDB\xE3\x55\x01\x05\x01\x02\x03\x04\x05", 11);
        REQUIRE(final_buffer == expected_buffer);
      }
    }
  }
}

SCENARIO(
    "Protocols::CRCElementReceiver: correctly parses CRCElement bodies and performs consistency "
    "checking on them",
    "[CRCElementReceiver]") {
  GIVEN(
      "A CRC element receiver of capacity 254 bytes and output_crcelement constructed with an "
      "empty payload buffer") {
    constexpr size_t buffer_size = 254UL;
    using TestCRCElementProps = PF::Protocols::CRCElementProps<buffer_size>;
    using TestCRCElement = PF::Protocols::CRCElement<TestCRCElementProps::PayloadBuffer>;
    using TestCRCElementReceiver = PF::Protocols::CRCElementReceiver<buffer_size>;
    PF::Util::ByteVector<buffer_size> input_buffer;

    PF::HAL::SoftCRC32 crc32c{PF::HAL::crc32c_params};
    PF::Protocols::CRCElementReceiver<buffer_size> crc_element_receiver{crc32c};

    TestCRCElementProps::PayloadBuffer input_payload;
    TestCRCElement crc_element{input_payload};

    REQUIRE(crc_element.crc() == 0);
    auto buffer = crc_element.payload();
    REQUIRE(buffer.empty() == true);

    WHEN("An empty input_buffer is given to the crc element receiver") {
      PF::Util::ByteVector<buffer_size> input_buffer;

      auto transform_status = crc_element_receiver.transform(input_buffer, crc_element);

      THEN("the transform status is equal to invalid parse") {
        REQUIRE(transform_status == TestCRCElementReceiver::Status::invalid_parse);
      }
      THEN(
          "After the transform method is called, the value returned by the output_crcelement's crc "
          "accessor method remains unchanged") {
        uint32_t expected_crc = 0x00000000;
        REQUIRE(crc_element.crc() == expected_crc);
      }
      THEN(
          "The output_crcelement's payload accessor method returns a reference to the payload "
          "given in its constructor") {
        auto expected = crc_element.payload();
        REQUIRE(expected.empty() == true);
      }
      THEN(
          "The data in the output_crcelement's payload buffer is independent of the data in "
          "input_buffer") {
        input_buffer.push_back(0x02);
        auto expected = crc_element.payload();
        REQUIRE(expected.empty() == true);
      }
      THEN("the constructor's payload buffer remains unchanged") {
        REQUIRE(input_payload.empty() == true);
      }
    }

    WHEN("body with less than 4 bytes is given to the crc element receiver") {
      PF::Util::ByteVector<buffer_size> input_buffer;

      // body of length 1 byte
      auto input = PF::Util::make_array<uint8_t>(0x00);

      for (auto& bytes : input) {
        input_buffer.push_back(bytes);
      }
      auto status = crc_element_receiver.transform(input_buffer, crc_element);

      THEN("the transform status is equal to invalid parse") {
        REQUIRE(status == TestCRCElementReceiver::Status::invalid_parse);
      }
      THEN(
          "After the transform method is called, the value returned by the output_crcelement's crc "
          "accessor method remains unchanged") {
        uint32_t expected_crc = 0x00;
        REQUIRE(crc_element.crc() == expected_crc);
      }
      THEN(
          "The output_crcelement's payload accessor method returns a reference to the payload "
          "given in its constructor") {
        auto expected = crc_element.payload();
        REQUIRE(expected.empty() == true);
      }
      THEN(
          "The data in the output_crcelement's payload buffer is independent of the data in "
          "input_buffer") {
        input_buffer.push_back(0x02);
        auto expected = crc_element.payload();
        REQUIRE(expected.empty() == true);
      }
      THEN("the constructor's payload buffer remains unchanged") {
        REQUIRE(input_payload.empty() == true);
      }

      // body of length 2 bytes
      auto input1 = std::string("\x01", 1);
      input_buffer.clear();

      for (auto& bytes : input1) {
        input_buffer.push_back(bytes);
      }
      auto first = crc_element_receiver.transform(input_buffer, crc_element);

      THEN("the transform status is equal to invalid parse") {
        REQUIRE(first == TestCRCElementReceiver::Status::invalid_parse);
      }
      THEN(
          "After the transform method is called, the value returned by the output_crcelement's crc "
          "accessor method remains unchanged") {
        uint32_t expected_crc = 0x00;
        REQUIRE(crc_element.crc() == expected_crc);
      }
      THEN(
          "The output_crcelement's payload accessor method returns a reference to the payload "
          "given in its constructor") {
        auto expected = crc_element.payload();
        REQUIRE(expected.empty() == true);
      }
      THEN(
          "The data in the output_crcelement's payload buffer is independent of the data in "
          "input_buffer") {
        input_buffer.push_back(0x02);
        auto expected = crc_element.payload();
        REQUIRE(expected.empty() == true);
      }
      THEN("the constructor's payload buffer remains unchanged") {
        REQUIRE(input_payload.empty() == true);
      }

      // body of length 3 bytes
      auto input2 = std::string("\x00\x01", 2);
      input_buffer.clear();

      for (auto& bytes : input2) {
        input_buffer.push_back(bytes);
      }
      auto second = crc_element_receiver.transform(input_buffer, crc_element);

      THEN("the transform status is equal to invalid parse") {
        REQUIRE(second == TestCRCElementReceiver::Status::invalid_parse);
      }
      THEN(
          "After the transform method is called, the value returned by the output_crcelement's crc "
          "accessor method remains unchanged") {
        uint32_t expected_crc = 0x00;
        REQUIRE(crc_element.crc() == expected_crc);
      }
      THEN(
          "The output_crcelement's payload accessor method returns a reference to the payload "
          "given in its constructor") {
        auto expected = crc_element.payload();
        REQUIRE(expected.empty() == true);
      }
      THEN(
          "The data in the output_crcelement's payload buffer is independent of the data in "
          "input_buffer") {
        input_buffer.push_back(0x02);
        auto expected = crc_element.payload();
        REQUIRE(expected.empty() == true);
      }
      THEN("the constructor's payload buffer remains unchanged") {
        REQUIRE(input_payload.empty() == true);
      }
    }

    WHEN(
        "A body with a complete 4-byte header, and a payload consistent with the CRC field of the "
        "header, is given to the receiver") {
      auto body = std::string("\x44\xeb\x77\x5f\x01\x07\x07\x12\x36\x57\x66\x77\x18", 13);
      PF::Util::ByteVector<buffer_size> input_buffer;
      PF::Util::convert_string_to_byte_vector(body, input_buffer);

      auto transform_status = crc_element_receiver.transform(input_buffer, crc_element);

      THEN("the transform method reports ok status") {
        REQUIRE(transform_status == TestCRCElementReceiver::Status::ok);
      }
      THEN(
          "After the transform method is called, the value returned by the output_crcelement's crc "
          "accessor method is equal to the crc field of the input_buffer body's header") {
        uint32_t expected_crc = 0x44EB775F;
        REQUIRE(crc_element.crc() == expected_crc);
      }
      THEN(
          "The payload buffer returned by the output_crcelement's payload accessor method is equal "
          "to the payload from the input_buffer body") {
        auto expected_payload = std::string("\x01\x07\x07\x12\x36\x57\x66\x77\x18", 9);
        REQUIRE(crc_element.payload() == expected_payload);
      }
      THEN(
          "The data in the output_crcelement's payload buffer is independent of the data in "
          "input_buffer") {
        // change input buffer
        input_buffer.push_back(0x01);

        auto expected_payload = std::string("\x01\x07\x07\x12\x36\x57\x66\x77\x18", 9);
        REQUIRE(crc_element.payload() == expected_payload);
      }
    }

    WHEN(
        "A body with a complete 4-byte header, and a payload inconsistent with the CRC field of "
        "the header, is given to the receiver") {
      auto body = std::string("\x12\x34\x56\x78\x01\x05\x01\x02\x03\x04\x05", 11);
      PF::Util::ByteVector<buffer_size> input_buffer;
      PF::Util::convert_string_to_byte_vector(body, input_buffer);

      uint32_t expected_crc = 0x12345678;

      auto transform_status = crc_element_receiver.transform(input_buffer, crc_element);

      THEN("the transform method reports invalid crc status") {
        REQUIRE(transform_status == TestCRCElementReceiver::Status::invalid_crc);
      }
      THEN(
          "After the transform method is called, the value returned by the output_crcelement's crc "
          "accessor method is equal to the crc field of the input_buffer body's header") {
        REQUIRE(crc_element.crc() == expected_crc);
      }
      THEN("the value returned by the crc accessor method is inconsistent with the payload") {
        uint32_t actual_crc = 0x6D551F4C;
        REQUIRE(crc_element.crc() != actual_crc);
      }
      auto expected = std::string("\x01\x05\x01\x02\x03\x04\x05", 7);
      THEN(
          "The payload buffer returned by the output_crcelement's payload accessor method is equal "
          "to the payload from the input_buffer body") {
        REQUIRE(crc_element.payload() == expected);
      }
      THEN(
          "The data in the output_crcelement's payload buffer is independent of the data in "
          "input_buffer") {
        input_buffer.push_back(0x02);
        REQUIRE(crc_element.payload() == expected);
      }
    }
  }

  GIVEN(
      "A CRC element receiver of capacity 254 bytes and output_crcelement constructed with an "
      "non-empty payload buffer") {
    constexpr size_t buffer_size = 254UL;
    using TestCRCElementProps = PF::Protocols::CRCElementProps<buffer_size>;
    using TestCRCElement = PF::Protocols::CRCElement<TestCRCElementProps::PayloadBuffer>;
    using TestCRCElementReceiver = PF::Protocols::CRCElementReceiver<buffer_size>;
    PF::Util::ByteVector<buffer_size> input_buffer;

    PF::HAL::SoftCRC32 crc32c{PF::HAL::crc32c_params};
    PF::Protocols::CRCElementReceiver<buffer_size> crc_element_receiver{crc32c};

    TestCRCElementProps::PayloadBuffer input_payload;
    auto body = std::string("\x01\x05\x12\x13\x14\x15\x16", 7);
    PF::Util::convert_string_to_byte_vector(body, input_payload);
    TestCRCElement crc_element{input_payload};

    REQUIRE(crc_element.crc() == 0);
    REQUIRE(crc_element.payload() == body);

    WHEN("An empty input_buffer is given to the crc element receiver") {
      PF::Util::ByteVector<buffer_size> input_buffer;

      auto transform_status = crc_element_receiver.transform(input_buffer, crc_element);

      THEN("the transform status is equal to invalid parse") {
        REQUIRE(transform_status == TestCRCElementReceiver::Status::invalid_parse);
      }
      THEN(
          "After the transform method is called, the value returned by the output_crcelement's crc "
          "accessor method remains unchanged") {
        uint32_t expected_crc = 0x00000000;
        REQUIRE(crc_element.crc() == expected_crc);
      }
      THEN(
          "The output_crcelement's payload accessor method returns a reference to the payload "
          "given in its constructor") {
        REQUIRE(crc_element.payload() == body);
      }
      THEN(
          "The data in the output_crcelement's payload buffer is independent of the data in "
          "input_buffer") {
        input_buffer.push_back(0x02);
        REQUIRE(crc_element.payload() == body);
      }
      THEN("the constructor's payload buffer remains unchanged") { REQUIRE(input_payload == body); }
    }

    WHEN(
        "A body with crc more than 4 bytes and equal to the crc of the payload is given to the "
        "crc_element receiver") {
      auto body = std::string("\x44\xeb\x77\x5f\x01\x07\x07\x12\x36\x57\x66\x77\x18", 13);
      PF::Util::ByteVector<buffer_size> input_buffer;
      PF::Util::convert_string_to_byte_vector(body, input_buffer);

      auto transform_status = crc_element_receiver.transform(input_buffer, crc_element);

      THEN("the transform method reports ok status") {
        REQUIRE(transform_status == TestCRCElementReceiver::Status::ok);
      }
      THEN(
          "After the transform method is called, the value returned by the output_crcelement's crc "
          "accessor method is equal to the crc field of the input_buffer body's header") {
        uint32_t expected_crc = 0x44EB775F;
        REQUIRE(crc_element.crc() == expected_crc);
      }
      THEN(
          "The payload buffer returned by the output_crcelement's payload accessor method is equal "
          "to the payload from the input_buffer body") {
        auto expected_payload = std::string("\x01\x07\x07\x12\x36\x57\x66\x77\x18", 9);
        REQUIRE(crc_element.payload() == expected_payload);
      }
      THEN(
          "The data in the output_crcelement's payload buffer is independent of the data in "
          "input_buffer") {
        // change input buffer
        input_buffer.push_back(0x01);

        auto expected_payload = std::string("\x01\x07\x07\x12\x36\x57\x66\x77\x18", 9);
        REQUIRE(crc_element.payload() == expected_payload);
      }
    }
  }

  GIVEN(
      "A CRC element receiver of capacity 254 bytes, and output_crcelement constructed with an "
      "non-empty payload buffer and non-zero crc member") {
    constexpr size_t buffer_size = 254UL;
    using TestCRCElementProps = PF::Protocols::CRCElementProps<buffer_size>;
    using TestCRCElement = PF::Protocols::CRCElement<TestCRCElementProps::PayloadBuffer>;
    using TestCRCElementReceiver = PF::Protocols::CRCElementReceiver<buffer_size>;
    PF::Util::ByteVector<buffer_size> input_buffer;

    PF::HAL::SoftCRC32 crc32c{PF::HAL::crc32c_params};
    PF::Protocols::CRCElementReceiver<buffer_size> crc_element_receiver{crc32c};

    TestCRCElementProps::PayloadBuffer input_payload;
    auto body = std::string("\x23\x84\xf5\x6a\xac\xae", 6);
    PF::Util::convert_string_to_byte_vector(body, input_payload);
    TestCRCElement crc_element{input_payload};

    PF::Util::ByteVector<buffer_size> output_buffer;
    auto write_status = crc_element.write(output_buffer, crc32c);
    REQUIRE(write_status == PF::IndexStatus::ok);
    REQUIRE(crc_element.crc() == 0x15fc40f);
    REQUIRE(crc_element.payload() == body);

    WHEN("An empty input_buffer is given to the crc element receiver") {
      auto transform_status = crc_element_receiver.transform(input_buffer, crc_element);

      THEN("the transform status is equal to invalid parse") {
        REQUIRE(transform_status == TestCRCElementReceiver::Status::invalid_parse);
      }
      THEN(
          "After the transform method is called, the value returned by the output_crcelement's crc "
          "accessor method remains unchanged") {
        uint32_t expected_crc = 0x15fc40f;
        REQUIRE(crc_element.crc() == expected_crc);
      }
      THEN(
          "The output_crcelement's payload accessor method returns a reference to the payload "
          "given in its constructor") {
        REQUIRE(crc_element.payload() == body);
      }
      THEN(
          "The data in the output_crcelement's payload buffer is independent of the data in "
          "input_buffer") {
        input_buffer.push_back(0x02);
        REQUIRE(crc_element.payload() == body);
      }
      THEN("the constructor's payload buffer remains unchanged") { REQUIRE(input_payload == body); }
    }

    WHEN(
        "A body with a complete 4-byte header, and a payload consistent with the CRC field of the "
        "header, is given to the receiver") {
      THEN("the transform method reports ok status") {}
      THEN(
          "After the transform method is called, the value returned by the output_crcelement's crc "
          "accessor method is equal to the crc field of the input_buffer body's header") {}
      THEN(
          "The payload buffer returned by the output_crcelement's payload accessor method is equal "
          "to the payload from the input_buffer body") {}
      THEN(
          "The data in the output_crcelement's payload buffer is independent of the data in "
          "input_buffer") {}
    }
  }
}

SCENARIO(
    "Protocols::CRCElementSender: The crc element receiver correctly generates datagrams from "
    "payload",
    "[CRCElementSender]") {
  GIVEN("A CRC element sender of capacity 254 bytes") {
    constexpr size_t buffer_size = 254UL;
    using TestCRCElementProps = PF::Protocols::CRCElementProps<buffer_size>;
    using TestCRCElementSender = PF::Protocols::CRCElementSender<buffer_size>;

    PF::HAL::SoftCRC32 crc32c{PF::HAL::crc32c_params};
    TestCRCElementSender crc_element_sender{crc32c};

    WHEN("A payload is given along with a sufficiently large output buffer") {
      auto body = std::string("\x13\x03\x05\x06\x23", 5);

      TestCRCElementProps::PayloadBuffer input_payload;
      PF::Util::convert_string_to_byte_vector(body, input_payload);

      PF::Util::ByteVector<buffer_size> output_buffer;

      auto transform_status = crc_element_sender.transform(input_payload, output_buffer);

      THEN("the transform status is ok") {
        REQUIRE(transform_status == TestCRCElementSender::Status::ok);
      }
      auto expected_output = std::string("\x81\xfc\x34\x57\x13\x03\x05\x06\x23", 9);
      THEN("output buffer is as expected, equal to (crc + input_buffer)") {
        REQUIRE(output_buffer == expected_output);
      }
      THEN("The CRC field of the output buffer's header is consistent with the input_payload.") {
        // Calculated using the Sunshine Online CRC Calculator
        uint32_t expected_crc = 0x81FC3457;
        auto crc_compute = crc32c.compute(input_payload.buffer(), input_payload.size());
        REQUIRE(expected_crc == crc_compute);
      }
      THEN("The contents of the output buffer remains unchanged") {
        REQUIRE(output_buffer == expected_output);
      }
    }
  }
}
