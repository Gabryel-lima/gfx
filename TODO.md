# TODO do projeto gfx

Backlog vivo para ir fechando as partes que ainda faltam até o projeto ficar completo e funcional.

## Como usar
- Marque o que já estiver pronto com `x`.
- Se um item virar muito grande, quebre em subtarefas menores abaixo dele.
- Se algo deixar de fazer sentido, mova para a seção de notas no fim.

## P0 - Funcionalidade essencial
- [x] Implementar `gfx_mesh_load` (hoje em `core/gfx_mesh.c`) com carregamento real de malha a partir de arquivo.
- [x] Implementar `gfx_mesh_free` (hoje em `core/gfx_mesh.c`) com liberação correta de todos os recursos alocados.
- [x] Corrigir qualquer fluxo de backend que ainda dependa de comportamento fictício ou de retorno artificial.
- [x] Garantir que o backend stub continue útil para smoke tests sem causar recursão ou comportamento confuso.

## P1 - Robustez do caminho CPU
- [x] Melhorar o tratamento de erro na abertura do framebuffer Linux (hoje em `platform/linux/fb_open.c`).
- [x] Revisar o framebuffer e o rasterizador (hoje `core/gfx_framebuffer.c` e `core/gfx_raster.c`) para cobrir melhor casos-limite e erros de entrada.
- [x] Reavaliar `gfx_fminf` e `gfx_fmaxf` (hoje em `core/gfx_math.c`) para lidar corretamente com NaN e infinitos, se isso for necessário para o projeto.
- [x] Decidir se o suporte a quadriláteros no rasterizador deve ser implementado ou removido definitivamente.

## P1 - Parser OBJ/MTL
- [x] Fechar os `@todo` restantes em `include/tinyobj_loader.h`.
- [x] Melhorar o tratamento de parâmetros desconhecidos em MTL.
- [x] Revisar suporte a múltiplos grupos e objetos no loader.
- [x] Validar se a estratégia de alocação atual do parser continua adequada ou se vale migrar para estruturas dinâmicas.
- [x] Adicionar testes com OBJ/MTL reais para cobrir casos simples e casos com arestas.

## P1 - Testes e validação
- [x] Criar uma suíte de testes automatizados para os módulos centrais.
- [x] Adicionar integração com `ctest` no CMake.
- [x] Fazer o alvo `test` do `Makefile` apontar para uma rotina real de testes.
- [x] Cobrir pelo menos: math básica, parser OBJ/MTL, rasterização CPU e smoke test da fachada pública.

## P2 - Infraestrutura
- [x] Adicionar CI para buildar o projeto em Linux.
- [x] Validar builds em `Debug` e `Release`.
- [x] Documentar dependências de runtime e permissões necessárias para o caminho CPU.
- [x] Verificar se o carregamento dinâmico de `libGL.so.1` e `libX11.so` precisa de mensagens de erro melhores.

## P2 - Janela nativa e portabilidade
- [x] Criar um backend de janela no Linux para renderizar em uma janela própria, com ciclo de apresentação e integração de eventos.
- [x] Separar a camada de plataforma para isolar criação de janela, contexto gráfico e apresentação do frame.
- [x] Validar o fluxo completo no Linux antes de iniciar a porta para Windows e macOS.
- [x] Preparar a abstração de plataforma para que a API pública de alto nível não precise mudar quando vier a portabilidade.

## P2 - Documentação e consistência
- [x] Revisar o `README.md` depois que os stubs GPU forem implementados.
- [x] Atualizar a documentação quando a API pública mudar.
- [x] Garantir que a documentação não prometa recursos que ainda não existem.
- [x] Manter exemplos e headers públicos alinhados com o contrato real.

## Critério de pronto
- [x] `cmake --build build` conclui sem erros.
- [x] `ctest` ou o alvo equivalente executa a suíte sem falhas.
- [x] `gfx_demo` roda como smoke test da fachada pública.
- [x] `tinyobj_demo` carrega um OBJ e gera a prévia PPM sem intervenção extra.
- [ ] O caminho GPU deixa de ser esboço e passa a funcionar de ponta a ponta.
	- [x] Implementar `gfx_platform_window_draw_mesh` em `gpu/linux_window.c` com OpenGL real.
	- [x] Implementar `gfx_platform_window_set_camera` em `gpu/linux_window.c` com matrizes de vista/projeção.
	- [x] Integrar upload de malhas, shaders e descarte dos recursos GL no backend de janela.
	- [x] Fazer `examples/window_demo.c` desenhar uma malha real em vez de apenas limpar a tela.
	- [ ] Adicionar texturas com correção perspectiva.
	- [ ] Adicionar iluminação Phong.
	- [ ] Adicionar sombras.
	- [x] Automatizar uma validação de saída do `gfx_window_demo` sob Xvfb.
	- [ ] Avaliar um backend Wayland.
	- [ ] Avaliar suporte Windows/WGL.

## P0 - Reorganização por responsabilidade (núcleo portátil vs. extras de plataforma)
- [x] Separar o repositório em `core/` (núcleo portátil: math, rasterizador, framebuffer, mesh, parser OBJ, fachada) e `platform/linux/` (extras: fb0, X11, GL via dlopen).
- [x] Remover `Material`, tipo fantasma nunca definido que era passado sempre `NULL`.
- [x] Adicionar o slot `cleanup` a `GfxBackend` e fazer `gfx_cleanup()` chamá-lo de verdade.
- [x] Remover `gpu/shader.c` e `src/internal/shader.h` (código morto: não compilava em nenhum alvo, duplicado inline em `linux_window.c`).
- [x] Tornar `Mesh` e o rasterizador (`gfx_raster.h`) parte do contrato público em `include/`, em vez de "internos" que os exemplos precisavam incluir por caminho relativo.
- [x] Tirar o campo `fd` de `Framebuffer`; quem precisa de um descritor de arquivo usa `GfxLinuxFb` (`gfx_platform_linux_fb.h`).
- [x] Adicionar `mat4_identity()` a `gfx_math.h` para não ter cada consumidor reescrevendo o mesmo literal.
- [x] Alvo `gfx_core` no CMake linkado sem `dl`/`m`; opção `GFX_CORE_FREESTANDING_CHECK` prova que o subconjunto matemática+rasterizador+framebuffer compila com `-ffreestanding`.
- [ ] Portar esse núcleo (`include/` + `core/`) para dentro do kernel do [AlmaOS](https://github.com/Gabryel-lima/AlmaOS) quando o boot lá passar a preencher o framebuffer no `boot_info`.

## Notas
- O foco inicial deve ser fechar os stubs e os testes; documentação vem logo depois para não cristalizar promessas falsas.
- Se aparecerem novos `TODO`, `FIXME` ou `HACK`, eles podem entrar aqui na categoria certa.
