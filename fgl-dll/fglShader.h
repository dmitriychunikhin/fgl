#ifndef _FGLSHADER_H_
#define _FGLSHADER_H_

#include "common.h"
#include "fgl3DGen.h"
#include "fglBind.h"

class fglShader : public fglBind
{
	friend class fglBindFactory;

protected:
	fglShader(void) = default;

private:
	unsigned int id{ 0 };
	unsigned int nType{ 0 };
	bool isCompiled{ false };

public:
	enum {VERTEX = 1, FRAGMENT, GEOMETRY};
	virtual ~fglShader(void);
	
	virtual fglString fgl_class(void) const { return "shader"; }
	void create(unsigned int pnType);
	void release(void);
	bool isReady(void) { return (id!=0 && isCompiled); }

	unsigned int getId(void) {return id;}

	void build(unsigned pnType, const fglString& pcPathOrData);

	virtual void call(fglEvent& poEvent);
};

class fglShaderProg : public fglBind
{
	friend class fglBindFactory;

protected:
	fglShaderProg(void) = default;

private:
	int id{ 0 };
	bool isLinked{ false };

public:
	virtual ~fglShaderProg(void);
	virtual fglString fgl_class(void) const { return "shaderprog"; }

	void release(void);
	bool isReady(void) { return isLinked; }
	unsigned int getId(void) {return id;}

	void attach(fglShader& poShader);
	void detach(fglShader& poShader);
	void build(void);
	void bind(void);
	static void unbind(void);

	virtual void call(fglEvent& poEvent);
};

#endif