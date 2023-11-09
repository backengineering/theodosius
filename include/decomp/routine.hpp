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
#include <map>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>
#include <decomp/symbol.hpp>

#define XED_ENCODER
extern "C" {
#include <xed-decode.h>
#include <xed-interface.h>
}

#define INSTR_SPLIT_SECTION_NAME ".obf"

namespace theo::decomp {
/// <summary>
/// the routine class which is responsible for creating symbols for routines. if
/// the routine is located inside a section with the name ".split" it will break
/// functions into instruction symbols.
/// </summary>
class routine_t {
 public:
  /// <summary>
  /// the explicit constructor for routine_t.
  /// </summary>
  /// <param name="sym">the coff symbol for the routine.</param>
  /// <param name="img">the coff image which contains the symbol.</param>
  /// <param name="scn">the section header of the section that contains the
  /// symbol.</param>
  /// <param name="fn">the data (bytes) of the function.</param>
  /// <param name="dcmp_type">the type of decomp to do. if this is
  /// sym_type_t::function then this class wont split the function up into
  /// individual instructions.</param>
  explicit routine_t(coff::symbol_t* sym,
                     coff::image_t* img,
                     coff::section_header_t* scn,
                     std::vector<std::uint8_t>& fn);

  /// <summary>
  /// decompose the function into symbol(s).
  /// </summary>
  /// <returns>symbol of the function.</returns>
  decomp::symbol_t decompose();

  /// <summary>
  /// gets the section header of the section in which the symbol is located in.
  /// </summary>
  /// <returns>the section header of the section in which the symbol is located
  /// in.</returns>
  coff::section_header_t* scn();

  /// <summary>
  /// gets the function bytes.
  /// </summary>
  /// <returns>the function bytes.</returns>
  std::vector<std::uint8_t> data();

 private:
  coff::symbol_t* m_sym;
  std::vector<std::uint8_t> m_data;
  coff::image_t* m_img;
  coff::section_header_t* m_scn;
};
}  // namespace theo::decomp