const vscode = require("vscode");
const { LanguageClient, TransportKind } = require("vscode-languageclient/node");

/** @type {LanguageClient | undefined} */
let client;

function serverArguments(configuration) {
  const args = [];

  const types = configuration.get("types");
  if (types) {
    args.push("--types", types);
  }

  const workers = configuration.get("workers");
  if (workers) {
    args.push("--workers", String(workers));
  }

  return args;
}

async function startClient() {
  const configuration = vscode.workspace.getConfiguration("nbl");
  const command = configuration.get("serverPath") || "nbl-lsp";

  /** @type {import("vscode-languageclient/node").ServerOptions} */
  const serverOptions = {
    run: {
      command,
      args: serverArguments(configuration),
      transport: TransportKind.stdio,
    },
    debug: {
      command,
      args: serverArguments(configuration),
      transport: TransportKind.stdio,
    },
  };

  const clientOptions = {
    documentSelector: [{ scheme: "file", language: "luau" }],
    synchronize: {
      fileEvents: vscode.workspace.createFileSystemWatcher("**/*.luau"),
    },
  };

  client = new LanguageClient(
    "nbl",
    "NBL",
    serverOptions,
    clientOptions,
  );

  await client.start();
}

async function stopClient() {
  if (client) {
    await client.stop();
    client = undefined;
  }
}

function activate(context) {
  context.subscriptions.push(
    vscode.commands.registerCommand("nbl.restartServer", async () => {
      await stopClient();
      await startClient();
    }),
  );

  context.subscriptions.push({ dispose: () => void stopClient() });
  void startClient();
}

function deactivate() {
  return stopClient();
}

module.exports = { activate, deactivate };
