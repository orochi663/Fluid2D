#ifndef FLUID_CHARACTER_H
#define FLUID_CHARACTER_H

#include <memory>

#include <DesignPattern/SpecificObserver.h>
#include <Camera/Camera.h>
#include <Camera/CameraManFree.h>
#include <GL/GlProgram.h>
#include <GL/GlVao.h>

#include <Hud/TextHud.h>
#include <Hud/ImageHud.h>

#include <Character/AbstractCharacter.h>

class FluidCharacter : public scaena::AbstractCharacter,
                       public cellar::SpecificObserver<media::CameraMsg>
{
public:
    FluidCharacter(scaena::AbstractStage& stage);
    virtual ~FluidCharacter();

    virtual void enterStage();
    virtual void beginStep(const scaena::StageTime &time);
    virtual void endStep(const scaena::StageTime &time);
    virtual void draw(const scaena::StageTime &time);
    virtual void exitStage();

    virtual bool keyPressEvent(const scaena::KeyboardEvent &event);
    virtual bool mouseMoveEvent(const scaena::MouseEvent &event);

    virtual void notify(media::CameraMsg &msg);

    static const int WIDTH;
    static const int HEIGHT;
    static const int AREA;
    static const int POINT_SIZE;


protected:
    cellar::Vec4f initDye(float s, float t);
    cellar::Vec4f initVelocity(float s, float t);
    cellar::Vec4f initPressure(float s, float t);
    cellar::Vec4f initHeat(float s, float t);
    cellar::Vec4f initFrontier(float s, float t);

    template<typename T>
    void initTexture(unsigned int texId, const T& img);

    void advect();
    void diffuse();
    void heat();
    void computePressure();
    void substractPressureGradient();
    void frontier();
    void drawFluid();


private:
    // Size
    const float DX;
    const float DT;
    const float VISCOSITY;
    const float HEATDIFF;

    // Fluid simulation GL specific attributes
    media::GlProgram _advectShader;
    media::GlProgram _heatShader;
    media::GlProgram _jacobiShader;
    media::GlProgram _divergenceShader;
    media::GlProgram _gradSubShader;
    media::GlProgram _frontierShader;
    media::GlProgram _drawShader;
    media::GlVao _vao;
    const int DRAW_TEX;
    const int FETCH_TEX;
    unsigned int _dyeTex[2];
    unsigned int _velocityTex[2];
    unsigned int _pressureTex[2];
    unsigned int _heatTex[2];
    unsigned int _frontierTex;
    unsigned int _tempDivTex;
    unsigned int _fbo;

    // Stats panel (FPS, UPS)
    std::shared_ptr<prop2::ImageHud> _statsPanel;
    std::shared_ptr<prop2::TextHud> _fps;
    std::shared_ptr<prop2::TextHud> _ups;
};

#endif // FLUID_CHARACTER_H
