/*
 * Copyright 2012, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bcc/Assert.h"
#include "bcc/Renderscript/RSCompiler.h"
#include "bcc/Renderscript/RSTransforms.h"
#include "bcc/Renderscript/RSInfo.h"
#include "bcc/Renderscript/RSExecutable.h"
#include "bcc/Renderscript/RSScript.h"
#include "bcc/Source.h"
#include "bcc/Support/Log.h"
#include "bcc/Support/FileBase.h"

#include "bcc/Renderscript/RSVectorizationSupport.h"
#include "bcc/Renderscript/RSVectorization.h"

#ifdef HAVE_ANDROID_OS
#include <cutils/properties.h>
#endif

#include <llvm/Analysis/Passes.h>
#include <llvm/DebugInfo.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Pass.h>
#include <llvm/PassManager.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/Casting.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include <cstdlib>
#include <cstring>
#include <new>
#include <vector>
#include <utils/String8.h>
#include <map>

// TODO[MA]: remove me
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

// begin - x86-vectorizer library api
namespace intel {
class OptimizerConfig;
}

extern "C" intel::OptimizerConfig* createRenderscriptConfiguration(int width);
extern "C" void deleteRenderscriptConfiguration(intel::OptimizerConfig*& pConfig);

extern "C" void* createRenderscriptRuntimeSupport(const llvm::Module *runtimeModule);
extern "C" void* destroyRenderscriptRuntimeSupport();

extern "C" llvm::Pass *createRenderscriptVectorizerPass(const llvm::Module *runtimeModule,
  const intel::OptimizerConfig* pConfig,
  llvm::SmallVectorImpl<llvm::Function*> &optimizerFunctions,
  llvm::SmallVectorImpl<int> &optimizerWidths);

extern "C" void* createShuffleCallToInstPass();
extern "C" void* createPreventDivisionCrashesPass();
// end   - x86-vectorizer library api

using namespace bcc;

/// -- DBG Utilities
// These functions used for debugging
void RSVectorizationSupport::dumpDebugPoint(const char* tag, const char* title) {
  std::string stag = tag;
  std::string stitle = title;
  dumpDebugPoint(stag, stitle);
}

void RSVectorizationSupport::dumpDebugPoint(std::string tag, std::string title) {
  static unsigned int dbg_counter = 0;
  std::stringstream fileName;
  fileName << "/sdcard/RS_DBGP_" << tag << "_" << title << "." << dbg_counter;
  std::ofstream out(fileName.str().c_str());
  ++dbg_counter;
  out.flush();
}

void RSVectorizationSupport::dumpModule(const char* tag,
  const char* title,
  llvm::Module& module) {
  std::string stag = tag;
  std::string stitle = title;
  dumpModule(stag, stitle, module);
}

void RSVectorizationSupport::dumpModule(std::string tag,
  std::string title,
  llvm::Module& module) {
  static unsigned int pre_counter = 0;
  std::string buffer;
  llvm::raw_string_ostream stream(buffer);
  std::stringstream fileName;
  fileName << "/sdcard/RS_" << tag << "_" << title << "_" << pre_counter << ".ll";
  std::ofstream out(fileName.str().c_str());
  stream << module;
  ++pre_counter;
  stream.flush();
  out << buffer;
  out.flush();
}
/// -- DBG Utilities

// TODO[MA]: remove this doublicate (need utils ?)

static bool hasIn(uint32_t Signature) {
  return Signature & 0x01;
}

static bool hasOut(uint32_t Signature) {
  return Signature & 0x02;
}

static bool hasUsrData(uint32_t Signature) {
  return Signature & 0x04;
}

static bool hasX(uint32_t Signature) {
  return Signature & 0x08;
}

static bool hasY(uint32_t Signature) {
  return Signature & 0x10;
}

static bool isKernel(uint32_t Signature) {
  return Signature & 0x20;
}
// ODOT[MA]

const char* g_NoInlineBuiltins[] = {

  "_Z5clampDv4_fS_S_",
  "_Z5clampDv3_fS_S_",
  "_Z5clampDv2_fS_S_",
  "_Z5clampfff",

  "_Z5clampDv4_fff",
  "_Z5clampDv3_fff",
  "_Z5clampDv2_fff",
  "_Z5clampfff",

  "_Z4acosf",
  "_Z4acosDv2_f",
  "_Z4acosDv4_f",
  "_Z4acosDv3_f",

  "_Z5acoshf",
  "_Z5acoshDv2_f",
  "_Z5acoshDv4_f",
  "_Z5acoshDv3_f",

  "_Z6acospif",
  "_Z6acospiDv2_f",
  "_Z6acospiDv4_f",
  "_Z6acospiDv3_f",

  "_Z4asinf",
  "_Z4asinDv2_f",
  "_Z4asinDv4_f",
  "_Z4asinDv3_f",

  "_Z5asinhf",
  "_Z5asinhDv2_f",
  "_Z5asinhDv4_f",
  "_Z5asinhDv3_f",

  "_Z6asinpif",
  "_Z6asinpiDv2_f",
  "_Z6asinpiDv4_f",
  "_Z6asinpiDv3_f",

  "_Z4atanf",
  "_Z4atanDv2_f",
  "_Z4atanDv4_f",
  "_Z4atanDv3_f",

  "_Z5atan2ff",
  "_Z5atan2Dv2_fS_",
  "_Z5atan2Dv4_fS_",
  "_Z5atan2Dv3_fS_",

  "_Z5atanhf",
  "_Z5atanhDv2_f",
  "_Z5atanhDv4_f",
  "_Z5atanhDv3_f",

  "_Z6atanpif",
  "_Z6atanpiDv2_f",
  "_Z6atanpiDv4_f",
  "_Z6atanpiDv3_f",

  "_Z7atan2piff",
  "_Z7atan2piDv2_fS_",
  "_Z7atan2piDv4_fS_",
  "_Z7atan2piDv3_fS_",

  "_Z4cbrtf",
  "_Z4cbrtDv2_f",
  "_Z4cbrtDv4_f",
  "_Z4cbrtDv3_f",

  "_Z4ceilf",
  "_Z4ceilDv2_f",
  "_Z4ceilDv4_f",
  "_Z4ceilDv3_f",

  "_Z8copysignff",
  "_Z8copysignDv2_fS_",
  "_Z8copysignDv4_fS_",
  "_Z8copysignDv3_fS_",

  "_Z3cosf",
  "_Z3cosDv2_f",
  "_Z3cosDv4_f",
  "_Z3cosDv3_f",

  "_Z4coshf",
  "_Z4coshDv2_f",
  "_Z4coshDv4_f",
  "_Z4coshDv3_f",

  "_Z5cospif",
  "_Z5cospiDv2_f",
  "_Z5cospiDv4_f",
  "_Z5cospiDv3_f",

  "_Z4erfcf",
  "_Z4erfcDv2_f",
  "_Z4erfcDv4_f",
 "_Z4erfcDv3_f",

  "_Z3erff",
  "_Z3erfDv2_f",
  "_Z3erfDv4_f",
  "_Z3erfDv3_f",

  "_Z3expf",
  "_Z3expDv2_f",
  "_Z3expDv4_f",
  "_Z3expDv3_f",

  "_Z4exp2f",
  "_Z4exp2Dv2_f",
  "_Z4exp2Dv4_f",
  "_Z4exp2Dv3_f",

  "_Z5exp10f",
  "_Z5exp10Dv2_f",
  "_Z5exp10Dv4_f",
  "_Z5exp10Dv3_f",

  "_Z5expm1f",
  "_Z5expm1Dv2_f",
  "_Z5expm1Dv4_f",
  "_Z5expm1Dv3_f",

  "_Z4fabsf",
  "_Z4fabsDv2_f",
  "_Z4fabsDv4_f",
  "_Z4fabsDv3_f",

  "_Z4fdimff",
  "_Z4fdimDv2_fS_",
  "_Z4fdimDv4_fS_",
  "_Z4fdimDv3_fS_",

  "_Z5floorf",
  "_Z5floorDv2_f",
  "_Z5floorDv4_f",
  "_Z5floorDv3_f",

  "_Z3fmafff",
  "_Z3fmaDv2_fS_S_",
  "_Z3fmaDv4_fS_S_",
  "_Z3fmaDv3_fS_S_",

  "_Z4fmodff",
  "_Z4fmodDv2_fS_",
  "_Z4fmodDv4_fS_",
  "_Z4fmodDv3_fS_",

  "_Z5fractfPf",
  "_Z5fractDv2_fPS_",
  "_Z5fractDv4_fPS_",
  "_Z5fractDv3_fPS_",

  "_Z5frexpfPi",
  "_Z5frexpDv2_fPDv2_i",
  "_Z5frexpDv4_fPDv4_i",
  "_Z5frexpDv3_fPDv3_i",

  "_Z5hypotff",
  "_Z5hypotDv2_fS_",
  "_Z5hypotDv4_fS_",
  "_Z5hypotDv3_fS_",

  "_Z5ilogbf",
  "_Z5ilogbDv2_f",
  "_Z5ilogbDv4_f",
  "_Z5ilogbDv3_f",

  "_Z5ldexpfi",
  "_Z5ldexpDv2_fDv2_i",
  "_Z5ldexpDv4_fDv4_i",
  "_Z5ldexpDv3_fDv3_i",

  "_Z5ldexpfi",
  "_Z5ldexpDv2_fi",
  "_Z5ldexpDv4_fi",
  "_Z5ldexpDv3_fi",

  "_Z6lgammaf",
  "_Z6lgammaDv2_f",
  "_Z6lgammaDv4_f",
  "_Z6lgammaDv3_f",

  "_Z6lgammafPi",
  "_Z6lgammaDv2_fPDv2_i",
  "_Z6lgammaDv4_fPDv4_i",
  "_Z6lgammaDv3_fPDv3_i",

  "_Z3logf",
  "_Z3logDv2_f",
  "_Z3logDv4_f",
  "_Z3logDv3_f",

  "_Z5log10f",
  "_Z5log10Dv2_f",
  "_Z5log10Dv4_f",
  "_Z5log10Dv3_f",

  "_Z4log2f",
  "_Z4log2Dv2_f",
  "_Z4log2Dv4_f",
  "_Z4log2Dv3_f",

  "_Z5log1pf",
  "_Z5log1pDv2_f",
  "_Z5log1pDv4_f",
  "_Z5log1pDv3_f",

  "_Z4logbf",
  "_Z4logbDv2_f",
  "_Z4logbDv4_f",
  "_Z4logbDv3_f",

  "_Z4modffPf",
  "_Z4modfDv2_fPS_",
  "_Z4modfDv4_fPS_",
  "_Z4modfDv3_fPS_",

  "_Z9nextafterff",
  "_Z9nextafterDv2_fS_",
  "_Z9nextafterDv4_fS_",
  "_Z9nextafterDv3_fS_",

  "_Z3powff",
  "_Z3powDv2_fS_",
  "_Z3powDv4_fS_",
  "_Z3powDv3_fS_",

  "_Z4pownfi",
  "_Z4pownDv2_fDv2_i",
  "_Z4pownDv4_fDv4_i",
  "_Z4pownDv3_fDv3_i",

  "_Z4powrff",
  "_Z4powrDv2_fS_",
  "_Z4powrDv4_fS_",
  "_Z4powrDv3_fS_",

  "_Z9remainderff",
  "_Z9remainderDv2_fS_",
  "_Z9remainderDv4_fS_",
  "_Z9remainderDv3_fS_",

  "_Z6remquoffPi",
  "_Z6remquoDv2_fS_PDv2_i",
  "_Z6remquoDv4_fS_PDv4_i",
  "_Z6remquoDv3_fS_PDv3_i",

  "_Z4rintf",
  "_Z4rintDv2_f",
  "_Z4rintDv4_f",
  "_Z4rintDv3_f",

  "_Z5rootnfi",
  "_Z5rootnDv2_fDv2_i",
  "_Z5rootnDv4_fDv4_i",
  "_Z5rootnDv3_fDv3_i",

  "_Z5roundf",
  "_Z5roundDv2_f",
  "_Z5roundDv4_f",
  "_Z5roundDv3_f",

  "_Z5rsqrtf",
  "_Z5rsqrtDv2_f",
  "_Z5rsqrtDv4_f",
  "_Z5rsqrtDv3_f",

  "_Z3sinf",
  "_Z3sinDv2_f",
  "_Z3sinDv4_f",
  "_Z3sinDv3_f",

  "_Z6sincosfPf",
  "_Z6sincosDv2_fPS_",
  "_Z6sincosDv4_fPS_",
  "_Z6sincosDv3_fPS_",

  "_Z4sinhf",
  "_Z4sinhDv2_f",
  "_Z4sinhDv4_f",
  "_Z4sinhDv3_f",

  "_Z5sinpif",
  "_Z5sinpiDv2_f",
  "_Z5sinpiDv4_f",
  "_Z5sinpiDv3_f",

  "_Z3tanf",
  "_Z3tanDv2_f",
  "_Z3tanDv4_f",
  "_Z3tanDv3_f",

  "_Z4tanhf",
  "_Z4tanhDv2_f",
  "_Z4tanhDv4_f",
  "_Z4tanhDv3_f",

  "_Z5tanpif",
  "_Z5tanpiDv2_f",
  "_Z5tanpiDv4_f",
  "_Z5tanpiDv3_f",

  "_Z6tgammaf",
  "_Z6tgammaDv2_f",
  "_Z6tgammaDv4_f",
  "_Z6tgammaDv3_f",

  "_Z5truncf",
  "_Z5truncDv2_f",
  "_Z5truncDv4_f",
  "_Z5truncDv3_f",

  "_Z3absc",
  "_Z3absDv2_c",
  "_Z3absDv4_c",
  "_Z3absDv3_c",

  "_Z3abss",
  "_Z3absDv2_s",
  "_Z3absDv4_s",
  "_Z3absDv3_s",

  "_Z3absi",
  "_Z3absDv2_i",
  "_Z3absDv4_i",
  "_Z3absDv3_i",

  "_Z3clzh",
  "_Z3clzDv2_h",
  "_Z3clzDv4_h",
  "_Z3clzDv3_h",

  "_Z3clzc",
  "_Z3clzDv2_c",
  "_Z3clzDv4_c",
  "_Z3clzDv3_c",

  "_Z3clzt",
  "_Z3clzDv2_t",
  "_Z3clzDv4_t",
  "_Z3clzDv3_t",

  "_Z3clzs",
  "_Z3clzDv2_s",
  "_Z3clzDv4_s",
  "_Z3clzDv3_s",

  "_Z3clzj",
  "_Z3clzDv2_j",
  "_Z3clzDv4_j",
  "_Z3clzDv3_j",

  "_Z3clzi",
  "_Z3clzDv2_i",
  "_Z3clzDv4_i",
  "_Z3clzDv3_i",

  "_Z9half_sqrtf",
  "_Z9half_sqrtDv2_f",
  "_Z9half_sqrtDv4_f",
  "_Z9half_sqrtDv3_f",

  "_Z10half_rsqrtf",
  "_Z10half_rsqrtDv2_f",
  "_Z10half_rsqrtDv4_f",
  "_Z10half_rsqrtDv3_f",

  "_Z4sqrtf",
  "_Z4sqrtDv2_f",
  "_Z4sqrtDv4_f",
  "_Z4sqrtDv3_f",

  "_Z3dotff",
  "_Z3dotDv2_fS_",
  "_Z3dotDv4_fS_",
  "_Z3dotDv3_fS_"

  };

unsigned int g_NoInlineBuiltinsNum = sizeof(g_NoInlineBuiltins) / sizeof(char*);

/**
 * rs vectorizer is default turned on.
 * turn off if rs.x86vectorizer.disable is set to 1 or true.
 */
