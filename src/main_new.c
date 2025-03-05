


#include <brutopolis.h>

void new_system(VirtualMachine* vm, char* name, int size_x, int size_y)
{
    eval(vm, "# (system = (:));"
    ""
    
    , NULL);

}


int main(int argc, char* argv[])
{
    VirtualMachine* vm = make_vm();
    init_std(vm);

}