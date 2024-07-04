#include "fglTexture.h"
#include "fglScene.h"


fglTexture::~fglTexture(void)
{
	if (id) glDeleteTextures(1, &id);
}

void fglTexture::build(void)
{
	if (path.empty()) return;

	if (id) { glDeleteTextures(1, &id); id=0; }
	id = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS );
	if ( ! id ) { throw fglException(SOIL_last_result()); return; }
	
	setSceneRenderFlag();

}

void fglTexture::bind(void)
{
	glBindTexture(GL_TEXTURE_2D, id);
}

void fglTexture::unbind(void)
{
	glBindTexture(GL_TEXTURE_2D, NULL);
}

