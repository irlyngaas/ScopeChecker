#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <iterator>
#include <vector>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

using namespace clang;
using namespace clang::tooling;

class FindScopeCapture : public RecursiveASTVisitor<FindScopeCapture> {
public :
  explicit FindScopeCapture(ASTContext *Context, std::vector<std::string>& collection, std::vector<std::string>& funcollection)
    : Context(Context), mCollection(collection), fCollection(funcollection) {}

  bool VisitDeclRefExpr(DeclRefExpr *DRE) {
    if(!DRE || !Context->getSourceManager().isWrittenInMainFile(DRE->getLocation()))
      return true;

    std::string foundString = DRE->getFoundDecl()->getNameAsString();
    std::string functionString;
    if(!current_func_) { 
      functionString = "NULL_FUNC";
    }
    else {
      functionString = current_func_->getNameInfo().getAsString();    
    }

    //Check DREs, if they are in LambdaExpr and have nonOdrUse -> remove from the list of DRE
    if(DRE->isNonOdrUse()) {
      int index = 0;
      std::vector<int> indexArr;
      for(auto itr : mCollection) {
        if(foundString == itr && functionString == fCollection[index]) {
          indexArr.push_back(index);
          break;
        }
        index += 1;
      }
      for(unsigned i = indexArr.size()-1; indexArr.size() > i; --i) {
        mCollection.erase(mCollection.begin()+indexArr[i]);
        fCollection.erase(fCollection.begin()+indexArr[i]);
      }
      return true;
    }

    //Some DRE in the list are operators from YAKL Array class these should be fine, since YAKL is unit tested, and are taken out of the the list of DRE
    
    std::string foundNamespace = DRE->getFoundDecl()->getQualifiedNameAsString();
    if(foundNamespace.find(':') == std::string::npos) {
      return true;
    }
    else {
      std::string leftMost = foundNamespace.substr(0, foundNamespace.find("::", 0));
      //if(leftMost != "yakl" and leftMost !="std")
      if(leftMost != "yakl")
        return true;

      std::string mystr = foundNamespace;
      std::size_t pos;
      //If has namespace qualifier, remove namespace qualifiers till at the root which gives the name of the DRE
      while(mystr.find(':') != std::string::npos) {
        pos = mystr.find("::");
        mystr = mystr.substr(pos+2);
      }
      int index = 0;
      std::vector<int> indexArr;
      for(auto itr : mCollection) {
        if(mystr == itr) {
          indexArr.push_back(index);
          break;
        }
        index += 1;
      }
      for(unsigned i = indexArr.size()-1; indexArr.size() > i; --i) {
        mCollection.erase(mCollection.begin()+indexArr[i]);
        fCollection.erase(fCollection.begin()+indexArr[i]);
      }
      return true;
    }
  }

  bool VisitVarDecl(VarDecl *VD) {
    if(!VD || !Context->getSourceManager().isWrittenInMainFile(VD->getLocation()))
      return true;
    
    //Not needed anymore since, VarDecls in global namespace aren't listed, only DRE from within LambdaExpr are listed.
    if(!current_func_) { 
      std::string foundString = VD->getNameAsString();
      std::string functionString;
      if(!current_func_) { 
        functionString = "NULL_FUNC";
      }
      else {
        functionString = current_func_->getNameInfo().getAsString();    
      }

      int index = 0;
      std::vector<int> indexArr;
      for(auto itr : mCollection) {
        if(foundString == itr && functionString == fCollection[index]) {
          indexArr.push_back(index);
          break;
        }
        index +=1;
      }

      for(unsigned i = indexArr.size()-1; indexArr.size() > i; --i) {
        mCollection.erase(mCollection.begin()+indexArr[i]);
        fCollection.erase(fCollection.begin()+indexArr[i]);
      }
      return true;
    }

    std::string foundString = VD->getNameAsString();
    std::string functionString;
    if(!current_func_) { 
      functionString = "NULL_FUNC";
    }
    else {
      functionString = current_func_->getNameInfo().getAsString();    
    }

    //VarDecl of variable with in a FunctionDecl, can delete all matching these from the DRE list
    //Variables passed within the function get declared at the start of the function, so these are also matched against the DRE from the list
    int index = 0;
    std::vector<int> indexArr;
    for(auto itr : mCollection) {
      if(foundString == itr && functionString == fCollection[index]) {
        indexArr.push_back(index);
      }
      index +=1;
    }

    for(unsigned i = indexArr.size()-1; indexArr.size() > i; --i) {
      mCollection.erase(mCollection.begin()+indexArr[i]);
      fCollection.erase(fCollection.begin()+indexArr[i]);
    }
    return true;

  }

