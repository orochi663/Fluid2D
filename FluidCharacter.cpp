#include "FluidCharacter.h"

#include <cmath>
#include <iostream>
using namespace std;

#include <GL3/gl3w.h>

#include <Misc/CellarUtils.h>
#include <Algorithm/Noise.h>
using namespace cellar;

#include <PropTeam/AbstractPropTeam.h>
using namespace prop2;

#include <Play/AbstractPlay.h>
#include <Stage/AbstractStage.h>
#include <Stage/Event/StageTime.h>
#include <Stage/Event/KeyboardEvent.h>
#include <Stage/Event/MouseEvent.h>
#include <Stage/Event/SynchronousKeyboard.h>
#include <Stage/Event/SynchronousMouse.h>
using namespace scaena;


const int FluidCharacter::WIDTH = 256;
const int FluidCharacter::HEIGHT = 256;
const int FluidCharacter::AREA = WIDTH * HEIGHT;
const int FluidCharacter::POINT_SIZE = 3;


FluidCharacter::FluidCharacter(AbstractStage& stage) :
    AbstractCharacter(stage, "FluidCharacter"),
    DX(1.0f),
    DT(1.0f),
    VISCOSITY(0.01f),
    HEATDIFF(0.01f),
    _drawShader(),
    _vao(),
    DRAW_TEX(1),
    FETCH_TEX(0),
    _statsPanel(),
    _fps(),
    _ups()
{
    stage.camera().registerObserver(*this);
}

FluidCharacter::~FluidCharacter()
{

}

