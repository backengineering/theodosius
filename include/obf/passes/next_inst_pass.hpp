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
/// This pass is used to generate transformations and jmp code to change RIP to
/// the next instruction.
///
/// given the following code (get pml4 address from cr3):
///
/// get_pml4:
///     0:  48 c7 c0 ff 0f 00 00    mov    rax,0xfff
///     7:  48 f7 d0                not    rax
///     a:  0f 20 da                mov    rdx,cr3
///     d:  48 21 c2                and    rdx,rax
///     10: b1 00                   mov    cl,0x0
///     12: 48 d3 e2                shl    rdx,cl
///     15: 48 89 d0                mov    rax,rdx
///     18: c3                      ret
///
/// this pass will break up each instruction so that it can be anywhere in a
/// linear virtual address space. this pass will not work on rip relative code,
/// however clang will not generate such code when compiled with
/// "-mcmodel=large"
///
/// get_pml4@0:
///     mov rax, 0xFFF
///     push [next_inst_addr_enc]
///     xor [rsp], 0x3243342
///     ; a random number of transformations here...
///     ret
/// next_inst_addr_enc:
///      ; encrypted address of the next instruction goes here.
///
/// get_pml4@7:
///     not    rax
///     push [next_inst_addr_enc]
///     xor [rsp], 0x93983498
///     ; a random number of transformations here...
///     ret
/// next_inst_addr_enc:
///      ; encrypted address of the next instruction goes here.
///
/// this process is continued for each instruction in the function. the last
/// instruction "ret" will have no code generated for it as there is no next
/// instruction.
///
///
/// this pass also only runs at the instruction level, theodosius internally
/// breaks up functions inside of the ".split" section into individual
/// instruction symbols. this process also creates a psuedo relocation which
/// simply tells this pass that there needs to be a relocation to the next
/// symbol. the offset for these psuedo relocations is zero.
/// </summary>
class next_inst_pass_t : public generic_pass_t {
  explicit next_inst_pass_t()
      : generic_pass_t(decomp::sym_type_t::instruction) {
    xed_state_t istate{XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b};
    xed_decoded_inst_zero_set_mode(&m_tmp_inst, &istate);
    xed_decode(&m_tmp_inst, m_type_inst_bytes, sizeof(m_type_inst_bytes));
  }

 public:
  static next_inst_pass_t* get();
  void generic_pass(decomp::symbol_t* sym, sym_map_t& sym_tbl) override;

 private:
  std::optional<recomp::reloc_t*> has_next_inst_reloc(decomp::symbol_t*);
  xed_decoded_inst_t m_tmp_inst;
  std::uint8_t m_type_inst_bytes[9] = {0x48, 0xC7, 0x44, 0x24, 0x08,
                                       0x44, 0x33, 0x22, 0x11};
};
}  // namespace theo::obf