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
#include <algorithm>
#include <obf/pass.hpp>
#include <vector>

namespace theo::obf {

/// <summary>
///	callback used by obfuscation engine to run passes...
/// </summary>
using engine_callback_t = std::function<void(decomp::symbol_t*, pass_t*)>;

/// <summary>
/// singleton obfuscation engine class. this class is responsible for keeping
/// track of the registered passes and the order in which to execute them.
/// </summary>
class engine_t {
  explicit engine_t(){};

 public:
  /// <summary>
  /// get the singleton object of this class.
  /// </summary>
  /// <returns>the singleton object of this class.</returns>
  static engine_t* get();

  /// <summary>
  /// add a pass to the engine. the order in which you call this function
  /// matters as the underlying data structure that contains the passes is a
  /// vector.
  /// </summary>
  /// <param name="pass">a pointer to the pass in which to add to the
  /// engine.</param>
  void add_pass(pass_t* pass);

  /// <summary>
  /// invokes the callback for each pass in order.
  /// </summary>
  /// <param name="callback">callback to be invoked. the callback is given a
  /// pointer to the pass.</param>
  /// <param name="sym">symbol to run callbacks on.</param>
  void for_each(decomp::symbol_t* sym, engine_callback_t callback);

 private:
  std::vector<pass_t*> passes;
};
}  // namespace theo::obf