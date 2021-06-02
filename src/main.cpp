#include <iostream>
#include <fstream>
#include <string.h>
#include <boost/dynamic_bitset.hpp>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <omp.h>
#include <stdio.h>

using namespace std;

struct IntervalScheme {
    int pre;
    int post;

public:
    IntervalScheme();
    IntervalScheme(int _pre, int _post)
    {
        this->pre = _pre;
        this->post = _post;
    }
    void print();
};


class Graph{

    unordered_map <int, vector<int>> GraphScheme;
    unordered_map<int, unordered_set<int>> AllEdgesGoingIntoKeyNode;
    unordered_map <int, vector<int>> GraphSchemeReverse;
    vector<int>postOrder;
    unordered_map<int, int> postOrderWithIndex;
    unordered_map<int, int> nodeHasPostorder;


public:
    unordered_set<int>* getAllChildren(int root){
        queue<int> Q;
        map<int, bool> alreadyVisited;
        unordered_set<int>*  allChildren = new unordered_set<int>;

        Q.push(root);

        while (!Q.empty())
        {
            int currnode = Q.front();
            allChildren->insert(currnode);
            Q.pop();
            if (this->AllEdgesGoingIntoKeyNode.count(currnode) != 0)
            {
                alreadyVisited[currnode] = true;
                for (unordered_set<int>::iterator t = AllEdgesGoingIntoKeyNode[currnode].begin(); t != AllEdgesGoingIntoKeyNode[currnode].end(); t++)
                {
                    allChildren->insert(*t);
                    alreadyVisited[*t] = true;
                }
            }
            else 
            {
                for (int iterator : this->GraphScheme[currnode])
                {
                    if (!alreadyVisited[iterator])
                    {
                        
                        Q.push(iterator);
                        alreadyVisited[iterator] = true;

                    }
                }
            }
            
        }
    return allChildren;
    };

    unordered_set<int>* getAllParents(int root){

        queue<int> Q;
        map<int, bool> alreadyVisited;
        unordered_set<int>* allChildren = new unordered_set<int>;

        Q.push(root);
        allChildren->insert(root);

        while (!Q.empty())
        {
            int currnode = Q.front();
            if (currnode != root)
            {
                allChildren->insert(currnode);
            }
            Q.pop();
            if (this->AllEdgesGoingIntoKeyNode.count(currnode) != 0)
            {
                allChildren->insert(AllEdgesGoingIntoKeyNode[currnode].begin(), AllEdgesGoingIntoKeyNode[currnode].end());
                alreadyVisited[currnode] = true;
            }
            for (int iterator : this->GraphSchemeReverse[currnode])
            {
                if(!alreadyVisited[iterator])
                {
                Q.push(iterator);
                alreadyVisited[iterator] = true;

                }
            }
        }
    return allChildren;
    };

    void graphPropagation(string fileName, bool createReverseScheme)
    {
        string filePath = createReverseScheme ?   "./src/interval_schemes/" + fileName + "_reverse_interval_scheme.txt" :
            "./src/interval_schemes/" + fileName + "_interval_scheme.txt";
        ofstream out(filePath);

        unordered_map<int, string> eval;
        #pragma omp parallel for
        for (int postOrderNode : postOrder)
        {
            unordered_set<int>* ParentNodes = new unordered_set<int>;
            ParentNodes = createReverseScheme ? getAllChildren(postOrderNode) : getAllParents(postOrderNode);

            vector<IntervalScheme> newCompressedIntervalScheme;
            boost::dynamic_bitset<> IntervalBitsetArray(postOrder.size() + 2);
            for (unordered_set<int>::iterator node = ParentNodes->begin(); node != ParentNodes->end(); node++)
            {
                IntervalBitsetArray[postOrderWithIndex[*node]] = 1;
            }
            delete ParentNodes;


            int pre = 0;
            int post = 0;
            for (boost::dynamic_bitset<>::size_type bit = 1; bit < IntervalBitsetArray.size() - 1; bit++) {
                if (IntervalBitsetArray[bit] == 1 && IntervalBitsetArray[(bit - 1)] == 0) {
                    pre = bit;
                }
                if (IntervalBitsetArray[bit] == 1 && IntervalBitsetArray[(bit + 1)] == 0) {
                    post = bit;
                    newCompressedIntervalScheme.push_back(IntervalScheme(pre, post));
                }
            }

            string interval_string = "";
            for (vector<IntervalScheme>::iterator interval = newCompressedIntervalScheme.begin(); interval != newCompressedIntervalScheme.end(); interval++)
            {
                interval_string.append("\t" + to_string(interval->pre) + "\t" + to_string(interval->post));
            }
            eval[postOrderNode] = interval_string;

        }
        auto* coutbuf = std::cout.rdbuf();
        cout.rdbuf(out.rdbuf());
        for (unordered_map<int, string>::iterator t = eval.begin(); t != eval.end(); t++) {
            cout << t->first << t->second << endl;;
        }
        cout.rdbuf(coutbuf);
    }

    bool readFiles(string fileName)
    {
        cout << "Reading Files...\n";
        ifstream reduced_graph_file;
        reduced_graph_file.open("./src/data/" + fileName + "_reduced_scheme.txt");
        string line;
        bool isRootNode = true;
        int rootNode, node;
        if (reduced_graph_file.is_open())
        {
            while (getline(reduced_graph_file, line))
            {
                stringstream   linestream(line);
                string  nodeString;
                while (getline(linestream, nodeString, '\t')) {
                    node = stoi(nodeString);
                    if (isRootNode) {
                        rootNode = node;
                        isRootNode = false;
                    }
                    if (rootNode != node) {
                        this->GraphScheme[rootNode].push_back(node);
                        this->GraphSchemeReverse[node].push_back(rootNode);
                    }
                }
                isRootNode = true;
            }
            reduced_graph_file.close();
            cout << "\tReduced Graph file read.\n";
        }
        else
        {
            cout << "Reduced Graph doesnt exist \n";
            return false;
        }

        ifstream postorder_file;
        postorder_file.open("./src/data/" + fileName + "_postorder.txt");
        if (postorder_file.is_open())
        {
            int counter = 1;
            while (getline(postorder_file, line))
            {
                stringstream   linestream(line);
                string  nodeString;
                while (getline(linestream, nodeString, ',')) {
                    node = stoi(nodeString);
                    this->nodeHasPostorder[counter] = node;
                    this->postOrderWithIndex[node] = counter;
                    this->postOrder.push_back(node);
                    counter++;
                }
            }
            postorder_file.close();
            cout << "\tPostorder file read.\n";
        }
        else
        {
            cout << "Postorder file  doesnt exist \n";
            return false;
        }
        postorder_file.close();
        cout << "All Files Read";
        return true;
    } 
};

int main(int argc, char **argv){

    try{
        int numberOfThreads = stoi(argv[1]);
        omp_set_num_threads(numberOfThreads);

    }
    catch(exception &err)
    {
        cout<<"Conversion failure: "<< argv[1] << " is not a number \n"; 
        return 0;
    }
    string fileName = argv[2];
    Graph* SocialGeoGraph = new Graph();


    bool filesRead = SocialGeoGraph->readFiles(fileName);
    if (!filesRead) return 0;
    SocialGeoGraph->graphPropagation(fileName, false);
    SocialGeoGraph->graphPropagation(fileName, true);


    return 0;
}