void FluidCharacter::enterStage()
{
    // GL resources
    GlVbo2Df buffPos;
    buffPos.attribLocation = 0;
    buffPos.dataArray.push_back(Vec2f(-1.0, -1.0));
    buffPos.dataArray.push_back(Vec2f( 1.0, -1.0));
    buffPos.dataArray.push_back(Vec2f( 1.0,  1.0));
    buffPos.dataArray.push_back(Vec2f(-1.0,  1.0));

    _vao.createBuffer("position", buffPos);


    GlInputsOutputs updateLocations;
    updateLocations.setInput(buffPos.attribLocation, "position");
    updateLocations.setOutput(0, "FragOut");

    _advectShader.setInAndOutLocations(updateLocations);
    _advectShader.addShader(GL_VERTEX_SHADER, "resources/shaders/update.vert");
    _advectShader.addShader(GL_FRAGMENT_SHADER, "resources/shaders/advect.frag");
    _advectShader.link();
    _advectShader.pushProgram();
    _advectShader.setInt("FragInTex", 0);
    _advectShader.setInt("VelocityTex", 1);
    _advectShader.setInt("FrontierTex", 2);
    _advectShader.setVec2f("Size", Vec2f(WIDTH, HEIGHT));
    _advectShader.setFloat("rDx", 1.0f / DX);
    _advectShader.setFloat("Dt",  DT);
    _advectShader.popProgram();


    _jacobiShader.setInAndOutLocations(updateLocations);
    _jacobiShader.addShader(GL_VERTEX_SHADER, "resources/shaders/update.vert");
    _jacobiShader.addShader(GL_FRAGMENT_SHADER, "resources/shaders/jacobi.frag");
    _jacobiShader.link();
    _jacobiShader.pushProgram();
    _jacobiShader.setInt("XTex", 0);
    _jacobiShader.setInt("BTex", 1);
    _jacobiShader.setVec2f("Size", Vec2f(WIDTH, HEIGHT));
    _jacobiShader.setFloat("Alpha", 1);
    _jacobiShader.setFloat("rBeta", 1);
    _jacobiShader.popProgram();

    _divergenceShader.setInAndOutLocations(updateLocations);
    _divergenceShader.addShader(GL_VERTEX_SHADER, "resources/shaders/update.vert");
    _divergenceShader.addShader(GL_FRAGMENT_SHADER, "resources/shaders/divergence.frag");
    _divergenceShader.link();
    _divergenceShader.pushProgram();
    _divergenceShader.setInt("VelocityTex", 0);
    _divergenceShader.setVec2f("Size", Vec2f(WIDTH, HEIGHT));
    _divergenceShader.setFloat("HalfrDx", 0.5f / DX);
    _divergenceShader.popProgram();


    _gradSubShader.setInAndOutLocations(updateLocations);
    _gradSubShader.addShader(GL_VERTEX_SHADER, "resources/shaders/update.vert");
    _gradSubShader.addShader(GL_FRAGMENT_SHADER, "resources/shaders/gradSub.frag");
    _gradSubShader.link();
    _gradSubShader.pushProgram();
    _gradSubShader.setInt("PressureTex", 0);
    _gradSubShader.setInt("VelocityTex", 1);
    _gradSubShader.setVec2f("Size", Vec2f(WIDTH, HEIGHT));
    _gradSubShader.setFloat("HalfrDx", 0.5f / DX);
    _gradSubShader.popProgram();


    GlInputsOutputs heatLocations;
    heatLocations.setInput(buffPos.attribLocation, "position");
    heatLocations.setOutput(0, "Velocity");
    heatLocations.setOutput(1, "Heat");
    _heatShader.setInAndOutLocations(heatLocations);
    _heatShader.addShader(GL_VERTEX_SHADER, "resources/shaders/update.vert");
    _heatShader.addShader(GL_FRAGMENT_SHADER, "resources/shaders/heat.frag");
    _heatShader.link();
    _heatShader.pushProgram();
    _heatShader.setInt("VelocityTex", 0);
    _heatShader.setInt("HeatTex", 1);
    _heatShader.setVec2f("Size", Vec2f(WIDTH, HEIGHT));
    _heatShader.setFloat("HalfrDx", 0.5f / DX);
    _heatShader.popProgram();


    GlInputsOutputs frontierLocations;
    frontierLocations.setInput(buffPos.attribLocation, "position");
    frontierLocations.setOutput(0, "Velocity");
    frontierLocations.setOutput(1, "Pressure");
    _frontierShader.setInAndOutLocations(frontierLocations);
    _frontierShader.addShader(GL_VERTEX_SHADER, "resources/shaders/update.vert");
    _frontierShader.addShader(GL_FRAGMENT_SHADER, "resources/shaders/frontier.frag");
    _frontierShader.link();
    _frontierShader.pushProgram();
    _frontierShader.setInt("VelocityTex", 0);
    _frontierShader.setInt("PressureTex", 1);
    _frontierShader.setInt("FrontierTex", 2);
    _frontierShader.setVec2f("Size", Vec2f(WIDTH, HEIGHT));
    _frontierShader.popProgram();


    GlInputsOutputs drawLocations;
    drawLocations.setInput(buffPos.attribLocation, "position");
    drawLocations.setOutput(0, "FragColor");
    _drawShader.setInAndOutLocations(drawLocations);
    _drawShader.addShader(GL_VERTEX_SHADER, "resources/shaders/drawFluid.vert");
    _drawShader.addShader(GL_FRAGMENT_SHADER, "resources/shaders/drawFluid.frag");
    _drawShader.link();
    _drawShader.pushProgram();
    _drawShader.setInt("DyeTex",      0);
    _drawShader.setInt("VelocityTex", 1);
    _drawShader.setInt("PressureTex", 2);
    _drawShader.setInt("HeatTex",     3);
    _drawShader.popProgram();
    // End GL resources


    // Stats Panel
    _statsPanel = stage().propTeam().createImageHud();
    _statsPanel->setSize(Vec2r(128, 64));
    _statsPanel->setImageName("resources/textures/statsPanel.bmp");
    _statsPanel->setHandlePosition(Vec2r(6.0, -70.0));
    _statsPanel->setHorizontalAnchor(HorizontalAnchor::LEFT);
    _statsPanel->setVerticalAnchor(VerticalAnchor::TOP);

    _fps = stage().propTeam().createTextHud();
    _fps->setColor(Vec4r(0/255.0, 3/255.0, 80/255.0, 1.0));
    _fps->setHeight(20);
    _fps->setHandlePosition(_statsPanel->handlePosition() + Vec2r(50, 33));
    _fps->setHorizontalAnchor(_statsPanel->horizontalAnchor());
    _fps->setVerticalAnchor(_statsPanel->verticalAnchor());

    _ups = stage().propTeam().createTextHud();
    _ups->setColor(_fps->color());
    _ups->setHeight(20);
    _ups->setHandlePosition(_statsPanel->handlePosition() + Vec2r(50, 9));
    _ups->setHorizontalAnchor(_statsPanel->horizontalAnchor());
    _ups->setVerticalAnchor(_statsPanel->verticalAnchor());
    // End Stats Panel

    // Camera and stage size
    stage().camera().setMode(Camera::EXPAND);
    stage().camera().setLens(Camera::Lens::ORTHOGRAPHIC,
                             0,  WIDTH*POINT_SIZE,
                             0,  HEIGHT*POINT_SIZE,
                             -1, 1);

    Vec3f from(0, 0, 0);
    stage().camera().setTripod(
        from,
        from + Vec3f(0, 0, -1),
        Vec3f(0, 1, 0));
    // End Camera and stage size


    // OpenGL states
    glClearColor(0.2, 0.2, 0.2, 1.0);
    glGenTextures(2, _dyeTex);
    glGenTextures(2, _velocityTex);
    glGenTextures(2, _pressureTex);
    glGenTextures(2, _heatTex);
    glGenTextures(1, &_frontierTex);
    glGenTextures(1, &_tempDivTex);

    typedef float texComp_t;
    typedef Vector<4, texComp_t> texVec_t;
    vector<texVec_t> dyeImg(AREA);
    vector<texVec_t> velocityImg(AREA);
    vector<texVec_t> pressureImg(AREA);
    vector<texVec_t> heatImg(AREA);
    vector<texVec_t> frontierImg(AREA);
    for(int i=0; i<AREA; ++i)
    {
        float s = (i%WIDTH)/(float)WIDTH;
        float t = (i/WIDTH)/(float)HEIGHT;
        dyeImg[i]      = initDye(s, t);
        velocityImg[i] = initVelocity(s, t);
        pressureImg[i] = initPressure(s, t);
        heatImg[i]     = initHeat(s, t);
        frontierImg[i] = initFrontier(s, t);
    }

    initTexture(_dyeTex[0],      dyeImg);
    initTexture(_dyeTex[1],      dyeImg);
    initTexture(_velocityTex[0], velocityImg);
    initTexture(_velocityTex[1], velocityImg);
    initTexture(_pressureTex[0], pressureImg);
    initTexture(_pressureTex[1], pressureImg);
    initTexture(_heatTex[0],     heatImg);
    initTexture(_heatTex[1],     heatImg);
    initTexture(_frontierTex,    frontierImg);
    initTexture(_tempDivTex,     vector<texVec_t>(AREA));


    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    // End OpenGL states
}

