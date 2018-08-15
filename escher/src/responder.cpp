#include <escher/responder.h>
#include <apps/apps_container.h>
#include <apps/i18n.h>
#include <escher/app.h>
#include <escher/toolbox.h>
#include <escher/metric.h>
#include <poincare.h>
#include <assert.h>

Responder::Responder(Responder * parentResponder) :
  m_parentResponder(parentResponder)
{
}

Responder * Responder::parentResponder() const {
  return m_parentResponder;
}

void Responder::setParentResponder(Responder * responder) {
  m_parentResponder = responder;
}

bool Responder::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Toolbox && toolbox() != nullptr) {
    toolbox()->setSender(this);
    app()->displayModalViewController(toolbox(), 0.f, 0.f, Metric::PopUpTopMargin, Metric::PopUpLeftMargin, 0, Metric::PopUpRightMargin);
    return true;
  }
  if (event == Ion::Events::AngleToggle) {
    if (Poincare::Preferences::sharedPreferences()->angleUnit() == Poincare::Expression::AngleUnit::Radian) {
      Poincare::Preferences::sharedPreferences()->setAngleUnit(Poincare::Expression::AngleUnit::Degree);
      app()->displayWarning(I18n::Message::Degres);
    } else {
      Poincare::Preferences::sharedPreferences()->setAngleUnit(Poincare::Expression::AngleUnit::Radian);
      app()->displayWarning(I18n::Message::Radian);
    }
    ((AppsContainer * )app()->container())->refreshPreferences();
    return true;
  }
  return false;
}

void Responder::didBecomeFirstResponder() {
}

void Responder::willResignFirstResponder() {
}

void Responder::didEnterResponderChain(Responder * previousFirstResponder) {
}

void Responder::willExitResponderChain(Responder * nextFirstResponder) {
}

Responder * Responder::commonAncestorWith(Responder * responder) {
  if (responder == nullptr) {
    return nullptr;
  }
  if (this == responder) {
    return this;
  }
  Responder * rootResponder = this;
  while (rootResponder->parentResponder() != responder && rootResponder->parentResponder() != nullptr) {
    rootResponder = rootResponder->parentResponder();
  }
  if (rootResponder->parentResponder() == responder) {
    return responder;
  }
  rootResponder = responder;
  while (rootResponder->parentResponder() != this && rootResponder->parentResponder() != nullptr) {
    rootResponder = rootResponder->parentResponder();
  }
  if (rootResponder->parentResponder() == this) {
    return this;
  }
  Responder * r = nullptr;
  if (parentResponder()) {
    r = parentResponder()->commonAncestorWith(responder);
   }
   Responder * s = nullptr;
   if (responder->parentResponder()) {
    s = commonAncestorWith(responder->parentResponder());
   }
   if (r) {
    return r;
  }
  return s;
}

/* We assume the app is the root parent. */
App * Responder::app() {
  Responder * rootResponder = this;
  while (rootResponder->parentResponder() != nullptr) {
    rootResponder = rootResponder->parentResponder();
  }
  /* If we used RTTI we could use a dynamic_cast, which would be a lot more
   * safe, as such:
   * return dynamic_cast<App *>(rootResponder); */
   App * result = (App *)rootResponder;
   assert(result->m_magic == App::Magic); // Poor man's RTTI
  return result;
}
