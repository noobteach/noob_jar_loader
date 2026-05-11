# noob jar injector

## good

- noob friendly
- gui
- get jar via net without touch disk
- auto sort cls deps
- supports jdk8\~26+ & minecraft1.0.0\~26.2+ autodetect
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

<img width="856" height="512" alt="1" src="https://github.com/user-attachments/assets/1736ee0f-dc26-4956-8dba-926586503786" />
<img width="856" height="512" alt="2" src="https://github.com/user-attachments/assets/48edc4a6-ed51-4a00-b98f-d8d6a41ccea9" />
<img width="856" height="512" alt="3" src="https://github.com/user-attachments/assets/4bc632d1-e568-4a2a-a260-bf8c00f96f45" />
<img width="856" height="512" alt="4" src="https://github.com/user-attachments/assets/f3493c33-17d4-4b27-ba65-0507e4298868" />
