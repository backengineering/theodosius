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
#include <obf/transform/operation.hpp>
#include <obf/transform/transform.hpp>

namespace theo::obf {
/// <summary>
/// this pass is like the next_inst_pass, however, relocations are encrypted
/// with transformations instead of the address of the next instruction. this
/// pass only runs at the instruction level and appends transformations into the
/// reloc_t object of the instruction symbol.
///
/// given the following code:
///
/// 	mov rax, &MessageBoxA
///
/// this pass will generate a random number of transformations to encrypt the
/// address of "MessageBoxA". These transformations will then be applied by
/// theodosius internally when resolving relocations.
///
/// 	mov rax, enc_MessageBoxA
/// 	xor rax, 0x389284324
/// 	add rax, 0x345332567
/// 	ror rax, 0x5353
/// </summary>
class reloc_transform_pass_t : public generic_pass_t {
  explicit reloc_transform_pass_t()
      : generic_pass_t(decomp::sym_type_t::instruction) {}

 public:
  static reloc_transform_pass_t* get();
  void generic_pass(decomp::symbol_t* sym, sym_map_t& sym_tbl) override;

 private:
  std::optional<recomp::reloc_t*> has_legit_reloc(decomp::symbol_t* sym);
};
}  // namespace theo::obf