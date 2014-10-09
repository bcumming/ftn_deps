CXX=clang++

ftn_deps: ftn_deps.cpp
	${CXX} -std=c++11 ftn_deps.cpp -o ftn_deps

clean:
	rm -f ftn_deps
	rm -f *.dot
	rm -f file_list.txt