Vec4f FluidCharacter::initDye(float s, float t)
{
    float zoom = 4.0f;
    float dye = SimplexNoise::noise2d(s*zoom, t*zoom);
    return Vec4f(dye, dye, dye, 1.0);
}

Vec4f FluidCharacter::initVelocity(float s, float t)
{
    /* Swirl
    const float cx = 0.5f, cy = 0.5f;
    const float ed = 0.35f;
    float dist = Vec2f(s, t).distanceTo(cx, cy);
    if(dist < ed)
        return Vec4f(t-cx, -(s-cy), 0, 0) * 3.0 + Vec4f(1.0, 1.0, 0, 0) * 1.0;
    return Vec4f();
    //*/

    /* Plank
    if(cellar::inRange(s, 0.3f, 0.5f) &&
       cellar::inRange(t, 0.2f, 0.40f))
    {
        return Vec4f(0.0, 1.0, 0.0, 0.0) * (0.1-absolute(t-0.3f))*10.0;
    }
    if(cellar::inRange(s, 0.5f, 0.7f) &&
       cellar::inRange(t, 0.6f, 0.8f))
    {
        return Vec4f(0.0, -1.0, 0.0, 0.0) * (0.1-absolute(t-0.7f))*10.0;
    }
    return Vec4f();
    //*/

    return Vec4f();
}

Vec4f FluidCharacter::initPressure(float s, float t)
{
    return Vec4f();
}

cellar::Vec4f FluidCharacter::initHeat(float s, float t)
{
    if((Vec2f(s, t) - Vec2f(0.2, 0.8)).length() < 0.08)
        return Vec4f(-5, 0, 0, 0);
    if((Vec2f(s, t) - Vec2f(0.5, 0.2)).length() < 0.08)
        return Vec4f(5, 0, 0, 0);
    return Vec4f();
}

