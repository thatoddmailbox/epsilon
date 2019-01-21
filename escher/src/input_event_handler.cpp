#include <apps/apps_container.h>
#include <apps/i18n.h>
#include <escher/input_event_handler.h>
#include <escher/input_event_handler_delegate.h>
#include <escher/app.h>
#include <escher/toolbox.h>
#include <escher/metric.h>
#include <poincare/preferences.h>

bool InputEventHandler::handleBoxEvent(App * app, Ion::Events::Event event) {
  NestedMenuController * box = nullptr;
  if (m_inputEventHandlerDelegate) {
    box = event == Ion::Events::Toolbox ? m_inputEventHandlerDelegate->toolboxForInputEventHandler(this) : box;
    box = event == Ion::Events::Var ? m_inputEventHandlerDelegate->variableBoxForInputEventHandler(this) : box;
  }
  if (box) {
    box->setSender(this);
    app->displayModalViewController(box, 0.f, 0.f, Metric::PopUpTopMargin, Metric::PopUpLeftMargin, 0, Metric::PopUpRightMargin);
    return true;
  }

  if (event == Ion::Events::AngleToggle) {
    if (Poincare::Preferences::sharedPreferences()->angleUnit() == Poincare::Preferences::AngleUnit::Radian) {
      Poincare::Preferences::sharedPreferences()->setAngleUnit(Poincare::Preferences::AngleUnit::Degree);
      app->displayWarning(I18n::Message::Degres);
    } else {
      Poincare::Preferences::sharedPreferences()->setAngleUnit(Poincare::Preferences::AngleUnit::Radian);
      app->displayWarning(I18n::Message::Radian);
    }
    ((AppsContainer * )app->container())->refreshPreferences();
    return true;
  }

  return false;
}
