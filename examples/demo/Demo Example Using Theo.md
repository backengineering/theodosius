Example run using demolib.lib as the target lib. Note the `hello_world_pass_t` output. Refer to main.cpp for more information.

```
[2022-04-22 15:44:21.680] [info] created hello world pass...
enter the name of the entry point: EntryPoint
[2022-04-22 15:44:28.547] [info] extracted obj from archive: ../demolib.dir/Debug/main.obj
[2022-04-22 15:44:28.549] [info] extracted 5 symbols being used...
[2022-04-22 15:44:28.550] [info] EntryPoint: sub rsp, 0x28
[2022-04-22 15:44:28.551] [info] EntryPoint@4: mov rax, 0x0
[2022-04-22 15:44:28.551] [info] EntryPoint@14: mov byte ptr [rax+0x10], 0x1
[2022-04-22 15:44:28.551] [info] EntryPoint@18: mov byte ptr [rax+0x11], 0x1
[2022-04-22 15:44:28.552] [info] EntryPoint@22: mov al, byte ptr [rax+0x10]
[2022-04-22 15:44:28.553] [info] EntryPoint@25: cmp al, 0x0
[2022-04-22 15:44:28.553] [info] EntryPoint@27: jz 0x2d
[2022-04-22 15:44:28.554] [info] EntryPoint@33: mov rax, 0x0
[2022-04-22 15:44:28.554] [info] EntryPoint@43: mov rdx, qword ptr [rax]
[2022-04-22 15:44:28.554] [info] EntryPoint@46: mov r8, qword ptr [rax+0x8]
[2022-04-22 15:44:28.555] [info] EntryPoint@50: xor eax, eax
[2022-04-22 15:44:28.555] [info] EntryPoint@52: mov rax, 0x0
[2022-04-22 15:44:28.555] [info] EntryPoint@62: xor ecx, ecx
[2022-04-22 15:44:28.556] [info] EntryPoint@64: mov r9d, ecx
[2022-04-22 15:44:28.558] [info] EntryPoint@67: mov rcx, r9
[2022-04-22 15:44:28.559] [info] EntryPoint@70: call rax
[2022-04-22 15:44:28.559] [info] EntryPoint@72: mov rax, 0x0
[2022-04-22 15:44:28.559] [info] EntryPoint@82: mov al, byte ptr [rax+0x11]
[2022-04-22 15:44:28.560] [info] EntryPoint@85: cmp al, 0x0
[2022-04-22 15:44:28.560] [info] EntryPoint@87: jz 0x2d
[2022-04-22 15:44:28.560] [info] EntryPoint@93: mov rax, 0x0
[2022-04-22 15:44:28.561] [info] EntryPoint@103: mov rdx, qword ptr [rax]
[2022-04-22 15:44:28.561] [info] EntryPoint@106: mov r8, qword ptr [rax+0x8]
[2022-04-22 15:44:28.561] [info] EntryPoint@110: xor eax, eax
[2022-04-22 15:44:28.562] [info] EntryPoint@112: mov rax, 0x0
[2022-04-22 15:44:28.562] [info] EntryPoint@122: xor ecx, ecx
[2022-04-22 15:44:28.563] [info] EntryPoint@124: mov r9d, ecx
[2022-04-22 15:44:28.563] [info] EntryPoint@127: mov rcx, r9
[2022-04-22 15:44:28.564] [info] EntryPoint@130: call rax
[2022-04-22 15:44:28.565] [info] EntryPoint@132: nop
[2022-04-22 15:44:28.566] [info] EntryPoint@133: add rsp, 0x28
[2022-04-22 15:44:28.566] [info] EntryPoint@137: ret
[2022-04-22 15:44:28.568] [info] decompose successful... 38 symbols
[2022-04-22 15:44:28.575] [info] decomposed 38 symbols...
[2022-04-22 15:44:28.576] [info] [hello_world_pass_t] symbol name: .rdata#5!1650667380, symbol hash: 2136714129526770328
[2022-04-22 15:44:28.576] [info] [hello_world_pass_t] symbol name: EntryPoint@14, symbol hash: 3130690789734940524
[2022-04-22 15:44:28.626] [info] [hello_world_pass_t] symbol name: EntryPoint@18, symbol hash: 3130695187781453368
[2022-04-22 15:44:28.627] [info] [hello_world_pass_t] symbol name: EntryPoint@33, symbol hash: 3132528073665302655
[2022-04-22 15:44:28.627] [info] adding transformations to relocation in symbol: EntryPoint@33
[2022-04-22 15:44:28.629] [info] [hello_world_pass_t] symbol name: EntryPoint@22, symbol hash: 3133523131688644385
[2022-04-22 15:44:28.629] [info] [hello_world_pass_t] symbol name: EntryPoint@25, symbol hash: 3133524231200272596
[2022-04-22 15:44:28.630] [info] [hello_world_pass_t] symbol name: EntryPoint@27, symbol hash: 3133526430223529018
[2022-04-22 15:44:28.631] [info] [hello_world_pass_t] symbol name: EntryPoint@50, symbol hash: 3134512692153845060
[2022-04-22 15:44:28.632] [info] [hello_world_pass_t] symbol name: EntryPoint@52, symbol hash: 3134514891177101482
[2022-04-22 15:44:28.632] [info] adding transformations to relocation in symbol: EntryPoint@52
[2022-04-22 15:44:28.634] [info] [hello_world_pass_t] symbol name: EntryPoint@46, symbol hash: 3135497854572532891
[2022-04-22 15:44:28.634] [info] [hello_world_pass_t] symbol name: EntryPoint@43, symbol hash: 3135501153107417524
[2022-04-22 15:44:28.635] [info] [hello_world_pass_t] symbol name: EntryPoint@72, symbol hash: 3136458827735400080
[2022-04-22 15:44:28.639] [info] adding transformations to relocation in symbol: EntryPoint@72
[2022-04-22 15:44:28.640] [info] [hello_world_pass_t] symbol name: EntryPoint@70, symbol hash: 3136461026758656502
[2022-04-22 15:44:28.640] [info] [hello_world_pass_t] symbol name: EntryPoint@64, symbol hash: 3137307650712189747
[2022-04-22 15:44:28.641] [info] [hello_world_pass_t] symbol name: EntryPoint@67, symbol hash: 3137308750223817958
[2022-04-22 15:44:28.642] [info] [hello_world_pass_t] symbol name: EntryPoint@62, symbol hash: 3137314247781959013
[2022-04-22 15:44:28.643] [info] [hello_world_pass_t] symbol name: EntryPoint@93, symbol hash: 3138307106782044321
[2022-04-22 15:44:28.643] [info] adding transformations to relocation in symbol: EntryPoint@93
[2022-04-22 15:44:28.644] [info] [hello_world_pass_t] symbol name: EntryPoint@82, symbol hash: 3139293368712360363
[2022-04-22 15:44:28.645] [info] [hello_world_pass_t] symbol name: EntryPoint@87, symbol hash: 3139296667247244996
[2022-04-22 15:44:28.646] [info] [hello_world_pass_t] symbol name: EntryPoint@85, symbol hash: 3139298866270501418
[2022-04-22 15:44:28.646] [info] [hello_world_pass_t] symbol name: ??_C@_05COLMCDPH@Hello?$AA@, symbol hash: 5698279015588437768
[2022-04-22 15:44:28.650] [info] [hello_world_pass_t] symbol name: .rdata#6!1650667380, symbol hash: 6592944029141340807
[2022-04-22 15:44:28.651] [info] [hello_world_pass_t] symbol name: EntryPoint@4, symbol hash: 7460014814245461327
[2022-04-22 15:44:28.652] [info] adding transformations to relocation in symbol: EntryPoint@4
[2022-04-22 15:44:28.653] [info] [hello_world_pass_t] symbol name: .data#1!1650667380, symbol hash: 7704517242855100748
[2022-04-22 15:44:28.653] [info] [hello_world_pass_t] symbol name: EntryPoint@103, symbol hash: 10406314031523193289
[2022-04-22 15:44:28.654] [info] [hello_world_pass_t] symbol name: EntryPoint@106, symbol hash: 10406317330058077922
[2022-04-22 15:44:28.655] [info] [hello_world_pass_t] symbol name: EntryPoint@112, symbol hash: 10407300293453509331
[2022-04-22 15:44:28.656] [info] adding transformations to relocation in symbol: EntryPoint@112
[2022-04-22 15:44:28.657] [info] [hello_world_pass_t] symbol name: EntryPoint@110, symbol hash: 10407302492476765753
[2022-04-22 15:44:28.658] [info] [hello_world_pass_t] symbol name: EntryPoint@127, symbol hash: 10408288754407081795
[2022-04-22 15:44:28.659] [info] [hello_world_pass_t] symbol name: EntryPoint@124, symbol hash: 10408289853918710006
[2022-04-22 15:44:28.663] [info] [hello_world_pass_t] symbol name: EntryPoint@122, symbol hash: 10408292052941966428
[2022-04-22 15:44:28.663] [info] [hello_world_pass_t] symbol name: EntryPoint@130, symbol hash: 10409145273965268939
[2022-04-22 15:44:28.664] [info] [hello_world_pass_t] symbol name: EntryPoint@133, symbol hash: 10409146373476897150
[2022-04-22 15:44:28.665] [info] [hello_world_pass_t] symbol name: EntryPoint@132, symbol hash: 10409147472988525361
[2022-04-22 15:44:28.666] [info] [hello_world_pass_t] symbol name: EntryPoint@137, symbol hash: 10409150771523409994
[2022-04-22 15:44:28.666] [info] [hello_world_pass_t] symbol name: ?t@@3Utest_t@@A, symbol hash: 11505995054509648020
[2022-04-22 15:44:28.667] [info] [hello_world_pass_t] symbol name: ??_C@_05MFLOHCHP@World?$AA@, symbol hash: 13259030918861006883
[2022-04-22 15:44:28.668] [info] [hello_world_pass_t] symbol name: EntryPoint, symbol hash: 13363872183459407611
[2022-04-22 15:44:28.670] [info] entry point address: 1DFFBDF0000
```