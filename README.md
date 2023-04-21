# [Year 11 Physics, Applied.](https://l-m.dev/physics-applied)

Welcome! This repository is a collection of interactive demonstrations and physics simulations designed to visualise and reinforce the physics concepts I have learned in my Year 11 physics class.

Each demo will be interactive and will cover a certain noteworthy topic. They will be created as I learn the content and will be provided free of charge for everyone.

The demos included in this repository are written purely in the C programming language and use Emscripten to compile them to WebAssembly.

**Visit the page at [l-m.dev/physics-applied](https://l-m.dev/physics-applied)**

Supporting libraries [sokol](https://github.com/floooh/sokol) and [Dear ImGui](https://github.com/ocornut/imgui) are used as an abstraction layer over the specifics of the web/browser and allow me to focus on writing the demos, not hassle with the looks.

All of the code in this repository is licensed under the MIT open source license. See the LICENSE file for more details.

<!-- To use the demos, simply visit my website and navigate to the Year 11 Physics Applied section. From there, you can select any demo you would like to run and begin interacting with it. -->

# Compilation

I don't like `cmake`, too complicated and not enough control. I took the existing compilation for `libsokol.a` and `libcimgui.a`, then blindly seralised it into a single `Makefile`.

Install Emscripten, this will provide `emcc`, `em++`, and `wasm-opt`.

Oh yeah, don't try this on Windows, just don't.

Call `make` to create HTML files in `public/`.

```
$ make
$ ls public
...
...
```

You can also create production/optimised builds, which will insert certain CFLAGS and run more code. Call with `-j<cores>` to run builds concurrently.

```
$ make PROD=1 -j$(nproc)
```