bool RSVectorizationSupport::isVectorizerEnabled() {
#ifdef HAVE_ANDROID_OS
  char buf[PROPERTY_VALUE_MAX];
  property_get("rs.x86vectorizer.disable", buf, "0");
  if ((::strcmp(buf, "1") == 0) || (::strcmp(buf, "true") == 0))
    return false;
  else
    return true;
#else
  return true;
#endif
}

/**
 * constract the get id function body
 */
bool RSVectorizationSupport::constructGetIdBody(llvm::Module* M,
  llvm::Function* F) {
  bccAssert(M);
  bccAssert(F);

  // Construct the actual function body.
  llvm::BasicBlock *BB =
    llvm::BasicBlock::Create(M->getContext(), "", F);
  llvm::IRBuilder<> Builder(BB);

  // return 0
  llvm::Value *idVal = llvm::ConstantInt::get(
                         llvm::Type::getInt32Ty(M->getContext()), 0);
  Builder.CreateRet(idVal);

  return true;
}

/**
 * creates get id function in the module; this function will serve as the
 * induction variable function for the vectorizer
 */
bool RSVectorizationSupport::createGetIdFunction(llvm::Module* M,
  llvm::Function*& F) {
  bccAssert(M);
  llvm::SmallVector<llvm::Type*, 8> ParamTys;
  llvm::FunctionType *FT =
      llvm::FunctionType::get(llvm::Type::getInt32Ty(M->getContext()), ParamTys
      , false);

  // TODO[MA]: BUG what if get.id is used in the module?
  F = llvm::Function::Create(FT,
                             llvm::GlobalValue::ExternalLinkage,
                             "get.id", M);

  // Construct the actual function body.
  return constructGetIdBody(M, F);
}

