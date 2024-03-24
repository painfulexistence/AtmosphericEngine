alias h := help
alias c := configure
alias b := build
alias run := run-debug

help:
  @echo "Available commands:"
  @echo "  just help"
  @echo "  just configure"
  @echo "  just build"
  @echo "  just run"
  @echo "  just run-debug"
  @echo "  just run-release"

configure:
  cmake -S . -B build

build:
  cmake --build build

run-debug: build
  ./build/MazeWorld

run-release: build
  ./build/release/MazeWorld