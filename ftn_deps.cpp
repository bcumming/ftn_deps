#include <cassert>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string>
split_string(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string>
split_string(const std::string &s, char delim) {
    std::vector<std::string> elems;
    return split_string(s, delim, elems);
}

typedef std::map<std::string, int>      NameMap;
typedef std::pair<std::string, int>     NamePair;
typedef std::vector<std::vector<int>>   Map;
typedef std::map<int, std::vector<int>> SparseMap;

int find_key(std::string const& name, NameMap& map) {
    auto key_pos = map.find(name);
    return key_pos==map.end() ? -1 : key_pos->second;
}

int get_key(std::string const& name, NameMap& map) {
    auto key = find_key(name, map);
    if( key<0 ) {
        key = map.size();
        map.insert( NamePair(name, key));
    }
    return key;
}

std::string get_name(int key, NameMap const& map) {
    for(auto pair : map)
        if(pair.second == key)
            return pair.first;
    return std::string();
}

void strip(std::string &str) {
    auto pos = std::find(str.begin(), str.end(), '.');
    str.erase(pos, str.end());
}

// return a set of (int,int) tuples that describe (parent,child) connections
// in the dependency DAG for key
void get_dependencies(  int key,
                        Map const& parent_map,
                        std::set<std::pair<int,int>> &key_pairs)
{
    for(auto k : parent_map[key]) {
        key_pairs.insert(std::make_pair(k, key));
        get_dependencies(k, parent_map, key_pairs);
    }
}

void print_dependencies(int key, Map const& parent_map, NameMap const& map) {
    std::ofstream fout("depend.dot");
    std::set<std::pair<int,int>> key_pairs;
    get_dependencies(key, parent_map, key_pairs);
    fout << "Digraph G {" << std::endl;
    for(auto const &p : key_pairs) {
        auto from_name = get_name(p.first,  map);
        auto to_name   = get_name(p.second, map);
        fout << "  " << from_name << " -> " << to_name << std::endl;
    }
    fout << "}" << std::endl;
}

template <typename T>
struct container_wrapper {};

// access wrapper for a Map
// the key has to be stored in state separate from the storage
template <>
struct container_wrapper<Map> {
    std::vector<int> const& get_vec(std::vector<int> const& c) {
        return c;
    }

    int get_k(std::vector<int> const& c) {
        return counter_;
    }

    void increment() {
        counter_++;
    }

    int counter_ = 0;
};

// access wrapper for a sparse map
// the key is int the iterator returned from SparseMap
template <>
struct container_wrapper<SparseMap> {
    std::vector<int> const& get_vec(std::pair<int, std::vector<int>> const& c) {
        return c.second;
    }

    int get_k(std::pair<int, std::vector<int>> const& c) {
        return c.first;
    }

    void increment() {};
};

// generate a list that describes a safe build order
// performs topological sort on the DAG described by (parent_map, child_map)
template <typename MapType>
std::list<int> generate_dependency_list(MapType &parent_map, MapType &child_map) {
    std::set<int> S;
    container_wrapper<MapType> wrapper;
    int k=0;
    for(auto const& parents : parent_map) {
        if( !wrapper.get_vec(parents).size() ) {
            S.insert(wrapper.get_k(parents));
        }
        wrapper.increment();
    }
    // build the ordered list
    std::list<int> L;
    while( S.size() ) {
        // remove an entry from S
        int n = *S.begin();
        S.erase(S.begin());
        L.push_back(n);
        for( auto m : child_map[n] ) {
            auto pos = std::find(parent_map[m].begin(), parent_map[m].end(), n);
            parent_map[m].erase(pos);
            if( !parent_map[m].size() ) {
                S.insert(m);
            }
        }
        // remove child edge references
        child_map[n].resize(0);
    }

    return L;
}

int main(int argc, char** argv) {
    ///////////////////////////////////////////////
    // scan the input file to build a list of dependencies
    ///////////////////////////////////////////////

    // open the input file
    std::ifstream fid("depend", std::ifstream::in);
    if( !fid.is_open() ) {
        std::cout << "ERROR: unable to open \"depend\" for input" << std::endl;
        return 1;
    }

    // read the lines from input file
    std::cout << "reading input file for dependencies" << std::endl;
    std::vector<std::string> lines;
    while( !fid.eof() ) {
        std::string line;
        std::getline(fid, line);

        if( line.size() ) { // ignore empty lines
            lines.push_back(line);
        }
    }

    ///////////////////////////////////////////////
    // build a DAG that identifies all dependencies
    ///////////////////////////////////////////////

    std::cout << "generating dependency graph" << std::endl;
    // parse once to get all the keys
    NameMap name_map;
    for(auto const& line : lines) {
        for(auto & name : split_string(line, ' ')) {
            strip(name); // remove trailing .o from name
            get_key(name, name_map);
        }
    }
    // parse again to build the DAG
    Map parent_map(name_map.size());
    Map child_map(name_map.size());
    for(auto const& line : lines) {
        auto members = split_string(line, ' ');
        std::vector<int> keys;
        for(auto & name : members) {
            strip(name); // remove trailing .o from name
            keys.push_back(get_key(name, name_map));
        }
        assert(keys.size()>1);
        auto key = keys[0];

        // enter dependencies for key
        for(int i=1; i<keys.size(); ++i) {
            int k = keys[i];
            parent_map[key].push_back(k);
            child_map[k].push_back(key);
        }
    }

    ///////////////////////////////////////////////
    // sort the file names according to dependency
    ///////////////////////////////////////////////
    std::list<int> L;
    if(argc>1) { // generate dependencies for user requested file
        std::string filename(argv[1]);
        int key = find_key(filename, name_map);
        std::cout << "generating dependency list for " << filename << ".o" << std::endl;
        if(key<0) {
            std::cout << "ERROR : unable to find key for file with name " << filename << std::endl;
            exit(1);
        }
        // print the DAG as a .dot file
        print_dependencies(key, parent_map, name_map);

        // generate the dependencies as a set of pairs
        std::set<std::pair<int,int>> key_pairs;
        get_dependencies(key, parent_map, key_pairs);

        // generate a list of the unique nodes in this set of dependencies
        std::set<int> nodes;
        for(auto const& p: key_pairs) {
            nodes.insert(p.first);
            nodes.insert(p.second);
        }

        // use sparse storage because only the nodes upon which filename depends
        // need to be stored and indexed
        SparseMap reduced_child_map;
        SparseMap reduced_parent_map;
        // initialize the storage for each node that filename depends on
        for(auto n: nodes) {
            reduced_parent_map[n] = std::vector<int>();
            reduced_child_map[n]  = std::vector<int>();
        }

        // build the edges in the dependency DAG of filename
        for(auto p: key_pairs) {
            reduced_child_map[p.first].push_back(p.second);
            reduced_parent_map[p.second].push_back(p.first);
        }

        L = generate_dependency_list(reduced_parent_map, reduced_child_map);
    }
    else { // generate all dependencies
        std::cout << "generating full dependency list" << std::endl;
        L = generate_dependency_list(parent_map, child_map);
    }

    ///////////////////////////////////////////////
    // output
    ///////////////////////////////////////////////

    std::ofstream fout("file_list.txt");
    for(auto entry : L) {
        fout << get_name(entry, name_map) << " ";
    }
    fout.close();

    // print DAG to screen
    std::cout << "there are " << L.size() << " files to compile" << std::endl;

    return 0;
}