/**
 * clones the given function signture
 */
llvm::Function* RSVectorizationSupport::cloneFunctionSingature(llvm::Function *F,
  std::string clonedFunctionName) {
  llvm::SmallVector<llvm::Type*, 8> ParamTys;
  bccAssert(F);

  // get the argument types of the cloned function
  for (llvm::Function::const_arg_iterator B = F->arg_begin(), E = F->arg_end();
         B != E;
         ++B) {
    ParamTys.push_back(B->getType());
  }

  // create the new function type
  llvm::FunctionType *FTy = llvm::FunctionType::get(
                              F->getFunctionType()->getReturnType(),
                              ParamTys, F->getFunctionType()->isVarArg());

  // create the new function
  std::string clonedName = clonedFunctionName.size() ? clonedFunctionName :
                          F->getName().str() + ".clone";
  llvm::Function *NewF = llvm::Function::Create(FTy, F->getLinkage(),
                         clonedName, F->getParent());

  // set the names of the args the same as the cloned function
  llvm::Function::arg_iterator NewArgIt = NewF->arg_begin();
  for (llvm::Function::const_arg_iterator B = F->arg_begin(),
                                      E = F->arg_end();
         B != E;
         ++B) {
    NewArgIt->setName(B->getName());
    ++NewArgIt;
  }

  return NewF;
}

