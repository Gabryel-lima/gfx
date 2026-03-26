#include "../include/tinyobj_loader.h"

// Tenho que impllementar as funções aqui, mas por enquanto vou deixar elas como stubs para compilar o projeto e testar a integração com o demo

int tinyobj_load_obj(Tinyobj_attrib *attrib, Tinyobj_shape **shapes,
                     size_t *num_shapes, Tinyobj_material **materials,
                     size_t *num_materials, const char *file_name, File_reader_callback file_reader,
                     void *ctx, unsigned int flags) {
    (void)attrib; (void)shapes; (void)num_shapes; (void)materials;
    (void)num_materials; (void)file_name; (void)file_reader; (void)ctx; (void)flags;
    return TINYOBJ_SUCCESS;    
}

int tinyobj_load_mtl(Tinyobj_material **materials_out,
                           size_t *num_materials_out,
                           const char *mtl_filename, const char *obj_filename, File_reader_callback file_reader,
                           void *ctx) {
    (void)materials_out; (void)num_materials_out; (void)mtl_filename;
    (void)obj_filename; (void)file_reader; (void)ctx;
    return TINYOBJ_SUCCESS;
}

void tinyobj_attrib_init(Tinyobj_attrib *attrib) {
    (void)attrib;
    return;
}

void tinyobj_attrib_free(Tinyobj_attrib *attrib) {
    (void)attrib;
    return;
}

void tinyobj_shapes_free(Tinyobj_shape *shapes, size_t num_shapes) {
    (void)shapes; (void)num_shapes;
    return;
}

void tinyobj_materials_free(Tinyobj_material *materials, 
                            size_t num_materials) {
    (void)materials; (void)num_materials;
    return;
}
