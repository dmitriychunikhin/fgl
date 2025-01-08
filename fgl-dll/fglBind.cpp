#include "fglBind.h"
#include "fglScene.h"
#include "fglCollider.h"

fglBindFactory::FGLABINDS fglBindFactory::aBinds;

////////////////////////////////////////////////////////////////////////////////
fglREF<fglBind> fglBindFactory::createByClassName(fglBindFactory::PersistentType tPersistent, fglString pcClassname)
{
	fglREF<fglBind> loFglObj;

	if (pcClassname.compare("scene") == 0) loFglObj = create<fglScene>(tPersistent);
	else if (pcClassname.compare("viewport") == 0) loFglObj = create<fglViewport>(tPersistent);
	else if (pcClassname.compare("node") == 0) loFglObj = create<fglNode>(tPersistent);
	else if (pcClassname.compare("cam") == 0) loFglObj = create<fglCam>(tPersistent);
	else if (pcClassname.compare("light") == 0) loFglObj = create<fglLight>(tPersistent);
	else if (pcClassname.compare("material") == 0) loFglObj = create<fglMaterial>(tPersistent);
	else if (pcClassname.compare("model") == 0) loFglObj = create<fglModel>(tPersistent);
	else if (pcClassname.compare("mesh") == 0) loFglObj = create<fglMesh>(tPersistent);
	else if (pcClassname.compare("joint") == 0) loFglObj = create<fglJoint>(tPersistent);
	else if (pcClassname.compare("text") == 0) loFglObj = create<fglText>(tPersistent);
	else if (pcClassname.compare("pickinfo") == 0) loFglObj = create<fglPickInfo>(tPersistent);
	else if (pcClassname.compare("collider") == 0) loFglObj = create<fglCollider>(tPersistent);
	else if (pcClassname.compare("collideinfo") == 0) loFglObj = create<fglCollideInfo>(tPersistent);
	else throw fglException("Invalid classname " + pcClassname);

	if (loFglObj.empty()) throw fglException("Object was not created");

	return loFglObj;
}

void fglBindFactory::releaseBind(HFGL phFGL)
{
	releaseBind(getBind<fglBind>(phFGL));
}

void fglBindFactory::releaseBind(const fglREF<fglBind>& poBind)
{
	if (poBind.empty()) return;

	poBind->destroy();

	const fglREF<fglScene>& loScene = poBind->getScene().lock();
	
	if (loScene.exist())
	{
		loScene->setRenderFlag(true);
		loScene->setCurrentContext();

		if (poBind == loScene)
		{
			auto onItem = [&loScene](BINDREF& poItem) { if (poItem.oRef.exist() && poItem.oRef != loScene && poItem.oRef->getScene().lock() == loScene) { poItem.oRef.release(); } };
			aBinds.walkItems(onItem);
		}
	}

	BINDREF* loBindRef = const_cast<BINDREF*>(aBinds.get(poBind->hFGL));
	if (loBindRef) loBindRef->oRef.release();
}

void fglBindFactory::unload(void)
{
	auto onItem = [](BINDREF& poItem) { poItem.oRef.release(); };
	aBinds.walkItems(onItem);
}


void fglBindFactory::freeBind(HFGL phFGL)
{
	aBinds.free(phFGL);
}