/**
 * marks all the called functions from the given function F to be inlined
 */
bool RSVectorizationSupport::markToInlineCalledFunctions(llvm::Function* F) {
  std::vector<llvm::Function*> toProcess, Processed;
  toProcess.push_back(F);

  while(toProcess.size() != 0) {
    llvm::Function* pF = toProcess.back();
    toProcess.pop_back();
    Processed.push_back(pF);

    for(llvm::Function::iterator bb = pF->begin(), bbe = pF->end();
        bb != bbe; ++bb) {
      for(llvm::BasicBlock::iterator i = bb->begin(), e = bb->end();
          i != e; ++i) {
        if (llvm::CallInst* callInst = llvm::dyn_cast<llvm::CallInst>(&*i)) {
          // check if the call is for internal function and mark it for inlining
          llvm::Module* M = F->getParent();
          llvm::Function* called = callInst->getCalledFunction();
          if (NULL != called && NULL != M->getFunction(called->getName())) {
            called->addFnAttr(llvm::Attribute::AlwaysInline);
            // if it's a new function then add it to the toProcess list
            bool newItem = true;
            for(std::vector<llvm::Function*>::iterator it = Processed.begin();
                it != Processed.end(); ++it) {
              if(*it == called) {
                newItem = false;
                break;
              }
            }
            if(true == newItem) toProcess.push_back(called);
          }
        }
      }
    }
  }
  F->addFnAttr(llvm::Attribute::AlwaysInline);
  return true;
}

