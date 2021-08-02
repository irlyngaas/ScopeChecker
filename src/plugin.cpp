#include "actions.hpp"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include <config.hpp>

std::vector<std::string> mOptions;
std::vector<std::string> fOptions;
std::vector<std::string> current;

struct ScopeCheckerCapturesAction : public clang::PluginASTAction {

  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &CI, llvm::StringRef InFile) override {
      return   std::make_unique<ScopeCaptureCheckerConsumer_passvar>(CI,mOptions,fOptions,current);
    }
  virtual bool ParseArgs(const CompilerInstance &CI,
      const std::vector<std::string>& args) override {
    return true;
   }
};

static FrontendPluginRegistry::Add<ScopeCheckerCapturesAction>
X(/*Name=*/PROJECT_NAME,
  /*Description=*/"Identify potentially problematic lambda captures of "
    "variables.");
