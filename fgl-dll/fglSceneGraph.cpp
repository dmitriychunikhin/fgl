#include "fglSceneGraph.h"
#include "fglScene.h"

//fglNode is defined in fglSceneGraphNode.cpp
//fglVisual is defined in fglSceneGraphVisual.cpp


void fglWalkNodes::walkNodes(fglNode* poNode, fglWalkNodes* poHandler)
{
	if (!poNode) return;
	if (!poHandler) return;

	fglNode* loNode(poNode);
	fglVisual* loVisual(NULL);
	unsigned liChild(0), lnSaveStack(poHandler->nStack), lnVisuals(0);
	bool llOnNode, llNestedCall(false);

	fglTRANSFORM loTrans = fglScene::identTrans;
	if ( poHandler->nStack )
	{
		llNestedCall=true;
		loTrans = poHandler->currentView;
	}
	
	poHandler->aStack[poHandler->nStack++] = liChild; //add root to stack
	while (loNode)
	{
		llOnNode=true;
		if ( liChild==0 ) 
		{
			if ( ! llNestedCall )
			{
				poHandler->currentView = loNode->trg;
				poHandler->currentViewMatrix = loNode->trgMatrix;
				poHandler->currentViewMatrix.setCurrentView();
			}
			else 
			{
				poHandler->currentView = loTrans * loNode->trg;
				poHandler->currentView.toMatrix(poHandler->currentViewMatrix);
				poHandler->currentViewMatrix.setCurrentView();
			}

			if (poHandler->onNode) llOnNode = poHandler->onNode(poHandler, loNode);
			lnVisuals = loNode->aVisuals->size();
			if (poHandler->onVisual && llOnNode)
			{
				while (lnVisuals--)
				{ 
					if ( loVisual = loNode->aVisuals->get(lnVisuals).map() ) poHandler->onVisual(poHandler, loNode, loVisual); 
				}
			}
		}

		if ( liChild >= loNode->aChilds->size() || llOnNode == false )
		{
			if (loNode==poNode) break;
			loNode = loNode->getParent().map();
			liChild = poHandler->aStack[--poHandler->nStack]+1;
			continue;
		}
		poHandler->aStack[poHandler->nStack++] = liChild;
		loNode = loNode->aChilds->get(liChild).map();
		liChild=0;
	}
	poHandler->nStack = lnSaveStack;
	
	if (llNestedCall)
	{
		poHandler->currentView = loTrans;
		poHandler->currentView.toMatrix(poHandler->currentViewMatrix);
	}

}
/////////////////////////////////////////////////////////////////////////////////////////////