  //Not need anymore since only accumulating DRE that are within the LambdaExpr
  bool VisitFunctionDecl(FunctionDecl *FD) {
    if(!FD || !Context->getSourceManager().isWrittenInMainFile(FD->getLocation()))
      return true;
      
    std::string foundString = FD->getNameInfo().getAsString();
    int index = 0;
    std::vector<int> indexArr;
    for(auto itr : mCollection) {
      if(foundString == itr) {
        indexArr.push_back(index);
        break;
      }
      index +=1;
    }

    for(unsigned i = indexArr.size()-1; indexArr.size() > i; --i) {
      mCollection.erase(mCollection.begin()+indexArr[i]);
      fCollection.erase(fCollection.begin()+indexArr[i]);
    }
    return true;
  }

  bool VisitDecl(Decl *decl) {
    if(!decl || !Context->getSourceManager().isWrittenInMainFile(decl->getLocation()))
      return true;

    if(decl->isFunctionOrFunctionTemplate())
      current_func_ = decl->getAsFunction();
    return true;

  }

private :
  ASTContext *Context;
  FunctionDecl *current_func_ = nullptr;
  std::vector<std::string>& mCollection;
  std::vector<std::string>& fCollection;

};

class FindLambdaRefExpr : public RecursiveASTVisitor<FindLambdaRefExpr> {
public :
  explicit FindLambdaRefExpr(ASTContext *Context, std::vector<std::string>& collection, std::vector<std::string>& funcollection, std::vector<std::string>& current_func)
    : Context(Context), mCollection(collection), fCollection(funcollection), current_func_str(current_func) {}

  bool VisitDeclRefExpr(DeclRefExpr *DRE) {
    std::string foundString = DRE->getFoundDecl()->getNameAsString();
    mCollection.push_back(foundString);

    std::string functionDeclStr = current_func_str.back();
    fCollection.push_back(functionDeclStr);
    return true;
  }

private :
  ASTContext *Context;
  std::vector<std::string>& mCollection;
  std::vector<std::string>& fCollection;
  std::vector<std::string>& current_func_str;

};

class ScopeMaterializeList : public RecursiveASTVisitor<ScopeMaterializeList> {
public :
  explicit ScopeMaterializeList(ASTContext *Context, std::vector<std::string>& collection, std::vector<std::string>& funcollection, std::vector<std::string>& current)
    : Context(Context), mCollection(collection), fCollection(funcollection), current_func(current), MemberVisitor(Context,collection,funcollection,current) {}

  bool VisitLambdaExpr(LambdaExpr *LE) {
    if(!LE || !Context->getSourceManager().isWrittenInMainFile(LE->getCaptureDefaultLoc()))
      return true;

    const CompoundStmt* LambdaBody = LE->getBody();
    for(auto Stmt : LambdaBody->body()) {
      MemberVisitor.TraverseStmt(Stmt);
    }
    return true;

  }

  bool VisitDecl(Decl *decl) {
    if(!decl || !Context->getSourceManager().isWrittenInMainFile(decl->getLocation()))
      return true;

    if(decl->isFunctionOrFunctionTemplate()) {
      FunctionDecl *current_func_;
      current_func_ = decl->getAsFunction();
      std::string functionString;
      if(current_func_ == nullptr) {
        functionString = "NULL_FUNC";
      }
      else {
        functionString = current_func_->getNameInfo().getAsString();
      }
        current_func.push_back(functionString);
    }
    return true;
  }

private :
  ASTContext *Context;
  FindLambdaRefExpr MemberVisitor;
  std::vector<std::string>& mCollection;
  std::vector<std::string>& fCollection;
  std::vector<std::string>& current_func;
};

struct MessageDiagnostics : public RecursiveASTVisitor<MessageDiagnostics> {
public :
  explicit MessageDiagnostics(ASTContext *Context, std::vector<std::string>& collection, std::vector<std::string>& funcollection)
    : Context(Context), mCollection(collection), fCollection(funcollection) {}