/**
 * creates a clone of the given function with induction variable use.
 */
llvm::Function* RSVectorizationSupport::createIndexedFunction(llvm::Function *F,
  unsigned int Signature, llvm::Function* idFunc) {
  bccAssert(F);
  bccAssert(idFunc);

  // TODO[MA]: maybe need to check if there is a recursive and roll-back

  // add always inline attribute for the orignal function to get it inlined
  // in the indexed version - assumption no recursive calls
  RSVectorizationSupport::markToInlineCalledFunctions(F);

  // clone the signture of the original function
  std::string clonedName = F->getName().str() + ".indexed";
  llvm::Function *indexedFunction = cloneFunctionSingature(F, clonedName);
  if(NULL == indexedFunction) return NULL;

  llvm::BasicBlock *BB = NULL;
  BB = llvm::BasicBlock::Create(indexedFunction->getContext(),
    "init", indexedFunction);

  // Create and name the actual arguments to this indexed function.
  llvm::SmallVector<llvm::Argument*, 8> ArgVec;
  for (llvm::Function::arg_iterator B = indexedFunction->arg_begin(),
                                    E = indexedFunction->arg_end();
       B != E;
       ++B) {
    ArgVec.push_back(B);
  }

  // Construct the actual function body.
  llvm::IRBuilder<> Builder(BB);

  bccAssert(idFunc);
  // Populate the actual call to get id
  llvm::SmallVector<llvm::Value*, 8> idArgs;
  llvm::Value* itemID = Builder.CreateCall(idFunc, idArgs);

  // call the original function with the item id with the relevant arguments
  unsigned int argCounter = 0;
  llvm::SmallVector<llvm::Value*, 8> RootArgs;
  if (hasIn(Signature)) {
    llvm::Value* InPtr =
      Builder.CreateInBoundsGEP(ArgVec[argCounter++], itemID, "inElement");
    RootArgs.push_back(InPtr);
  }

  if (hasOut(Signature)) {
    llvm::Value* OutPtr =
      Builder.CreateInBoundsGEP(ArgVec[argCounter++], itemID, "outElement");
    RootArgs.push_back(OutPtr);
  }

  if (hasUsrData(Signature)) {
    RootArgs.push_back(ArgVec[argCounter++]);
  }

  if (hasX(Signature)) {
    llvm::Value* currX =
      Builder.CreateAdd(ArgVec[argCounter++], itemID, "currentX");
    RootArgs.push_back(currX);
  }

  if (hasY(Signature)) {
    RootArgs.push_back(ArgVec[argCounter++]);
  }

  bccAssert(argCounter == ArgVec.size());
  // Populate the actual call to the root
  if(F->getReturnType()->isVoidTy()) {
    Builder.CreateCall(F, RootArgs);
    Builder.CreateRetVoid();
  }
  else {
    llvm::Value* retVal = Builder.CreateCall(F, RootArgs);
    Builder.CreateRet(retVal);
  }

  return indexedFunction;
}

