/**
 * Andersen.cpp
 * @author kisslune
 */

#include "A6Header.h"

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
    auto cg = pag->getCallGraph();

    // TODO: complete the following two methods
    andersen.runPointerAnalysis();
    andersen.updateCallGraph(cg);

    cg->dump();
    SVF::LLVMModuleSet::releaseLLVMModuleSet();
	return 0;
}

void Andersen::runPointerAnalysis()
{
    // TODO: complete this method. Point-to set and worklist are defined in A5Header.h
    //  The implementation of constraint graph is provided in the SVF library
    WorkList<unsigned> workList;

    auto addCopyIfNeeded = [&](unsigned src, unsigned dst) -> bool
    {
        auto *srcNode = consg->getConstraintNode(src);
        if (srcNode == nullptr)
            return false;

        for (auto edge : srcNode->getCopyOutEdges())
        {
            if (edge->getDstID() == dst)
                return false;
        }

        consg->addCopyCGEdge(src, dst);
        return true;
    };

    for (auto iter = consg->begin(); iter != consg->end(); ++iter)
    {
        auto nodeId = iter->first;
        SVF::ConstraintNode *node = iter->second;

        for (auto edge : node->getAddrInEdges())
        {
            auto *addrEdge = SVF::SVFUtil::dyn_cast<SVF::AddrCGEdge>(edge);
            const auto srcId = addrEdge->getSrcID();
            auto &ptSet = pts[nodeId];

            if (ptSet.insert(srcId).second)
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

        for (auto obj : pPts)
        {
            for (auto edge : pNode->getStoreInEdges())
            {
                auto *storeEdge = SVF::SVFUtil::dyn_cast<SVF::StoreCGEdge>(edge);
                if (addCopyIfNeeded(storeEdge->getSrcID(), obj))
                    workList.push(storeEdge->getSrcID());
            }

            for (auto edge : pNode->getLoadOutEdges())
            {
                auto *loadEdge = SVF::SVFUtil::dyn_cast<SVF::LoadCGEdge>(edge);
                if (addCopyIfNeeded(obj, loadEdge->getDstID()))
                    workList.push(obj);
            }
        }

        for (auto edge : pNode->getCopyOutEdges())
        {
            auto *copyEdge = SVF::SVFUtil::dyn_cast<SVF::CopyCGEdge>(edge);
            auto dstId = copyEdge->getDstID();
            auto &dstPts = pts[dstId];
            bool changed = false;

            for (auto val : pPts)
                changed |= dstPts.insert(val).second;

            if (changed)
                workList.push(dstId);
        }

        for (auto edge : pNode->getGepOutEdges())
        {
            auto *gepEdge = SVF::SVFUtil::dyn_cast<SVF::GepCGEdge>(edge);
            auto dstId = gepEdge->getDstID();
            auto &dstPts = pts[dstId];
            bool changed = false;

            for (auto val : pPts)
            {
                auto gepObj = consg->getGepObjVar(val, gepEdge);
                changed |= dstPts.insert(gepObj).second;
            }

            if (changed)
                workList.push(dstId);
        }
    }
}

void Andersen::updateCallGraph(SVF::CallGraph *cg)
{
    for (const auto &siteInfo : consg->getIndirectCallsites())
    {
        auto *callNode = siteInfo.first;
        const auto calleePtrId = siteInfo.second;
        auto *caller = callNode->getCaller();
        const auto &candidateTargets = pts[calleePtrId];

        for (auto objId : candidateTargets)
        {
            if (!consg->isFunction(objId))
                continue;

            auto *callee = consg->getFunction(objId);
            cg->addIndirectCallGraphEdge(callNode, caller, callee);
        }
    }
}
