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
#include <cstdint>
#include <linuxpe>
#include <optional>
#include <set>
#include <tuple>
#include <vector>

#include <decomp/routine.hpp>
#include <recomp/symbol_table.hpp>

#include <coff/archive.hpp>
#include <coff/image.hpp>

/// <summary>
/// the namespace that contains all of the decomposition related code.
/// </summary>
namespace theo::decomp {

/// <summary>
/// meta symbol data. consists of the coff image which contains the coff symbol,
/// the coff symbol itself, and the size (if any) of the symbol.
/// </summary>
using sym_data_t = std::tuple<coff::image_t*, coff::symbol_t*, std::uint32_t>;

/// <summary>
/// the main decomposition class which is responsible for breaking down lib file
/// into coff files, and extracted used symbols from the coff files.
/// </summary>
class decomp_t {
 public:
  /// <summary>
  /// the explicit constructor for decomp_t
  /// </summary>
  /// <param name="lib">vector of bytes containing the lib file.</param>
  /// <param name="syms">symbol table that gets populated and managed by this
  /// class.</param>
  explicit decomp_t(std::vector<std::uint8_t>& lib,
                    recomp::symbol_table_t* syms);

  /// <summary>
  /// gets all of the routine objects.
  /// </summary>
  /// <returns>vector of routine objects.</returns>
  std::vector<routine_t> rtns();

  /// <summary>
  /// gets a vector of bytes consisting of the lib file.
  /// </summary>
  /// <returns>a vector of bytes consisting of the lib file.</returns>
  std::vector<std::uint8_t> lib();

  /// <summary>
  /// gets all the obj files as a vector of a vector of bytes.
  /// </summary>
  /// <returns>all the obj files as a vector of a vector of bytes.</returns>
  std::vector<std::vector<std::uint8_t>> objs();

  /// <summary>
  /// gets the symbol table.
  /// </summary>
  /// <returns>the symbol table.</returns>
  recomp::symbol_table_t* syms();

  /// <summary>
  /// gets the section hash table section header --> hash of the section header
  /// ptr.
  /// </summary>
  /// <returns>the section hash table section header --> hash of the section
  /// header ptr.</returns>
  std::map<coff::section_header_t*, std::size_t>& scn_hash_tbl();

  /// <summary>
  /// decomposes (extracts) the symbols used. this function determines all used
  /// symbols given the entry point.
  /// </summary>
  /// <param name="entry_sym">the entry point symbol name.</param>
  /// <returns>returns an optional pointer to the symbol table. no value in the
  /// optional object on failure.</returns>
  std::optional<recomp::symbol_table_t*> decompose(std::string& entry_sym);

 private:
  /// <summary>
  /// extracts used symbols from coff files.
  /// </summary>
  /// <param name="entry_sym">the entry point symbol name</param>
  /// <returns>number of symbols used</returns>
  std::uint32_t ext_used_syms(const std::string&& entry_sym);

  /// <summary>
  /// get symbol meta data by name.
  /// </summary>
  /// <param name="name">symbol name</param>
  /// <returns>optional symbol meta data if it exists.</returns>
  std::optional<sym_data_t> get_symbol(const std::string_view& name);

  /// <summary>
  /// the next symbol in the section.
  /// </summary>
  /// <param name="img">coff image that contains the symbol.</param>
  /// <param name="hdr">coff section header of the section that contains the
  /// symbol.</param>
  /// <param name="s">symbol in which to get the next one of.</param>
  /// <returns>offset into the section where the next symbol is at.</returns>
  std::uint32_t next_sym(coff::image_t* img,
                         coff::section_header_t* hdr,
                         coff::symbol_t* s);

  const std::vector<std::uint8_t> m_lib;
  std::vector<std::vector<std::uint8_t>> m_objs;
  std::vector<routine_t> m_rtns;
  std::set<sym_data_t> m_used_syms;
  std::set<coff::image_t*> m_processed_objs;
  std::map<coff::section_header_t*, std::size_t> m_scn_hash_tbl;
  std::map<std::size_t, std::vector<sym_data_t>> m_lookup_tbl;
  recomp::symbol_table_t* m_syms;
};
}  // namespace theo::decomp