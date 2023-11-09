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

#include <decomp/decomp.hpp>

namespace theo::decomp {
decomp_t::decomp_t(std::vector<std::uint8_t>& lib, recomp::symbol_table_t* syms)
    : m_lib(lib), m_syms(syms) {}

std::optional<recomp::symbol_table_t*> decomp_t::decompose(
    std::string& entry_sym) {
  // extract obj files from the archive file...
  //
  ar::view<false> lib(m_lib.data(), m_lib.size());
  std::for_each(
      lib.begin(), lib.end(),
      [&](std::pair<std::string_view, ar::entry_t&> itr) {
        // if the entry isnt the symbol table or the string table
        // then we know its an obj file...
        //
        if (!itr.second.is_symbol_table() && !itr.second.is_string_table()) {
          spdlog::info("extracted obj from archive: {}", itr.first);
          std::vector<std::uint8_t> data(itr.second.begin(), itr.second.end());
          m_objs.push_back(data);
        }
      });

  std::for_each(
      m_objs.begin(), m_objs.end(), [&](std::vector<std::uint8_t>& img_data) {
        auto img = reinterpret_cast<coff::image_t*>(img_data.data());
        for (auto idx = 0u; idx < img->file_header.num_symbols; ++idx) {
          auto sym = img->get_symbol(idx);
          if (sym->section_index - 1 > img->file_header.num_sections)
            continue;

          auto sym_name = symbol_t::name(img, sym);
          if (sym_name.length()) {
            auto sym_hash = symbol_t::hash(sym_name.data());
            auto sym_size =
                sym->has_section()
                    ? next_sym(img, img->get_section(sym->section_index - 1),
                               sym)
                    : 0u;

            m_lookup_tbl[sym_hash].emplace_back(img, sym, sym_size);
          }
        }
      });

  // extract used symbols from objs and create a nice little set of them so that
  // we can easily decompose them... no need deal with every single symbol...
  spdlog::info("extracted {} symbols being used...",
               ext_used_syms(entry_sym.data()));

  // generate symbols, populate section hash table, for each object file
  // extracted from the archive file...
  //
  std::for_each(m_used_syms.begin(), m_used_syms.end(), [&](sym_data_t data) {
    auto [img, sym, size] = data;

    // populate section hash table with sections for the img of this
    // symbol... only populate the hash table if its not been populated for
    // this obj before...
    //
    if (m_processed_objs.emplace(img).second) {
      for (auto idx = 0u; idx < img->file_header.num_sections; ++idx) {
        auto scn = img->get_section(idx);
        auto scn_sym_name =
            std::string(scn->name.to_string(img->get_strings()))
                .append("#")
                .append(std::to_string(idx))
                .append("!")
                .append(std::to_string(img->file_header.timedate_stamp));

        // hash the name of the section + the index + the timestamp of the
        // obj file it is in...
        //
        m_scn_hash_tbl.insert({scn, decomp::symbol_t::hash(scn_sym_name)});
      }
    }

    // if the symbol is a function then we are going to decompose it...
    // data symbols are handled after this...
    //
    if (sym->has_section()) {
      if (sym->derived_type == coff::derived_type_id::function) {
        auto scn = img->get_section(sym->section_index - 1);
        auto fn_size = next_sym(img, scn, sym);
        auto fn_bgn = scn->ptr_raw_data + reinterpret_cast<std::uint8_t*>(img) +
                      sym->value;

        // extract the bytes the function is composed of...
        //
        std::vector<std::uint8_t> fn(fn_bgn, fn_bgn + fn_size);
        decomp::routine_t rtn(sym, img, scn, fn);

        auto fsym = rtn.decompose();
        m_syms->put_symbol(fsym);
        // else the symbol isnt a function and its public or private (some data
        // symbols are private)...
      } else if (sym->storage_class == coff::storage_class_id::public_symbol ||
                 sym->storage_class == coff::storage_class_id::private_symbol) {
        auto scn = img->get_section(sym->section_index - 1);
        auto scn_sym = m_syms->sym_from_hash(m_scn_hash_tbl[scn]);

        // if the section containing this symbol doesnt have a symbol then make
        // one and put it into the symbol table...
        //
        if (!scn_sym.has_value()) {
          auto scn_sym_name =
              std::string(scn->name.to_string(img->get_strings()))
                  .append("#")
                  .append(std::to_string(sym->section_index - 1))
                  .append("!")
                  .append(std::to_string(img->file_header.timedate_stamp));

          std::vector<std::uint8_t> scn_data(scn->size_raw_data);
          if (scn->characteristics.cnt_uninit_data) {
            scn_data.insert(scn_data.begin(), scn->size_raw_data, 0);
          } else {
            scn_data.insert(
                scn_data.begin(),
                reinterpret_cast<std::uint8_t*>(img) + scn->ptr_raw_data,
                reinterpret_cast<std::uint8_t*>(img) + scn->ptr_raw_data +
                    scn->size_raw_data);
          }

          // extract the relocations needed for this section...
          //
          std::vector<recomp::reloc_t> relocs;
          auto scn_relocs = reinterpret_cast<coff::reloc_t*>(
              scn->ptr_relocs + reinterpret_cast<std::uint8_t*>(img));

          for (auto idx = 0u; idx < scn->num_relocs; ++idx) {
            auto scn_reloc = &scn_relocs[idx];
            auto sym_reloc = img->get_symbol(scn_relocs[idx].symbol_index);
            auto sym_name = symbol_t::name(img, sym_reloc);
            auto sym_hash = decomp::symbol_t::hash(sym_name.data());
            relocs.push_back(
                recomp::reloc_t(scn_reloc->virtual_address - sym->value,
                                sym_hash, sym_name.data()));
          }

          // create a new section symbol...
          //
          decomp::symbol_t new_scn_sym(img, scn_sym_name, 0, scn_data, scn, {},
                                       relocs, sym_type_t::section);

          m_syms->put_symbol(new_scn_sym);
        }

        // create a symbol for the data...
        //
        decomp::symbol_t new_sym(img, symbol_t::name(img, sym).data(),
                                 sym->value, {}, scn, sym, {},
                                 sym_type_t::data);

        m_syms->put_symbol(new_sym);
      }
    } else if (sym->storage_class ==
               coff::storage_class_id::
                   external_definition) {  // else if the symbol has no
                                           // section... these symbols
                                           // require the linker to allocate
                                           // space for them...

      std::vector<std::uint8_t> data(sym->value, 0);
      decomp::symbol_t bss_sym(img, symbol_t::name(img, sym).data(), {}, data,
                               {}, sym, {}, sym_type_t::data);

      m_syms->put_symbol(bss_sym);
    }
  });

  // return the extract symbols to the caller...
  //
  return m_syms;
}

std::uint32_t decomp_t::next_sym(coff::image_t* img,
                                 coff::section_header_t* hdr,
                                 coff::symbol_t* s) {
  // loop over all symbols in this object...
  // find the next symbol inside of the same section...
  // if there is no next symbol then we use the end of the section...
  std::uint32_t res = hdr->size_raw_data;
  for (auto idx = 0u; idx < img->file_header.num_symbols; ++idx) {
    auto q = img->get_symbol(idx);
    if (q->derived_type == coff::derived_type_id::function &&
        q->section_index == s->section_index && q != s)
      if (q->value > s->value && q->value < res)
        res = q->value;
  }
  return res;
}

std::uint32_t decomp_t::ext_used_syms(const std::string&& entry_sym) {
  // start with the entry point symbol...
  std::optional<sym_data_t> entry = get_symbol(entry_sym.data());

  // if the entry point symbol cant be found simply return 0 (for 0 symbols
  // extracted)...
  if (!entry.has_value())
    return 0u;

  // little memoization for perf boost... :^)
  std::set<coff::symbol_t*> cache;
  const auto finding_syms = [&]() -> bool {
    // for all the symbols...
    for (auto itr = m_used_syms.begin(); itr != m_used_syms.end(); ++itr) {
      auto [img, sym, size] = *itr;
      // if the symbol has a section and its not already in the cache...
      if (sym->has_section() && !cache.count(sym) && size) {
        auto scn = img->get_section(sym->section_index - 1);
        auto num_relocs = scn->num_relocs;
        auto relocs = reinterpret_cast<coff::reloc_t*>(
            scn->ptr_relocs + reinterpret_cast<std::uint8_t*>(img));

        // see if there are any relocations inside of the symbol...
        for (auto idx = 0u; idx < num_relocs; ++idx) {
          auto reloc = &relocs[idx];
          // if the reloc is inside of the current symbol...
          if (reloc->virtual_address >= sym->value &&
              reloc->virtual_address < sym->value + size) {
            // get the symbol for the relocation and add it to the m_used_symbol
            // vector...
            auto reloc_sym = img->get_symbol(reloc->symbol_index);
            auto sym_name = symbol_t::name(img, reloc_sym);
            entry = get_symbol(sym_name);
            // if the symbol already exists in the m_used_syms vector then we
            // return out of this function and continue looping over all symbols
            // recursively...
            if (m_used_syms.emplace(entry.value()).second)
              return true;  // returns to the for loop below this lambda. the
                            // return true here means we added yet another
                            // symbol to the "m_used_syms" vector...
          }
        }

        // cache the symbol so we dont need to process it again...
        cache.emplace(sym);
      }
    }
    return false;  // only ever returns false here when every single symbol
                   // inside of m_used_syms has been looked at to see if all of
                   // its relocation symbols are included in the m_used_syms
                   // vector (meaning we got all the used symbols...)
  };

  // add the entry point symbol...
  m_used_syms.emplace(entry.value());

  // keep recursively adding symbols until we found them all..
  for (m_used_syms.emplace(entry.value()); finding_syms();)
    ;

  return m_used_syms.size();
}

std::optional<sym_data_t> decomp_t::get_symbol(const std::string_view& name) {
  coff::image_t* img = {};
  coff::symbol_t* sym = {};
  std::uint32_t size = {};

  auto& syms = m_lookup_tbl[symbol_t::hash(name.data())];
  for (auto idx = 0u; idx < syms.size(); ++idx) {
    img = std::get<0>(syms[idx]);
    sym = std::get<1>(syms[idx]);
    size = std::get<2>(syms[idx]);
    if (sym->has_section())
      return {{img, sym, size}};
  }

  if (img && sym)
    return {{img, sym, size}};

  return {};
}

std::vector<routine_t> decomp_t::rtns() {
  return m_rtns;
}

std::vector<std::uint8_t> decomp_t::lib() {
  return m_lib;
}

std::vector<std::vector<std::uint8_t>> decomp_t::objs() {
  return m_objs;
}

recomp::symbol_table_t* decomp_t::syms() {
  return m_syms;
}

std::map<coff::section_header_t*, std::size_t>& decomp_t::scn_hash_tbl() {
  return m_scn_hash_tbl;
}
}  // namespace theo::decomp