llvm::Function* RSVectorizationSupport::createKernelWrapper(llvm::Function* F,
  unsigned int Signture) {
  bccAssert(F);
  llvm::SmallVector<llvm::Type*, 8> WrapperParamTys;

  // add always inline attribute for the orignal function to get it inlined
  // in the indexed version
  F->addFnAttr(llvm::Attribute::AlwaysInline);

  llvm::Function::const_arg_iterator B = F->arg_begin();
  llvm::Function::const_arg_iterator E = F->arg_end();

  if (hasOut(Signture)) {
    llvm::Type *OutBaseTy = F->getReturnType();
    if (OutBaseTy->isVoidTy()) {
      // means the kernel already passing it's out by reference
      bccAssert(B != E);
      WrapperParamTys.push_back(B->getType());
      B++;
    } else {
      // we put the return type as the first argument and move it by reference
      WrapperParamTys.push_back(OutBaseTy->getPointerTo());
    }
  }

  if (hasIn(Signture)) {
    // need to get in argument by reference
    bccAssert(B != E);
    llvm::Type *InBaseTy = B->getType();
    llvm::Type *InTy =InBaseTy->getPointerTo();
    // notice that need to put "in" param at first regardless of the out param
    WrapperParamTys.insert(WrapperParamTys.begin(), InTy);
    B++;
  }

  // move over all the remianed argument types of the wrapped function
  for (;
         B != E;
         ++B) {
    WrapperParamTys.push_back(B->getType());
  }

  // create the new function type
  llvm::FunctionType *FTy = llvm::FunctionType::get(
                              llvm::Type::getVoidTy(F->getParent()->getContext()),
                              WrapperParamTys, F->getFunctionType()->isVarArg());

  // create the new function
  llvm::Function *WrapperF = llvm::Function::Create(FTy, F->getLinkage(),
                         F->getName().str() + ".wrapper", F->getParent());

  // list the actual arguments to the orginal function.
  llvm::SmallVector<llvm::Argument*, 8> ArgVec;
  for (llvm::Function::arg_iterator B = F->arg_begin(),
                                    E = F->arg_end();
       B != E;
       ++B) {
    ArgVec.push_back(B);
  }

  // set the names of the args the same as the wrapped function
  llvm::Function::arg_iterator WrapperArgIt = WrapperF->arg_begin();
  B = F->arg_begin();
  E = F->arg_end();

  // these are the indices for in\out in the original F
  int inIndex = (hasOut(Signture) && F->getReturnType()->isVoidTy())
      ? 1 : 0;
  int outIndex = (hasOut(Signture) && F->getReturnType()->isVoidTy())
      ? 0 : -1;
  if (hasIn(Signture)) {
    // just adding "p" prefix for the in argument
    bccAssert(B != E);
    std::string inName = "p" + ArgVec[inIndex]->getName().str();
    WrapperArgIt->setName(inName);
    B++;
    WrapperArgIt++;
  }

  if (hasOut(Signture)) {
    llvm::Type *OutBaseTy = F->getReturnType();
    if (OutBaseTy->isVoidTy()) {
      // means the kernel passing it's out by reference
      bccAssert(B != E);
      WrapperArgIt->setName(ArgVec[outIndex]->getName());
      B++;
      WrapperArgIt++;
    } else {
      // BUG: if the name already in use, just keep it debug feature
      //WrapperArgIt->setName("out");
      WrapperArgIt++;
    }
  }

  // get the names for the other arguments
  for (;
         B != E;
         ++B) {
    WrapperArgIt->setName(B->getName());
    ++WrapperArgIt;
  }

  // final step: build the wrapper function body

  // list the actual arguments to the wrapper function.
  llvm::SmallVector<llvm::Argument*, 8> WrapperArgVec;
  for (llvm::Function::arg_iterator B = WrapperF->arg_begin(),
                                    E = WrapperF->arg_end();
       B != E;
       ++B) {
    WrapperArgVec.push_back(B);
  }

  llvm::BasicBlock *BB =
      llvm::BasicBlock::Create(WrapperF->getParent()->getContext(),
      "init", WrapperF);
  llvm::IRBuilder<> Builder(BB);

  // call the original function with the relevant arguments
  int wrapperInIndex = (hasIn(Signture)) ? 0 : -1;
  int wrapperOutIndex = (hasIn(Signture)) ? 1 : 0;

  unsigned int argCounter = 0;
  llvm::SmallVector<llvm::Value*, 8> CallArgs;
  bool outInRet = false;
  if (hasOut(Signture)) {
    llvm::Type *OutBaseTy = F->getReturnType();
    if (OutBaseTy->isVoidTy()) {
      // means the kernel passing it's out by reference
      CallArgs.push_back(WrapperArgVec[wrapperOutIndex]);
      argCounter++;
    } else {
      argCounter++;
      outInRet = true;
    }
  }

  if (hasIn(Signture)) {
    llvm::Value* InValue =
      Builder.CreateLoad(WrapperArgVec[wrapperInIndex], "Input");
    argCounter++;
    CallArgs.push_back(InValue);
  }

  // pass other arguments directly
  for(unsigned int i = argCounter; i < WrapperF->arg_size(); ++i) {
    CallArgs.push_back(WrapperArgVec[argCounter++]);
  }

  // Populate the actual call to the root
  llvm::Value* retVal = Builder.CreateCall(F, CallArgs);
  if(outInRet) {
    Builder.CreateStore(retVal, WrapperArgVec[wrapperOutIndex]);
  }
  Builder.CreateRetVoid();
  return WrapperF;
}

