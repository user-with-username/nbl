#pragma once

namespace nbl::lsp {

/// Job kind of "publish diagnostics for a document" - newer jobs of this kind
/// replace older queued ones for the same document.
inline constexpr int kDiagnosticsJob = 1;

/// Worker key of workspace-wide jobs (e.g. building the require index).
inline constexpr const char *kWorkspaceKey = "__nbl_workspace__";

} // namespace nbl::lsp
