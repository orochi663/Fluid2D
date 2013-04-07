#include <Misc/Log.h>

#include <ScaenaApplication/Application.h>
#include <ScaenaApplication/GlMainWindow.h>
#include <Stage/QGLStage.h>
#include <Play/TrivialPlay.h>

#include "FluidCharacter.h"

using namespace std;
using namespace cellar;
using namespace scaena;


int main(int argc, char** argv) try
{
    getLog().setOuput(cout);
    getApplication().init(argc, argv);

    QGLStage* stage = new QGLStage();
    stage->setDrawSynch(true);
    stage->setDrawInterval(20);
    stage->setUpdateInterval(0);
    getApplication().addCustomStage(stage);

    GlMainWindow window(stage);
    window.setGlWindowSpace(FluidCharacter::WIDTH  * FluidCharacter::POINT_SIZE,
                            FluidCharacter::HEIGHT * FluidCharacter::POINT_SIZE);
    window.centerOnScreen();
    window.show();

    shared_ptr<AbstractCharacter> character(new FluidCharacter(*stage));
    shared_ptr<AbstractPlay> play(new TrivialPlay("Fluid2D",character));
    getApplication().setPlay(play);

    return getApplication().execute(stage->id());
}
catch(exception& e)
{
    cerr << "Exception caught : " << e.what() << endl;
}
catch(...)
{
    cerr << "Exception passed threw.." << endl;
    throw;
}
