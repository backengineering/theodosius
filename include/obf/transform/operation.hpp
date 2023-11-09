// Copyright (c) 2022, _xeroxz
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

#pragma once
#include <spdlog/spdlog.h>
#include <bit>
#include <bitset>
#include <functional>
#include <map>
#include <random>

#define XED_ENCODER
extern "C" {
#include <xed-decode.h>
#include <xed-interface.h>
}

/// <summary>
/// this namespace encompasses the code for transforming relocations.
/// </summary>
namespace theo::obf::transform {

/// <summary>
/// lambda function which takes in a 64bit value (relocation address) and a
/// 32bit value (random value used in transformation).
/// </summary>
using transform_t = std::function<std::size_t(std::size_t, std::uint32_t)>;

/// <summary>
/// operation_t is the base class for all types of transformations. classes that
/// inherit this class are singleton and simply call the super constructor
/// (operation_t::operation_t).
/// </summary>
class operation_t {
 public:
  /// <summary>
  /// explicit constructor for operation_t
  /// </summary>
  /// <param name="op">lambda function when executed applies
  /// transformations.</param> <param name="type">type of transformation, such
  /// as XOR, ADD, SUB, etc...</param>
  explicit operation_t(transform_t op, xed_iclass_enum_t type)
      : m_transform(op), m_type(type) {}

  /// <summary>
  /// generates a native transform instruction given an existing instruction. it
  /// works like so:
  ///
  /// mov rax, &MessageBoxA ; original instruction with relocation
  ///
  /// ; this function takes the first operand and out of the original
  /// ; instruction and uses it to generate a transformation.
  ///
  /// xor rax, 0x39280928   ; this would be an example output for the xor
  ///                       ;operation.
  ///
  /// </summary>
  /// <param name="inst">instruction with a relocation to generate a
  /// transformation for.</param> <param name="imm">random 32bit number used in
  /// the generate transform.</param> <returns>returns the bytes of the native
  /// instruction that was encoded.</returns>
  std::vector<std::uint8_t> native(const xed_decoded_inst_t* inst,
                                   std::uint32_t imm) {
    std::uint32_t inst_len = {};
    std::uint8_t inst_buff[XED_MAX_INSTRUCTION_BYTES];

    xed_error_enum_t err;
    xed_encoder_request_init_from_decode((xed_decoded_inst_s*)inst);
    xed_encoder_request_t* req = (xed_encoder_request_t*)inst;

    switch (m_type) {
      case XED_ICLASS_ROR:
      case XED_ICLASS_ROL:
        xed_encoder_request_set_uimm0(req, imm, 1);
        break;
      default:
        xed_encoder_request_set_uimm0(req, imm, 4);
        break;
    }

    xed_encoder_request_set_iclass(req, m_type);
    xed_encoder_request_set_operand_order(req, 1, XED_OPERAND_IMM0);

    if ((err = xed_encode(req, inst_buff, sizeof(inst_buff), &inst_len)) !=
        XED_ERROR_NONE) {
      spdlog::error("failed to encode instruction... reason: {}",
                    xed_error_enum_t2str(err));

      assert(err == XED_ERROR_NONE);
    }

    return std::vector<std::uint8_t>(inst_buff, inst_buff + inst_len);
  }

  /// <summary>
  /// gets the inverse operation of the current operation.
  /// </summary>
  /// <returns>the inverse operation of the current operation.</returns>
  xed_iclass_enum_t inverse() { return m_inverse_op[m_type]; }

  /// <summary>
  /// gets a pointer to the lambda function which contains the transform logic.
  /// </summary>
  /// <returns>a pointer to the lambda function which contains the transform
  /// logic.</returns>
  transform_t* get_transform() { return &m_transform; }

  /// <summary>
  /// gets the operation type. such as XED_ICLASS_ADD, XED_ICLASS_SUB, etc...
  /// </summary>
  /// <returns>the operation type. such as XED_ICLASS_ADD, XED_ICLASS_SUB,
  /// etc...</returns>
  xed_iclass_enum_t type() { return m_type; }

  /// <summary>
  /// generate a random number in a range.
  /// </summary>
  /// <param name="lowest">lowest value of the range.</param>
  /// <param name="largest">highest value of the range.</param>
  /// <returns>a random value in a range.</returns>
  static std::size_t random(std::size_t lowest, std::size_t largest) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::size_t> distr(lowest, largest);
    return distr(gen);
  }

 private:
  transform_t m_transform;
  xed_iclass_enum_t m_type;

  std::map<xed_iclass_enum_t, xed_iclass_enum_t> m_inverse_op = {
      {XED_ICLASS_ADD, XED_ICLASS_SUB},
      {XED_ICLASS_SUB, XED_ICLASS_ADD},
      {XED_ICLASS_ROL, XED_ICLASS_ROR},
      {XED_ICLASS_ROR, XED_ICLASS_ROL},
      {XED_ICLASS_XOR, XED_ICLASS_XOR}};
};

}  // namespace theo::obf::transform