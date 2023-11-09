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
#include <decomp/decomp.hpp>
#include <obf/engine.hpp>
#include <recomp/symbol_table.hpp>

/// <summary>
/// this namespace encompasses all recomposition related code.
/// </summary>
namespace theo::recomp {

/// <summary>
/// a function which is called by recomp_t to resolve external symbols
/// </summary>
using resolver_t = std::function<std::uintptr_t(std::string)>;

/// <summary>
/// a function which is called by recomp_t to copy symbols into memory.
/// </summary>
using copier_t = std::function<void(std::uintptr_t, void*, std::uint32_t)>;

/// <summary>
/// a function which is called to allocate space for a symbol.
///
/// the first param is the size of the symbol, the second param is the
/// characteristics of the section which the symbol is allocated in.
/// </summary>
using allocator_t =
    std::function<std::uintptr_t(std::uint32_t,
                                 coff::section_characteristics_t)>;

/// <summary>
/// the main class responsible for recomposition
/// </summary>
class recomp_t {
 public:
  /// <summary>
  /// the explicit constructor for the recomp_t class.
  /// </summary>
  /// <param name="dcmp">pointer to a decomp_t class.</param>
  /// <param name="alloc">lambda function which is used to  allocate memory for
  /// symbols.</param> <param name="copy">lambda function used to copy bytes
  /// into allocations.</param> <param name="resolve">lambda function used to
  /// resolve external symbols.</param>
  explicit recomp_t(decomp::decomp_t* dcmp,
                    allocator_t alloc,
                    copier_t copy,
                    resolver_t resolve);

  /// <summary>
  /// when called, this function allocates space for every symbol.
  /// </summary>
  void allocate();

  /// <summary>
  /// when called, this function resolves all relocations in every symbol.
  /// </summary>
  void resolve();

  /// <summary>
  /// when called, this function copies symbols into allocations.
  /// </summary>
  void copy_syms();

  /// <summary>
  /// setter for the allocater lambda function.
  /// </summary>
  /// <param name="alloc">lambda function which allocates memory for
  /// symbols.</param>
  void allocator(allocator_t alloc);

  /// <summary>
  /// setter for the copier lambda function.
  /// </summary>
  /// <param name="copy">copier lambda function used to copy bytes into
  /// allocations made by the allocator.</param>
  void copier(copier_t copy);

  /// <summary>
  /// setter for the resolve lambda function.
  /// </summary>
  /// <param name="resolve">lambda function to resolve external symbols.</param>
  void resolver(resolver_t resolve);

  /// <summary>
  /// resolves the address of a function given its name.
  /// </summary>
  /// <param name="sym">the name of the symbol to resolve the location
  /// of.</param> <returns>the address of the symbol.</returns>
  std::uintptr_t resolve(const std::string&& sym);

 private:
  decomp::decomp_t* m_dcmp;
  resolver_t m_resolver;
  copier_t m_copier;
  allocator_t m_allocator;
};
}  // namespace theo::recomp