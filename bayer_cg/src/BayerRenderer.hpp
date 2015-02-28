/**
 * @file   BayerRenderer.hpp
 * @author Max Smolens
 * @brief  Bayer pattern image renderer.
 */

#ifndef BAYER_RENDERER_HPP
#define BAYER_RENDERER_HPP

#include <GL/glew.h>
#ifdef WIN32
# include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glext.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include "RenderTexture.h"

/**
 * Bayer pattern image renderer.  Renders a Bayer pattern image in
 * memory to an OpenGL texture.
 */
class BayerRenderer {
public:
	/** 
	 * Constructor.
	 */
	BayerRenderer ();

	/** 
	 * Deconstructor.
	 */
	~BayerRenderer ();

	/** 
	 * Initializes the renderer.
	 * @param context	the Cg context.
	 * @param vertexProfile	the vertex profile.
	 * @param fragmentProfile	the fragment profile.
	 * 
	 * @return	true on success, false otherwise.
	 */
	bool Initialize (CGcontext context,
			 CGprofile vertexProfile,
			 CGprofile fragmentProfile);

	/** 
	 * Initializes the renderer with specified image width and height.
	 * 
	 * @param width		the image width.
	 * @param height	the image height.
	 * @param context	the Cg context.
	 * @param vertexProfile	the vertex profile.
	 * @param fragmentProfile	the fragment profile.
	 * 
	 * @return	true on success, false otherwise.
	 */
	bool Initialize (int width,
			 int height,
			 CGcontext context,
			 CGprofile vertexProfile,
			 CGprofile fragmentProfile);

	/**
	 * Updates Bayer texture from image data in memory and renders to texture.
	 */
	void SetBayer (const GLubyte *) const;
	
	/**
	 * Binds the texture to the active texture unit.
	 */
	void Bind () const;

	/**
	 * Enables the texture target.
	 */
	void EnableTextureTarget () const;

	/**
	 * Disables the texture target.
	 */
	void DisableTextureTarget () const;

	/** 
	 * Gets the image width.
	 * 
	 * @return	the image width.
	 */
	int GetWidth () const;

	/** 
	 * Gets the image height.
	 * 
	 * @return	the image height.
	 */
	int GetHeight () const;

	/** 
	 * Sets the image width.
	 * 
	 * @param width		the image width.
	 */
	void SetWidth (int width);

	/** 
	 * Sets the image height.
	 * 
	 * @param height	the image height.
	 */
	void SetHeight (int height);
	
private:
	/// Initialization string for RenderTexture.
	static const char *RENDERTEXTURE_INIT;

	/// Colormask texture.
	static const GLfloat MASK[];
	
	/// Image width.
	int width;

	/// Image Height.
	int height;

	/// Texture ids.
	GLuint tex_bayer;
	GLuint tex_mask;

	/// RenderTexture
	RenderTexture *rt;

	// Cg context.
	CGcontext context;

	/// Cg vertex profile handle.
	CGprofile vertexProfile;

	/// Cg fragment profile handle.
	CGprofile fragmentProfile;

	/// Cg vertex program handle.
	CGprogram vertexProgram;

	/// Cg fragment program handle.
	CGprogram fragmentProgram;

private:
	/** 
	 * Creates and sets defaults for a RenderTexture object.
	 * 
	 * @param w	width of the RenderTexture object.
	 * @param h	height of the RenderTexture object.
	 * 
	 * @return	a pointer to the new RenderTexture object, or NULL on failure.
	 */
	RenderTexture* CreateRenderTexture (int w,
					    int h) const;

	/** 
	 * Sets texture parameters for all textures.
	 */
	void InitializeTextures ();

	/** 
	 * Performs conversion of Bayer image to RGB image.
	 */
	void Render () const;

	/** 
	 * Loads the Cg programs.
	 *
	 * @return	True if programs load successfully.
	 */
	bool LoadCgPrograms ();
};

inline int
BayerRenderer::GetWidth () const {
	return width;
}

inline int
BayerRenderer::GetHeight () const {
	return height;
}

inline void
BayerRenderer::SetWidth (int w) {
	width = w;
}

inline void
BayerRenderer::SetHeight (int h) {
	height = h;
}

inline void
BayerRenderer::Bind () const
{
	rt->Bind ();
}

inline void
BayerRenderer::EnableTextureTarget () const
{
	rt->EnableTextureTarget ();
}

inline void
BayerRenderer::DisableTextureTarget () const
{
	rt->DisableTextureTarget ();
}

#endif // BAYER_RENDERER_HPP
