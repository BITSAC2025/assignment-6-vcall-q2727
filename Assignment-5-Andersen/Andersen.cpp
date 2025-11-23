/**
 * Andersen.cpp
 * @author kisslune
 */

#include "A5Header.h"

using namespace llvm;
using namespace std;

int main(int argc, char** argv)
{
    auto moduleNameVec =
            OptionBase::parseOptions(argc, argv, "Whole Program Points-to Analysis",
                                     "[options] <input-bitcode...>");

    SVF::LLVMModuleSet::buildSVFModule(moduleNameVec);

    SVF::SVFIRBuilder builder;
    auto pag = builder.build();
    auto consg = new SVF::ConstraintGraph(pag);
    consg->dump();

    Andersen andersen(consg);

    // TODO: complete the following method
    andersen.runPointerAnalysis();

    andersen.dumpResult();
    SVF::LLVMModuleSet::releaseLLVMModuleSet();
	return 0;
}


void Andersen::runPointerAnalysis()
{
    // TODO: complete this method. Point-to set and worklist are defined in A5Header.h
    //  The implementation of constraint graph is provided in the SVF library
    WorkList<unsigned> workList;

    auto scheduleCopyEdge = [&](unsigned src, unsigned dst)
    {
        SVF::ConstraintNode *dstNode = consg->getConstraintNode(dst);
        bool alreadyExists = false;

        if (dstNode != nullptr)
        {
            for (auto edge : dstNode->getCopyInEdges())
            {
                if (edge->getSrcID() == src)
                {
                    alreadyExists = true;
                    break;
                }
            }
        }

        if (!alreadyExists)
        {
            consg->addCopyCGEdge(src, dst);
            workList.push(src);
        }
    };

    for (auto nodeIt = consg->begin(); nodeIt != consg->end(); ++nodeIt)
    {
        const auto nodeId = nodeIt->first;
        SVF::ConstraintNode *node = nodeIt->second;

        for (auto edge : node->getAddrInEdges())
        {
            auto *addrEdge = SVF::SVFUtil::dyn_cast<SVF::AddrCGEdge>(edge);
            const auto srcId = addrEdge->getSrcID();
            auto &pointSet = pts[nodeId];

            if (pointSet.insert(srcId).second)
            {
                workList.push(nodeId);
            }
        }
    }

    while (!workList.empty())
    {
        const auto p = workList.pop();
        SVF::ConstraintNode *pNode = consg->getConstraintNode(p);
        auto &pPts = pts[p];

        for (auto object : pPts)
        {
            for (auto edge : pNode->getStoreInEdges())
            {
                auto *storeEdge = SVF::SVFUtil::dyn_cast<SVF::StoreCGEdge>(edge);
                scheduleCopyEdge(storeEdge->getSrcID(), object);
            }

            for (auto edge : pNode->getLoadOutEdges())
            {
                auto *loadEdge = SVF::SVFUtil::dyn_cast<SVF::LoadCGEdge>(edge);
                scheduleCopyEdge(object, loadEdge->getDstID());
            }
        }

        for (auto edge : pNode->getCopyOutEdges())
        {
            auto *copyEdge = SVF::SVFUtil::dyn_cast<SVF::CopyCGEdge>(edge);
            const auto dstId = copyEdge->getDstID();
            auto &dstPts = pts[dstId];
            bool changed = false;

            for (auto object : pPts)
            {
                changed |= dstPts.insert(object).second;
            }

            if (changed)
            {
                workList.push(dstId);
            }
        }

        for (auto edge : pNode->getGepOutEdges())
        {
            auto *gepEdge = SVF::SVFUtil::dyn_cast<SVF::GepCGEdge>(edge);
            const auto dstId = gepEdge->getDstID();
            auto &dstPts = pts[dstId];
            bool changed = false;

            for (auto object : pPts)
            {
                const auto fieldObj = consg->getGepObjVar(object, gepEdge);
                changed |= dstPts.insert(fieldObj).second;
            }

            if (changed)
            {
                workList.push(dstId);
            }
        }
    }
}
