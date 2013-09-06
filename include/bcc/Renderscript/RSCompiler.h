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

#ifndef BCC_RS_COMPILER_H
#define BCC_RS_COMPILER_H

#include "bcc/Compiler.h"

namespace llvm {
  class Module;
  class Function;
};

namespace bcc {

class RSCompiler : public Compiler {
public:
  virtual bool performCodeTransformations(Script &pScript);
private:
  virtual bool beforeAddLTOPasses(Script &pScript,
                                  llvm::PassManager &pPM,
                                  const char *mTriple);
  virtual bool beforeExecuteLTOPasses(Script &pScript,
                                      llvm::PassManager &pPM,
                                      const char *mTriple);
};

} // end namespace bcc

#endif // BCC_RS_COMPILER_H
