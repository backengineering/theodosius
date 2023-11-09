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
#include <coff/image.hpp>
#include <cstdint>
#include <recomp/reloc.hpp>
#include <string>
#include <vector>

namespace theo::decomp {
/// <summary>
/// meta symbol type. this is an abstraction upon the coff symbol storage/class
/// type.
/// </summary>
enum sym_type_t {
  function = 0b00000001,
  instruction = 0b00000010,
  data = 0b00000100,
  section = 0b00001000,
  all = section | data | instruction | function
};

/// <summary>
/// symbol_t is an abstraction upon the coff symbol. this allows for easier
/// manipulation of the symbol. symbols can be different things, sections,
/// functions, and even instructions (when functions are broken down).
///
/// this class is used throughout theodosius and is a keystone of the project.
/// ensure you understand how this class works and what it contains.
/// </summary>
class symbol_t {
 public:
  /// <summary>
  /// the explicit constructor of this symbol.
  /// </summary>
  /// <param name="img">the image in which the symbol is located in.</param>
  /// <param name="name">the name of the symbol.</param>
  /// <param name="offset">offset into the section where this symbol is
  /// located.</param>
  /// <param name="data">the data of the symbol. there can be
  /// no data.</param>
  /// <param name="scn">the section header describing the
  /// section which contains the symbol.</param>
  /// <param name="sym">the coff symbol itself.</param>
  /// <param name="relocs">a vector of relocations this symbol has (if
  /// any).</param>
  /// <param name="dcmp_type">the type of symbol</param>
  explicit symbol_t(coff::image_t* img,
                    std::string name,
                    std::uintptr_t offset,
                    std::vector<std::uint8_t> data,
                    coff::section_header_t* scn = {},
                    coff::symbol_t* sym = {},
                    std::vector<recomp::reloc_t> relocs = {},
                    sym_type_t dcmp_type = {});

  /// <summary>
  /// gets the name of the symbol.
  /// </summary>
  /// <returns>the name of the symbol.</returns>
  std::string name() const;

  /// <summary>
  /// gets the offset into the section where the symbol is located.
  /// </summary>
  /// <returns>the offset into the section where the symbol is
  /// located.</returns>
  std::uintptr_t offset() const;

  /// <summary>
  /// returns the address where the symbol is allocated.
  /// </summary>
  /// <returns>the address where the symbol is allocated.</returns>
  std::uintptr_t allocated_at() const;

  /// <summary>
  /// returns the size of the symbol.
  /// </summary>
  /// <returns>the size of the symbol.</returns>
  std::uint32_t size() const;

  /// <summary>
  /// gets the section header of the section in which the symbol is contained.
  /// </summary>
  /// <returns>the section header of the section in which the symbol is
  /// contained.</returns>
  coff::section_header_t* scn() const;

  /// <summary>
  /// gets the imagine in which the symbol is located inside of.
  /// </summary>
  /// <returns>the imagine in which the symbol is located inside of.</returns>
  coff::image_t* img() const;

  /// <summary>
  /// returns a vector by reference of bytes containing the data of the symbol.
  /// </summary>
  /// <returns>a vector by reference of bytes containing the data of the
  /// symbol.</returns>
  std::vector<std::uint8_t>& data();

  /// <summary>
  /// returns a pointer to the coff symbol object.
  /// </summary>
  /// <returns>a pointer to the coff symbol object.</returns>
  coff::symbol_t* sym() const;

  /// <summary>
  /// returns the type of the symbol.
  /// </summary>
  /// <returns>the type of the symbol.</returns>
  sym_type_t type() const;

  /// <summary>
  /// setter for the type value.
  /// </summary>
  /// <param name="type">type of symbol.</param>
  void type(sym_type_t type);

  /// <summary>
  /// returns a vector of relocations.
  /// </summary>
  /// <returns>a vector of relocations.</returns>
  std::vector<recomp::reloc_t>& relocs();

  /// <summary>
  /// set the address where the symbol is allocated at.
  /// </summary>
  /// <param name="allocated_at">where the symbol is allocated at.</param>
  void allocated_at(std::uintptr_t allocated_at);

  /// <summary>
  /// gets the hash of the symbol name.
  /// </summary>
  /// <returns>the hash of the symbol name.</returns>
  std::size_t hash();

  /// <summary>
  /// generate a hash given the name of the symbol.
  /// </summary>
  /// <param name="sym">the symbol name to create a hash from.</param>
  /// <returns>the symbol name hash</returns>
  static std::size_t hash(const std::string& sym);

  /// <summary>
  /// get the name of a symbol. this function will create a symbol name if the
  /// symbol is opaquely named.
  ///
  /// for example in c++ if you define something like this:
  ///
  ///   some_struct_t val = { value_one, value_two };
  ///
  /// "val" will be stored in the .data section of the coff file. however the
  /// symbol name will be opaque (the name of the symbol will be ".data"). this
  /// causes issues with theo since each symbol needs its own unqiue name to
  /// generate a unique symbol name hash. for symbols like this, theo will
  /// create a name for it with the following format:
  ///
  ///   .data#section_index!coff_file_timestamp+offset_into_section
  ///
  /// </summary>
  /// <param name="img">the coff file containing the symbol.</param>
  /// <param name="sym">the coff symbol itself.</param>
  /// <returns>the name of the symbol, or a created one.</returns>
  static std::string name(const coff::image_t* img, coff::symbol_t* sym);

 private:
  std::string m_name;
  std::uintptr_t m_offset, m_allocated_at;
  std::vector<std::uint8_t> m_data;
  coff::section_header_t* m_scn;
  std::vector<recomp::reloc_t> m_relocs;
  sym_type_t m_sym_type;
  coff::symbol_t* m_sym;
  coff::image_t* m_img;
};
}  // namespace theo::decomp