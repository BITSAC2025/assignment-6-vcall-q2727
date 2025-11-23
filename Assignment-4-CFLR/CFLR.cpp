/**
 * CFLR.cpp
 * @author kisslune 
 */

#include "A4Header.h"

using namespace SVF;
using namespace llvm;
using namespace std;

int main(int argc, char **argv)
{
    auto moduleNameVec =
            OptionBase::parseOptions(argc, argv, "Whole Program Points-to Analysis",
                                     "[options] <input-bitcode...>");

    LLVMModuleSet::buildSVFModule(moduleNameVec);

    SVFIRBuilder builder;
    auto pag = builder.build();
    pag->dump();

    CFLR solver;
    solver.buildGraph(pag);
    // TODO: complete this method
    solver.solve();
    solver.dumpResult();

    LLVMModuleSet::releaseLLVMModuleSet();
    return 0;
}


void CFLR::solve()
{
    std::unordered_set<unsigned> allNodes;
    auto &succMap = graph->getSuccessorMap();
    auto &predMap = graph->getPredecessorMap();

    for (const auto &nodeItr : succMap)
    {
        unsigned src = nodeItr.first;
        allNodes.insert(src);
        for (const auto &lblItr : nodeItr.second)
        {
            for (unsigned dst : lblItr.second)
            {
                allNodes.insert(dst);
                workList.push(CFLREdge(src, dst, lblItr.first));
            }
        }
    }

    auto addEdge = [this](unsigned src, unsigned dst, EdgeLabel label) {
        if (!graph->hasEdge(src, dst, label))
        {
            graph->addEdge(src, dst, label);
            workList.push(CFLREdge(src, dst, label));
        }
    };

    for (unsigned node : allNodes)
    {
        addEdge(node, node, VF);
        addEdge(node, node, VFBar);
        addEdge(node, node, VA);
    }

    auto forEachSucc = [&](unsigned node, EdgeLabel lbl, auto &&fn) {
        auto nodeIt = succMap.find(node);
        if (nodeIt == succMap.end())
            return;
        auto lblIt = nodeIt->second.find(lbl);
        if (lblIt == nodeIt->second.end())
            return;
        for (unsigned next : lblIt->second)
            fn(next);
    };

    auto forEachPred = [&](unsigned node, EdgeLabel lbl, auto &&fn) {
        auto nodeIt = predMap.find(node);
        if (nodeIt == predMap.end())
            return;
        auto lblIt = nodeIt->second.find(lbl);
        if (lblIt == nodeIt->second.end())
            return;
        for (unsigned next : lblIt->second)
            fn(next);
    };

    while (!workList.empty())
    {
        CFLREdge edge = workList.pop();
        unsigned x = edge.src;
        unsigned z = edge.dst;
        EdgeLabel label = edge.label;

        switch (label)
        {
        case VFBar:
            forEachSucc(z, AddrBar, [&](unsigned w) { addEdge(x, w, PT); });
            forEachSucc(z, VFBar, [&](unsigned w) { addEdge(x, w, VFBar); });
            forEachPred(x, VFBar, [&](unsigned y) { addEdge(y, z, VFBar); });
            forEachSucc(z, VA, [&](unsigned w) { addEdge(x, w, VA); });
            break;

        case AddrBar:
            forEachPred(x, VFBar, [&](unsigned y) { addEdge(y, z, PT); });
            break;

        case Addr:
            forEachSucc(z, VF, [&](unsigned w) { addEdge(x, w, PTBar); });
            break;

        case VF:
            forEachPred(x, Addr, [&](unsigned y) { addEdge(y, z, PTBar); });
            forEachSucc(z, VF, [&](unsigned w) { addEdge(x, w, VF); });
            forEachPred(x, VF, [&](unsigned y) { addEdge(y, z, VF); });
            forEachPred(x, VA, [&](unsigned y) { addEdge(y, z, VA); });
            break;

        case Copy:
            addEdge(x, z, VF);
            break;

        case SV:
            forEachSucc(z, Load, [&](unsigned w) { addEdge(x, w, VF); });
            break;

        case Load:
            forEachPred(x, SV, [&](unsigned y) { addEdge(y, z, VF); });
            forEachPred(x, PV, [&](unsigned y) { addEdge(y, z, VF); });
            forEachPred(x, LV, [&](unsigned y) { addEdge(y, z, VA); });
            break;

        case PV:
            forEachSucc(z, Load, [&](unsigned w) { addEdge(x, w, VF); });
            forEachSucc(z, StoreBar, [&](unsigned w) { addEdge(x, w, VFBar); });
            break;

        case Store:
            forEachSucc(z, VP, [&](unsigned w) { addEdge(x, w, VF); });
            forEachSucc(z, VA, [&](unsigned w) { addEdge(x, w, SV); });
            break;

        case VP:
            forEachPred(x, Store, [&](unsigned y) { addEdge(y, z, VF); });
            forEachPred(x, LoadBar, [&](unsigned y) { addEdge(y, z, VFBar); });
            break;

        case CopyBar:
            addEdge(x, z, VFBar);
            break;

        case LoadBar:
            forEachSucc(z, SVBar, [&](unsigned w) { addEdge(x, w, VFBar); });
            forEachSucc(z, VP, [&](unsigned w) { addEdge(x, w, VFBar); });
            forEachSucc(z, VA, [&](unsigned w) { addEdge(x, w, LV); });
            break;

        case SVBar:
            forEachPred(x, LoadBar, [&](unsigned y) { addEdge(y, z, VFBar); });
            break;

        case StoreBar:
            forEachPred(x, PV, [&](unsigned y) { addEdge(y, z, VFBar); });
            forEachPred(x, VA, [&](unsigned y) { addEdge(y, z, SVBar); });
            break;

        case LV:
            forEachSucc(z, Load, [&](unsigned w) { addEdge(x, w, VA); });
            break;

        case VA:
            forEachPred(x, VFBar, [&](unsigned y) { addEdge(y, z, VA); });
            forEachSucc(z, VF, [&](unsigned w) { addEdge(x, w, VA); });
            forEachPred(x, Store, [&](unsigned y) { addEdge(y, z, SV); });
            forEachSucc(z, StoreBar, [&](unsigned w) { addEdge(x, w, SVBar); });
            forEachPred(x, PTBar, [&](unsigned y) { addEdge(y, z, PV); });
            forEachSucc(z, PT, [&](unsigned w) { addEdge(x, w, VP); });
            forEachPred(x, LoadBar, [&](unsigned y) { addEdge(y, z, LV); });
            break;

        case PTBar:
            forEachSucc(z, VA, [&](unsigned w) { addEdge(x, w, PV); });
            break;

        case PT:
            forEachPred(x, VA, [&](unsigned y) { addEdge(y, z, VP); });
            break;

        default:
            break;
        }
    }
}
