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

#include <llvm/ADT/Triple.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DataLayout.h>

#include "bcc/AndroidBitcode/ABCExpandVAArgPass.h"

#include "USC/USCABCCompilerDriver.h"

namespace {

class USCABCExpandVAArg : public bcc::ABCExpandVAArgPass {
public:
  virtual const char *getPassName() const {
    return "USC LLVM va_arg Instruction Expansion Pass";
  }

private:
  // Pass taken from ARMABCExpandVAArg.:
  llvm::Value *expandVAArg(llvm::Instruction *pInst) {
    llvm_unreachable("Pass not supported in USC");
    return NULL;
  }

};

} // end anonymous namespace

namespace bcc {

ABCExpandVAArgPass *USCABCCompilerDriver::createExpandVAArgPass() const {
  return new USCABCExpandVAArg();
}

} // end namespace bcc
