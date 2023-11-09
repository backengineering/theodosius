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

#include <recomp/recomp.hpp>

namespace theo::recomp {
recomp_t::recomp_t(decomp::decomp_t* dcmp,
                   allocator_t alloc,
                   copier_t copy,
                   resolver_t resolve)
    : m_dcmp(dcmp), m_allocator(alloc), m_copier(copy), m_resolver(resolve) {}

void recomp_t::allocate() {
  // map code & data/rdata/bss sections first...
  //
  static const auto engine = obf::engine_t::get();
  m_dcmp->syms()->for_each([&](theo::decomp::symbol_t& sym) {
    switch (sym.type()) {
      case decomp::sym_type_t::section:
      case decomp::sym_type_t::function:
      case decomp::sym_type_t::instruction: {
        engine->for_each(&sym, [&](decomp::symbol_t* sym, obf::pass_t* pass) {
          if (sym->allocated_at())
            return;

          auto res = pass->allocation_pass(sym, sym->size(), m_allocator);
          if (res.has_value())
            sym->allocated_at(res.value());
        });

        if (!sym.allocated_at())
          sym.allocated_at(m_allocator(sym.size(), sym.scn()->characteristics));
        break;
      }
      default:
        break;
    }
  });

  // then map data/rdata/bss symbols to the allocated sections...
  //
  m_dcmp->syms()->for_each([&](theo::decomp::symbol_t& sym) {
    if (sym.type() == decomp::sym_type_t::data) {
      // if the symbol has a section then we will refer to the allocation made
      // for that section...
      //
      if (sym.scn()) {
        auto scn_sym =
            m_dcmp->syms()->sym_from_hash(m_dcmp->scn_hash_tbl()[sym.scn()]);

        if (!scn_sym.has_value()) {
          spdlog::error("failed to locate section: {} for symbol: {}",
                        sym.scn()->name.to_string(), sym.name());

          assert(scn_sym.has_value());
        }

        sym.allocated_at(scn_sym.value()->allocated_at() + sym.offset());
      } else {  // else if there is no section then we allocate based upon the
                // size of the symbol... this is only done for symbols that are
                // bss...
                //

        // bss is read write...
        //
        coff::section_characteristics_t prot = {};
        prot.mem_read = true;
        prot.mem_write = true;

        engine->for_each(&sym, [&](decomp::symbol_t* sym, obf::pass_t* pass) {
          if (sym->allocated_at())
            return;

          auto res = pass->allocation_pass(sym, sym->size(), m_allocator);
          if (res.has_value())
            sym->allocated_at(res.value());
        });

        if (!sym.allocated_at())
          sym.allocated_at(m_allocator(sym.size(), sym.scn()->characteristics));
      }
    }
  });
}

void recomp_t::resolve() {
  // resolve relocations in all symbols...
  //
  m_dcmp->syms()->for_each([&](theo::decomp::symbol_t& sym) {
    auto& relocs = sym.relocs();
    std::for_each(relocs.begin(), relocs.end(), [&](reloc_t& reloc) {
      if (reloc.offset() > sym.data().size()) {
        spdlog::error(
            "invalid relocation... writing outside of symbol length... offset: "
            "{} sym size: {}",
            sym.offset(), sym.data().size());

        assert(reloc.offset() > sym.data().size());
      }

      // try and resolve the symbol by refering to the internal symbol table
      // first... if there is no symbol then refer to the resolver...
      //
      auto reloc_sym = m_dcmp->syms()->sym_from_hash(reloc.hash());
      auto allocated_at = reloc_sym.has_value()
                              ? reloc_sym.value()->allocated_at()
                              : m_resolver(reloc.name());

      // run passes related to post symbol relocation...
      //
      static const auto engine = obf::engine_t::get();
      engine->for_each(&sym, [&](decomp::symbol_t* sym, obf::pass_t* pass) {
        auto res = pass->resolver_pass(sym, &reloc, allocated_at);
        if (res.has_value())
          allocated_at = res.value();
      });

      if (!allocated_at) {
        spdlog::error("failed to resolve reloc from symbol: {} to symbol: {}",
                      sym.name(), reloc.name());

        assert(allocated_at);
      }

      switch (sym.type()) {
        case decomp::sym_type_t::section: {
          auto scn_sym =
              m_dcmp->syms()->sym_from_hash(m_dcmp->scn_hash_tbl()[sym.scn()]);

          *reinterpret_cast<std::uintptr_t*>(scn_sym.value()->data().data() +
                                             reloc.offset()) = allocated_at;
          break;
        }
        case decomp::sym_type_t::instruction: {
          auto& transforms = reloc.get_transforms();
          std::for_each(
              transforms.begin(), transforms.end(),
              [&](std::pair<obf::transform::transform_t*, std::uint32_t>& t) {
                allocated_at = (*t.first)(allocated_at, t.second);
              });

          *reinterpret_cast<std::uintptr_t*>(sym.data().data() +
                                             reloc.offset()) = allocated_at;
          break;
        }
        case decomp::sym_type_t::function: {
          *reinterpret_cast<std::uintptr_t*>(sym.data().data() +
                                             reloc.offset()) = allocated_at;
          break;
        }
        default:
          break;
      }
    });
  });
}

void recomp_t::copy_syms() {
  // copy symbols into memory using the copier supplied...
  //
  static const auto engine = obf::engine_t::get();
  m_dcmp->syms()->for_each([&](theo::decomp::symbol_t& sym) {
    bool cpy = false;
    engine->for_each(&sym, [&](decomp::symbol_t* sym, obf::pass_t* pass) {
      if (cpy)
        return;
      // returns true if the copy was done for this symbol...
      //
      cpy = pass->copier_pass(sym, m_copier);
    });

    if (!cpy)
      m_copier(sym.allocated_at(), sym.data().data(), sym.data().size());
  });
}

void recomp_t::allocator(allocator_t alloc) {
  m_allocator = alloc;
}

void recomp_t::copier(copier_t copy) {
  m_copier = copy;
}

void recomp_t::resolver(resolver_t resolve) {
  m_resolver = resolve;
}

std::uintptr_t recomp_t::resolve(const std::string&& sym) {
  auto res = m_dcmp->syms()->sym_from_hash(decomp::symbol_t::hash(sym));
  return res.has_value() ? res.value()->allocated_at() : 0;
}
}  // namespace theo::recomp