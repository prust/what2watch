struct ll_element;

typedef struct ll_element {
   int width;
   const char* text;
   int num_children;
   ll_element* children;
} ll_element;

void ll_render(NVGcontext* vg, ll_element el);
