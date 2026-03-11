//
// Created by Paolo on 25/11/2020.
//


#include "BendersCut.h"

BendersCut::BendersCut(Problem* problemRef)
{
    problem = problemRef;
    zVal.resize(problem->nEdge, 0.0);
}

void BendersCut::callback()
{
    clock_t timeStartCallback = clock();
    try
    {
        if(where == GRB_CB_MIPSOL)
        {
            for(int e = 0; e < problem->nEdge; e++)
                zVal[e] = max(0.0, getSolution(problem->getZ(e)));

            shuffle(problem->orderedScenario.begin(), problem->orderedScenario.end(), problem->rng);
            for(int j = 0; j < problem->nScenario; j++)
            {
                bool cutFound;
                const int i = problem->orderedScenario[j];
                if(problem->algorithm == 2)
                    cutFound = problem->gurobiFeasibilityCut(i, zVal);
                else
                    cutFound = problem->buildIntFeasibilityCut(i, zVal);
                if(cutFound)
                {
                    addLazy(problem->expr >= 0);
                    problem->expr.clear();
                    problem->nAddedCut++;
                    problem->callbackTime += (double)(clock() - timeStartCallback) / CLOCKS_PER_SEC;
                    return;
                }
            }
        }

        if(where == GRB_CB_MIPNODE && getIntInfo(GRB_CB_MIPNODE_STATUS) == GRB_OPTIMAL)
        {
            if(getDoubleInfo(GRB_CB_MIPNODE_NODCNT) >= 1)
                return;

            for(int e = 0; e < problem->nEdge; e++)
                zVal[e] = max(0.0, getNodeRel(problem->getZ(e)));

            shuffle(problem->orderedScenario.begin(), problem->orderedScenario.end(), problem->rng);
            for(int j = 0; j < problem->nScenario; j++)
            {
                bool cutFound;
                const int i = problem->orderedScenario[j];
                if(problem->algorithm == 2)
                    cutFound = problem->gurobiFeasibilityCut(i, zVal);
                else
                    cutFound = problem->buildContFeasibilityCut(i, zVal);
                if(cutFound)
                {
                    addCut(problem->expr >= 0);
                    problem->expr.clear();
                    problem->nAddedCut++;
                    problem->callbackTime += (double)(clock() - timeStartCallback) / CLOCKS_PER_SEC;
                    return;
                }
            }
        }
    }
    catch(GRBException& e)
    {
        cout << "Error callback number : " << e.getErrorCode() << endl;
        cout << "Error callback: " << e.getMessage() << endl;
        exit(1);
    }catch(...)
    {
        cout << " Error during callback " << endl;
    }

    problem->callbackTime += (double)(clock() - timeStartCallback) / CLOCKS_PER_SEC;
}
