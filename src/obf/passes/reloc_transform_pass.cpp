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

#include <obf/passes/reloc_transform_pass.hpp>

namespace theo::obf {
reloc_transform_pass_t* reloc_transform_pass_t::get() {
  static reloc_transform_pass_t obj;
  return &obj;
}

void reloc_transform_pass_t::generic_pass(decomp::symbol_t* sym,
                                          sym_map_t& sym_tbl) {
  std::optional<recomp::reloc_t*> reloc;
  if (!(reloc = has_legit_reloc(sym)).has_value())
    return;

  spdlog::info("adding transformations to relocation in symbol: {}",
               sym->name());

  xed_error_enum_t err;
  xed_decoded_inst_t inst;
  xed_state_t istate{XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b};
  xed_decoded_inst_zero_set_mode(&inst, &istate);

  if ((err = xed_decode(&inst, sym->data().data(), sym->data().size())) !=
      XED_ERROR_NONE) {
    spdlog::error("failed to decode instruction, reason: {} in symbol: {}",
                  xed_error_enum_t2str(err), sym->name());

    assert(err == XED_ERROR_NONE);
  }

  auto transforms_bytes = transform::generate(&inst, reloc.value(), 3, 6);
  sym->data().insert(sym->data().end(), transforms_bytes.begin(),
                     transforms_bytes.end());
};

std::optional<recomp::reloc_t*> reloc_transform_pass_t::has_legit_reloc(
    decomp::symbol_t* sym) {
  auto res =  // see if there are any relocations with offset not equal to
              // zero... relocations with zero mean its a relocation to the next
              // instruction...
      std::find_if(
          sym->relocs().begin(), sym->relocs().end(),
          [&](recomp::reloc_t& reloc) -> bool { return reloc.offset(); });

  return res != sym->relocs().end() ? &(*res)
                                    : std::optional<recomp::reloc_t*>();
}
}  // namespace theo::obf