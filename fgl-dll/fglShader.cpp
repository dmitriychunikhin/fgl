#include "fglShader.h"
#include "fglScene.h"

void fglShader::call(fglEvent& poEvent)
{
	if (poEvent.name == fglEVENT_build) {if (poEvent.nParams==2) build(_GetValue2i(poEvent.aParams[0]),_GetValue2c(poEvent.aParams[1])); _RetValue2l(isCompiled); return;}
	if (poEvent.name == fglEVENT_isCompiled) {_RetValue2l(isCompiled); return;}

	fglBind::call(poEvent);
}

void fglShaderProg::call(fglEvent& poEvent)
{
	fglBind::call(poEvent);
	
	if (poEvent.name == fglEVENT_attach) 
	{
		_RetValue2l(0);
		if (!poEvent.nParams) return;
		const fglREF<fglShader>& loShader = getBind<fglShader>(_GetValue2i(poEvent.aParams[0]));
		if (loShader.empty()) return;
		attach(loShader.get()); 
		_RetValue2l(1);
		return;
	}

	if (poEvent.name == fglEVENT_detach) 
	{
		_RetValue2l(0);
		if (!poEvent.nParams) return;
		const fglREF<fglShader>& loShader = getBind<fglShader>(_GetValue2i(poEvent.aParams[0]));
		if (loShader.empty()) return;
		detach(loShader.get()); 
		_RetValue2l(1);
		return;
	}

	if (poEvent.name == fglEVENT_build) {build(); _RetValue2l(isLinked); return;}
	if (poEvent.name == fglEVENT_isLinked) {_RetValue2l(isLinked); return;}
}


fglShader::~fglShader(void)
{
	release();
}


void fglShader::release(void)
{
	if (id && glDeleteObjectARB) glDeleteObjectARB(id);
	id = 0;
	nType = 0;
	isCompiled = false;
}

void fglShader::build(unsigned pnType, const fglString& pcPathOrData)
{
	release();
	
	if ( ! glCreateShaderObjectARB ) return;
	if (pcPathOrData.empty()) return;
		
	switch (pnType)
	{
		case VERTEX: id = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB); break;
		case FRAGMENT: id = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB); break;
		case GEOMETRY: id = glCreateShaderObjectARB(GL_GEOMETRY_SHADER_ARB); break;
	}
	if ( ! id ) return;
	nType = pnType;
	
	char* lcData = NULL;
	int lnLen = 0;

	std::fstream loFile;
	loFile.open(pcPathOrData, std::fstream::in | std::fstream::binary);
	if (loFile.is_open()) 
	{
		loFile.seekg (0, std::fstream::end);
		lnLen = int(loFile.tellg());
		loFile.seekg (0, std::fstream::beg);
		lcData = new char[lnLen+1];
		loFile.read(lcData, lnLen);
		lcData[lnLen]='\0';
	} else {
		lnLen = pcPathOrData.length();
		lcData = new char[lnLen];
		pcPathOrData.copy(lcData, lnLen);
	}
	
	glShaderSourceARB(id, 1, (const GLcharARB**)&lcData, &lnLen); 
	if (lcData) delete [] lcData; 
	
	glCompileShaderARB(id);
	
	glGetShaderiv(id, GL_COMPILE_STATUS, (GLint*)&isCompiled);
	if (!isCompiled)
	{
		int lnBLen = 0;	
		int lnSLen = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH , &lnBLen);
		if (lnBLen > 1)
		{
			fglREF<char> lcLog;
			lcLog.set_size(lnBLen);
			glGetInfoLogARB(id, lnBLen, &lnSLen, lcLog.map());
			throw fglException(lcLog.map());
		}
	}

	setSceneRenderFlag();
}

fglShaderProg::~fglShaderProg(void)
{
	release();
}

void fglShaderProg::release(void)
{
	if (id && glDeleteObjectARB) glDeleteObjectARB(id);
	id = 0;
	isLinked = false;
}


void fglShaderProg::attach(fglShader& poShader)
{
	if (id) glAttachObjectARB(id, poShader.getId());
}

void fglShaderProg::detach(fglShader& poShader)
{
	if (id) glDetachObjectARB(id, poShader.getId());
}

void fglShaderProg::build(void)
{
	release();
	if (glCreateProgramObjectARB) id = glCreateProgramObjectARB();
	if (!id) return;

	glBindAttribLocation(id, 0, "InVertex");
	glBindAttribLocation(id, 1, "InNormal");
	glBindAttribLocation(id, 2, "InTexCoord0");

	glLinkProgramARB(id);
	glGetProgramiv(id, GL_LINK_STATUS, (GLint*)&isLinked);
	if (!isLinked)
	{
		int lnBLen = 0;	
		int lnSLen = 0;
		glGetProgramiv(id, GL_INFO_LOG_LENGTH , &lnBLen);
		if (lnBLen > 1)
		{
			fglREF<char> lcLog;
			lcLog.set_size(lnBLen);
			glGetInfoLogARB(id, lnBLen, &lnSLen, lcLog.map());
			throw fglException(lcLog.map());
		}
	}
	setSceneRenderFlag();
}

void fglShaderProg::bind(void)
{
	if (isReady()) glUseProgramObjectARB(id);
}

void fglShaderProg::unbind(void)
{
	if (glUseProgramObjectARB) glUseProgramObjectARB(0);
}

