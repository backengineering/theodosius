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
#include <obf/pass.hpp>

namespace theo::obf {
/// <summary>
/// jcc rewrite pass which rewrites rip relative jcc's so that they are position
/// independent.
///
/// given the following code:
///
/// 	jnz label1
/// 	; other code goes here
/// label1:
///		; more code here
///
/// the jnz instruction will be rewritten so that the following code is
/// generated:
///
/// 	jnz br2
/// br1:
///		jmp [rip] ; address after this instruction contains the address
///				  ; of the instruction after the jcc.
/// br2:
/// 	jmp [rip] ; address after this instruction contains the address of where
///				  ; branch 2 is located.
///
/// its important to note that other passes will encrypt (transform) the address
/// of the next instruction. There is actually no jmp [rip] either, push/ret is
/// used.
/// </summary>
class jcc_rewrite_pass_t : public generic_pass_t {
  explicit jcc_rewrite_pass_t()
      : generic_pass_t(decomp::sym_type_t::instruction) {}

 public:
  static jcc_rewrite_pass_t* get();
  void generic_pass(decomp::symbol_t* sym, sym_map_t& sym_tbl) override;
};
}  // namespace theo::obf