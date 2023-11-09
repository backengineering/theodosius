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
#include <cstddef>
#include <cstdint>
#include <obf/transform/transform.hpp>
namespace theo::recomp {
/// <summary>
/// meta data about a relocation for a symbol
/// </summary>
class reloc_t {
 public:
  /// <summary>
  /// explicit constructor for this class.
  /// </summary>
  /// <param name="offset">offset into the symbol data where the relocation is
  /// at. all relocations are assumed to be linear virtual addresses of the
  /// symbol.</param>
  /// <param name="hash">hash of the symbol to which the relocation is
  /// of.</param> <param name="sym_name">the name of the symbol to which the
  /// relocation is of.</param>
  explicit reloc_t(std::uint32_t offset,
                   std::size_t hash,
                   const std::string&& sym_name)
      : m_offset(offset), m_hash(hash), m_sym_name(sym_name) {}
  /// <summary>
  /// returns the hash of the relocation symbol.
  /// </summary>
  /// <returns>hash of the relocation symbol</returns>
  std::size_t hash() { return m_hash; }
  /// <summary>
  /// returns the name of the relocation symbol.
  /// </summary>
  /// <returns>returns the name of the relocation symbol.</returns>
  std::string name() { return m_sym_name; }
  /// <summary>
  /// returns the offset into the symbol to which the relocation will be
  /// applied. the offset is in bytes. zero based.
  /// </summary>
  /// <returns>returns the offset into the symbol to which the relocation will
  /// be applied. the offset is in bytes. zero based.</returns>
  std::uint32_t offset() { return m_offset; }
  /// <summary>
  /// sets the offset to which the relocation gets applied too.
  /// </summary>
  /// <param name="offset">offset to which the relocation gets applied
  /// too.</param>
  void offset(std::uint32_t offset) { m_offset = offset; }
  /// <summary>
  /// adds a transformation to be applied to the relocation prior to writing it
  /// into the symbol.
  /// </summary>
  /// <param name="entry">a pair containing a lambda function that when executed
  /// transforms a relocation. the second value in the pair is a random value
  /// which is passed to the lambda.</param>
  void add_transform(
      std::pair<obf::transform::transform_t*, std::uint32_t> entry) {
    m_transforms.push_back(entry);
  }
  /// <summary>
  /// gets the vector of transformation.
  /// </summary>
  /// <returns>returns the vector of transformations.</returns>
  std::vector<std::pair<obf::transform::transform_t*, std::uint32_t>>&
  get_transforms() {
    return m_transforms;
  }

 private:
  std::vector<std::pair<obf::transform::transform_t*, std::uint32_t>>
      m_transforms;
  std::string m_sym_name;
  std::size_t m_hash;
  std::uint32_t m_offset;
};
}  // namespace theo::recomp