#pragma GCC push_options
#pragma GCC optimize ("-O0")

bool RSVectorizationSupport::prepareModuleForVectorization(const RSInfo *info,
  llvm::Module* M) {
  bccAssert(info);
  bccAssert(M);

  // in case there is no exported foreach functions no need to do anything with
  // the given module
  RSInfo::ExportForeachFuncListTy::const_iterator it_begin =
    info->getExportForeachFuncs().begin();
  RSInfo::ExportForeachFuncListTy::const_iterator it_end   =
    info->getExportForeachFuncs().end();
  if (it_begin == it_end) return false;

  // this named metadata will contain all the indexed functions
  // to be used by the vectorizer
  llvm::StringRef rsIndexedKernelsMD = "rs.indexed.kernels";
  llvm::NamedMDNode* kernels = M->getOrInsertNamedMetadata(rsIndexedKernelsMD);

  // first step need to create get id function in order to use it as item-id
  llvm::Function* idFunc = NULL;
  createGetIdFunction(M, idFunc);
  bccAssert(idFunc);

  // iterate over all the exported foreach functions list and make indexed
  // version for each one of them
  const RSInfo::ExportForeachFuncListTy &foreach_func =
      info->getExportForeachFuncs();
  for (RSInfo::ExportForeachFuncListTy::const_iterator
             func_iter = foreach_func.begin(), func_end = foreach_func.end();
         func_iter != func_end; func_iter++) {
    const char *name = func_iter->first;
    unsigned int signature = func_iter->second;
    llvm::Function *kernel = M->getFunction(name);

    llvm::Function* indexedFunc = NULL;

    if (kernel && isKernel(signature)) {
      llvm::Function* wrapperF = createKernelWrapper(kernel, signature);
      indexedFunc = createIndexedFunction(wrapperF, signature, idFunc);
    }
    else if (kernel && kernel->getReturnType()->isVoidTy()) {
      // NOTE: this pass doesn't support getRootSignture yet
      if(signature)
        indexedFunc = createIndexedFunction(kernel, signature, idFunc);
    }

    if (NULL != indexedFunc) {
      // pass the indexed function ptr to the vectorizer
      llvm::ArrayRef<llvm::Value*> values(indexedFunc);
      llvm::MDNode* Node = llvm::MDNode::get(M->getContext(), values);
      kernels->addOperand(Node);
    }
  }

  doFunctionPreOptimizations(M);
  inlineFunctions(M);

  return true;
}

#pragma GCC pop_options

void RSVectorizationSupport::doFunctionPreOptimizations(llvm::Module* M) {
  llvm::FunctionPassManager fpm = llvm::FunctionPassManager(M);
  void* functionOptimizationPass = createShuffleCallToInstPass();

  fpm.add((llvm::Pass*)functionOptimizationPass);

  for(llvm::Module::iterator it = M->begin(); it != M->end(); ++it) {
    if(it->isDeclaration()) continue;
    fpm.run(*it);
  }
}

void RSVectorizationSupport::doFunctionPostOptimizations(llvm::Module* M) {
  llvm::FunctionPassManager fpm = llvm::FunctionPassManager(M);
  void* functionOptimizationPass = createPreventDivisionCrashesPass();

  fpm.add((llvm::Pass*)functionOptimizationPass);

  for(llvm::Module::iterator it = M->begin(); it != M->end(); ++it) {
    if(it->isDeclaration()) continue;
    fpm.run(*it);
  }
}

