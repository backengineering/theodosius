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
#include <decomp/decomp.hpp>
#include <obf/engine.hpp>
#include <recomp/recomp.hpp>
#include <recomp/symbol_table.hpp>

#include <obf/passes/jcc_rewrite_pass.hpp>
#include <obf/passes/next_inst_pass.hpp>
#include <obf/passes/reloc_transform_pass.hpp>

#include <optional>
#include <tuple>
#include <vector>

#define XED_ENCODER
extern "C" {
#include <xed-decode.h>
#include <xed-interface.h>
}

/// <summary>
/// The outer most encompassing namespace of this project.
/// </summary>
namespace theo {

/// <summary>
/// tuple of functions used by theo to allocate, copy, and resolve symbols.
/// </summary>
using lnk_fns_t =
    std::tuple<recomp::allocator_t, recomp::copier_t, recomp::resolver_t>;

/// <summary>
/// the main class which encapsulates a symbol table, decomp, and recomp
/// objects. This class is a bridge that connects all three: decomp, obf,
/// recomp.
///
/// You will create an object of this type when using theo.
/// </summary>
class theo_t {
 public:
  /// <summary>
  /// explicit constructor for theo class.
  /// </summary>
  /// <param name="lib">a vector of bytes consisting of a lib</param>
  /// <param name="lnkr_fns"></param>
  /// <param name="entry_sym">the name of the function which will be used as the
  /// entry point</param>
  explicit theo_t(std::vector<std::uint8_t>& lib,
                  lnk_fns_t lnkr_fns,
                  const std::string&& entry_sym);

  /// <summary>
  /// decomposes the lib file and return the number of symbols that are used.
  /// </summary>
  /// <returns>optional amount of symbols that are used. no value if
  /// decomposition fails.</returns>
  std::optional<std::uint32_t> decompose();

  /// <summary>
  /// compose the decomposed module. This will run obfuscation passes, the map
  /// and resolve symbols to each other.
  /// </summary>
  /// <returns>returns the address of the entry point symbol</returns>
  std::uintptr_t compose();

  /// <summary>
  /// given the name of a symbol, it returns the address of where its mapped.
  /// </summary>
  /// <param name="sym">the name of the symbol</param>
  /// <returns>the address of the symbol</returns>
  std::uintptr_t resolve(const std::string&& sym);

 private:
  std::string m_entry_sym;
  decomp::decomp_t m_dcmp;
  recomp::recomp_t m_recmp;
  recomp::symbol_table_t m_sym_tbl;
};
}  // namespace theo