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
	- [x] Implementar `gfx_platform_window_draw_mesh` em `platform/linux/window.c` com OpenGL real.
	- [x] Implementar `gfx_platform_window_set_camera` em `platform/linux/window.c` com matrizes de vista/projeção.
	- [x] Integrar upload de malhas, shaders e descarte dos recursos GL no backend de janela.
	- [x] Fazer `examples/window_demo.c` desenhar uma malha real em vez de apenas limpar a tela.
	- [x] Iluminação Phong por fragmento no backend GL (difusa + especular, matriz
	      de normais, `GfxLight` pública). Normais por vértice viraram parte da API
	      (`gfx_mesh_normals`); OBJ sem `vn` recebe a normal geométrica da face.
	- [x] Correção perspectiva na interpolação. **Atenção ao que este item
	      significava:** varyings de GLSL já são interpolados com correção
	      perspectiva por especificação — não havia nada a corrigir na GPU. O
	      problema real estava no rasterizador por software (`core/gfx_raster.c`),
	      que interpolava cor com baricêntricas lineares em espaço de tela. O
	      campo `w` do vértice, antes ignorado, virou parte do contrato.
	- [ ] Adicionar texturas de verdade: carregar imagem, amostrar no shader e
	      honrar `map_Kd` do MTL. Os UVs já saem do carregador
	      (`gfx_mesh_texcoords`), então falta o decodificador de imagem e o
	      caminho de textura nos dois backends — nada disso existe hoje.
	- [ ] Adicionar sombras (shadow mapping: FBO, textura de profundidade,
	      segunda passada do ponto de vista da luz).
	- [x] Automatizar uma validação de saída do `gfx_window_demo` sob Xvfb. Deixou
	      de ser "o binário não quebrou": a demo lê o framebuffer de volta
	      (`gfx_platform_window_read_pixel`), amostra uma grade sobre o cubo e sai
	      com código 1 se a variação de luminância for baixa demais.
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
- [x] Portar esse núcleo (`include/` + `core/`) para dentro do kernel do [AlmaOS](https://github.com/Gabryel-lima/AlmaOS) quando o boot lá passar a preencher o framebuffer no `boot_info`.
	- Feito. O AlmaOS vendorizou `gfx_math.{c,h}`, `gfx_raster.{c,h}` e
	  `gfx_framebuffer.c` em `third_party/gfx/`, compilados com o toolchain do
	  kernel (`gcc -m32 -ffreestanding -nostdinc`), e escreveu a ponte em
	  `src/kernel/gfx_bridge.c`.
	- A condição do "quando" foi resolvida por outro caminho, e vale registrar:
	  não bastava o stage2 preencher o `boot_info`. Ligar um modo gráfico no
	  bootloader custaria a saída de texto do kernel (panic, log, shell) logo no
	  boot. O AlmaOS implementou um trampolim de modo real para chamar `int 10h`
	  em runtime, então quem escolhe o modo é o kernel, não o boot.
	- A ponte não aponta o gfx direto para o framebuffer do hardware: o gfx
	  desenha num backbuffer RGBA de 32 bits e a ponte converte para o formato do
	  modo ativo. Modos VBE costumam ser BGRX e o VGA 13h é indexado de 8 bits;
	  o gfx produz `0xRRGGBBAA`.
	- Um `make gfx-test` do lado do AlmaOS trava os contratos de que a ponte
	  depende (formato de cor, sentido do z-buffer, pitch em bytes, correção
	  perspectiva), para que uma ressincronização com este repositório não quebre
	  o kernel em silêncio.

## Notas
- O foco inicial deve ser fechar os stubs e os testes; documentação vem logo depois para não cristalizar promessas falsas.
- Se aparecerem novos `TODO`, `FIXME` ou `HACK`, eles podem entrar aqui na categoria certa.
- O rasterizador ganhou também a regra top-left, que não estava no backlog. Sem
  ela, dois triângulos que compartilham uma aresta desenham o mesmo pixel duas
  vezes: invisível com cores opacas e z-buffer, mas errado assim que houver
  blending. O único caso ambíguo é o zero exato da função de aresta, e a regra
  o atribui a exatamente um dos dois triângulos.
- Os caminhos que faltam no item de GPU (texturas, sombras, Wayland, WGL) não
  foram começados. Nenhum deles é "quase pronto": texturas precisam de um
  decodificador de imagem que o projeto não tem, e sombras precisam de FBO e de
  uma segunda passada de renderização.