llvm::Function* RSVectorizationSupport::SearchForWrapperFunction(const
  llvm::Module& M, llvm::Function* kernel) {
  bccAssert(kernel);
  // need to search for the wrapper function prototype for the given kernel
  std::string suffix = ".wrapper";
  std::string functionName = kernel->getName();
  std::string wrapperName = functionName + suffix;
  return M.getFunction(wrapperName);
}

llvm::Function* RSVectorizationSupport::SearchForVectorizedKernel( const
  llvm::Module& M, llvm::Function* kernel) {
  // need to search for the vectorized version of the specified kernel
  // notice that if we have kernel foo we generate a tuned version foo.indexed
  // and our vectorized version will be called __Vectorized_.foo.indexed
  std::string prefix = "__Vectorized_.";
  std::string suffix = ".indexed";
  std::string kernelName = kernel->getName();
  std::string vectorizedName = prefix + kernelName + suffix;

  return M.getFunction(vectorizedName);
}

bool RSVectorizationSupport::inlineFunctions(llvm::Module* M) {
  bccAssert(M);
  std::string idFunctionName = "get.id";
  llvm::Function* idFunc = M->getFunction(idFunctionName);

  if(idFunc) {
    idFunc->deleteBody();
  }

  // maps the duplicated functions to the orginal functions
  std::map<llvm::Function*,llvm::Function*> duplicateFunctionsMap;

  // point all the NoInline function list to some empty duplicate
  // and we will return them back once we finish inlining
  for(unsigned int i = 0; i < g_NoInlineBuiltinsNum; ++i) {
    llvm::Function* F = M->getFunction(g_NoInlineBuiltins[i]);
    if(NULL!= F && false == F->use_empty()) {
      // Clone the function, so that we can hack away on it.
      llvm::Function* duplicateFunction = cloneFunctionSingature(F, "");

      F->replaceAllUsesWith(duplicateFunction);
      duplicateFunction->deleteBody();

      duplicateFunctionsMap[duplicateFunction] = F;
    }
  }

  llvm::PassManager trans_passes;
  trans_passes.add(llvm::createAlwaysInlinerPass());
  trans_passes.add(llvm::createFunctionInliningPass(4096));
  trans_passes.run(*M);

  std::map<llvm::Function*,llvm::Function*>::iterator it;
  for(it = duplicateFunctionsMap.begin();
      it != duplicateFunctionsMap.end(); ++it) {
    llvm::Function* hackedF  = it->first;
    llvm::Function* orginalF = it->second;

    hackedF->replaceAllUsesWith(orginalF);
  }

  if(idFunc) {
    constructGetIdBody(M, idFunc);
  }

  return true;
}

bool RSVectorizationSupport::vectorizeModule(llvm::Module& M,
  llvm::SmallVector<llvm::Function*, 4>& vectorizedFunctions,
  llvm::SmallVector<int, 4>& vectorizedWidths) {
  llvm::PassManager passes;

  // shortcut if there is no indexed kernels to vectorize
  llvm::NamedMDNode *KernelsMD = M.getNamedMetadata("rs.indexed.kernels");
  if (NULL == KernelsMD || 0 == KernelsMD->getNumOperands()) return false;

#ifdef __VECTORIZER_HUERISTIC
  int width = 0;
#else
  int width = 4;
#endif

  intel::OptimizerConfig* vectorizerConfig = createRenderscriptConfiguration(width);
  llvm::LLVMContext &BuiltinsContext = M.getContext();

  llvm::SMDiagnostic Err;
  std::auto_ptr<llvm::Module> BuiltinsModuleAuto;
  const char* core_lib = RSInfo::GetCLCorePath();

  llvm::Module* builtinsModule = llvm::ParseIRFile(RSInfo::GetCLCorePath(), Err, BuiltinsContext);
  if(NULL == builtinsModule) {
    ALOGE("Vectorizer driver failed to find/parse x86 built-ins library '%s'"
          , core_lib);
    return false;
  }

  BuiltinsModuleAuto.reset(builtinsModule);
  createRenderscriptRuntimeSupport(BuiltinsModuleAuto.get());

  // as a first integration phase inline the built-ins
  RSVectorizationSupport::inlineFunctions(&M);

  // calling the vectorizer on the script module
  llvm::Pass *vectorizerPass = createRenderscriptVectorizerPass(BuiltinsModuleAuto.get(),
      vectorizerConfig,
      vectorizedFunctions,
      vectorizedWidths);

  passes.add(vectorizerPass);
  passes.run(M);

  deleteRenderscriptConfiguration(vectorizerConfig);
  destroyRenderscriptRuntimeSupport();

  doFunctionPostOptimizations(&M);
  return true;
}
