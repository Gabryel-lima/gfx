#include <stdio.h>
#include <stdlib.h>

#include "tinyobj_loader.h"
#include "gfx.h"

static void file_reader(void *ctx, const char *filename, 
                        int is_mtl, const char *obj_filename, 
                        char **buf, size_t *len) {

    (void)ctx; (void)is_mtl; (void)obj_filename;

    FILE *f = fopen(filename, "rb");

    if (!f) { *buf = NULL; *len = 0; return; }
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); *buf = NULL; *len = 0; return; }

    long n = ftell(f);
    if (n < 0) { fclose(f); *buf = NULL; *len = 0; return; }
    rewind(f);

    char *b = malloc((size_t)n + 1);
    if (!b) { fclose(f); *buf = NULL; *len = 0; return; }
    size_t r = fread(b, 1, (size_t)n, f);
    b[r] = '\0';
    fclose(f);
    
    *buf = b;
    *len = r + 1;
}

int main(int argc, char **argv) {
    const char *filename = (argc > 1) ? argv[1] : "./models/triangle.obj";
    Tinyobj_attrib attrib = {0};
    tinyobj_attrib_init(&attrib);
    Tinyobj_shape *shapes = NULL;
    Tinyobj_material *materials = NULL;
    size_t num_shapes = 0, num_materials = 0;

    int rc = tinyobj_load_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials,
                               filename, file_reader, NULL, TINYOBJ_FLAG_TRIANGULATE);
    if (rc != TINYOBJ_SUCCESS) {
        fprintf(stderr, "Erro ao carregar modelo: %d\n", rc);
        return 1;
    }

    printf("Modelo carregado com sucesso!\n");
    printf("Número de formas: %zu\n", num_shapes);
    printf("Número de materiais: %zu\n", num_materials);
    printf("Vertices=%u normals=%u texcoords=%u faces=%u\n",
           attrib.num_vertices, attrib.num_normals, attrib.num_texcoords, attrib.num_faces);

    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);

    return 0;
}
