#pragma once

#include "../../include/gfx.h"
#include "mesh.h"

/**
 * @brief Suporte esboçado para carregamento e liberação de malhas.
 * @author Gabryel-lima
 * @date 2026-03-02
 * @file src/internal/mesh.h
*/

/** Carrega uma malha a partir de um arquivo.
 * @param path Caminho para o arquivo da malha
 * @return Ponteiro para Mesh carregada ou NULL em caso de erro.
 * @note Implementação real em gpu/mesh.c.
*/
Mesh *gfx_mesh_load(const char *path);

/** Libera recursos associados a uma `Mesh`.
 * @param m Ponteiro para a Mesh a ser liberada
 * @return void
 * @note Implementação real em gpu/mesh.c.
*/
void gfx_mesh_free(Mesh *m);