////////////////////////////////////////////////////////////////////////////////
void fglBind::call(fglEvent& poEvent) 
{
	if (poEvent.name == fglEVENT_hScene) { _RetValue2i(0); if (oScene.exist()) _RetValue2i(oScene.lock()->getHFGL()); return; }
	if (poEvent.name == fglEVENT_name) { if (poEvent.nParams) name = _GetValue2c(poEvent.aParams[0]); _RetValue2c(name); return; }
	if (poEvent.name == fglEVENT_fgl_class) { _RetValue2c(fgl_class()); return; }

	if (poEvent.name == fglEVENT_copy) 
	{
		_RetValue2l(0);
		if (poEvent.nParams!=2) return;
		const fglREF<fglBind>& loSrc = getBind<fglBind>(_GetValue2i(poEvent.aParams[0]));
		if (loSrc.empty()) return;
		if ( copy( loSrc.map(), _GetValue2i(poEvent.aParams[1]) ) ) _RetValue2l(1);
		return;
	}

	if (poEvent.name == fglEVENT_duplicateMode) { if (poEvent.nParams) duplicateMode = _GetValue2i(poEvent.aParams[0]); _RetValue2i(duplicateMode); return; }

	if (poEvent.name == fglEVENT_duplicate) 
	{
		_RetValue2i(0);
		if (poEvent.nParams!=3) return;
		const fglREF<fglBind>& loSrc = getBind<fglBind>(_GetValue2i(poEvent.aParams[0]));
		if (loSrc.empty()) return;

		fglREF<fglBind> loDuplicate = fglBindFactory::duplicatePersistent<fglBind>( loSrc, _GetValue2i(poEvent.aParams[1]), _GetValue2i(poEvent.aParams[2]) );
		if (loDuplicate.exist()) _RetValue2i(loDuplicate->getHFGL());
		return;
	}

	if (poEvent.name == fglEVENT_setProp) 
	{
		if (poEvent.nParams!=2) return;
		fglString lcName = _GetValue2c(poEvent.aParams[0]);
		if (lcName.empty()) return;

		fglExtVal loVal;
		if (poEvent.aParams[1].ev_type == 'I') loVal.setVal(_GetValue2i(poEvent.aParams[1]));
		else if (poEvent.aParams[1].ev_type == 'N') loVal.setVal(_GetValue2f(poEvent.aParams[1]));
		else if (poEvent.aParams[1].ev_type == 'C') loVal.setVal(_GetValue2c(poEvent.aParams[1]));
		else if (poEvent.aParams[1].ev_type == 'L') loVal.setVal(_GetValue2l(poEvent.aParams[1]));
		
		getExtProps().set(lcName, loVal);
		return;
	}

	if (poEvent.name == fglEVENT_getProp) 
	{
		_RetValue2null();
		if (!poEvent.nParams) return;
		fglString lcName = _GetValue2c(poEvent.aParams[0]);
		if (lcName.empty()) return;
		
		const fglExtVal* loVal = getExtProps().get(lcName);
		
		if (!loVal) _RetValue2null();
		else if (loVal->getType() == 'I') _RetValue2i(loVal->getValI());
		else if (loVal->getType() == 'F') _RetValue2f(loVal->getValF());
		else if (loVal->getType() == 'C') _RetValue2c(loVal->getValC());
		else if (loVal->getType() == 'L') _RetValue2l(loVal->getValL());

		return;
	}

}


fglBind::~fglBind(void) 
{ 
	fglBindFactory::freeBind(hFGL);
	setExtProps(NULL);
}

void fglBind::destroy(void) {

}

fglWeakREF<fglScene> fglBind::getScene(void) const 
{ 
	return oScene; 
}
bool fglBind::setScene(const fglWeakREF<fglScene>& poScene) 
{ 
	if ( oScene.empty() ) oScene = poScene; 
	return (oScene.lock() == poScene.lock()); 
}
	
fglExtProps& fglBind::getExtProps(void) 
{ 
	if (aExtProps==NULL) aExtProps = new fglExtProps(); 
	return *aExtProps; 
}

void fglBind::setExtProps(const fglExtProps* poExtProps) 
{
	if (poExtProps==NULL) { if (aExtProps) delete aExtProps; aExtProps=NULL; return; } 
	if (aExtProps==NULL) aExtProps = new fglExtProps(); 
	*aExtProps = *poExtProps;
}


bool fglBind::copy(const fglBind* poSrc, fglBITFLAG pnShare)
{
	if (!poSrc) return false;
	if (poSrc==this) return false;
	if ( ! setScene(poSrc->getScene()) ) return false;

	name = poSrc->name;
	setExtProps(poSrc->aExtProps);
	duplicateMode = poSrc->duplicateMode;
	setSceneRenderFlag();
	return true;
}


void fglBind::setSceneRenderFlag(void)
{
	fglScene* loScene;
	if ( loScene = getScene().lock().map() ) loScene->setRenderFlag(true);
}
