typedef struct ll_element {
   int width;
   const char* text;
} ll_element;

void ll_render(NVGcontext* vg, ll_element el);
