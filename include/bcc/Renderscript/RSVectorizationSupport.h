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

#ifndef BCC_RS_VECTORIZATION_SUPPORT_H
#define BCC_RS_VECTORIZATION_SUPPORT_H

#include "RSVectorization.h"
#include "llvm/ADT/SmallVector.h"

bool vectorizer_enabled();

namespace llvm {
  class Module;
  class Function;
};

namespace bcc {

class RSInfo;

class RSVectorizationSupport {
public:
  static bool isVectorizerEnabled();
  static bool prepareModuleForVectorization(const RSInfo *info, llvm::Module* M);
  static llvm::Function* SearchForWrapperFunction(const llvm::Module& M,
         llvm::Function* kernel);
  static llvm::Function* SearchForVectorizedKernel( const llvm::Module& M,
         llvm::Function* kernel);
  static bool inlineFunctions(llvm::Module* M);
  static bool vectorizeModule(llvm::Module& M,
         llvm::SmallVector<llvm::Function*, 4>& vectorizedFunctions,
         llvm::SmallVector<int, 4>& vectorizedWidths);

  /// for debug usage
  static void dumpDebugPoint(const char* tag, const char* title);
  static void dumpDebugPoint(std::string tag, std::string title);
  static void dumpModule(const char* tag, const char* title, llvm::Module& module);
  static void dumpModule(std::string tag, std::string title, llvm::Module& module);
private:
  static bool markToInlineCalledFunctions(llvm::Function* F);
  static bool constructGetIdBody(llvm::Module* M, llvm::Function* F);
  static bool createGetIdFunction(llvm::Module* M, llvm::Function*& F);
  static llvm::Function* cloneFunctionSingature(llvm::Function *F,
         std::string clonedFunctionName);
  static llvm::Function* createIndexedFunction(llvm::Function *F, unsigned int
         Signature, llvm::Function* idFunc);
  static llvm::Function* createKernelWrapper(llvm::Function* F, unsigned int
         Signture);
  static void doFunctionOptimizations(llvm::Module* M);
};

} // end namespace bcc

#endif // BCC_RS_VECTORIZATION_SUPPORT_H
