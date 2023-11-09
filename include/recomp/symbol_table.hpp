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
#include <algorithm>
#include <functional>
#include <map>
#include <optional>
#include <vector>

#include <decomp/symbol.hpp>

namespace theo::recomp {
/// <summary>
/// this class is a high level wrapper for a hashmap that contains
/// decomp::symbol_t values. the symbol values are references by a hashcode.
/// </summary>
class symbol_table_t {
 public:
  /// <summary>
  /// default constructor. does nothing.
  /// </summary>
  symbol_table_t() {}

  /// <summary>
  /// this constructor will populate the m_table private field with symbols.
  /// </summary>
  /// <param name="syms">vector of decomp::symbol_t</param>
  symbol_table_t(const std::vector<decomp::symbol_t>&& syms);

  /// <summary>
  /// add symbol to m_table
  /// </summary>
  /// <param name="sym">symbol to be added.</param>
  void put_symbol(decomp::symbol_t& sym);

  /// <summary>
  /// add a vector of symbol to m_table
  /// </summary>
  /// <param name="syms"></param>
  void put_symbols(std::vector<decomp::symbol_t>& syms);

  /// <summary>
  /// returns an optional pointer to a symbol from the symbol table given the
  /// symbols hash (hash of its name) the hash is produced by
  /// theo::decomp::symbol_t::hash
  /// </summary>
  /// <param name="hash">hashcode of the symbol to get from the symbol
  /// table...</param> <returns>returns an optional pointer to a
  /// theo::decomp::symbol_t</returns>
  std::optional<decomp::symbol_t*> sym_from_hash(std::size_t hash);

  /// <summary>
  /// returns an optional pointer to a symbol given its allocation location.
  /// </summary>
  /// <param name="allocated_at">the address where the symbol is allocated
  /// at.</param> <returns>returns an optional pointer to a
  /// theo::decomp::symbol_t</returns>
  std::optional<decomp::symbol_t*> sym_from_alloc(std::uintptr_t allocated_at);

  /// <summary>
  /// this function is a wrapper function that allows you to get at each entry
  /// in the symbol table by reference.
  /// </summary>
  /// <param name="fn">a callback function that will be called for each
  /// symbol</param>
  void for_each(std::function<void(decomp::symbol_t& sym)> fn);

  /// <summary>
  /// returns the size of the symbol table.
  /// </summary>
  /// <returns>returns the size of the symbol table.</returns>
  std::uint32_t size();

  /// <summary>
  /// getter for underlying symbol hash map.
  /// </summary>
  /// <returns>returns the symbol hashmap.</returns>
  std::map<std::size_t, decomp::symbol_t>& get();

 private:
  std::map<std::size_t, decomp::symbol_t> m_table;
};
}  // namespace theo::recomp