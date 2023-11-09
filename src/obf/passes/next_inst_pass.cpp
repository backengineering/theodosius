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

#include <obf/passes/next_inst_pass.hpp>

namespace theo::obf {
next_inst_pass_t* next_inst_pass_t::get() {
  static next_inst_pass_t obj;
  return &obj;
}

void next_inst_pass_t::generic_pass(decomp::symbol_t* sym, sym_map_t& sym_tbl) {
  std::optional<recomp::reloc_t*> reloc;
  if (!(reloc = has_next_inst_reloc(sym)).has_value())
    return;

  xed_decoded_inst_t inst = m_tmp_inst;
  std::vector<std::uint8_t> new_inst_bytes =
      transform::generate(&inst, reloc.value(), 3, 6);

  // add a push [rip+offset] and update reloc->offset()...
  //
  std::uint32_t inst_len = {};
  std::uint8_t inst_buff[XED_MAX_INSTRUCTION_BYTES];

  xed_error_enum_t err;
  xed_encoder_request_t req;
  xed_state_t istate{XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b};

  xed_encoder_request_zero_set_mode(&req, &istate);
  xed_encoder_request_set_effective_operand_width(&req, 64);
  xed_encoder_request_set_iclass(&req, XED_ICLASS_PUSH);

  xed_encoder_request_set_mem0(&req);
  xed_encoder_request_set_operand_order(&req, 0, XED_OPERAND_MEM0);

  xed_encoder_request_set_base0(&req, XED_REG_RIP);
  xed_encoder_request_set_seg0(&req, XED_REG_INVALID);
  xed_encoder_request_set_index(&req, XED_REG_INVALID);
  xed_encoder_request_set_scale(&req, 0);

  xed_encoder_request_set_memory_operand_length(&req, 8);
  xed_encoder_request_set_memory_displacement(&req, new_inst_bytes.size() + 1,
                                              1);

  if ((err = xed_encode(&req, inst_buff, sizeof(inst_buff), &inst_len)) !=
      XED_ERROR_NONE) {
    spdlog::info("failed to encode instruction... reason: {}",
                 xed_error_enum_t2str(err));

    assert(err == XED_ERROR_NONE);
  }

  new_inst_bytes.insert(new_inst_bytes.begin(), inst_buff,
                        inst_buff + inst_len);

  // put a return instruction at the end of the decrypt instructions...
  //
  new_inst_bytes.push_back(0xC3);

  sym->data().insert(sym->data().end(), new_inst_bytes.begin(),
                     new_inst_bytes.end());

  reloc.value()->offset(sym->data().size());
  sym->data().resize(sym->data().size() + 8);
}

std::optional<recomp::reloc_t*> next_inst_pass_t::has_next_inst_reloc(
    decomp::symbol_t* sym) {
  auto res = std::find_if(
      sym->relocs().begin(), sym->relocs().end(),
      [&](recomp::reloc_t& reloc) -> bool { return !reloc.offset(); });

  return res != sym->relocs().end() ? &(*res)
                                    : std::optional<recomp::reloc_t*>();
}
}  // namespace theo::obf