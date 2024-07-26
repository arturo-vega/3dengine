#pragma once

class WaterFrameBuffers {
public:
    int SCR_WIDTH = 800;
    int SCR_HEIGHT = 600;

    WaterFrameBuffers(int SCR_WIDTH, int SCR_HEIGHT) {
        this->SCR_WIDTH = SCR_WIDTH;
        this->SCR_HEIGHT = SCR_HEIGHT;
		initializeReflectionFrameBuffer(reflectionFrameBuffer, reflectionTexture, reflectionDepthBuffer);
		initializeRefractionFrameBuffer(refractionFrameBuffer, refractionTexture, refractionDepthTexture);
	}

    void bindReflectionFrameBuffer() {
		bindFrameBuffer(reflectionFrameBuffer, REFLECTION_WIDTH, REFLECTION_HEIGHT);
	}

    void bindRefractionFrameBuffer() {
        bindFrameBuffer(refractionFrameBuffer, REFRACTION_WIDTH, REFRACTION_HEIGHT);
    }

    unsigned int getReflectionTexture() {
		return reflectionTexture;
	}

    unsigned int getRefractionTexture() {
		return refractionTexture;
	}

    unsigned int getRefractionDepthTexture() {
		return refractionDepthTexture;
	}

    void unbindCurrentFrameBuffer() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    }

    void cleanUpFrameBuffers() {
		glDeleteFramebuffers(1, &reflectionFrameBuffer);
		glDeleteTextures(1, &reflectionTexture);
		glDeleteRenderbuffers(1, &reflectionDepthBuffer);

		glDeleteFramebuffers(1, &refractionFrameBuffer);
		glDeleteTextures(1, &refractionTexture);
		glDeleteTextures(1, &refractionDepthTexture);
	}


 private:
    const int REFLECTION_WIDTH = 320;
    const int REFLECTION_HEIGHT = 180;

    const int REFRACTION_WIDTH = 1280;
    const int REFRACTION_HEIGHT = 720;

    unsigned int reflectionFrameBuffer;
    unsigned int reflectionTexture;
    unsigned int reflectionDepthBuffer;

    unsigned int refractionFrameBuffer;
    unsigned int refractionTexture;
    unsigned int refractionDepthTexture;


    unsigned int createFrameBuffer() {
        unsigned int frameBufferID;
        glGenFramebuffers(1, &frameBufferID);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        return frameBufferID;
    }

    unsigned int createTextureAttachment(int width, int height) {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

        return textureID;
    }

    unsigned int createDepthTextureAttachment(int width, int height) {
        unsigned int depthTextureID;
        glGenTextures(1, &depthTextureID);
        glBindTexture(GL_TEXTURE_2D, depthTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT32, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTextureID, 0);

        return depthTextureID;
    }

    unsigned int createDepthBufferAttachment(int width, int height) {
        unsigned int depthBufferID;
        glGenRenderbuffers(1, &depthBufferID);
        glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);

        return depthBufferID;
    }

    void initializeRefractionFrameBuffer(unsigned int refractionFrameBuffer, unsigned int refractionTexture, unsigned int refractionDepthTexture) {
        refractionFrameBuffer = createFrameBuffer();
        refractionTexture = createTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT);
        refractionDepthTexture = createDepthTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT);
        unbindCurrentFrameBuffer();
    }

    void initializeReflectionFrameBuffer(unsigned int reflectionFrameBuffer, unsigned int reflectionTexture, unsigned int reflectionDepthBuffer) {
        reflectionFrameBuffer = createFrameBuffer();
        reflectionTexture = createTextureAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
        reflectionDepthBuffer = createDepthBufferAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
        unbindCurrentFrameBuffer();
    }

    void bindFrameBuffer(unsigned int frameBufferID, int width, int height) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
        glViewport(0, 0, width, height);
    }
};