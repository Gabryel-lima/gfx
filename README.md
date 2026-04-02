# gfx — API gráfica em C puro

Biblioteca de renderização 2D/3D escrita em C99, organizada em uma fachada pública pequena e em módulos de suporte para math, framebuffer, rasterização, OBJ/MTL e integração dinâmica com GL/X11.

Hoje o contrato público vive em `include/` e o suporte interno fica em `src/internal/`; os helpers específicos dos exemplos ficam em `examples/`. A documentação abaixo separa essas fronteiras para deixar claro o que é contrato público e o que ainda é suporte interno ou esboço de backend.

Uma síntese visual adicional está em [svg/dual_backend_architecture.svg](svg/dual_backend_architecture.svg), mas as visões Mermaid abaixo são a referência principal desta documentação.

![C99](https://img.shields.io/badge/C99-00599C?style=for-the-badge) ![Linux](https://img.shields.io/badge/Linux-2E2E2E?style=for-the-badge) ![Mermaid%20Docs](https://img.shields.io/badge/Mermaid%20Docs-1F6FEB?style=for-the-badge) ![CPU%20%2B%20GPU](https://img.shields.io/badge/CPU_%2B_GPU-FF8C00?style=for-the-badge)

> [!TIP]
> O contrato público vive em `include/gfx.h`, `include/gfx_math.h` e `include/tinyobj_loader.h`. O restante da árvore apoia esse contrato, fica em `src/internal/` ou serve apenas aos exemplos.

## Estrutura do projeto

```
gfx/
├── include/                ← headers públicos estáveis (`gfx.h`, `gfx_math.h`, `tinyobj_loader.h`)
├── cpu/                    ← framebuffer Linux e rasterização software
├── gpu/                    ← carregamento dinâmico GL/X11 e stubs GPU
├── src/                    ← implementação comum, parser OBJ/MTL e stubs
│   └── internal/           ← headers privados de suporte
├── examples/               ← smoke tests, demos e helpers locais
├── svg/                    ← artefatos visuais auxiliares
└── CMakeLists.txt
```

## 🧭 Visão geral da arquitetura

A documentação foi particionada em quatro visões: fachada pública, dados/carregamento, CPU/framebuffer e GPU/dlopen. Cada diagrama é legível por si só e termina apontando para a próxima camada mais específica.

### 1. 🏛️ Fachada pública e pontos de entrada

`gfx.h` concentra o contrato estável do projeto. `GfxContext` guarda o backend escolhido pela aplicação e despacha as chamadas para `GfxBackend`; os exemplos mostram dois usos diferentes: `examples/main.c` exercita o dispatcher com o backend stub, enquanto `examples/tinyobj_demo.c` usa os módulos de suporte para gerar uma prévia do OBJ em CPU.

```mermaid
flowchart TD
  app[Aplicação]

  subgraph Public["Fachada pública"]
    gfxh[gfx.h]
    ctx[GfxContext]
    backend[GfxBackend]
    api[gfx_begin_frame<br/>gfx_end_frame<br/>gfx_draw_mesh<br/>gfx_set_camera<br/>gfx_cleanup<br/>gfx_get_stub_backend]
  end

  subgraph Shared["Tipos compartilhados"]
    math[gfx_math.h]
  end

  subgraph Samples["Exemplos"]
    demo1[examples/main.c]
    demo2[examples/tinyobj_demo.c]
  end

  app --> gfxh
  gfxh --> ctx
  ctx --> backend
  gfxh -. usa .-> math
  demo1 --> gfxh
  demo1 --> api
  demo2 --> math
  demo2 --> tinyobj[include/tinyobj_loader.h]
  demo2 --> fb[src/internal/framebuffer.h]
  demo2 --> rast[src/internal/rasterizer.h]
```

Próxima visão: dados compartilhados e parser OBJ/MTL.

### 2. 🧱 Dados compartilhados e parser OBJ/MTL

`include/gfx_math.h` fornece os tipos geométricos e o `Framebuffer` usado pelos módulos de suporte. `include/tinyobj_loader.h` é o parser de OBJ/MTL em estilo header-only; `src/tinyobj_loader.c` apenas habilita a implementação. Essa camada produz `TINYOBJ_ATTRIB`, `TINYOBJ_SHAPE` e `TINYOBJ_MATERIAL`, que alimentam as demos e os módulos de rasterização.

```mermaid
flowchart TD
  subgraph Data["Dados e carregamento"]
    math[gfx_math.h]
    tinyhdr[include/tinyobj_loader.h]
    tinyimpl[src/tinyobj_loader.c]
    tinyapi[tinyobj_load_obj<br/>tinyobj_load_mtl<br/>tinyobj_attrib_init/free<br/>tinyobj_shapes_free<br/>tinyobj_materials_free]
    tinydata[TINYOBJ_ATTRIB<br/>TINYOBJ_SHAPE<br/>TINYOBJ_MATERIAL]
  end

  demo[examples/tinyobj_demo.c]

  tinyimpl --> tinyhdr
  tinyhdr --> tinyapi
  tinyapi --> tinydata
  demo --> math
  demo --> tinyhdr
```

Detalhado em: suporte CPU e framebuffer.

### 3. 🖥️ Suporte CPU e framebuffer

A camada `cpu/` e `src/internal/framebuffer.h` cobre o caminho software. `cpu/fb0_platform.c` abre e escreve em `/dev/fb0`; `cpu/framebuffer.c` fecha e limpa a superfície; `cpu/rasterizer.c` desenha triângulos com z-buffer usando os tipos de `include/gfx_math.h`. A demo `examples/tinyobj_demo.c` usa essas primitivas para renderizar em memória e exportar um PPM.

```mermaid
flowchart TD
  subgraph CPU["Suporte CPU"]
    fbapi[src/internal/framebuffer.h]
    fbplat[cpu/fb0_platform.c]
    fbcore[cpu/framebuffer.c]
    rastapi[src/internal/rasterizer.h]
    rastimpl[cpu/rasterizer.c]
    math[include/gfx_math.h]
  end

  dev[/dev/fb0/]
  demo[examples/tinyobj_demo.c]
  preview[Framebuffer em memória]
  out[Preview PPM]

  fbapi --> fbplat
  fbapi --> fbcore
  rastapi --> rastimpl
  math --> rastimpl
  demo --> preview
  demo --> fbapi
  demo --> rastapi
  demo --> out
  fbplat --> dev
```

Detalhado em: suporte GPU e carregamento dinâmico.

### 4. 🧩 Suporte GPU e carregamento dinâmico

Os arquivos em `gpu/` são infraestrutura de integração. `gpu/gl_loader.c` e `gpu/x11_platform.c` resolvem um subconjunto pequeno de símbolos de `libGL.so.1` e `libX11.so` em runtime; `gpu/mesh.c` e `gpu/shader.c` ainda são esboços, então esta visão deve ser lida como scaffolding de backend, não como pipeline GPU completo.

```mermaid
flowchart TD
  subgraph GPU["Suporte GPU"]
    glapi[src/internal/gl_loader.h]
    x11api[src/internal/x11_platform.h]
    meshapi[src/internal/mesh.h]
    shaderapi[src/internal/shader.h]
    glimpl[gpu/gl_loader.c]
    x11impl[gpu/x11_platform.c]
    meshimpl[gpu/mesh.c]
    shaderimpl[gpu/shader.c]
  end

  libgl[(libGL.so.1)]
  libx11[(libX11.so)]

  glapi --> glimpl --> libgl
  x11api --> x11impl --> libx11
  x11impl --> libgl
  meshapi --> meshimpl
  shaderapi --> shaderimpl
```

Detalhado em: API atual e notas de implementação.

## ✨ API atual

A superfície estável do projeto está em `include/gfx.h`, `include/gfx_math.h` e `include/tinyobj_loader.h`. O restante dos headers é suporte técnico ou infraestrutura de integração.

### `gfx.h`
- `GfxBackend` e `GfxContext`
- `gfx_begin_frame`, `gfx_end_frame`
- `gfx_draw_mesh`, `gfx_set_camera`
- `gfx_cleanup`
- `gfx_get_stub_backend`

### Tipos compartilhados em `include/gfx_math.h`
- `Framebuffer`
- `Vec2`, `Vec3`, `Vec4`, `Mat4`
- `fminf`, `fmaxf`
- `vec3_min`, `vec3_max`, `vec3_clamp`
- `edge2d`, `vec3_to_rgba`

### Suporte CPU em `src/internal/framebuffer.h` e `src/internal/rasterizer.h`
- `fb_open`, `fb_close`
- `fb_set_pixel`, `fb_clear`
- `rasterize_triangle`

### Suporte GPU em `src/internal/gl_loader.h`, `src/internal/x11_platform.h`, `src/internal/mesh.h` e `src/internal/shader.h`
- `GLProcs`, `gl_load`
- `PlatformGL`, `platform_gl_init`
- `gfx_mesh_load`, `gfx_mesh_free`
- `shader_create_from_source`, `shader_destroy`

Observação: `gfx_mesh_load`/`gfx_mesh_free` e `shader_create_from_source`/`shader_destroy` ainda são stubs ou esboços, então a integração de backend GPU ainda não está completa.

### Parser OBJ/MTL em `include/tinyobj_loader.h`
- `tinyobj_load_obj`, `tinyobj_load_mtl`
- `tinyobj_attrib_init`, `tinyobj_attrib_free`
- `tinyobj_shapes_free`, `tinyobj_materials_free`
- `tinyobj_attrib_get_vertex`, `tinyobj_attrib_compute_bounds`
- `tinyobj_material_color`, `tinyobj_project_vertex`
- `tinyobj_save_preview_ppm` (`src/tinyobj_preview.c`)

### Utilitários de filesystem em `examples/tinyobj_utils.h`
- `tinyobj_get_executable_path`
- `tinyobj_strip_basename`, `tinyobj_copy_path`, `tinyobj_join_path`
- `tinyobj_read_file`
- `tinyobj_default_file_reader`

## ▶️ Uso básico

```c
#include "gfx.h"

int main(void) {
    GfxContext ctx = {
        .backend = gfx_get_stub_backend(),
        .backend_ctx = NULL,
    };

    Mat4 model = {
        .col = {
            { 1, 0, 0, 0 },
            { 0, 1, 0, 0 },
            { 0, 0, 1, 0 },
            { 0, 0, 0, 1 },
        },
    };

    gfx_begin_frame(&ctx);
    gfx_set_camera(&ctx, (Vec3){0, 1, 3}, (Vec3){0, 0, 0}, 60.0f);
    gfx_draw_mesh(&ctx, NULL, model, NULL);
    gfx_end_frame(&ctx);
    gfx_cleanup(&ctx);
    return 0;
}
```

## 🛠️ Compilação

Recomendado (CMake):

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

O `CMakeLists.txt` gera dois alvos principais: `gfx_demo` e `tinyobj_demo`. O `Makefile` da raiz apenas delega para esses passos.

## 🚀 Executando os exemplos

- `./build/gfx_demo` executa o smoke test da fachada pública usando `gfx_get_stub_backend()`.
- `./build/tinyobj_demo [modelo.obj] [saida.ppm]` carrega OBJ/MTL e grava uma prévia em PPM; se nenhum argumento for passado, ele resolve caminhos padrão a partir do diretório do executável.

## ⚙️ Dependências em tempo de execução

- Linux, porque o projeto usa `/dev/fb0` e `linux/fb.h` no caminho CPU.
- `libdl` no link, para `dlopen`/`dlsym`.
- `libGL.so.1` e `libX11.so` apenas se você usar os módulos GPU.
- Acesso ao framebuffer do sistema somente se a aplicação chamar `fb_open("/dev/fb0")`.

## 🔎 Notas técnicas

- `src/stubs.c` implementa a fachada pública atual e o backend stub.
- `src/tinyobj_loader.c` existe para compilar a implementação que mora no header `include/tinyobj_loader.h`.
- `src/tinyobj_preview.c` concentra o helper de preview em PPM e depende de `cpu/rasterizer.c`.
- `examples/tinyobj_utils.h` concentra helpers de filesystem usados pelos exemplos e pelo carregamento padrão.
- `examples/main.c` é um exemplo mínimo de despacho.
- `examples/tinyobj_demo.c` usa um framebuffer em memória e exporta um PPM; ele não depende de X11.

## 🗺️ Roadmap

### ✅ Já presente

- Math library (vec/mat/quat)
- Framebuffer `/dev/fb0` (CPU)
- Rasterizador por software e z-buffer
- Clipping (Sutherland–Hodgman)
- Loader GL via `dlopen`
- Janela X11 + contexto GLX
- VAO/VBO management (GPU)
- Parser `.obj` próprio
- Demos e exemplos

### 🔭 Próximos passos

- Integrar um backend CPU completo por meio de uma API de alto nível
- Completar a implementação de `gfx_mesh_load` e `gfx_mesh_free`
- Completar `shader_create_from_source` e `shader_destroy`
- Adicionar texturas com correção perspectiva
- Adicionar iluminação Phong
- Adicionar sombras
- Avaliar um backend Wayland
- Avaliar suporte Windows/WGL

## 📜 Licença

Veja [LICENSE](LICENSE) — MIT License.
