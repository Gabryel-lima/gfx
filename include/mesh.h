#pragma once

#include "../gfx.h"

/**
 * @file include/mesh.h
 * @brief Carregamento e liberação de malhas (Mesh).
 */

/** Carrega uma malha a partir de um arquivo.
 * @param path Caminho para o arquivo da malha
 * @return Ponteiro para Mesh carregada ou NULL em caso de erro
 * @note O formato do arquivo e os detalhes de carregamento ainda não estão definidos. Esta função é um stub e deve ser implementada para suportar o formato de malha desejado.
 */
Mesh *gfx_mesh_load(const char *path);

/** Libera recursos associados a uma `Mesh` 
 * @param m Ponteiro para a Mesh a ser liberada
 * @return void
 * @note Após chamar esta função, o ponteiro `m` não deve ser usado novamente sem recarregá-lo.
*/
void gfx_mesh_free(Mesh *m);
