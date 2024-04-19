#include "fglVBO.h"
#include "fglScene.h"



fglVBOPool::~fglVBOPool(void)
{
	for (unsigned i=0; i != aBlocks.size(); i++)
	{
		//aBlocks[i].unlock();
		//aBlocks[i].releasedata();
		aBlocks[i].release();
	}
	aBlocks.clear();
}


void fglVBOPool::setData(fglVBO& poVBO, unsigned pnSize, const void* pData, unsigned pnUsage) 
{
	if ( ! pData || ! pnSize ) return;

	if ( poVBO.getId() ) 
	{
		if (poVBO.oSubBlock->size != pnSize || poVBO.oSubBlock->usage != pnUsage )
		{
			poVBO.clear();
		}
	}

	if ( ! poVBO.getId() ) 
	{
		for (unsigned i=0; i != aBlocks.size(); i++)
		{
			if ( aBlocks[i]->usage == pnUsage ) aBlocks[i]->addSubBlock(poVBO, pnSize);
			if ( poVBO.getId() ) break;
		}
	}

	if ( ! poVBO.getId() ) 
	{
		fglREF<fglVBOPoolBlock> loNewBlock;
		loNewBlock.create(); 
		loNewBlock->init(poVBO.target, pnUsage, pnSize); 
		aBlocks.push_back(loNewBlock); 
		loNewBlock->addSubBlock(poVBO, pnSize);
	}

	if ( poVBO.getId() )
	{
		if (poVBO.bind() ) glBufferSubDataARB(poVBO.target, poVBO.getOffset(), pnSize, pData); 
		fglVBO::unbind(poVBO.target);
	}
}

void fglVBOPool::freeData(fglVBO& poVBO) 
{
	if ( ! poVBO.getId() ) return;
	//poVBO.oSubBlock.unlock();
	//poVBO.oSubBlock.releasedata();
	poVBO.oSubBlock.release();
	reorganizeBlocks();
}

void fglVBOPool::reorganizeBlocks(void)
{
	if ( ! fglScene::oCurrentScene ) return; //scene is releasing
	
	unsigned lnCopySize=0, lnCopyOffset=0, lnHoles=0;
	void* lpCopy=NULL;
	
	for (unsigned i=0; i!=aBlocks.size(); i++)
	{
		if (aBlocks[i].empty()) continue;
		if ( ! aBlocks[i]->id ) continue;
		lnHoles = 0;
		aBlocks[i]->freeSize = aBlocks[i]->size;
		aBlocks[i]->freeOffset = 0;
		for (unsigned j=0; j!=aBlocks[i]->aSubBlocks.size(); j++)
		{
			if (aBlocks[i]->aSubBlocks[j].empty())
			{
				lnHoles++;
				continue;
			}

			if (lnHoles)
			{
				lnCopyOffset = aBlocks[i]->aSubBlocks[j]->offset;
				lnCopySize = aBlocks[i]->size - lnCopyOffset;
				lpCopy = new char[lnCopySize];
				memset(lpCopy, 0, lnCopySize);

				glBindBufferARB(aBlocks[i]->target, aBlocks[i]->id);
				glGetBufferSubDataARB(aBlocks[i]->target, lnCopyOffset, lnCopySize, lpCopy);
				glBufferSubDataARB(aBlocks[i]->target, aBlocks[i]->freeOffset, lnCopySize, lpCopy); 
				fglVBO::unbind(aBlocks[i]->target);

				delete [] lpCopy;
				lpCopy = NULL;
				
				for (unsigned k=j; k!=aBlocks[i]->aSubBlocks.size(); k++)
				{
					if (aBlocks[i]->aSubBlocks[k].exist())
					{
						aBlocks[i]->aSubBlocks[k]->offset -= lnCopyOffset - aBlocks[i]->freeOffset;
					}
					aBlocks[i]->aSubBlocks[k-lnHoles] = aBlocks[i]->aSubBlocks[k];
					aBlocks[i]->aSubBlocks[k].clear();
				}
				j -= lnHoles;
				lnHoles = 0;
			}
			aBlocks[i]->freeSize -= aBlocks[i]->aSubBlocks[j]->size;
			aBlocks[i]->freeOffset += aBlocks[i]->aSubBlocks[j]->size;

		}
		
		while (lnHoles--) aBlocks[i]->aSubBlocks.pop_back();
		
	}
}


/////////////////////////////////////////////////////////////////

fglVBOPoolBlock::~fglVBOPoolBlock(void) 
{
	if (id) glDeleteBuffersARB(1, &id); 
	id = 0;

	for (unsigned i=0; i != aSubBlocks.size(); i++)
	{
		//aSubBlocks[i].unlock();
		//aSubBlocks[i].releasedata();
		aSubBlocks[i].release();
	}
	aSubBlocks.clear();
}

