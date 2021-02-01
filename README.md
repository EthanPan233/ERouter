# ERouter

A simple one step router which can use A* routing or Maze routing.

## How to run

```bash
git clone https://github.com/EthanPan233/ERouter.git
cd ERouter
mkdir build
cd build
cmake ../src
cmake --build
./OneStepRouter [benchmark-name] [routing-algorithm]
```

for example:

```bash
./OneStepRouter oswald maze
```

or

```bash
./OneStepRouter wavy astar
```
