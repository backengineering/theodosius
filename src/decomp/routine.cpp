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

#include <decomp/routine.hpp>

namespace theo::decomp {
routine_t::routine_t(coff::symbol_t* sym,
                     coff::image_t* img,
                     coff::section_header_t* scn,
                     std::vector<std::uint8_t>& fn)
    : m_img(img), m_scn(scn), m_data(fn), m_sym(sym) {}

decomp::symbol_t routine_t::decompose() {
  std::vector<recomp::reloc_t> relocs;
  auto scn_relocs = reinterpret_cast<coff::reloc_t*>(
      m_scn->ptr_relocs + reinterpret_cast<std::uint8_t*>(m_img));

  // extract all of the relocations that this function has...
  //
  for (auto idx = 0u; idx < m_scn->num_relocs; ++idx) {
    auto scn_reloc = &scn_relocs[idx];
    // if the reloc is in the current function...
    if (scn_reloc->virtual_address >= m_sym->value &&
        scn_reloc->virtual_address < m_sym->value + m_data.size()) {
      auto sym_reloc = m_img->get_symbol(scn_relocs[idx].symbol_index);
      auto sym_name = symbol_t::name(m_img, sym_reloc);
      auto sym_hash = decomp::symbol_t::hash(sym_name.data());
      relocs.push_back(
          recomp::reloc_t(scn_reloc->virtual_address - m_sym->value, sym_hash,
                          sym_name.data()));
    }
  }

  // return the created symbol_t for this function...
  //
  return decomp::symbol_t(m_img, symbol_t::name(m_img, m_sym).data(),
                          m_sym->value, m_data, m_scn, m_sym, relocs,
                          sym_type_t::function);
}

coff::section_header_t* routine_t::scn() {
  return m_scn;
}

std::vector<std::uint8_t> routine_t::data() {
  return m_data;
}
}  // namespace theo::decomp