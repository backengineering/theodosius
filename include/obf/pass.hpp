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
#include <decomp/symbol.hpp>
#include <obf/transform/gen.hpp>

#define XED_ENCODER
extern "C" {
#include <xed-decode.h>
#include <xed-interface.h>
}

/// <summary>
/// this is the main namespace for obfuscation related things.
/// </summary>
namespace theo::obf {

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

// symbol table map associated with the symbol table data structure.
//
using sym_map_t = std::map<std::size_t, decomp::symbol_t>;

/// <summary>
/// the pass_t class is a base clase for all passes made. you must override the
/// pass_t::run virtual function and declare the logic of your pass there.
///
/// in the constructor of your pass you must call the super constructor (the
/// pass_t constructor) and pass it the type of symbol which you are interesting
/// in receiving.
/// </summary>
class pass_t {
 public:
  /// <summary>
  /// the explicit constructor of the pass_t base class.
  /// </summary>
  /// <param name="sym_type">the type of symbol in which the pass will run on.
  /// every symbol passed to the virtual "run" instruction will be of this
  /// type.</param>
  explicit pass_t(decomp::sym_type_t sym_type) : m_sym_type(sym_type){};

  /// <summary>
  /// This virtual method is invoked before the recomp stage of Theodosius. This
  /// allows you to manipulate symbols in a generic manner.
  /// </summary>
  /// <param name="sym">a symbol of the same type of m_sym_type.</param>
  virtual void generic_pass(decomp::symbol_t* sym, sym_map_t& sym_tbl) = 0;

  /// <summary>
  /// This virtual method is invoked prior to calling the "copier". This allows
  /// you to manipulate the symbol prior to it being copied into memory.
  /// </summary>
  /// <param name="sym">Symbol being copied into memory.</param>
  /// <param name="copier">copier lambda that the pass can use. if it does, then
  /// you must return true.</param> <returns>returns true if the copy was done
  /// by the pass. false if copying still needs to be done.</returns>
  virtual bool copier_pass(decomp::symbol_t* sym, copier_t& copier) = 0;

  /// <summary>
  /// This virtual method is invoked prior to calling the "allocator". This
  /// allows you to manipulate the symbol prior to memory being allocated for
  /// it.
  /// </summary>
  /// <param name="sym">The symbol that memory is being allocated for.</param>
  /// <param name="size">The size of the symbol that memory is being allocated
  /// for.</param>
  /// <returns>returns the optional address of the allocation, if the pass didnt
  /// allocate space for the symbol than this optional value will have no
  /// value.</returns>
  virtual std::optional<std::uintptr_t> allocation_pass(
      decomp::symbol_t* sym,
      std::uint32_t size,
      allocator_t& allocator) = 0;

  /// <summary>
  /// This virtual method is invoked before the call to the "resolver". This
  /// allows you the manipulate the relocation address and symbol before
  /// relocations are made.
  /// </summary>
  /// <param name="sym">The symbol that has the relocation inside of it.</param>
  /// <param name="reloc">The relocation be be applied.</param>
  /// <param name="allocated_t">The address of the symbol pointed too by the
  /// relocation.</param>
  /// <returns></returns>
  virtual std::optional<std::uintptr_t> resolver_pass(
      decomp::symbol_t* sym,
      recomp::reloc_t* reloc,
      std::uintptr_t allocated_t) = 0;

  /// <summary>
  /// gets the passes symbol type.
  /// </summary>
  /// <returns>the passes symbol type.</returns>
  decomp::sym_type_t sym_type() { return m_sym_type; }

 private:
  decomp::sym_type_t m_sym_type;
};

/// <summary>
/// generic pass class overloads non-generic pass methods to return default
/// values... this makes it so generic passes dont need to overload all of these
/// methods and return default values every single time...
/// </summary>
class generic_pass_t : public pass_t {
 public:
  explicit generic_pass_t(decomp::sym_type_t sym_type) : pass_t(sym_type) {}

  // default non-generic passes to return generic values...
  //
  bool copier_pass(decomp::symbol_t* sym, copier_t& copier) { return false; }
  std::optional<std::uintptr_t> allocation_pass(decomp::symbol_t* sym,
                                                std::uint32_t size,
                                                allocator_t& allocator) {
    return {};
  }

  std::optional<std::uintptr_t> resolver_pass(decomp::symbol_t* sym,
                                              recomp::reloc_t* reloc,
                                              std::uintptr_t allocated_t) {
    return {};
  }
};
}  // namespace theo::obf