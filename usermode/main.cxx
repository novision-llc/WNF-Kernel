#include <impl/includes.h>

#define return_error std::cin.get(); return 1

int main()
{
    if (!Interface.IsRunning())
    {
        std::cout << "Cannot locate kernel module!" << std::endl;
        return_error;
    }

    while (!global::process::pid)
    {
        global::process::pid = Interface.GetProcessIdByName(L"Notepad.exe");
        Sleep(100);
    }
    std::cout << "Found process!" << std::endl;

    global::process::base_address = Interface.GetProcessBaseAddress(global::process::pid);
    if (!global::process::base_address)
    {
        std::cout << "Cannot locate base address!" << std::endl;
        return_error;
    }

    std::cout << "Found base address: 0x" << std::hex << global::process::base_address << std::dec << std::endl;
    Interface.TestRead(global::process::base_address);

    std::cin.get();
    return 0;
}