#pragma once

#include <Wt/WLoadingIndicator.h>

class LoadingWidget
    : public Wt::WLoadingIndicator
{
public:
    LoadingWidget();

    void setMessage(const Wt::WString& text) override;
    void setHidden(bool hidden, const Wt::WAnimation& animation) override;

private:
    Wt::WTemplate* _impl;
};
