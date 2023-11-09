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

#include <theo.hpp>

namespace theo {
theo_t::theo_t(std::vector<std::uint8_t>& lib,
               lnk_fns_t lnkr_fns,
               const std::string&& entry_sym)
    : m_dcmp(lib, &m_sym_tbl),
      m_recmp(&m_dcmp, {}, {}, {}),
      m_entry_sym(entry_sym) {
  m_recmp.allocator(std::get<0>(lnkr_fns));
  m_recmp.copier(std::get<1>(lnkr_fns));
  m_recmp.resolver(std::get<2>(lnkr_fns));
}

std::optional<std::uint32_t> theo_t::decompose() {
  auto res = m_dcmp.decompose(m_entry_sym);
  if (!res.has_value()) {
    spdlog::error("failed to decompose...\n");
    return {};
  }

  spdlog::info("decompose successful... {} symbols", res.value()->size());
  return res.value()->size();
}

std::uintptr_t theo_t::compose() {
  auto engine = obf::engine_t::get();
  auto& sym_tbl = m_sym_tbl.get();

  // run obfuscation engine on function symbols...
  //
  m_sym_tbl.for_each([&](decomp::symbol_t& sym) {
    engine->for_each(&sym, [&](decomp::symbol_t* sym, obf::pass_t* pass) {
      if (sym->type() == decomp::sym_type_t::function)
        pass->generic_pass(sym, sym_tbl);
    });
  });

  // run obfuscation engine on instruction symbols...
  //
  m_sym_tbl.for_each([&](decomp::symbol_t& sym) {
    engine->for_each(&sym, [&](decomp::symbol_t* sym, obf::pass_t* pass) {
      if (sym->type() == decomp::sym_type_t::instruction)
        pass->generic_pass(sym, sym_tbl);
    });
  });

  // run obfuscation engine on all other symbols...
  //
  m_sym_tbl.for_each([&](decomp::symbol_t& sym) {
    engine->for_each(&sym, [&](decomp::symbol_t* sym, obf::pass_t* pass) {
      if (sym->type() != decomp::sym_type_t::instruction &&
          sym->type() != decomp::sym_type_t::function)
        pass->generic_pass(sym, sym_tbl);
    });
  });

  m_recmp.allocate();
  m_recmp.resolve();
  m_recmp.copy_syms();
  return m_recmp.resolve(m_entry_sym.data());
}

std::uintptr_t theo_t::resolve(const std::string&& sym) {
  auto val = m_sym_tbl.sym_from_hash(decomp::symbol_t::hash(sym));
  if (!val.has_value())
    return {};

  return val.value()->allocated_at();
}
}  // namespace theo