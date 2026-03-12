#pragma once
#include <osdui/action_graph.hpp>
#include <osdui/iregistry.hpp>

namespace osdui::actions {

// Scans HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall for installed software.
// Writes OSDSoftware001...OSDSoftwareNNN and OSDSoftware (count) into vars.
// Deferred: full implementation requires registry enumeration (not in scope for this task).
class SoftwareDiscoveryAction : public IAction {
public:
    explicit SoftwareDiscoveryAction(IRegistry& reg) : reg_{reg} {}
    ActionResult execute(ActionContext& ctx) override;
private:
    IRegistry& reg_;
};

} // namespace osdui::actions
