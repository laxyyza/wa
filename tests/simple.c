#include <wa.h>

int main(void)
{
    wa_window_t* window = wa_window_create("WA Simple Window", 640, 480, false);
    wa_print_opengl();

    wa_window_mainloop(window);

    wa_window_delete(window);

    return 0;
}