cellar::Vec4f FluidCharacter::initFrontier(float s, float t)
{
    const Vec4f block(1.0, 1.0, 1.0, 1.0);
    const Vec4f fluid(0.0, 0.0, 0.0, 0.0);

    const float W = 0.03;
    if(s < W || s > 1-W || t < W || t > 1-W)
        return block;

    if((t > 0.45 && t < 0.52) && (
        !inRange(s, 0.22f, 0.24f) &&
        !inRange(s, 0.50f, 0.53f) &&
        !inRange(s, 0.78f, 0.80f)))
        return block;
/*
    if(Vec2f(s, t).distanceTo(0.75, 0.66) < 0.2)
        return block;

    if(Vec2f(s, t).distanceTo(0.3, 0.2) < 0.03)
        return block;
*/
    return fluid;
}

template<typename T>
void FluidCharacter::initTexture(unsigned int texId, const T& img)
{
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0,
                 GL_RGBA, GL_FLOAT, img.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void FluidCharacter::beginStep(const scaena::StageTime &time)
{
}

void FluidCharacter::endStep(const scaena::StageTime &time)
{
    _ups->setText(toString(1.0 / time.elapsedTime()));
}

void FluidCharacter::draw(const scaena::StageTime &time)
{
    _fps->setText(toString(1.0 / time.elapsedTime()));

    _vao.bind();

    glViewport(0, 0, WIDTH, HEIGHT);
    advect();
    diffuse();
    heat();
    computePressure();
    substractPressureGradient();
    frontier();

    glViewport(0, 0, stage().width(), stage().height());
    drawFluid();

    _vao.unbind();
}

void FluidCharacter::advect()
{
    GLenum drawBuffers;

    _advectShader.pushProgram();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    glDrawBuffers(1, &(drawBuffers = GL_COLOR_ATTACHMENT0));

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _frontierTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _velocityTex[FETCH_TEX]);
    glActiveTexture(GL_TEXTURE0);

    // Dye
    glBindTexture(GL_TEXTURE_2D, _dyeTex[FETCH_TEX]);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,       _dyeTex[DRAW_TEX], 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    swap(_dyeTex[FETCH_TEX], _dyeTex[DRAW_TEX]);

    // Heat
    glBindTexture(GL_TEXTURE_2D, _heatTex[FETCH_TEX]);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,       _heatTex[DRAW_TEX], 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    swap(_heatTex[FETCH_TEX], _heatTex[DRAW_TEX]);

    // Velocity
    glBindTexture(GL_TEXTURE_2D, _velocityTex[FETCH_TEX]);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,       _velocityTex[DRAW_TEX], 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    swap(_velocityTex[FETCH_TEX], _velocityTex[DRAW_TEX]);

    _advectShader.popProgram();
}

void FluidCharacter::diffuse()
{
    GLenum drawBuffers;

    _jacobiShader.pushProgram();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    glDrawBuffers(1, &(drawBuffers = GL_COLOR_ATTACHMENT0));

    const int NB_ITERATIONS = 60;


    // Velocity
    _jacobiShader.setFloat("Alpha", DX*DX / (VISCOSITY*DT));
    _jacobiShader.setFloat("rBeta", 1.0f / (4.0f + DX*DX/(VISCOSITY*DT)) );

    for(int i=0; i < (NB_ITERATIONS/2)*2; ++i)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, _velocityTex[FETCH_TEX]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _velocityTex[FETCH_TEX]);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D,       _velocityTex[DRAW_TEX], 0);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // Swap textures
        swap(_velocityTex[FETCH_TEX], _velocityTex[DRAW_TEX]);
    }

    // Heat
    _jacobiShader.setFloat("Alpha", DX*DX / (HEATDIFF*DT));
    _jacobiShader.setFloat("rBeta", 1.0f / (4.0f + DX*DX/(HEATDIFF*DT)) );
    for(int i=0; i < (NB_ITERATIONS/2)*2; ++i)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, _heatTex[FETCH_TEX]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _heatTex[FETCH_TEX]);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D,       _heatTex[DRAW_TEX], 0);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // Swap textures
        swap(_heatTex[FETCH_TEX], _heatTex[DRAW_TEX]);
    }


    _jacobiShader.popProgram();
}

