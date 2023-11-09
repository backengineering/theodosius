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

#include <obf/passes/func_split_pass.hpp>

namespace theo::obf {
func_split_pass_t* func_split_pass_t::get() {
  static func_split_pass_t obj;
  return &obj;
}

void func_split_pass_t::generic_pass(decomp::symbol_t* sym,
                                     sym_map_t& sym_tbl) {
  std::uint32_t offset = {};
  xed_error_enum_t err;
  xed_decoded_inst_t instr;
  std::vector<decomp::symbol_t> result;
  xed_state_t istate{XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b};
  xed_decoded_inst_zero_set_mode(&instr, &istate);

  // keep looping over the function, lower the number of bytes each time...
  //
  while ((err = xed_decode(&instr, sym->data().data() + offset,
                           sym->data().size() - offset)) == XED_ERROR_NONE) {
    // symbol name is of the format: symbol@instroffset, I.E: main@11...
    //
    auto new_sym_name = decomp::symbol_t::name(sym->img(), sym->sym());

    // first instruction doesnt need the @offset...
    //
    if (offset)
      new_sym_name.append("@").append(std::to_string(offset));

    std::vector<recomp::reloc_t> relocs;
    auto scn_relocs = reinterpret_cast<coff::reloc_t*>(
        sym->scn()->ptr_relocs + reinterpret_cast<std::uint8_t*>(sym->img()));

    // find if this instruction has a relocation or not...
    // if so, return the reloc_t...
    //
    auto reloc = std::find_if(
        scn_relocs, scn_relocs + sym->scn()->num_relocs,
        [&](coff::reloc_t reloc) {
          return reloc.virtual_address >= sym->sym()->value + offset &&
                 reloc.virtual_address <
                     sym->sym()->value + offset +
                         xed_decoded_inst_get_length(&instr);
        });

    // if there is indeed a reloc for this instruction...
    //
    if (reloc != scn_relocs + sym->scn()->num_relocs) {
      auto sym_reloc = sym->img()->get_symbol(reloc->symbol_index);
      auto sym_name = decomp::symbol_t::name(sym->img(), sym_reloc);
      auto sym_hash = decomp::symbol_t::hash(sym_name.data());
      auto reloc_offset = reloc->virtual_address - sym->sym()->value - offset;
      relocs.push_back(
          recomp::reloc_t(reloc_offset, sym_hash, sym_name.data()));
    }
    // add a reloc to the next instruction...
    // note that the offset is ZERO... comp_t will understand that
    // relocs with offset ZERO means the next instructions...
    //
    auto next_inst_sym = decomp::symbol_t::name(sym->img(), sym->sym())
                             .append("@")
                             .append(std::to_string(
                                 offset + xed_decoded_inst_get_length(&instr)));

    relocs.push_back(recomp::reloc_t(0, decomp::symbol_t::hash(next_inst_sym),
                                     next_inst_sym.data()));
    // get the instructions bytes
    //
    std::vector<std::uint8_t> inst_bytes(
        sym->data().data() + offset,
        sym->data().data() + offset + xed_decoded_inst_get_length(&instr));

    result.push_back(decomp::symbol_t(sym->img(), new_sym_name, offset,
                                      inst_bytes, sym->scn(), sym->sym(),
                                      relocs, decomp::sym_type_t::instruction));
    // after creating the symbol and dealing with relocs then print the
    // information we have concluded...
    //
    char buff[255];
    offset += xed_decoded_inst_get_length(&instr);
    xed_format_context(XED_SYNTAX_INTEL, &instr, buff, sizeof buff, NULL, NULL,
                       NULL);
    spdlog::info("[func_split_pass_t] {}: {}", new_sym_name, buff);
    // need to set this so that instr can be used to decode again...
    xed_decoded_inst_zero_set_mode(&instr, &istate);
  }

  // remove the relocation to the next symbol from the last instruction
  //
  auto& last_inst = result.back();
  auto& last_inst_relocs = last_inst.relocs();
  last_inst_relocs.erase(last_inst_relocs.end() - 1);

  // insert the split instructions into the symbol table.
  //
  for (auto& symbol : result) {
    auto itr = sym_tbl.find(symbol.hash());
    if (itr != sym_tbl.end())
      itr->second = symbol;
    else
      sym_tbl.insert({symbol.hash(), symbol});
  }
}
}  // namespace theo::obf