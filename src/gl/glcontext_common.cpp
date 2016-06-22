
#include <iostream>
#include <cstring>

#include "glcontext.h"
#include "glcontext_private.h"
#include "glwindow.h"
#include "io/error.h"
#include "io/log_gl.h"


// -------------------- system independent implementation ------------------

namespace MO {
namespace GL {


/* --------------------------- Context ---------------------------- */

GlContext::GlContext(GlWindow * win, int major, int minor)
    : p_    (new Private(this))
{
    if (win)
        create(win, major, minor);
}

GlContext::~GlContext()
{
    release();
    delete p_;
}

bool GlContext::isOk() const { return p_->isValid; }
int GlContext::width() const { return p_->window ? p_->window->width() : 0; }
int GlContext::height() const { return p_->window ? p_->window->height() : 0; }
GlContext::Sync GlContext::syncMode() const { return p_->syncMode; }
const GlContextTech& GlContext::info() const { return p_->info; }



} // namespace GL
} // namespace MO
