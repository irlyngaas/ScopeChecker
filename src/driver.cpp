#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include "actions.hpp"
#include <config.hpp>
#include "clang/Tooling/CommonOptionsParser.h"

//#include "clang/ASTMatchers/ASTMatchers.h"
//#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace llvm;
using namespace clang;
using namespace clang::tooling;

template<class SomeFrontEndAction>
std::unique_ptr<FrontendActionFactory> myNewFrontendActionFactory(std::vector<std::string>& options, std::vector<std::string>& options2, std::vector<std::string>& current) {
  class SimpleFrontendActionFactory : public FrontendActionFactory {
   public:
    SimpleFrontendActionFactory(std::vector<std::string>& options, std::vector<std::string>& options2, std::vector<std::string>& current) : mOptions(options), fOptions(options2), current_func(current) {}

    std::unique_ptr<FrontendAction> create() override {
      return std::make_unique<SomeFrontEndAction>(mOptions,fOptions,current_func);
    }

   private:
    std::vector<std::string>& mOptions;
    std::vector<std::string>& fOptions;
    std::vector<std::string>& current_func;
    
  };

  return std::unique_ptr<FrontendActionFactory>(
      new SimpleFrontendActionFactory(options,options2,current));
}

//class ScopeCaptureCheckerAction : public clang::ASTFrontendAction {
//public:
//
//  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
//    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
//    return std::unique_ptr<clang::ASTConsumer>(
//        new ScopeCaptureCheckerConsumer(&Compiler.getASTContext()));
//  }
//};

class ScopeCaptureCheckerAction_passvar : public clang::ASTFrontendAction {
public:
  explicit ScopeCaptureCheckerAction_passvar(std::vector<std::string>& options, std::vector<std::string>& funoptions, std::vector<std::string>& current) : mOptions(options), fOptions(funoptions), current_func(current) {}

  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(
        new ScopeCaptureCheckerConsumer_passvar(&Compiler.getASTContext(),mOptions,fOptions,current_func));
  }
private:
  std::vector<std::string>& mOptions;
  std::vector<std::string>& fOptions;
  std::vector<std::string>& current_func;
};

static cl::OptionCategory ScopeCaptureCheckerCategory( PROJECT_NAME " options");

int main(int argc, const char **argv) {

  CommonOptionsParser Op(argc, argv, ScopeCaptureCheckerCategory);
  ClangTool Tool(Op.getCompilations(), Op.getSourcePathList());

  //Tool.run(myNewFrontendActionFactory<ScopeCaptureCheckerAction_passvar>(stringer).get());
  //llvm::outs() << "Collection: ";
  //for(auto itr : stringer) {
  //  llvm::outs() << itr << " ";
  //}
  
  //return Tool.run(newFrontendActionFactory<ScopeCaptureCheckerAction>().get());
  //return 0;
  std::vector<std::string> stringer;
  std::vector<std::string> stringer2;
  std::vector<std::string> current;
  return Tool.run(myNewFrontendActionFactory<ScopeCaptureCheckerAction_passvar>(stringer,stringer2,current).get());
}
