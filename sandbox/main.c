#include "wa.h"
#include <stdio.h>

int main(void)
{
    int ret;
    wa_window_t* window;
    
    if ((window = wa_window_create("Test", 900, 600, false)) == NULL)
        return -1;

    ret = wa_window_mainloop(window);

    wa_window_delete(window);

    printf("exit code: %d\n", ret);

    return ret;
}
