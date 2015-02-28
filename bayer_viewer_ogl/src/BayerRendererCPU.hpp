/**
 * @file   BayerRendererCPU.hpp
 * @author Max Smolens
 * @brief  Bayer pattern image renderer using CPU.
 */

#ifndef BAYER_RENDERER_CPU_HPP
#define BAYER_RENDERER_CPU_HPP

#include <GL/glew.h>

/**
 * Bayer pattern image renderer using CPU.  Renders a Bayer pattern
 * image in memory to an OpenGL texture.
 */
class BayerRendererCPU {
public:
	/** 
	 * Constructor.
	 */
	BayerRendererCPU ();

	/** 
	 * Destructor.
	 */
	~BayerRendererCPU ();

	/** 
	 * Initializes the renderer.
	 * 
	 * @return	true on success, false otherwise.
	 */
	bool Initialize ();

	/** 
	 * Initializes the renderer.
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
	/// Image width.
	int width;

	/// Image height.
	int height;

	/// Texture id.
	GLuint tex;

	/// RGB image.
	GLubyte *rgb;

private:
	/** 
	 * Sets texture parameters for all textures.
	 */
	void InitializeTextures ();
};

inline int
BayerRendererCPU::GetWidth () const {
	return width;
}

inline int
BayerRendererCPU::GetHeight () const {
	return height;
}

inline void
BayerRendererCPU::SetWidth (int w) {
	width = w;
}

inline void
BayerRendererCPU::SetHeight (int h) {
	height = h;
}

#endif // BAYER_RENDERER_CPU_HPP
