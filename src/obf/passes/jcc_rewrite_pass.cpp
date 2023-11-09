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

#include <obf/passes/jcc_rewrite_pass.hpp>
#include <obf/passes/next_inst_pass.hpp>

namespace theo::obf {
jcc_rewrite_pass_t* jcc_rewrite_pass_t::get() {
  static jcc_rewrite_pass_t obj;
  return &obj;
}

void jcc_rewrite_pass_t::generic_pass(decomp::symbol_t* sym,
                                      sym_map_t& sym_tbl) {
  std::int32_t disp = {};
  xed_decoded_inst_t inst;
  xed_state_t istate{XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b};
  xed_decoded_inst_zero_set_mode(&inst, &istate);
  xed_decode(&inst, sym->data().data(), XED_MAX_INSTRUCTION_BYTES);

  // if the instruction is branching...
  if ((disp = xed_decoded_inst_get_branch_displacement(&inst))) {
    disp += xed_decoded_inst_get_length(&inst);

    // update displacement...
    xed_decoded_inst_set_branch_displacement(
        &inst, sym->data().size() - xed_decoded_inst_get_length(&inst),
        xed_decoded_inst_get_branch_displacement_width(&inst));

    xed_encoder_request_init_from_decode(&inst);
    xed_encoder_request_t* req = &inst;

    // update jcc in the buffer...
    std::uint32_t len = {};
    xed_encode(req, sym->data().data(), xed_decoded_inst_get_length(&inst),
               &len);

    // create a relocation to the instruction the branch would normally go
    // too...
    auto offset = disp < 0 ? sym->offset() - std::abs(disp)
                           : sym->offset() + std::abs(disp);

    auto sym_name =
        std::string(
            sym->sym()->name.to_string(sym->img()->get_strings()).data())
            .append("@")
            .append(std::to_string(offset));

    sym->relocs().push_back(
        recomp::reloc_t(0, decomp::symbol_t::hash(sym_name), sym_name.data()));

    // run next_inst_pass on this symbol to generate the transformations for the
    // relocation to the jcc branch dest instruction...
    next_inst_pass_t::get()->generic_pass(sym, sym_tbl);
  }
};
}  // namespace theo::obf