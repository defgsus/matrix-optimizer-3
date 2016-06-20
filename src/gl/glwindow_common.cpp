#include <iostream>
#include <cstring>

#include "glwindow.h"
#include "glwindow_private.h"
#include "io/error.h"
#include "io/log_gl.h"


// ------------------ system independend implementation -------------

namespace MO {
namespace GL {

GlWindow::GlWindow(int width, int height, int offx, int offy)
    : p_w_      (new PrivateW(this))
{
    p_w_->curWidth = width;
    p_w_->curHeight = height;
    p_w_->curX = offx;
    p_w_->curY = offy;
    //p_w_->parent = parent;
    p_w_->createWindow();
}

GlWindow::~GlWindow()
{
    p_w_->destroyWindow();
    delete p_w_;
}


bool GlWindow::isOk() const { return p_w_->isValid && !p_w_->isDestroyed; }
bool GlWindow::isFocus() const { return p_w_->isFocus; }
bool GlWindow::isFullscreen() const { return p_w_->isFullscreen; }
bool GlWindow::isMouseOver() const { return p_w_->isMouseOver; }
bool GlWindow::isCloseable() const { return p_w_->isCloseable; }
int GlWindow::width() const { return p_w_->curWidth; }
int GlWindow::height() const { return p_w_->curHeight; }
const std::string& GlWindow::title() const { return p_w_->title; }
const GlWindowTech& GlWindow::info() const { return p_w_->info; }
GlWindowTech& GlWindow::info() { return p_w_->info; }
int GlWindow::mouseX() const { return p_w_->mouseX; }
int GlWindow::mouseY() const { return p_w_->mouseY; }
MouseKeyCodes GlWindow::mouseKeys() const { return p_w_->mouseKeys; }

double GlWindow::fps() const { return p_w_->framesPerSecond; }
#ifdef MO_ENABLE_FPS_TRACE
const ValueBuffer * GlWindow::fpsTrace() const { return p_w_->fpsTrace; }
ValueBuffer * GlWindow::fpsTrace() { return p_w_->fpsTrace; }
#endif

void GlWindow::setCloseable(bool enable) { p_w_->isCloseable = enable; }

} // namespace GL
} // namespace MO
