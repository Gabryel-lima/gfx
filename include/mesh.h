#pragma once

#include "../gfx.h"

/**
 * @file include/mesh.h
 * @brief Carregamento e liberação de malhas (Mesh).
 */

/** Carrega uma malha a partir de um arquivo.
 * @param path Caminho para o arquivo da malha
 * @return Ponteiro para Mesh carregada ou NULL em caso de erro
 */
Mesh *gfx_mesh_load(const char *path);

/** Libera recursos associados a uma `Mesh` */
void  gfx_mesh_free(Mesh *m);