void FluidCharacter::heat()
{
    GLenum drawBuffers [] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
    };

    _heatShader.pushProgram();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    glDrawBuffers(2, drawBuffers);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _heatTex[FETCH_TEX]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _velocityTex[FETCH_TEX]);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
                           GL_TEXTURE_2D,       _heatTex[DRAW_TEX],     0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,       _velocityTex[DRAW_TEX], 0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    swap(_heatTex[FETCH_TEX],     _heatTex[DRAW_TEX]);
    swap(_velocityTex[FETCH_TEX], _velocityTex[DRAW_TEX]);

    _heatShader.popProgram();
}

void FluidCharacter::computePressure()
{
    GLenum drawBuffers;

    _divergenceShader.pushProgram();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    glDrawBuffers(1, &(drawBuffers = GL_COLOR_ATTACHMENT0));

    glBindTexture(GL_TEXTURE_2D, _velocityTex[FETCH_TEX]);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,       _tempDivTex, 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    _divergenceShader.popProgram();



    _jacobiShader.pushProgram();
    _jacobiShader.setFloat("Alpha", -DX*DX);
    _jacobiShader.setFloat("rBeta", 1.0f / 4.0f);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    glDrawBuffers(1, &(drawBuffers = GL_COLOR_ATTACHMENT0));

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _tempDivTex);
    glActiveTexture(GL_TEXTURE0);

    const int NB_ITERATIONS = 200;
    for(int i=0; i < (NB_ITERATIONS/2)*2; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, _pressureTex[FETCH_TEX]);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D,       _pressureTex[DRAW_TEX], 0);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // Swap textures
        swap(_pressureTex[FETCH_TEX], _pressureTex[DRAW_TEX]);
    }

    _jacobiShader.popProgram();
}

void FluidCharacter::substractPressureGradient()
{
    GLenum drawBuffers;

    _gradSubShader.pushProgram();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    glDrawBuffers(1, &(drawBuffers = GL_COLOR_ATTACHMENT0));

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _velocityTex[FETCH_TEX]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _pressureTex[FETCH_TEX]);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,       _velocityTex[DRAW_TEX], 0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    swap(_velocityTex[FETCH_TEX], _velocityTex[DRAW_TEX]);

    _gradSubShader.popProgram();
}

void FluidCharacter::frontier()
{
    GLenum drawBuffers [] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
    };

    _frontierShader.pushProgram();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    glDrawBuffers(2, drawBuffers);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,       _velocityTex[DRAW_TEX], 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
                           GL_TEXTURE_2D,       _pressureTex[DRAW_TEX], 0);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _frontierTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _pressureTex[FETCH_TEX]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _velocityTex[FETCH_TEX]);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    swap(_velocityTex[FETCH_TEX], _velocityTex[DRAW_TEX]);
    swap(_pressureTex[FETCH_TEX], _pressureTex[DRAW_TEX]);

    _frontierShader.popProgram();
}

void FluidCharacter::drawFluid()
{
    _drawShader.pushProgram();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, _heatTex[FETCH_TEX]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _pressureTex[FETCH_TEX]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _velocityTex[FETCH_TEX]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _dyeTex[FETCH_TEX]);

    glPointSize(POINT_SIZE);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glPointSize(1.0f);

    _drawShader.popProgram();
}

void FluidCharacter::exitStage()
{
    stage().propTeam().deleteImageHud(_statsPanel);
    stage().propTeam().deleteTextHud(_fps);
    stage().propTeam().deleteTextHud(_ups);
}

bool FluidCharacter::keyPressEvent(const KeyboardEvent &event)
{
    if(event.getAscii() == 'R')
    {
        stage().play().restart();
        return true;
    }
    else if(event.getAscii() == 'S')
    {
        _fps->setIsVisible(!_statsPanel->isVisible());
        _ups->setIsVisible(!_statsPanel->isVisible());
        _statsPanel->setIsVisible(!_statsPanel->isVisible());
    }

    return false;
}

bool FluidCharacter::mouseMoveEvent(const scaena::MouseEvent &event)
{
    Vec2f candlePos(event.position().x(), stage().height() - event.position().y());
    candlePos *= 2.0f / POINT_SIZE;
    _heatShader.pushProgram();
    _heatShader.setVec2f("MousePos", candlePos);
    _heatShader.popProgram();
    cout << candlePos << endl;
    return true;
}

void FluidCharacter::notify(cellar::CameraMsg &)
{
}