  bool VisitDeclRefExpr(DeclRefExpr *DRE) {

    std::string refString = DRE->getFoundDecl()->getNameAsString();
    std::string functionString = current_func_->getNameInfo().getAsString();
    clang::DiagnosticsEngine &DE = Context->getDiagnostics();

    int index = 0;
    for(auto itr : mCollection) {
      //if(functionString == fCollection[index] && refString == mCollection[index]) {
      if(functionString == fCollection[index] && refString == itr) {
        auto ID = DE.getCustomDiagID(
          clang::DiagnosticsEngine::Error,
          "Found global variable that is not referenced in the global scope here.\n");
        DE.Report(DRE->getBeginLoc(),ID);

        ID = DE.getCustomDiagID(clang::DiagnosticsEngine::Remark,
          "Consider creating a local copy of the variable in local scope \njust outside the parallel_for.");
        DE.Report(Parent->getBeginLoc(),ID);
      }
      index += 1;
    }
    return true;
  }
  

  ASTContext *Context;
  LambdaExpr *Parent;
  FunctionDecl *current_func_;
  std::vector<std::string>& mCollection;
  std::vector<std::string>& fCollection;

};

class PrintErrorMessage : public RecursiveASTVisitor<PrintErrorMessage> {
public :
  explicit PrintErrorMessage(ASTContext *Context, std::vector<std::string>& collection, std::vector<std::string>& funcollection)
    : Context(Context), mCollection(collection), fCollection(funcollection), MemberVisitor(Context,collection,funcollection) {}

  bool VisitLambdaExpr(LambdaExpr *LE) {
    if(!LE || !Context->getSourceManager().isWrittenInMainFile(LE->getCaptureDefaultLoc()))
      return true;


    const CompoundStmt* LambdaBody = LE->getBody();
    if(LambdaBody->body_empty())
      return true;

    for(auto Stmt : LambdaBody->body()) {
      MemberVisitor.Parent = LE;
      MemberVisitor.current_func_ = current_func_;
      MemberVisitor.TraverseStmt(Stmt);
    }
    return true;
  }

  bool VisitDecl(Decl *decl) {
    if(!decl || !Context->getSourceManager().isWrittenInMainFile(decl->getLocation()))
      return true;

    //llvm::outs() << "GOTHERE\n";
    if(decl->isFunctionOrFunctionTemplate())
      current_func_ = decl->getAsFunction();
    return true;

  }
    
private :
  ASTContext *Context;
  FunctionDecl *current_func_ = nullptr;
  MessageDiagnostics MemberVisitor;
  std::vector<std::string>& mCollection;
  std::vector<std::string>& fCollection;
};

class ScopeCaptureCheckerConsumer_passvar : public clang::ASTConsumer {
public :
  explicit ScopeCaptureCheckerConsumer_passvar(ASTContext *Context, std::vector<std::string>& options, std::vector<std::string>& funoptions, std::vector<std::string>& current)
    : mOptions(options), fOptions(funoptions), current_func(current), Visitor1(Context,options,funoptions,current), Visitor2(Context,options,funoptions), Visitor3(Context,options,funoptions) {}
  explicit ScopeCaptureCheckerConsumer_passvar(CompilerInstance& CI, std::vector<std::string>& options, std::vector<std::string>& funoptions, std::vector<std::string>& current)
    : mOptions(options), fOptions(funoptions), current_func(current), Visitor1(&CI.getASTContext(),options,funoptions,current), Visitor2(&CI.getASTContext(),options,funoptions), Visitor3(&CI.getASTContext(),options,funoptions) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor1.TraverseDecl(Context.getTranslationUnitDecl());
    Visitor2.TraverseDecl(Context.getTranslationUnitDecl());
    if(!mOptions.empty()) {
      Visitor3.TraverseDecl(Context.getTranslationUnitDecl());
    }
  }
  
private:
  ScopeMaterializeList Visitor1; 
  FindScopeCapture Visitor2;
  PrintErrorMessage Visitor3;
  std::vector<std::string>& mOptions;
  std::vector<std::string>& fOptions;
  std::vector<std::string>& current_func;
};

