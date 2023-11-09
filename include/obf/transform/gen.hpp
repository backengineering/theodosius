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
#include <obf/transform/transform.hpp>
#include <recomp/reloc.hpp>

namespace theo::obf::transform {
/// <summary>
/// generate a sequence of transformations given an instruction that has a
/// relocation in it.
/// </summary>
/// <param name="inst">instruction that has a relocation in it.</param>
/// <param name="reloc">meta data relocation object for the instruction.</param>
/// <param name="low">lowest number of transformations to generate.</param>
/// <param name="high">highest number of transformations to generate.</param>
/// <returns></returns>
inline std::vector<std::uint8_t> generate(xed_decoded_inst_t* inst,
                                          recomp::reloc_t* reloc,
                                          std::uint8_t low,
                                          std::uint8_t high) {
  auto num_transforms = transform::operation_t::random(low, high);
  auto num_ops = transform::operations.size();
  std::vector<std::uint8_t> new_inst_bytes;

  std::uint32_t inst_len = {};
  std::uint8_t inst_buff[XED_MAX_INSTRUCTION_BYTES];
  xed_encoder_request_t req;

  xed_state_t istate{XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b};
  xed_encoder_request_zero_set_mode(&req, &istate);
  xed_encoder_request_set_effective_operand_width(&req, 64);
  xed_encoder_request_set_iclass(&req, XED_ICLASS_PUSHFQ);
  xed_encode(&req, inst_buff, sizeof(inst_buff), &inst_len);
  new_inst_bytes.insert(new_inst_bytes.end(), inst_buff, inst_buff + inst_len);

  for (auto cnt = 0u; cnt < num_transforms; ++cnt) {
    std::uint32_t imm = transform::operation_t::random(
        0, std::numeric_limits<std::int32_t>::max());

    auto itr = transform::operations.begin();
    std::advance(itr, transform::operation_t::random(0, num_ops - 1));
    auto transform_bytes = itr->second->native(inst, imm);
    new_inst_bytes.insert(new_inst_bytes.end(), transform_bytes.begin(),
                          transform_bytes.end());

    reloc->add_transform(
        {transform::operations[itr->second->inverse()]->get_transform(), imm});
  }

  xed_encoder_request_zero_set_mode(&req, &istate);
  xed_encoder_request_set_effective_operand_width(&req, 64);
  xed_encoder_request_set_iclass(&req, XED_ICLASS_POPFQ);
  xed_encode(&req, inst_buff, sizeof(inst_buff), &inst_len);
  new_inst_bytes.insert(new_inst_bytes.end(), inst_buff, inst_buff + inst_len);

  // inverse the order in which the transformations are executed...
  //
  std::reverse(reloc->get_transforms().begin(), reloc->get_transforms().end());
  return new_inst_bytes;
}
}  // namespace theo::obf::transform