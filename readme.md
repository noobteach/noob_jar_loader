# noob jar injector

## good

- noob friendly
- gui
- get jar via net without touch disk
- auto sort cls deps
- supports jdk8~26+ & minecraft1.0.0~26.2+ autodetect
- your injector only needs to do init .bss section & fix iats
- you can move .bss to stack, but must set linker option to enable 1MB+ stack (your injector needs support create big stack thread too)
- no windows.h kernel32 user32 ntdll
- no heap alloc
- no thread creation
- no stl, no crt init needed
- no jvmti
- no memory leak
- no localref leak

## bad

- must use manual mapping injection, using LoadLibrary will make game stuck
- hardcoded limits: max jar size, max cls/iface count (adjustable)
- you still need to know what is `<clinit>`
- manual mapping is still too hard for noobs
