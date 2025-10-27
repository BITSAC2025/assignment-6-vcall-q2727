/**
 * cfga_lib.cpp
 * @author kisslune 
 */

#include "CFGA.h"
#include <fstream>

using namespace SVF;
using namespace llvm;
using namespace std;


CFGAnalysis::CFGAnalysis(SVF::ICFG *icfg)
{
    for (auto &it : *icfg)
    {
        auto node = it.second;
        if (auto fEntry = dyn_cast<FunEntryICFGNode>(node))
        {
            if (fEntry->getFun()->getName() == "main")
                sources.insert(it.first);
        }
        if (auto fExit = dyn_cast<FunExitICFGNode>(node))
        {
            if (fExit->getFun()->getName() == "main")
                sinks.insert(it.first);
        }
    }
}


void CFGAnalysis::recordPath(const std::vector<unsigned int>& path)
{
    if (path.empty())
        return;
    reachablePaths.insert(path);
}


void CFGAnalysis::dumpPaths()
{
    std::string fname = PAG::getPAG()->getModuleIdentifier() + ".res.txt";
    std::ofstream outFile(fname, std::ios::out);
    if (!outFile)
    {
        std::cout << "error opening " + fname + "!!\n";
        return;
    }

    for (auto &path : reachablePaths)
    {
        for (auto node : path)
            outFile << node << ", ";
        outFile << endl;
    }

    outFile.close();
}