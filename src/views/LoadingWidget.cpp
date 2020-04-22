#include "LoadingWidget.h"

#include <Wt/WTemplate.h>

LoadingWidget::LoadingWidget()
    : Wt::WLoadingIndicator()
{
    _impl = setImplementation(std::make_unique<Wt::WTemplate>(tr("loadingView")));

    setMessage(Wt::WString::Empty);
}

void LoadingWidget::setMessage(const Wt::WString& text)
{
    _impl->bindString("message", text.empty() ? tr("str.loadingPleaseWait") : text);
}

void LoadingWidget::setHidden(bool hidden, const Wt::WAnimation& animation)
{
    if (hidden)
        setMessage(Wt::WString::Empty);

    Wt::WLoadingIndicator::setHidden(hidden, animation);
}
