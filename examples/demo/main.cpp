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

#include <Windows.h>
#include <psapi.h>

#include <filesystem>
#include <fstream>
#include <iostream>

#include <spdlog/spdlog.h>
#include <theo.hpp>

#include <obf/engine.hpp>
#include <obf/passes/func_split_pass.hpp>
#include <obf/passes/jcc_rewrite_pass.hpp>
#include <obf/passes/next_inst_pass.hpp>
#include <obf/passes/reloc_transform_pass.hpp>

#include "hello_world_pass.hpp"

namespace fs = std::filesystem;

/// <summary>
/// example usage of how to interface with theo. please refer to the source code
/// of this function for details.
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <returns></returns>
int main(int argc, char* argv[]) {
  if (argc < 2)
    return -1;

  // read in lib file...
  std::ifstream f(argv[1], std::ios::binary);
  auto fsize = fs::file_size(fs::path(argv[1]));
  std::vector<std::uint8_t> fdata;
  fdata.resize(fsize);
  f.read((char*)fdata.data(), fsize);

  LoadLibraryA("user32.dll");
  LoadLibraryA("win32u.dll");

  // declare your allocator, resolver, and copier lambda functions.
  //

  theo::recomp::allocator_t allocator =
      [&](std::uint32_t size,
          coff::section_characteristics_t section_type) -> std::uintptr_t {
    return reinterpret_cast<std::uintptr_t>(VirtualAlloc(
        NULL, size, MEM_COMMIT | MEM_RESERVE,
        section_type.mem_execute ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE));
  };

  theo::recomp::copier_t copier = [&](std::uintptr_t ptr, void* buff,
                                      std::uint32_t size) {
    std::memcpy((void*)ptr, buff, size);
  };

  theo::recomp::resolver_t resolver = [&](std::string sym) -> std::uintptr_t {
    auto loaded_modules = std::make_unique<HMODULE[]>(64);
    std::uintptr_t result = 0u, loaded_module_sz = 0u;
    if (!EnumProcessModules(GetCurrentProcess(), loaded_modules.get(), 512,
                            (PDWORD)&loaded_module_sz))
      return {};

    for (auto i = 0u; i < loaded_module_sz / 8u; i++) {
      wchar_t file_name[MAX_PATH] = L"";
      if (!GetModuleFileNameExW(GetCurrentProcess(), loaded_modules.get()[i],
                                file_name, _countof(file_name)))
        continue;

      if ((result = reinterpret_cast<std::uintptr_t>(
               GetProcAddress(LoadLibraryW(file_name), sym.c_str()))))
        break;
    }
    return result;
  };

  // init enc/dec tables only once... important that this is done before adding
  // obfuscation passes to the engine...
  //
  xed_tables_init();

  // order matters, the order in which the pass is added is the order they
  // will be executed!
  //
  auto engine = theo::obf::engine_t::get();

  // add the rest of the passes in this order. this order is important.
  //
  engine->add_pass(theo::obf::func_split_pass_t::get());
  engine->add_pass(theo::obf::reloc_transform_pass_t::get());
  engine->add_pass(theo::obf::next_inst_pass_t::get());
  engine->add_pass(theo::obf::jcc_rewrite_pass_t::get());

  // add in our hello world pass here
  //
  engine->add_pass(theo::obf::hello_world_pass_t::get());

  std::string entry_name;
  std::cout << "enter the name of the entry point: ";
  std::cin >> entry_name;

  // create a theo object and pass in the lib, your allocator, copier, and
  // resolver functions, as well as the entry point symbol name.
  //
  theo::theo_t t(fdata, {allocator, copier, resolver}, entry_name.data());

  // call the decompose method to decompose the lib into coff files and extract
  // the symbols that are used. the result of this call will be an optional
  // value containing the number of symbols extracted.
  //
  auto res = t.decompose();

  if (!res.has_value()) {
    spdlog::error("decomposition failed...\n");
    return -1;
  }

  spdlog::info("decomposed {} symbols...", res.value());
  auto entry_pnt = t.compose();
  spdlog::info("entry point address: {:X}", entry_pnt);
  spdlog::info("press enter to execute {}", entry_name.c_str());

  std::getchar();
  reinterpret_cast<void (*)()>(entry_pnt)();
}