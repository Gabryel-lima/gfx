#include <stdio.h>
#include <stdlib.h>

#include "tinyobj_utils.h"

#define PREVIEW_WIDTH 800
#define PREVIEW_HEIGHT 600
#define PREVIEW_MARGIN 32

int main(int argc, char **argv) {
    char executable_path[TINYOBJ_UTILS_PATH_MAX];
    char build_dir[TINYOBJ_UTILS_PATH_MAX];
    char project_root[TINYOBJ_UTILS_PATH_MAX];
    char default_model_path[TINYOBJ_UTILS_PATH_MAX];
    char default_output_path[TINYOBJ_UTILS_PATH_MAX];
    const char *filename = (argc > 1) ? argv[1] : NULL;
    const char *output_path = (argc > 2) ? argv[2] : NULL;

    if (tinyobj_get_executable_path(executable_path, sizeof(executable_path)) == 0) {
        if (tinyobj_copy_path(build_dir, sizeof(build_dir), executable_path) == 0 &&
            tinyobj_strip_basename(build_dir) == 0 &&
            tinyobj_copy_path(project_root, sizeof(project_root), build_dir) == 0 &&
            tinyobj_strip_basename(project_root) == 0 &&
            tinyobj_join_path(default_model_path, sizeof(default_model_path), project_root, "examples/models/triangle.obj") == 0 &&
            tinyobj_join_path(default_output_path, sizeof(default_output_path), build_dir, "tinyobj_triangle.ppm") == 0) {
            /* default paths resolved from the executable location */
        } else {
            snprintf(default_model_path, sizeof(default_model_path), "examples/models/triangle.obj");
            snprintf(default_output_path, sizeof(default_output_path), "tinyobj_triangle.ppm");
        }
    } else {
        snprintf(default_model_path, sizeof(default_model_path), "examples/models/triangle.obj");
        snprintf(default_output_path, sizeof(default_output_path), "tinyobj_triangle.ppm");
    }

    if (!filename) {
        filename = default_model_path;
    }
    if (!output_path) {
        output_path = default_output_path;
    }

    TinyObj_Attrib attrib = { 0 };
    tinyobj_attrib_init(&attrib);

    TinyObj_Shape *shapes = NULL;
    TinyObj_Material *materials = NULL;
    size_t num_shapes = 0;
    size_t num_materials = 0;

    int rc = tinyobj_load_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials,
                               filename, tinyobj_default_file_reader, NULL, TINYOBJ_FLAG_TRIANGULATE);
    if (rc != TINYOBJ_SUCCESS) {
        fprintf(stderr, "Falha ao carregar modelo: %d\n", rc);
        return 1;
    }

    Vec3 min_v, max_v;
    tinyobj_attrib_compute_bounds(&attrib, &min_v, &max_v);
    tinyobj_print_summary(filename, &attrib, shapes, num_shapes, materials, num_materials, min_v, max_v);

    if (!attrib.face_num_verts || attrib.num_face_num_verts == 0) {
        fprintf(stderr, "Modelo sem faces para renderizar\n");
        tinyobj_attrib_free(&attrib);
        tinyobj_shapes_free(shapes, num_shapes);
        tinyobj_materials_free(materials, num_materials);
        return 1;
    }

    if (tinyobj_save_preview_ppm(&attrib, shapes, num_shapes, materials, num_materials,
                                 output_path, PREVIEW_WIDTH, PREVIEW_HEIGHT,
                                 0x101018FF, PREVIEW_MARGIN) != TINYOBJ_SUCCESS) {
        fprintf(stderr, "Falha ao salvar preview em %s\n", output_path);
        tinyobj_attrib_free(&attrib);
        tinyobj_shapes_free(shapes, num_shapes);
        tinyobj_materials_free(materials, num_materials);
        return 1;
    }

    printf("Preview salvo em %s\n", output_path);

    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);

    return 0;
}
