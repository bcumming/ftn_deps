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

typedef std::map<std::string, int> NameMap;
typedef std::pair<std::string, int> NamePair;
typedef std::vector<std::vector<int>> Map;

int get_key(std::string const& name, NameMap& map) {
    auto key_pos = map.find(name);
    bool is_present = (key_pos != map.end());
    if( is_present ) {
        return key_pos->second;
    } else {
        int key = map.size();
        map.insert( NamePair(name, key));
        return key;
    }
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

void print_deps(std::vector<int> const& keys, Map const& child_map, Map const& parent_map, NameMap const& map) {
    std::ofstream fout("depend.dot");
    fout << "Digraph G {" << std::endl;
    for(auto k : keys) {
        auto const& children = child_map[k];
        std::string name = get_name(k, map);
        for(auto child : children) {
            fout << "  " << name << " -> " << get_name(child, map) << std::endl;
        }
        auto const& parents = parent_map[k];
        for(auto parent : parents) {
            fout << "  " << get_name(parent, map) << " -> " << name << std::endl;
        }
    }
    fout << "}" << std::endl;
}

int main(void) {
    ///////////////////////////////////////////////
    // scan the input file to build a list of dependencies
    ///////////////////////////////////////////////

    // open the input file
    std::ifstream fid("depend", std::ifstream::in);
    if( !fid.is_open() ) {
        std::cout << "ERROR: unable to open \"depend\" for input" << std::endl;
        return 0;
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
        for(auto const& name : split_string(line, ' ')) {
            get_key(name, name_map);
        }
    }
    // parse again to build the DAG
    Map parent_map(name_map.size());
    Map child_map(name_map.size());
    for(auto const& line : lines) {
        auto members = split_string(line, ' ');
        std::vector<int> keys;
        for(auto const& name : members) {
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

    // sample of how to print out a graphviz file to visualize dependencies of
    // a key. Don't try this for the global dependencies list, the graph is
    // too big to realistically render
    std::vector<int> keys_to_print = {32, 4};
    print_deps(keys_to_print, child_map, parent_map, name_map);

    std::cout << "sorting" << std::endl;
    // topological sort on the DAG
    // initialize set S with all nodes with no incoming edges
    std::set<int> S;
    int k = 0;
    for(auto const& parents : parent_map) {
        if( !parents.size() ) {
            S.insert(k);
        }
        ++k;
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

    ///////////////////////////////////////////////
    // output
    ///////////////////////////////////////////////

    std::ofstream fout("file_list.txt");
    for(auto entry : L) {
        auto name = get_name(entry, name_map);
        strip(name);
        fout << name << " ";
    }
    fout.close();

    // print DAG to screen
    std::cout << "there are " << L.size() << " files to compile" << std::endl;

    return 0;
}

