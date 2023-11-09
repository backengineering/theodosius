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

#include <decomp/symbol.hpp>

namespace theo::decomp {
symbol_t::symbol_t(coff::image_t* img,
                   std::string name,
                   std::uintptr_t offset,
                   std::vector<std::uint8_t> data,
                   coff::section_header_t* scn,
                   coff::symbol_t* sym,
                   std::vector<recomp::reloc_t> relocs,
                   sym_type_t dcmp_type)
    : m_name(name),
      m_offset(offset),
      m_data(data),
      m_scn(scn),
      m_relocs(relocs),
      m_sym_type(dcmp_type),
      m_sym(sym),
      m_img(img),
      m_allocated_at(0) {}

std::string symbol_t::name() const {
  return m_name;
}

std::uintptr_t symbol_t::offset() const {
  return m_offset;
}

std::uintptr_t symbol_t::allocated_at() const {
  return m_allocated_at;
}

coff::section_header_t* symbol_t::scn() const {
  return m_scn;
}

coff::image_t* symbol_t::img() const {
  return m_img;
}

std::uint32_t symbol_t::size() const {
  return m_data.size();
}

std::vector<std::uint8_t>& symbol_t::data() {
  return m_data;
}

sym_type_t symbol_t::type() const {
  return m_sym_type;
}

void symbol_t::type(sym_type_t type) {
  m_sym_type = type;
}

void symbol_t::allocated_at(std::uintptr_t allocated_at) {
  m_allocated_at = allocated_at;
}

std::size_t symbol_t::hash() {
  return hash(m_name);
}

coff::symbol_t* symbol_t::sym() const {
  return m_sym;
}

std::vector<recomp::reloc_t>& symbol_t::relocs() {
  return m_relocs;
}

std::size_t symbol_t::hash(const std::string& sym) {
  return std::hash<std::string>{}(sym);
}

std::string symbol_t::name(const coff::image_t* img, coff::symbol_t* sym) {
  if (sym->has_section() &&
      sym->storage_class == coff::storage_class_id::private_symbol &&
      sym->derived_type == coff::derived_type_id::none) {
    auto scn = img->get_section(sym->section_index - 1);
    auto res = std::string(scn->name.to_string(img->get_strings()).data())
                   .append("#")
                   .append(std::to_string(sym->section_index))
                   .append("!")
                   .append(std::to_string(img->file_header.timedate_stamp))
                   .append("+")
                   .append(std::to_string(sym->value));

    return res;
  }
  return std::string(sym->name.to_string(img->get_strings()));
}
}  // namespace theo::decomp