void fglVBOPoolBlock::init(unsigned pnTarget, unsigned pnUsage, unsigned pnSize)
{
	if ( pnTarget != fglVBO_VERTICES && pnTarget != fglVBO_INDICES ) return;
	if ( pnUsage != fglVBO_STATIC && pnUsage != fglVBO_DYNAMIC ) return;
	if ( pnSize == 0 ) return;
	if ( id ) return;

	unsigned lnMinSize = 8388608;
	pnSize = std::max<unsigned>(pnSize, lnMinSize);

	target = pnTarget;
	usage = pnUsage;
	size = pnSize;
	freeSize = pnSize;
	freeOffset = 0;

	if ( ! glGenBuffersARB ) return;
	glGenBuffersARB(1, &id);
	if (glIsBufferARB(id) == GL_FALSE) { id = 0; return; }

	glBindBufferARB(target, id);
	glBufferDataARB(target, size, NULL, usage); 
	fglVBO::unbind(target);

}


void fglVBOPoolBlock::addSubBlock(fglVBO& poVBO, unsigned pnSize)
{
	if ( ! id ) return;
	if ( poVBO.getId() ) return;
	if ( freeSize < pnSize || target != poVBO.target || pnSize==0 ) return;
	
	poVBO.oSubBlock.create();
	poVBO.oSubBlock->init(id, target, usage, freeOffset, pnSize);
	freeOffset += pnSize;
	freeSize -= pnSize;
	aSubBlocks.push_back(poVBO.oSubBlock);
}

///////////////////////////////////////////////////////////////////////////

void fglVBOPoolSubBlock::init(fglVBO_ID pnId, unsigned pnTarget, unsigned pnUsage, unsigned pnOffset, unsigned pnSize)
{
	id = pnId;
	target = pnTarget;
	usage = pnUsage;
	offset = pnOffset;
	size = pnSize;
}

bool fglVBOPoolSubBlock::bind(void) const 
{ 
	if ( pMap ) return false;
	if ( id ) glBindBufferARB(target, id);
	return id!=0;
}

bool fglVBOPoolSubBlock::getData(void* pData) const
{
	if ( ! bind() ) return false;
	glGetBufferSubDataARB(target, offset, size, pData);
	return true;
}

void* fglVBOPoolSubBlock::map(unsigned pnAccess)
{
	if ( ! bind() ) return NULL;
	if (pMap) return pMap;
	pMap = glMapBufferARB(GL_ARRAY_BUFFER, pnAccess);
	return pMap;
}

void fglVBOPoolSubBlock::unmap(void)
{
	pMap = NULL; 
	glUnmapBufferARB(target);
}



///////////////////////////////////////////////////////////////////////////

fglVBO::~fglVBO(void)
{ 
	clear();
}

void fglVBO::setScene(const fglWeakREF<fglScene>& poScene)
{ 
	if ( oScene.empty() ) oScene = poScene;
}

fglVBO_ID fglVBO::getId(void) const
{ 
	if (oSubBlock.exist()) return oSubBlock->id;
	return 0; 
}

unsigned fglVBO::getOffset(void) const
{ 
	if (oSubBlock.exist()) return oSubBlock->offset;
	return 0; 
}

unsigned fglVBO::getSize(void) const
{ 
	if (oSubBlock.exist()) return oSubBlock->size;
	return 0; 
}

unsigned fglVBO::getUsage(void) const
{ 
	if (oSubBlock.exist()) return oSubBlock->usage;
	return 0; 
}

bool fglVBO::bind(void) const
{ 
	if ( oSubBlock.exist() ) return oSubBlock->bind();
	return false;
}
void fglVBO::unbind(unsigned pnTarget)
{ 
	if (glBindBufferARB) glBindBufferARB(pnTarget, 0); 
}

void fglVBO::setData(unsigned pnSize, const void* pData, unsigned pnUsage) 
{
	fglScene* loScene;
	if (loScene = oScene.lock().map())
	{
		loScene->setCurrentContext();
		loScene->oVBOPool.setData(*this, pnSize, pData, pnUsage);
	}
}
	
bool fglVBO::getData(void* pData) const 
{
	if ( oSubBlock.exist() ) return oSubBlock->getData(pData); 
	return false;
}
void* fglVBO::map(unsigned pnAccess) 
{ 
	if ( oSubBlock.exist() ) return oSubBlock->map(pnAccess); 
	return NULL; 
}

void fglVBO::unmap(void) 
{ 
	if ( oSubBlock.exist() ) oSubBlock->unmap(); 
}

void fglVBO::clear(void) 
{ 
	if ( oScene.exist() ) oScene.lock()->oVBOPool.freeData(*this); 
}


///////////////////////////////////////////////////////////////////////////
fglVBO_Array::fglVBO_Array(void)
{
	target = fglVBO_VERTICES;
}

fglVBO_Element_Array::fglVBO_Element_Array(void)
{
	target = fglVBO_INDICES;
}

