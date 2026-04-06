
# C# Web Export Demo (With bonus LibGodot)

Only works from Windows and Linux. Link https://noctemcat.itch.io/wasm-demo

## Major considerations 

- It will force the support for Emscripten 3.1.56 until C# is updated (https://github.com/dotnet/runtime/issues/113786).
- CoreCLR also uses `Object` without namespace, so it causes duplicate symbols for `Object` vtable. If wasm moves to CoreCLR 
https://github.com/dotnet/runtime/issues/121511 before `llvm-objcopy` adds support for wasm --redefine-sym https://github.com/llvm/llvm-project/issues/50623 , it will force either putting godot's `Object` inside a namespace or to somehow continuing using mono. Actually maybe if emscripten is updated 
it will be new enough to use allow-multiple-definition https://github.com/llvm/llvm-project/pull/97699 

## Small annoyancies

- C# with multithreading runs main C# thread as a thread https://github.com/dotnet/runtime/issues/101421 .
- Added offscreen canvas for it, but it needs a hack to transfer offscreen canvas to the C# "main" thread, see "web_interop/transferCanvas.js".
- No webxr. Should be possible to add it, but I think it would be better to wait for a newer emscripten.
- Function pointers with long or ulong parameters(like, `delegate* unmanaged<ulong, void>`) don't generate trampolines to native code.
`[UnmanagedFunctionPointer(CallingConvention.Winapi)]` delegate can be used to force their generation.

## What is left to do

- Validate scons changes for correct usage.
- Only 2 trampolines were generated with `UnmanagedFunctionPointer`, need to force the generation of others.
- Export plugin. for now it just doesn't use it for C# and skips it. Basically it treats it as a C++ custom template.
- SDK integration. Figured there would be no point in it if it won't be used. And I have no idea how to integrate it gracefully.

## How to build

All the genaration runs through bash script, bash script commands expects you to be in the base folder to work correctly.
Also after it are the native commands if you want to see what it generates or want to use them yourself.

### C# editor
```
bash run build godot editor mono
```
Replace "godot_app" with the actual created editor
```
Godot folder:
scons target=editor library_type=executable extra_suffix=executable production=yes debug_symbols=yes compiledb=yes disable_path_overrides=no module_mono_enabled=yes
./bin/godot_app --generate-mono-glue ./modules/mono/glue --headless
./modules/mono/build_scripts/build_assemblies.py --godot-output-dir ./bin --push-nupkgs-local ./../.nuget_local/
```
### C# editor LibGodot (optional)

Tested if LibGodot C# editor would work, it does(export templates also work). It needs bootstrapping from a native C# editor with already built C# libraries.
```
bash run build godot editor shared mono disable_crash_handler=yes
bash run start csharp --editor
```
```
Godot folder:
scons target=editor library_type=shared_library extra_suffix=shared production=yes debug_symbols=yes compiledb=yes disable_path_overrides=no module_mono_enabled=yes disable_crash_handler=yes
CSharp folder:
dotnet run -c Debug -p:GodotType=shared -p:GodotArch=x86_64 --editor
```
### C# web static LibGodot

Prerequesites:
- Built C# editor with C# libraries. 
- **Install and activate Emscripten 3.1.56**.
- Replace -p:GodotEditor=REPLACEWITHGODOTEDITOREXE with the actual name of your godot editor that was previous compiled.
- Cannot have a space in the path leading up to the project.
- Must be on C: drive (if using Windows).
```
bash run build godot template_release static mono platform=web disable_crash_handler=yes
bash run build csharp template_release static platform=web -p:GodotEditor=REPLACEWITHGODOTEDITOREXE
emrun csharp_project/export/Web_static_release/
```
```
Godot folder:
scons target=template_release platform=web library_type=static_library extra_suffix=static compiledb=yes disable_path_overrides=no module_mono_enabled=yes disable_crash_handler=yes
CSharp folder:
dotnet publish -v:d -c ExportRelease -r browser-wasm -p:GodotType=static -p:GodotArch=wasm32 -p:GodotEditor=REPLACEWITHGODOTEDITOREXE
emrun csharp_project/export/Web_static_release/
```
Or a version without threads:
```
bash run build godot template_release static mono platform=web disable_crash_handler=yes threads=no
bash run build csharp template_release static platform=web suffix=nothreads -p:GodotEditor=REPLACEWITHGODOTEDITOREXE
emrun csharp_project/export/WebNothreads_static_release/
```
```
Godot folder:
scons target=template_release platform=web library_type=static_library extra_suffix=static compiledb=yes disable_path_overrides=no module_mono_enabled=yes disable_crash_handler=yes threads=no
CSharp folder:
dotnet publish -v:d -c ExportRelease -r browser-wasm -p:GodotType=static -p:GodotArch=wasm32 -p:ExtraSuffix=nothreads
emrun csharp_project/export/WebNothreads_static_release/
```
## Help
```
bash run --help

Available commands:
  bash run build godot [editor] [template_debug] [template_release] [executable|static|shared] [mono] [platform=(linuxbsd|windows|macos|web)] [scons args]
  bash run build cpp [editor] [template_debug] [template_release] [static|shared] [suffix=*] [arch=*] platform=(linuxbsd|windows|macos|web) [cmake configure args]
  bash run build csharp [editor] [template_debug] [template_release] [static|shared] [suffix=*] [arch=*] [platform=(linuxbsd|windows|macos|web)] [msbuild args]
  bash run start cpp [suffix=*] [arch=*] platform=(linuxbsd|windows|macos|web) [cmake configure args]
  bash run start csharp [suffix=*] [arch=*] [platform=(linuxbsd|windows|macos|web)] [msbuild args]

Arguments inside [] are optional, arguments with '=' expect value, '*' is any value

Available modifiers:
  bash run -i [command]: will enable interactive mode, you can choose if you want to lauch the command
```
## Other

### Why are there C++?
It's what made it all possible, as at first I needed to add static LibGodot, which is so much easier to make work with C++ first.

### Any commands for it?
Nope, they should be similar to C# ones tho. For the web I moved to C# after the basics were working, so cmake is not updated.