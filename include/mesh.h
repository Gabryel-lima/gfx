#pragma once

#include "../gfx.h"

/**
 * @file include/mesh.h
 * @brief Suporte esboçado para carregamento e liberação de malhas.
 */

/** Carrega uma malha a partir de um arquivo.
 * @param path Caminho para o arquivo da malha
 * @return Ponteiro para Mesh carregada ou NULL em caso de erro.
 * @note Esta função ainda é um stub e deve ser implementada antes de uso real.
 */
Mesh *gfx_mesh_load(const char *path);

/** Libera recursos associados a uma `Mesh`.
 * @param m Ponteiro para a Mesh a ser liberada
 * @return void
 * @note Esta função ainda é um stub.
*/
void gfx_mesh_free(Mesh *m);
