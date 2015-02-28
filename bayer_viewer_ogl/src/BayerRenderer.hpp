/**
 * @file   BayerRenderer.hpp
 * @author Max Smolens
 * @brief  Bayer pattern image renderer.
 */

#ifndef BAYER_RENDERER_HPP
#define BAYER_RENDERER_HPP

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
	 * 
	 * @return	true on success, false otherwise.
	 */
	bool Initialize ();

	/** 
	 * Initializes the renderer with specified image width and height.
	 * 
	 * @param width		the image width.
	 * @param height	the image height.
	 * 
	 * @return	true on success, false otherwise.
	 */
	bool Initialize (int width,
			 int height);

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
	/// Required number of texture units.
	static const int REQUIRED_TEXTURE_UNITS;

	/// Texture offset for minification.
	static const float OFFSET;

	/// Blending constant for the two green channels.
	static const float BLEND[4];

	/// Initialization string for RenderTexture.
	static const char *RENDERTEXTURE_INIT;
	
	/// (x,y) pixel offsets for red channel in Bayer image.
	static const int OFFSET_RED[2];

	/// (x,y) pixel offsets for blue channel in Bayer image.
	static const int OFFSET_BLUE[2];

	/// (x,y) pixel offsets for first green channel in Bayer image.
	static const int OFFSET_GREEN1[2];

	/// (x,y) pixel offsets for second green channel in Bayer image.
	static const int OFFSET_GREEN2[2];

	/// Image width.
	int width;

	/// Image Height.
	int height;

	/// Texture id constants.
	enum {BAYER = 0, RED, BLUE, GREEN1, GREEN2};

	/// Texture ids.
	GLuint tex[5];

	/// Display lists
	GLuint texMagList;
	GLuint texMinListMain;
	GLuint texMinListRed;
	GLuint texMinListBlue;
	GLuint texMinListGreen1;
	GLuint texMinListGreen2;
	
	/// RenderTexture
	RenderTexture *rt;

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
	 * Draws minified textured quad with specified texture offset.
	 * 
	 * @param shift_x	x texture offset.
	 * @param shift_y	y texture offset.
	 */
	void DrawTexQuadMin (int shift_x,
			     int shift_y) const;

	/** 
	 * Draws magnified textured quad using multitexturing.
	 */
	void DrawTexQuadMag () const;

	/** 
	 * Performs conversion of Bayer image to RGB image.
	 */
	void Render () const;

	/** 
	 * Creates display list for magnified textured quad.
	 */
	void CreateMagnificationDisplayList ();

	/** 
	 * Creates display lists for minified textured quads.
	 */
	void CreateMinificationDisplayLists ();
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

#endif // BAYER_RENDERER_HPP
