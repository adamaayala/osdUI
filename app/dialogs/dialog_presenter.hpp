#pragma once
#include <osdui/interfaces.hpp>
#include <windows.h>

namespace osdui::dialogs {

class DialogPresenter : public IDialogPresenter {
public:
    explicit DialogPresenter(HINSTANCE instance);

    model::DialogResult present(const model::DialogSpec& spec,
                                const IVariableStore& vars) override;

private:
    HINSTANCE instance_;
};

} // namespace osdui::dialogs
