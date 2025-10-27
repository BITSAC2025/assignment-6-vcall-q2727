/**
 * A4Lib.cpp
 * @author kisslune 
 */

#include "A4Header.h"

CFLRGraph::CFLRGraph(SVF::SVFIR *pag)
{
    for (SVF::PAGEdge *edge : pag->getSVFStmtSet(SVF::PAGEdge::Addr))
    {
        addEdge(edge->getSrcID(), edge->getDstID(), Addr);
        addEdge(edge->getDstID(), edge->getSrcID(), AddrBar);
    }

    for (SVF::PAGEdge *edge : pag->getSVFStmtSet(SVF::PAGEdge::Copy))
    {
        addEdge(edge->getSrcID(), edge->getDstID(), Copy);
        addEdge(edge->getDstID(), edge->getSrcID(), CopyBar);
    }

    for (SVF::PAGEdge *edge : pag->getSVFStmtSet(SVF::PAGEdge::Phi))
    {
        const SVF::PhiStmt *phi = SVF::SVFUtil::cast<SVF::PhiStmt>(edge);
        for (const auto opVar : phi->getOpndVars())
        {
            addEdge(opVar->getId(), phi->getResID(), Copy);
            addEdge(phi->getResID(), opVar->getId(), CopyBar);
        }
    }

    for (SVF::PAGEdge *edge : pag->getSVFStmtSet(SVF::PAGEdge::Select))
    {
        const SVF::SelectStmt *sel = SVF::SVFUtil::cast<SVF::SelectStmt>(edge);
        for (const auto opVar : sel->getOpndVars())
        {
            addEdge(opVar->getId(), sel->getResID(), Copy);
            addEdge(sel->getResID(), opVar->getId(), CopyBar);
        }
    }

    for (SVF::PAGEdge *edge : pag->getSVFStmtSet(SVF::PAGEdge::Call))
    {
        addEdge(edge->getSrcID(), edge->getDstID(), Copy);
        addEdge(edge->getDstID(), edge->getSrcID(), CopyBar);
    }

    for (SVF::PAGEdge *edge : pag->getSVFStmtSet(SVF::PAGEdge::Ret))
    {
        addEdge(edge->getSrcID(), edge->getDstID(), Copy);
        addEdge(edge->getDstID(), edge->getSrcID(), CopyBar);
    }

    for (SVF::PAGEdge *edge : pag->getSVFStmtSet(SVF::PAGEdge::ThreadFork))
    {
        addEdge(edge->getSrcID(), edge->getDstID(), Copy);
        addEdge(edge->getDstID(), edge->getSrcID(), CopyBar);
    }

    for (SVF::PAGEdge *edge : pag->getSVFStmtSet(SVF::PAGEdge::ThreadJoin))
    {
        addEdge(edge->getSrcID(), edge->getDstID(), Copy);
        addEdge(edge->getDstID(), edge->getSrcID(), CopyBar);
    }

    // opt load and store
    for (SVF::PAGEdge *edge : pag->getSVFStmtSet(SVF::PAGEdge::Store))
    {
        addEdge(edge->getSrcID(), edge->getDstID(), Store);
        addEdge(edge->getDstID(), edge->getSrcID(), StoreBar);
    }
    for (SVF::PAGEdge *edge : pag->getSVFStmtSet(SVF::PAGEdge::Load))
    {
        addEdge(edge->getSrcID(), edge->getDstID(), Load);
        addEdge(edge->getDstID(), edge->getSrcID(), LoadBar);
    }
}


bool CFLRGraph::hasEdge(unsigned int src, unsigned int dst, EdgeLabel EdgeLabel)
{
    return succMap[src][EdgeLabel].count(dst);
}


void CFLRGraph::addEdge(unsigned int src, unsigned int dst, EdgeLabel EdgeLabel)
{
    succMap[src][EdgeLabel].insert(dst);
    predMap[dst][EdgeLabel].insert(src);
}


void CFLR::buildGraph(SVF::PAG *pag)
{
    if (!graph)
        graph = new CFLRGraph(pag);
}


void CFLR::dumpResult()
{
    std::string fname = SVF::PAG::getPAG()->getModuleIdentifier() + ".res.txt";
    std::ofstream outFile(fname, std::ios::out);
    if (!outFile)
    {
        std::cout << "error opening " + fname + "!!\n";
        return;
    }

    // Collect S-edges
    std::map<unsigned, std::set<unsigned >> edgeSet;  // ordered edge set
    for (auto &nodeItr : graph->getSuccessorMap())
    {
        unsigned src = nodeItr.first;
        for (auto &lblItr : nodeItr.second)
        {
            if (lblItr.first == PT)
                for (auto dst : lblItr.second)
                    edgeSet[src].insert(dst);
        }
    }

    // Write S-edges
    for (auto &srcItr : edgeSet)
    {
        for (auto dst : srcItr.second)
        {
            outFile << srcItr.first << '\t' << "points to" << '\t' << dst << std::endl;
        }
    }
}