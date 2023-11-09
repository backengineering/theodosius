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

#include <recomp/symbol_table.hpp>

namespace theo::recomp {
symbol_table_t::symbol_table_t(const std::vector<decomp::symbol_t>&& syms) {
  std::for_each(syms.begin(), syms.end(), [&](decomp::symbol_t sym) {
    m_table.insert({sym.hash(), sym});
  });
}

void symbol_table_t::put_symbol(decomp::symbol_t& sym) {
  m_table.insert({sym.hash(), sym});
}

void symbol_table_t::put_symbols(std::vector<decomp::symbol_t>& syms) {
  std::for_each(syms.begin(), syms.end(),
                [&](decomp::symbol_t sym) { put_symbol(sym); });
}

void symbol_table_t::for_each(std::function<void(decomp::symbol_t& sym)> fn) {
  for (auto itr = m_table.begin(); itr != m_table.end(); ++itr)
    fn(itr->second);
}

std::optional<decomp::symbol_t*> symbol_table_t::sym_from_hash(
    std::size_t hash) {
  return m_table.count(hash) ? &m_table.at(hash)
                             : std::optional<decomp::symbol_t*>{};
}

std::optional<decomp::symbol_t*> symbol_table_t::sym_from_alloc(
    std::uintptr_t allocated_at) {
  auto res =
      std::find_if(m_table.begin(), m_table.end(),
                   [&](std::pair<const std::size_t, decomp::symbol_t> itr) {
                     return itr.second.allocated_at() == allocated_at;
                   });

  return res != m_table.end() ? &res->second
                              : std::optional<decomp::symbol_t*>{};
}

std::map<std::size_t, decomp::symbol_t>& symbol_table_t::get() {
  return m_table;
}

std::uint32_t symbol_table_t::size() {
  return m_table.size();
}
}  // namespace theo::recomp