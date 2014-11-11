An application for
- generating a list of input files in order of compilation so that dependencies are respected
- generate the set of dependencies and their order of compilation for a single file

###Step 1
convert the __.depend__ file using strip.sh (there is a dummy .depend file already in the repository)
```
> cp <location of .depend>/.depend .
> strip.sh
```

###Step 2
Build the application, for which you need a recent version of __gcc__ or __clang__ that supports C++11
```
> module swap PrgEnv-cray PrgEnv-gnu
> make
g++ -std=c++11 ftn_deps.cpp -o ftn_deps
```

###Step 3
Generate dependencies in __file\_list.txt__.

To generate dependencies for the entire project run the executable with no arguments
```
> ./ftn_deps
reading input file for dependencies
generating dependency graph
generating full dependency list
there are 156 files to compile
```

To generate dependencies for a single file pass the file name (without an extension) as an argument
```
> ./ftn_deps src_leapfrog
reading input file for dependencies
generating dependency graph
generating dependency list for src_leapfrog.o
there are 25 files to compile

> ./ftn_deps pp_utilities
reading input file for dependencies
generating dependency graph
generating dependency list for pp_utilities.o
there are 3 files to compile
```

###How it works

- build a DAG representing the dependencies in a Fortran build
- perform topological sort to determine a safe ordering for build

###Drawing a picture

You can generate a visualization of the dependency graph from the .dot file that is generated automatically when you run ftn\_deps.

Note that it is not a good idea to try drawing the dependencies for the full project, because the resulting graph is enormous. This features is much more useful for viewing the dependencies of an individual file.

```
> dot -Tjpg depend.dot -o depend.jpg
```

![Image]
(https://github.com/bcumming/ftn_deps/blob/master/depend.jpg)
