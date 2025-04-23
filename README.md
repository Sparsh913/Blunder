# Blunder: A Play Tool for Mesh Editing


## Introduction
Welcome! Blunder (inspired from the open source software, Blender) is a light weight and user-friendly mesh manipulation play software to edit STL files. Users can import STL files from a set of primitives or load their own. It uses a Laplacian deformation module and gradient descent-based mesh smoothing to manipulate the mesh vertices. Please feel free to have a look at the presentation to understand how to use the controls.


## Compile Instruction
Prerequisite is a compiler and cmake.

Blunder requires [CaptainYS'](https://github.com/captainys) public libraries to build. Follow the steps below:

```
(In your working directory)
git clone https://github.com/Sparsh913/Blunder.git
cd Blunder
mkdir build
cd build
cmake ../src
cmake --build . --target laplacian
```

## Contribution
We'd like to invite the community to develop on it and make it better by improving the UI, extending the support for additional file formats. It's just a fun project that we did for our advanced C++ course at CMU (24783) and it's not no way close to a real product. But it's a start. Hope you have fun playing with it!